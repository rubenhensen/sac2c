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
#include "dbug.h"
#include "print.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "new_types.h"

#include <string.h>

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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITHID (result) = NULL;
    INFO_RC (result) = NULL;
    INFO_GENWIDTH (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * Helper functions
 *
 *****************************************************************************/
static node *
IdentifyNoopArray (node *with)
{
    node *res = NULL;
    node *code = WITH_CODE (with);
    node *ivavis = IDS_AVIS (WITH_VEC (with));

    DBUG_ENTER ("IdentifyNoopArray");

    while (code != NULL) {
        node *cass = AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (code)));

        if ((cass != NULL) && (NODE_TYPE (ASSIGN_RHS (cass)) == N_prf)
            && (PRF_PRF (ASSIGN_RHS (cass)) == F_sel)
            && (NODE_TYPE (PRF_ARG1 (ASSIGN_RHS (cass))) == N_id)
            && (NODE_TYPE (PRF_ARG2 (ASSIGN_RHS (cass))) == N_id)
            && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (cass))) == ivavis)) {
            res = TBmakeId (ID_AVIS (PRF_ARG2 (ASSIGN_RHS (cass))));
            break;
        }

        code = CODE_NEXT (code);
    }
    DBUG_RETURN (res);
}

static bool
IsNoopPart (node *part, node *rc)
{
    bool res = FALSE;
    node *code = PART_CODE (part);
    node *ivavis = IDS_AVIS (PART_VEC (part));
    node *cass;

    DBUG_ENTER ("IsNoopPart");

    cass = AVIS_SSAASSIGN (ID_AVIS (CODE_CEXPR (code)));

    if ((cass != NULL) && (NODE_TYPE (ASSIGN_RHS (cass)) == N_prf)
        && (PRF_PRF (ASSIGN_RHS (cass)) == F_sel)
        && (NODE_TYPE (PRF_ARG1 (ASSIGN_RHS (cass))) == N_id)
        && (NODE_TYPE (PRF_ARG2 (ASSIGN_RHS (cass))) == N_id)
        && (ID_AVIS (PRF_ARG1 (ASSIGN_RHS (cass))) == ivavis)
        && (ID_AVIS (PRF_ARG2 (ASSIGN_RHS (cass))) == ID_AVIS (rc))) {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

static node *
IdentifyOtherPart (node *with, node *rc)
{
    node *hotpart = NULL;
    node *part = WITH_PART (with);

    DBUG_ENTER ("IdentifyOtherGenerator");

    while (part != NULL) {
        if (!IsNoopPart (part, rc)) {
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

    DBUG_ENTER ("RWOdoOffsetAwareReuseCandidateInference");

    DBUG_ASSERT (NODE_TYPE (with) == N_with, "Illegal node type!");

    if (((NODE_TYPE (WITH_WITHOP (with)) == N_genarray)
         || (NODE_TYPE (WITH_WITHOP (with)) == N_modarray))
        && (WITHOP_NEXT (WITH_WITHOP (with)) == NULL)) {
        node *hotpart = NULL;

        cand = IdentifyNoopArray (with);

        if (cand != NULL) {
            DBUG_PRINT ("RWO", ("Identified RC: %s\n", ID_NAME (cand)));
            hotpart = IdentifyOtherPart (with, cand);

            if (hotpart != NULL) {
                DBUG_EXECUTE ("RWO", PRTdoPrintNode (hotpart););
                node *hotcode, *oldnext;
                info *arg_info;

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
                    cand = TBmakeExprs (INFO_RC (arg_info), NULL);
                    INFO_RC (arg_info) = NULL;
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
    DBUG_ENTER ("RWOids");

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
    DBUG_ENTER ("RWOid");

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

    DBUG_ENTER ("RWOprf");

    if ((PRF_PRF (arg_node) == F_sel) && (INFO_RC (arg_info) != NULL)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)
        && (ID_AVIS (PRF_ARG2 (arg_node)) == ID_AVIS (INFO_RC (arg_info)))) {
        node *ass;

        /*
         * The access may either be A[iv]
         */
        if ((ID_AVIS (PRF_ARG1 (arg_node))
             == IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info))))) {
            DBUG_EXECUTE ("RWO", PRTdoPrintNode (arg_node););
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
                        && (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (elem))) != NULL)
                        && (NODE_TYPE (EXPRS_EXPR (gwelem)) == N_num)) {
                        int gwval = NUM_VAL (EXPRS_EXPR (gwelem));
                        rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (elem))));

                        if ((NODE_TYPE (rhs) == N_prf)
                            && ((PRF_PRF (rhs) == F_add_SxS)
                                || (PRF_PRF (rhs) == F_sub_SxS))) {

                            if ((NODE_TYPE (PRF_ARG1 (rhs)) == N_id)
                                && (ID_AVIS (PRF_ARG1 (rhs)) == IDS_AVIS (ids))
                                && (NODE_TYPE (PRF_ARG2 (rhs)) == N_num)
                                && (abs (NUM_VAL (PRF_ARG2 (rhs))) >= gwval)) {
                                DBUG_EXECUTE ("RWO", PRTdoPrintNode (arg_node););
                                traverse = FALSE;
                            }

                            if ((PRF_PRF (rhs) == F_add_SxS)
                                && (NODE_TYPE (PRF_ARG2 (rhs)) == N_id)
                                && (ID_AVIS (PRF_ARG2 (rhs)) == IDS_AVIS (ids))
                                && (NODE_TYPE (PRF_ARG1 (rhs)) == N_num)
                                && (abs (NUM_VAL (PRF_ARG1 (rhs))) >= gwval)) {
                                DBUG_EXECUTE ("RWO", PRTdoPrintNode (arg_node););
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
