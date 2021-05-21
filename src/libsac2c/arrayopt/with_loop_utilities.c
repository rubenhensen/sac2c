/** <!--********************************************************************-->
 *
 * @defgroup wlut With-Loop utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            services for manipulating and examining with-loop.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file with_loop_utilities.c
 *
 * Prefix: WLUT
 *
 *****************************************************************************/
#include "with_loop_utilities.h"

#include "globals.h"

#define DBUG_PREFIX "WLUT"

#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "compare_tree.h"
#include "new_types.h"
#include "DupTree.h"
#include "constants.h"
#include "lacfun_utilities.h"
#include "indexvectorutils.h"
#include "new_typecheck.h"

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisEmptyPartitionCodeBlock( node *partn)
 *
 * @brief Predicate for finding N_part node with no code block.
 * @param N_part
 * @result TRUE if code block is empty
 *
 *****************************************************************************/
bool
WLUTisEmptyPartitionCodeBlock (node *partn)
{
    bool z;

    DBUG_ENTER ();

    z = (NULL == BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (partn))));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisIdsMemberPartition( node *arg_node, node *partn)
 *
 * @brief: Predicate for checking if arg_node's definition point
 *         is within a specified WL partition.
 *
 * @param: arg_node - a WLINTERSECT1/2 node.
 *         partn:   - a WL partition. In our case, it is
 *                    that of the consumerWL.
 *
 * @result: TRUE if arg_node is defined within the partition.
 *
 * @note: This is required because we can produce an inverse
 *        projection that can be used for cube slicing ONLY if said
 *        projection is defined OUTSIDE the current WL.
 *
 *****************************************************************************/
bool
WLUTisIdsMemberPartition (node *arg_node, node *partn)
{
    bool z = FALSE;
    node *nassgns;
    bool isIdsMember;

    DBUG_ENTER ();

    if (NULL != partn) {
        nassgns = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (partn)));
        while ((NULL != nassgns) && (!z)) {
            LFUindexOfMemberIds (ID_AVIS (arg_node),
                                 LET_IDS (ASSIGN_STMT (nassgns)),
                                 &isIdsMember);
            if (isIdsMember) {
                z = TRUE;
            }
            nassgns = ASSIGN_NEXT (nassgns);
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLUTfindArrayForBound( node *bnd)
 *
 * @brief Assuming that bnd is GENERATOR_BOUND1/2 for a WL generator,
 *        try to find the N_array that gives its elements.
 *
 * @param:  N_id or N_array
 * @result: N_array, or NULL if we can't find an N_array.
 *
 *****************************************************************************/
node *
WLUTfindArrayForBound (node *bnd)
{
    node *z = NULL;
    pattern *pat;

    DBUG_ENTER ();

    if (NULL != bnd) {
        if (N_array == NODE_TYPE (bnd)) {
            z = bnd;
        }

        if ((NULL == z) && (N_id == NODE_TYPE (bnd))) {
            pat = PMarray (1, PMAgetNode (&z), 1, PMskip (0));
            PMmatchFlat (pat, bnd);
            pat = PMfree (pat);
        }
    }

    DBUG_ASSERT ((NULL == z) || N_array == NODE_TYPE (z), "result node type wrong");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ivMatchCase1( node *withid,
 *                                node *cexpr)
 *
 * @brief: Attempt to match IV in _sel_VxA_( IV, srcwl)
 *         directly against WITHID_VEC withid_avis.
 *         We also attempt to match against:
 *               offset = _vect2offset( shape(target), IV);
 *               _idx_sel_( offset, srcwl);
 *
 * @params: withid is the N_withid of the WL we are trying to
 *          show is a copy-WL.
 *          cexprs is the WL result element. We determine if it
 *          was derived from the above _sel_VxA_ operation.
 *
 * @return: srcwl as N_avis of srcwl of _sel_VxA_( IV, srcwl),
 *          if found, else NULL.
 *
 *****************************************************************************/
static node *
ivMatchCase1 (node *withid, node *cexpr)
{
    node *srcwl = NULL;
    node *z = NULL;
    node *withid_son = NULL;
    node *withid_avis;
    node *offset = NULL;
    node *shp = NULL;
    node *id = NULL;
    node *iv = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    pattern *pat5;

    DBUG_ENTER ();
    withid_avis = IDS_AVIS (WITHID_VEC (withid));
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMparam (1, PMAgetNode (&withid_son)),
                  PMvar (1, PMAgetAvis (&srcwl), 0));

    pat5 = PMparam (1, PMAhasAvis (&withid_avis));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, cexpr)
        && PMmatchFlatSkipExtremaAndGuards (pat5, withid_son)) {
        /* withid_son may be guarded withid. */
        z = srcwl;
        DBUG_PRINT ("Case 1: body matches _sel_VxA_(, iv, pwl)");
    }

    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAgetNode (&srcwl), 0));

    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&iv), 0));

    if ((NULL == z) && (PMmatchFlatSkipGuards (pat2, cexpr))
        && (PMmatchFlatSkipExtremaAndGuards (pat3, offset))
        && (IVUTisIvMatchesWithid (iv, WITHID_VEC (withid), WITHID_IDS (withid)))) {
        z = ID_AVIS (srcwl);
        DBUG_PRINT ("Case 2: body matches _idx_sel( offset, pwl) with pwl=%s",
                    AVIS_NAME (z));
    }

    pat4 = PMprf (1, PMAisPrf (F_idxs2offset), 3, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&id), 0), PMskip (0));

    if ((NULL == z) && (PMmatchFlatSkipExtremaAndGuards (pat2, cexpr))
        && (PMmatchFlatSkipExtremaAndGuards (pat4, offset))) {
        DBUG_UNREACHABLE ("Case 3: coding time for matching WITHID_IDS to ids");
        z = ID_AVIS (srcwl);
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
    pat5 = PMfree (pat5);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ivMatchCase4( node *withid,
 *                                node *cexpr)
 *
 * @brief: Attempt to match [i,j] in _sel_VxA_( [i,j], srcwl)
 *         against WL_IDS, [i,j]
 *
 *         We have to be careful of stuff like:
 *
 *              _sel_VxA_( [i], srcwl)
 *
 *         in which the full index vector is not used,
 *         and its converse:
 *
 *              _sel_VxA_( [i,notme, k], srcwl)
 *
 *         In the case where we have guards present on the index
 *         vectors, we can safely ignore them, because the
 *         WL that we are copying will have identical guards
 *         on its WITH_ID index vector, which are guaranteed
 *         to be identical to ours. Hence, we ignore both
 *         guards and extrema on this search. As an example from
 *         histlp.sac, we have this pattern, sort of:
 *
 *           z = with { ...
 *             ( [0] <= iv < [lim]) {
 *              iv'  = _noteminval( iv, [0]);
 *              iv'' = _notemaxval( iv', [lim]);
 *              iv''', p0 = _val_lt_val_VxV_( iv'', [lim]);
 *              el = _sel_VxA_( iv''', producerWL);
 *              el' = _afterguard_( el, p0);
 *             } : el';
 *
 *        We also have to handle the scalar i equivalent of this,
 *        which is slightly more complex:
 *
 *           z = with { ...
 *             ( [0] <= iv=[i] < [lim]) {
 *              i'  = _noteminval( i, 0);
 *              i'' = _notemaxval( i', lim);
 *              i''', p0 = _val_lt_val_SxS_( i'', lim);
 *              iv' = [ i'''];
 *              el = _sel_VxA_( iv', producerWL);
 *              el' = _afterguard_( el, p0);
 *             } : el';
 *
 *
 * @params: withid is the N_withid of the WL we are trying to
 *          show is a copy-WL.
 *
 *          cexprs is the WL result element. We determine if it
 *          was derived from the above _sel_VxA_ operation.
 *
 * @return: srcwl as PRF_ARG2 of _sel_VxA_( IV, srcwl),
 *          if found, else NULL.
 *
 *****************************************************************************/
static node *
ivMatchCase4 (node *withid, node *cexpr)
{
    node *srcwl = NULL;
    node *withid_avis;
    node *withids;
    node *narray;
    node *narrayels;
    pattern *pat2;
    pattern *pat3;
    bool z = TRUE;

    DBUG_ENTER ();

    pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMarray (1, PMAgetNode (&narray), 0),
                  PMvar (1, PMAgetAvis (&srcwl), 0));
    pat3 = PMparam (1, PMAhasAvis (&withid_avis));

    withids = WITHID_IDS (withid);

    DBUG_ASSERT ((N_prf != NODE_TYPE (cexpr)) || (F_idx_sel != PRF_PRF (cexpr)),
                 "Start coding, Mr doivecyc4!");
    if (PMmatchFlatSkipExtremaAndGuards (pat2, cexpr)) {
        /* Match all elements. If we exhaust elements on either side, no match */
        narrayels = ARRAY_AELEMS (narray);
        while (z && (NULL != withids) && (NULL != narrayels)) {
            withid_avis = IDS_AVIS (withids);
            z = PMmatchFlatSkipExtremaAndGuards (pat3, EXPRS_EXPR (narrayels));
            withids = IDS_NEXT (withids);
            narrayels = EXPRS_NEXT (narrayels);
        }
        /* If we didn't exhaust both sides, no match */
        z = z && (NULL == withids) && (NULL == narrayels);

        if (z) {
            DBUG_PRINT ("Case 4: body matches _sel_VxA_( withid, &srcwl)");
        } else {
            srcwl = NULL;
        }
    }
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (srcwl);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLUTfindCopyPartition( node *partn)
 * @fn node *WLUTfindCopyPartitionFromCexpr node *withidvec, node *cexpr)
 *           (This is for when we are called from N_code in CWLE.)
 *
 *
 * @brief: function for determining if N_part partn is a copy
 *         partition, i.e., it performs  pwl'[ iv] = pwl[ iv],
 *         as:
 *
 *           (lb <= iv < ub) :  _sel_VxA_( iv, pwl);
 *
 *           or
 *
 *           (lb <= iv < ub) {
 *             offset = vect2offset( shape( pwl), iv);
 *             el = _idx_sel( offset, pwl);
 *             } : el;
 *
 *
 * @param: part: An N_part
 *
 * @result: If partn is a copy partition, then partn, else NULL;
 *
 *****************************************************************************/
node *
WLUTfindCopyPartition (node *partn)
{
    node *res = NULL;
    node *cexpr;
    node *withid;

    DBUG_ENTER ();

    cexpr = EXPRS_EXPR (CODE_CEXPRS (PART_CODE (partn)));
    withid = PART_WITHID (partn);
    res = ivMatchCase1 (withid, cexpr);
    res = (NULL != res) ? res : ivMatchCase4 (withid, cexpr);

    DBUG_RETURN (res);
}

node *
WLUTfindCopyPartitionFromCexpr (node *cexpr, node *withidvec)
{
    node *res = NULL;

    DBUG_ENTER ();

    res = ivMatchCase1 (withidvec, cexpr);
    res = (NULL != res) ? res : ivMatchCase4 (withidvec, cexpr);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisCopyPartition( node *partn)
 *
 * @brief: Predicate for determining if N_part partn is a copy
 *         partition, i.e., it performs  pwl'[ iv] = pwl[ iv],
 *         as:
 *
 *           (lb <= iv < ub) :  _sel_VxA_( iv, pwl);
 *
 *           or
 *
 *           (lb <= iv < ub) {
 *             offset = vect2offset( shape( pwl), iv);
 *             el = _idx_sel( offset, pwl);
 *             } : el;
 *
 *
 * @param: part: An N_part
 *
 * @result: true if partn is a copy partition, else false;
 *
 *****************************************************************************/
bool
WLUTisCopyPartition (node *partn)
{
    bool res;

    DBUG_ENTER ();

    res = NULL != WLUTfindCopyPartition (partn);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisEmptyGenerator( node *partn)
 *
 * @brief: Predicate for determining if N_part partn has an empty generator,
 *         i.e., [:int]
 *
 * @param: part: An N_part
 *
 * @result: true if partn has an empty generator; else false.
 *          Do NOT depend a FALSE result meaning that the generator is not
 *          empty. We may just not be able to find an N_array!
 *
 *****************************************************************************/
bool
WLUTisEmptyGenerator (node *partn)
{
    node *bnd;
    bool res = FALSE;

    DBUG_ENTER ();

    bnd = WLUTfindArrayForBound (GENERATOR_BOUND1 (PART_GENERATOR (partn)));
    if (NULL != bnd) {
        res = 0 == TCcountExprs (ARRAY_AELEMS (bnd));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn nodeWLUTremoveUnusedCodes(node *codes)
 *
 *   @brief removes all unused N_codes recursively
 *
 *   @param  node *codes : N_code chain
 *   @return node *      : modified N_code chain
 *
 ******************************************************************************/
node *
WLUTremoveUnusedCodes (node *codes)
{
    DBUG_ENTER ();

    DBUG_ASSERT (codes != NULL, "no codes available!");

    DBUG_ASSERT (NODE_TYPE (codes) == N_code, "type of codes is not N_code!");

    if (CODE_NEXT (codes) != NULL) {
        CODE_NEXT (codes) = WLUTremoveUnusedCodes (CODE_NEXT (codes));
    }

    if (CODE_USED (codes) == 0) {
        codes = FREEdoFreeNode (codes);
    }

    DBUG_RETURN (codes);
}

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisSingleOpWl( node *arg_node)
 *
 * @brief: predicate for determining if node is single-op WL
 *
 * @param: arg_node: an N_with
 *
 * @return: TRUE if only one result from WL
 *
 *****************************************************************************/
bool
WLUTisSingleOpWl (node *arg_node)
{
    bool z;

    DBUG_ENTER ();

    switch (NODE_TYPE (WITH_WITHOP (arg_node))) {
    default:
        z = FALSE;
        DBUG_UNREACHABLE ("WITHOP confusion");
        break;
    case N_genarray:
        z = (NULL == GENARRAY_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_modarray:
        z = (NULL == MODARRAY_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_fold:
        z = (NULL == FOLD_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_spfold:
        z = (NULL == SPFOLD_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_propagate:
        z = (NULL == PROPAGATE_NEXT (WITH_WITHOP (arg_node)));
        break;
    case N_break:
        z = (NULL == BREAK_NEXT (WITH_WITHOP (arg_node)));
        break;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLUTid2With( node *arg_node)
 *
 * @brief: Given an N_id, if its value is derived from a with-loop,
 *         return that N_with. Otherwise, arg_node.
 *
 * @param: arg_node: an N_id
 *
 * @return: The N_with code that created arg_node, or arg_node
 *
 *****************************************************************************/
node *
WLUTid2With (node *arg_node)
{
    node *wl;
    pattern *pat;

    DBUG_ENTER ();

    wl = arg_node;
    if (N_id == NODE_TYPE (arg_node)) { // Find N_with from N_id
        pat = PMwith (1, PMAgetNode (&wl), 0);
        PMmatchFlatWith (pat, wl);
        pat = PMfree (pat);
    }

    DBUG_RETURN (wl);
}

/** <!--********************************************************************-->
 *
 * @fn bool  WLUTisGenarrayScalar( node *arg_node, bool nowithid)
 * @brief:   Predicate for WLUTgetGenarrayScalar
 *
 * @fn node *WLUTgetGenarrayScalar( node *arg_node, bool nowithid)
 *
 * @brief: If N_with arg_node is a
 *         genarray,
 *         single-generator,
 *         single-partition with-loop
 *         with a scalar value that is NOT a member of WITHID_IDS,
 *         as its CODE_CEXPRS value, return that scalar N_id, else NULL.
 *
 *         If not NULL, then all elements of the resulting with-loop
 *         are identical. E.g.:
 *
 *           Q = with {
 *                 ( [0] <= iv < [ub])  : Scalar;
 *               } : genarray( [shp, Scalar);
 *
 *         FIXME: I think we already have a utility like this around
 *                somewhere, but I can not find it.
 *
 * @param: wl: An N_with, or N_id.
 * @param: nowithid: If TRUE, then do not allow Scalar to be
 *                   a member of WITHID_IDS.
 *                   If FALSE, require Scalar to be
 *                   a member of WITHID_IDS.
 *
 * @result: N_avis of the value Scalar (representing all
 *          elements of the with-loop result), or NULL.
 *
 * NB. This code currently handles only "Scalar". It could be
 *     fancied up to handle simple expressions, such as "Scalar+2".
 *
 *****************************************************************************/
node *
WLUTgetGenarrayScalar (node *arg_node, bool nowithid)
{
    node *wl;
    node *res = NULL;
    bool z;
    bool memberwithids;

    DBUG_ENTER ();

    wl = WLUTid2With (arg_node);

    z = (N_with == NODE_TYPE (wl));
    z = z && (N_genarray == NODE_TYPE (WITH_WITHOP (wl)));
    z = z && WLUTisSingleOpWl (wl);
    z = z && (NULL == BLOCK_ASSIGNS (CODE_CBLOCK (WITH_CODE (wl))));
    z = z && (NULL == GENARRAY_NEXT (WITH_WITHOP (wl)));
    z = z && (1 == TCcountParts (WITH_PART (wl)));
    z = z
        && (TUisScalar (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (WITH_CODE (wl)))))));
    if (z) {
        res = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (WITH_CODE (wl))));

        // We are almost there. We have to ensure that res [IS/ IS NOT]
        // a member of WITHID_IDS.
        TClookupIdsNode (WITHID_IDS (PART_WITHID (WITH_PART (wl))), res, &memberwithids);
        z = memberwithids ^ nowithid; // XOR corrects value
        res = z ? res : NULL;
    }

    DBUG_RETURN (res);
}

bool
WLUTisGenarrayScalar (node *arg_node, bool nowithid)
{
    bool z;

    DBUG_ENTER ();

    z = NULL != WLUTgetGenarrayScalar (arg_node, nowithid);

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
