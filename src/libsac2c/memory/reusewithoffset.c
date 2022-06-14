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
#include "pattern_match.h"
#include "with_loop_utilities.h"

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

    result = (info *)MEMmalloc (sizeof (info));

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

#ifdef DEADCODE // poop. hard to compute abs().
/** <!--********************************************************************-->
 *
 * @fn  bool isGE( node *arg1, node *arg2)
 * @brief performs abs( arg1) >= arg2
 *
 * @param arg1 and arg2 are numeric
 *
 * @return TRUE ONLY if we know for sure, return FALSE.
 *         I.e., caller must NOT make any assumption about arguments
 *         when we return FALSE - it could just mean Do Not Know!
 *
 *****************************************************************************/
static bool
isGEabs (node *arg1, node *arg2)
{
    bool z = FALSE;
    bool relres = FALSE;

    DBUG_ENTER ();

    if (SCSisRelationalOnDyadicFn (F_ge_SxS, arg1, arg2, NULL, &relres)) {
        z = relres;
    }

    DBUG_RETURN (info);
}
#endif // DEADCODE  // poop. hard to compute abs().

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
    node *part;
    node *srcwl;

    DBUG_ENTER ();

    part = WITH_PART (wl);
    while ((NULL == res) && (part != NULL)) {
        srcwl = WLUTfindCopyPartition (part);
        if (NULL != srcwl) {
            res = TBmakeId (srcwl);
        }
        part = PART_NEXT (part);
    }
    DBUG_RETURN (res);
}

node *
RWOidentifyOtherPart (node *with, node *rc)
{
    node *hotpart = NULL;
    node *part;

    DBUG_ENTER ();

    part = WITH_PART (with);
    while (part != NULL) {
        if (!WLUTisCopyPartition (part)) {
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

    DBUG_ENTER ();

    part = WITH_PART (with);
    while (part != NULL) {
        PART_ISCOPY (part) = WLUTisCopyPartition (part);
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

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

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
    pattern *pat1;
    pattern *pat2;
    pattern *patarray;
    bool traverse = TRUE;
    node *arg1 = NULL;
    node *arg2 = NULL;
    prf selop;
#ifdef UNFINISHED
    node *other = NULL;
#endif
    node *rhs = NULL;
    node *gwelem = NULL;
    node *elem = NULL;
    node *ids = NULL;
    int gwval;
    constant *con = NULL;
    node *iv = NULL;
    node *narray = NULL;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAgetPrf (&selop), 2, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAisVar (&arg2), 0));

    pat2 = PMprf (1, PMAgetPrf (&selop), 2, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAgetNode (&arg2), 0));

    patarray = PMarray (1, PMAgetNode (&narray), 0);

    arg2 = INFO_RC (arg_info);
    if ((NULL != arg2) && (PMmatchFlatSkipExtremaAndGuards (pat1, arg_node))
        && ((F_sel_VxA == selop) || (F_idx_sel == selop))) {
        DBUG_PRINT ("Found selection op on potential RC candidate %s",
                    AVIS_NAME (ID_AVIS (arg2)));
        iv = arg1;

        /*
         * The access may either be A[iv]...
         */
        if (ID_AVIS (iv) == IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)))) {
            DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
            DBUG_PRINT ("Found A{iv] selection op: RC set for %s",
                        AVIS_NAME (ID_AVIS (arg2)));
            traverse = FALSE;
        }

        /*
         * ... or it may be A[iv+offset], where ANY( offset >= genwidth)
         */

        // Skip vect2offset, if present, and look at its PRF_ARG2
        if (PMmatchFlatSkipExtremaAndGuards (pat2, iv)) {
            DBUG_ASSERT (F_idxs2offset != selop, "idxs2offset coding time for Bonzo");
            if (F_vect2offset == selop) {
                iv = arg2;
                DBUG_PRINT ("Got vect2offset. iv now %s", AVIS_NAME (ID_AVIS (iv)));
            }
        }

#ifdef UNFINISHED
        // match on N_prf.
        if ((PMmatchFlatSkipExtremaAndGuards (pat2, iv))
            && ((F_add_SxS == selop) || (F_sub_SxS == selop))) {
            // Look for IV + offset OR IV - offset OR IV - offset  OR offset - IV
            if (ID_AVIS (arg1) == WITHID_VEC (INFO_WITHID (arg_info))) {
                other = arg2; // IV +- offset
            } else {
                if (ID_AVIS (arg2) == WITHID_VEC (INFO_WITHID (arg_info))) {
                    other = arg1; // offset +- IV
                }
            }
            // FIXME: how interesting. This code now does nothing more with other.
        }
#endif

        // This block of code handles a scalarized index vector, stored
        // as an N_array. GENWIDTH must be constant:
        // Reuse is allowed if we have one of these situations for any
        // elements of the N_array:
        //
        //    ids + constant
        //    ids - constant
        //    constant + ids
        //
        //    where abs( constant) >= GENWIDTH
        //
        //    I think the (undocumented) idea here is that
        //    reuse is allowed if we can show that the sel()
        //    operation will never reference an element of this WL-partition.

        if (PMmatchFlatSkipExtremaAndGuards (patarray, iv)) {
            gwelem = ARRAY_AELEMS (INFO_GENWIDTH (arg_info));
            elem = ARRAY_AELEMS (narray);
            ids = WITHID_IDS (INFO_WITHID (arg_info));

            while (elem != NULL) {
                if ((NODE_TYPE (EXPRS_EXPR (elem)) == N_id)
                    && (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (elem))) != NULL)
                    && (COisConstant (EXPRS_EXPR (gwelem)))) {
                    con = COaST2Constant (EXPRS_EXPR (gwelem));
                    gwval = COconst2Int (con);
                    con = COfreeConstant (con);
                    rhs = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (EXPRS_EXPR (elem))));

                    if ((NODE_TYPE (rhs) == N_prf)
                        && ((PRF_PRF (rhs) == F_add_SxS)
                            || (PRF_PRF (rhs) == F_sub_SxS))) {
                        // ids +- constant
                        if ((NODE_TYPE (PRF_ARG1 (rhs)) == N_id)
                            && (ID_AVIS (PRF_ARG1 (rhs)) == IDS_AVIS (ids))
                            && (((COisConstant (PRF_ARG2 (rhs)))))) {
                            con = COaST2Constant (PRF_ARG2 (rhs));
                            if (abs (COconst2Int (con)) >= gwval) {
                                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
                                DBUG_PRINT ("Found A[ids+-constant] - RC set for %s",
                                            AVIS_NAME (ID_AVIS (arg2)));
                                traverse = FALSE;
                            }
                            con = COfreeConstant (con);
                        }

                        // constant + ids
                        if ((PRF_PRF (rhs) == F_add_SxS)
                            && (NODE_TYPE (PRF_ARG2 (rhs)) == N_id)
                            && (ID_AVIS (PRF_ARG2 (rhs)) == IDS_AVIS (ids))
                            && (((COisConstant (PRF_ARG2 (rhs)))))) {
                            con = COaST2Constant (PRF_ARG1 (rhs));
                            if (abs (COconst2Int (con)) >= gwval) {
                                DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, arg_node));
                                DBUG_PRINT ("Found A[constant+ids] - RC set for %s",
                                            AVIS_NAME (ID_AVIS (arg2)));
                                traverse = FALSE;
                            }
                            con = COfreeConstant (con);
                        }
                    }
                }
                ids = IDS_NEXT (ids);
                elem = EXPRS_NEXT (elem);
                gwelem = EXPRS_NEXT (gwelem);
            }
        }
    }

    if (traverse) {
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    patarray = PMfree (patarray);

    DBUG_RETURN (arg_node);
}

/* @} */

#undef DBUG_PREFIX
