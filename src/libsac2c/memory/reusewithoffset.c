/*
 * $Id$
 */

/**
 *
 * @defgroup rwo Offset-Aware With-Loop Reuse Candidate Inference
 *
 * @ingroup mm
 *
 * @{
 */

/**
 *
 * @file reusewithoffset.c
 *
 * Prefix: RWO
 */
#include "reusewithoffset.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "RWO"
#include "debug.h"

#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"
#include "constants.h"
#include "copywlelim.h"

/*
 * INFO structure
 */
struct INFO {
    node *withid;
    node *rc;
    node *genwidth;
};

/*
 * INFO macros
 */
#define INFO_WITHID(n) ((n)->withid)
#define INFO_RC(n) ((n)->rc)
#define INFO_GENWIDTH(n) ((n)->genwidth)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_WITHID (result) = NULL;
    INFO_RC (result) = NULL;
    INFO_GENWIDTH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *RWOidentifyNoopArray( node *wl)
 * @brief If ANY partition of this wl is a potential copy-WL,
 *        return the N_id of the putative source-WL for that partition.
 *
 * @param wl: with-loop to search for reusable arrays
 *
 * @return
 *
 *****************************************************************************/
node *
RWOidentifyNoopArray (node *wl)
{
    node *res = NULL;
    node *ivavis;
    node *expr;
    node *withid;
    node *part;
    node *srcwl;

    DBUG_ENTER ();

    ivavis = IDS_AVIS (WITH_VEC (wl));
    part = WITH_PART (wl);
    while (part != NULL) {
        withid = PART_WITHID (part);
        expr = EXPRS_EXPR (CODE_CEXPRS (PART_CODE (part)));
        srcwl = CWLEfindCopyPartitionSrcWl (withid, expr);
        if (NULL != srcwl) {
            res = TBmakeId (srcwl);
            break;
        }
        part = PART_NEXT (part);
    }
    DBUG_RETURN (res);
}

bool
RWOisNoopPart (node *part, node *rc, node *withid)
{
    bool res = FALSE;
    node *srcwl;
    node *expr;

    DBUG_ENTER ();

    expr = EXPRS_EXPR (CODE_CEXPRS (PART_CODE (part)));
    srcwl = CWLEfindCopyPartitionSrcWl (withid, expr);
    res = (NULL != srcwl);

    DBUG_RETURN (res);
}

node *
RWOidentifyOtherPart (node *with, node *rc)
{
    node *hotpart = NULL;
    node *part;
    node *withid;

    DBUG_ENTER ();

    part = WITH_PART (with);
    while (part != NULL) {
        withid = PART_WITHID (part);
        if (!RWOisNoopPart (part, rc, withid)) {
            if (hotpart == NULL) {
                hotpart = part;
            } else {
                hotpart = NULL;
                break;
            }
        }

        part = PART_NEXT (part);
    }

    /*
     * The hot part must have a known GENWIDTH
     */
    if ((hotpart != NULL)
        && ((NODE_TYPE (PART_GENERATOR (hotpart)) != N_generator)
            || (GENERATOR_GENWIDTH (PART_GENERATOR (hotpart)) == NULL))) {
        hotpart = NULL;
    }

    DBUG_RETURN (hotpart);
}

node *
RWOannotateCopyPart (node *with, node *rc)
{
    node *part;
    node *withid;

    DBUG_ENTER ();

    part = WITH_PART (with);
    while (part != NULL) {
        withid = PART_WITHID (part);
        if (RWOisNoopPart (part, rc, withid)) {
            PART_ISCOPY (part) = TRUE;
        } else {
            PART_ISCOPY (part) = FALSE;
        }

        part = PART_NEXT (part);
    }

    DBUG_RETURN (with);
}

/******************************************************************************
 *
 * Offset-aware With-Loop reuse candidate inference (rwo_tab)
 *
 * prefix: RWO
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *RWOdoOffsetAwareReuseCandidateInference( node *with)
 *
 * @brief
 *
 * @param with-loop to search for reusable arrays
 *
 * @return list of reuse candidates
 *
 *****************************************************************************/
node *
RWOdoOffsetAwareReuseCandidateInference (node *with)
{
    node *cand = NULL;
    node *hotpart = NULL;
    node *hotcode;
    node *oldnext;
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "Illegal node type!");

    if (((NODE_TYPE (WITH_WITHOP (with)) == N_genarray)
         || (NODE_TYPE (WITH_WITHOP (with)) == N_modarray))
        && (WITHOP_NEXT (WITH_WITHOP (with)) == NULL)) {

        cand = RWOidentifyNoopArray (with);

        if (cand != NULL) {
            DBUG_PRINT ("Identified RC: %s\n", ID_NAME (cand));
            hotpart = RWOidentifyOtherPart (with, cand);

            if (hotpart != NULL) {
                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, hotpart));

                arg_info = MakeInfo ();

                INFO_WITHID (arg_info) = WITH_WITHID (with);
                INFO_RC (arg_info) = cand;
                INFO_GENWIDTH (arg_info) = GENERATOR_GENWIDTH (PART_GENERATOR (hotpart));

                cand = NULL;

                hotcode = PART_CODE (hotpart);
                oldnext = CODE_NEXT (hotcode);
                CODE_NEXT (hotcode) = NULL;

                TRAVpush (TR_rwo);
                hotcode = TRAVdo (hotcode, arg_info);
                TRAVpop ();

                CODE_NEXT (hotcode) = oldnext;

                if (INFO_RC (arg_info) != NULL) {
                    with = RWOannotateCopyPart (with, INFO_RC (arg_info));
                    cand = TBmakeExprs (INFO_RC (arg_info), NULL);
                    INFO_RC (arg_info) = NULL;
                    WITH_HASRC (with) = TRUE;
                }

                arg_info = FreeInfo (arg_info);
            } else {
                cand = FREEdoFreeTree (cand);
            }
        }
    }

    DBUG_RETURN (cand);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWOids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWOids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((INFO_RC (arg_info) != NULL)
        && (IDS_AVIS (arg_node) == ID_AVIS (INFO_RC (arg_info)))) {
        INFO_RC (arg_info) = FREEdoFreeNode (INFO_RC (arg_info));
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWOid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWOid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if ((INFO_RC (arg_info) != NULL)
        && (ID_AVIS (arg_node) == ID_AVIS (INFO_RC (arg_info)))) {
        INFO_RC (arg_info) = FREEdoFreeNode (INFO_RC (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RWOprf( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RWOprf (node *arg_node, info *arg_info)
{
    bool traverse = TRUE;

    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) == F_sel_VxA) && (INFO_RC (arg_info) != NULL)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)
        && (ID_AVIS (PRF_ARG2 (arg_node)) == ID_AVIS (INFO_RC (arg_info)))) {
        node *ass;

        /*
         * The access may either be A[iv]
         */
        if ((ID_AVIS (PRF_ARG1 (arg_node))
             == IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info))))) {
            DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
            traverse = FALSE;
        }

        /*
         * ... or it may be A[iv+offset], where ANY( offset >= genwidth)
         */
        ass = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node)));
        if (ass != NULL) {
            node *rhs = ASSIGN_RHS (ass);
            if ((NODE_TYPE (rhs) == N_prf)
                && ((PRF_PRF (rhs) == F_add_VxV) || (PRF_PRF (rhs) == F_sub_VxV))) {
                node *other = NULL;

                if ((NODE_TYPE (PRF_ARG1 (rhs)) == N_id)
                    && (ID_AVIS (PRF_ARG1 (rhs)) = WITHID_VEC (INFO_WITHID (arg_info)))) {
                    other = PRF_ARG2 (rhs);
                }
                if ((NODE_TYPE (PRF_ARG2 (rhs)) == N_id)
                    && (ID_AVIS (PRF_ARG2 (rhs)) = WITHID_VEC (INFO_WITHID (arg_info)))) {
                    other = PRF_ARG1 (rhs);
                }

                /*
                 * Implement me
                 */
            }
            if (NODE_TYPE (rhs) == N_array) {
                node *gwelem = ARRAY_AELEMS (INFO_GENWIDTH (arg_info));
                node *elem = ARRAY_AELEMS (rhs);
                node *ids = WITHID_IDS (INFO_WITHID (arg_info));

                while (elem != NULL) {
                    if ((NODE_TYPE (EXPRS_EXPR (elem)) == N_id)
                        && (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (elem))) != NULL) &&
                        // ( NODE_TYPE( EXPRS_EXPR( gwelem)) == N_num)) {
                        (COisConstant (EXPRS_EXPR (gwelem)))) {
                        // int gwval = NUM_VAL( EXPRS_EXPR( gwelem));
                        int gwval = COconst2Int (COaST2Constant (EXPRS_EXPR (gwelem)));
                        rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (elem))));

                        if ((NODE_TYPE (rhs) == N_prf)
                            && ((PRF_PRF (rhs) == F_add_SxS)
                                || (PRF_PRF (rhs) == F_sub_SxS))) {

                            if ((NODE_TYPE (PRF_ARG1 (rhs)) == N_id)
                                && (ID_AVIS (PRF_ARG1 (rhs)) == IDS_AVIS (ids))
                                && (((NODE_TYPE (PRF_ARG2 (rhs)) == N_num)
                                     && (abs (NUM_VAL (PRF_ARG2 (rhs))) >= gwval))
                                    || ((COisConstant (PRF_ARG2 (rhs)))
                                        && (abs (COconst2Int (
                                              COaST2Constant (PRF_ARG2 (rhs))))
                                            >= gwval)))) {
                                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
                                traverse = FALSE;
                            }

                            if ((PRF_PRF (rhs) == F_add_SxS)
                                && (NODE_TYPE (PRF_ARG2 (rhs)) == N_id)
                                && (ID_AVIS (PRF_ARG2 (rhs)) == IDS_AVIS (ids))
                                && (NODE_TYPE (PRF_ARG1 (rhs)) == N_num)
                                && (abs (NUM_VAL (PRF_ARG1 (rhs))) >= gwval)) {
                                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
                                traverse = FALSE;
                            }
                        }
                    }

                    ids = IDS_NEXT (ids);
                    elem = EXPRS_NEXT (elem);
                    gwelem = EXPRS_NEXT (gwelem);
                }
            }
        }
    }

    if ((traverse) && (PRF_ARGS (arg_node) != NULL)) {
        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
