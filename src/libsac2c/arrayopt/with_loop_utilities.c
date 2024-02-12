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
#include "flattengenerators.h"
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

/*******************************************************************************
 * @fn node *WLUTcreatePartitionCopies( node *fundef, node *partn,
 *                                      size_t nr_required_partitions)
 *
 * @brief: Creates copies of the given partition and its associated code block
 *         in SSA form, reusing the partition and code block if possible.
 *
 * @param: fundef                 - An N_fundef node
 *         partn                  - An N_part node within the N_fundef
 *         nr_required_partitions - The desired number of partitions
 *
 * @return: A partition chain consisting of the requested number of partitions.
 *          Each partition points to its own N_code.
 *          Each N_code is added to the N_code chain of the original partition's
 *          code
 *
 * @note: This function has the following preconditions:
 *        1. fundef must be of type N_fundef
 *        2. partition must be of type N_part
 *        3. nr_required_partitions must be > 0
 *        4. partition may not have a next
 *        5. partition's code may not have a next
 *
 * See wl_modulo_partitioning.c for an example usage of this function.
 * Required structure of the input partition (arrows denote the NEXT pointer):
 *       part1 -> code1
 *         |        |
 *        \|/      \|/
 *        NULL     NULL
 *
 * Output structure of the returned partition part1 that is copied N times:
 *       part1 -> code1
 *         |        |
 *        \|/      \|/
 *        ...  ->  ...
 *         |        |
 *        \|/      \|/
 *       partN -> codeN
 *         |        |
 *        \|/      \|/
 *        NULL     NULL
 *
 *
 ******************************************************************************/
node *
WLUTcreatePartitionCopies (node *fundef, node *partn,
                           size_t nr_required_partitions)
{
    int dbug_orig_code_used;

    node *original_code;
    node *previous_code;
    node *current_code;
    node *previous_partition;
    node *current_partition;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "Expected an N_fundef node but got %s!", NODE_TEXT (fundef));
    DBUG_ASSERT (NODE_TYPE (partn) == N_part,
                 "Expected an N_Part node but got %s!", NODE_TEXT (partn));
    DBUG_ASSERT (nr_required_partitions != 0, "Unable to make 0 copies!");
    DBUG_ASSERT (PART_NEXT (partn) == NULL,
                 "Partition must not have a next!");
    DBUG_ASSERT (CODE_NEXT (PART_CODE (partn)) == NULL,
                 "Partition's code must not have a next!");

    original_code = PART_CODE (partn);
    dbug_orig_code_used = CODE_USED (original_code);
    DBUG_ASSERT (dbug_orig_code_used > 0,
                 "The AST is in an illegal state: the code belonging to this"
                 " partition indicates it is not used!");

    // First, we either copy or reuse the code for the first partition
    if (CODE_USED (original_code) > 1) {
        // Since the code has multiple usages, we copy it
        current_code = DUPdoDupNodeSsa (original_code, fundef);
        CODE_INC_USED (current_code); // Copies start at 0, we need it to be 1.
        CODE_DEC_USED (original_code);

        PART_CODE (partn) = current_code;
        CODE_NEXT (original_code) = current_code;

        previous_code = current_code;
    } else {
        previous_code = original_code;
    }

    // Verify we used original_code correctly, then throw away the reference
    DBUG_ASSERT (dbug_orig_code_used != 1 || CODE_USED (original_code) == 1,
                 "CODE_USED was originally 1 and should still be 1,"
                 " but it is %d!", CODE_USED (original_code));
    DBUG_ASSERT (dbug_orig_code_used == 1
                 || CODE_USED (original_code) == dbug_orig_code_used - 1,
                 "CODE_USED was originally %d, should now be %d, but is %d!",
                 dbug_orig_code_used, dbug_orig_code_used - 1,
                 CODE_USED (original_code));
    original_code = NULL;

    // For the remaining partitions, we copy the partitions and code each time
    previous_partition = partn;
    for (size_t partition_nr = 1;
         partition_nr < nr_required_partitions;
         partition_nr++)
    {
        // We intentionally don't copy the partition in SSA form.
        // Copying it in ssa form makes the references of the copied code invalid
        current_partition = DUPdoDupNode (partn);
        current_code = DUPdoDupNodeSsa (PART_CODE (partn), fundef);

        // Copying the partition implicitly increments the CODE_USED counter,
        // of the partition's code, so we we decrement it to compensate.
        CODE_DEC_USED (PART_CODE (partn));
        CODE_INC_USED (current_code); // Copies start at 0, we need it to be 1.

        PART_CODE (current_partition) = current_code;
        PART_NEXT (previous_partition) = current_partition;
        CODE_NEXT (previous_code) = current_code;

        previous_partition = current_partition;
        previous_code = current_code;
    }

#ifndef DBUG_OFF
    for (node *cur_part = partn, *cur_code = PART_CODE (partn);
         cur_part != NULL || cur_code != NULL;
         cur_part = PART_NEXT (cur_part), cur_code = CODE_NEXT (cur_code))
    {
        DBUG_ASSERT (PART_CODE (cur_part) == cur_code,
                     "The CODE chain is inconsistent with the PART chain!");

        // Every partition's code block should have a single usage.
        DBUG_ASSERT (CODE_USED (cur_code) == 1,
                     "Invalid CODE_USED count: %d", CODE_USED (cur_code));
    }
#endif // DBUG_OFF

    DBUG_RETURN (partn);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLUTfindArrayForBound( node *bound)
 *
 * @brief Assuming that bound is GENERATOR_BOUND1/2/STEP/WIDTH for a WL
 *        generator, try to find the N_array that gives its elements.
 *        Reasons for failing and returning NULL include:
 *        1. The bound is NULL.
 *        2. The bound is an N_id that is or points to a function argument.
 *
 * @param bound: An N_id or N_array node, or NULL.
 *
 * @result: N_array if one is found, NULL otherwise.
 *
 *****************************************************************************/
node *
WLUTfindArrayForBound (node *bound)
{
    node *res;
    pattern *pat;

    DBUG_ENTER ();

    if (bound == NULL) {
        DBUG_RETURN (NULL);
    }

    switch (NODE_TYPE (bound)) {
    case N_array:
        res = bound;
        break;
    case N_id:
        // Note that we can't use ASSIGN_RHS (ID_SSAASSIGN (bound)
        // because ID_SSAASSIGN doesn't exist for fun args and index variables.
        // Additionally, B would match A instead of [5] for A = [5]; B = A;
        res = NULL;
        pat = PMarray (1, PMAgetNode (&res), 0);
        PMmatchFlat (pat, bound);
        pat = PMfree (pat);
        break;
    default:
        DBUG_UNREACHABLE ("Got unexpected node type %s!", NODE_TEXT (bound));
    }
    DBUG_ASSERT (res == NULL || NODE_TYPE (res) == N_array,
                 "Array conversion failed!");
    DBUG_RETURN (res);
}

/*******************************************************************************
 * @fn void *WLUTupdateBoundNthDim( node **bound, size_t dimension,
 *                                  node *new_scalar_avis, node **vardecs,
 *                                  node **preassigns)
 *
 * @brief: Replaces the `dimension`-th value in the given `bound`, step, or width
 *         with the `new_scalar_avis` argument.
 *         If `bound` points at an array, the update is done in-place, and the
 *         function returns.
 *         If the `bound` points to an N_id, the underlying array is copied,
 *         given an avis, and added to the preassigns. The update is applied to
 *         the copy. The N_id is freed, a new N_id is made for the avis of the
 *         new array. The new id replaces the freed id.
 *         Memory management is done internally. The call site does not have to
 *         do anything other than call this function.
 *
 * @param:  bound           - Pointer to a pointer of an N_id or N_array, to be
 *                            replaced with an N_id if vardecs/preassigns are
 *                            given, and replaced with an N_array otherwise.
 *          dimension       - The index of the element to be replaced
 *          new_scalar_avis - The avis of the value to insert at dimension
 *          vardecs         - Variable declarations, used for the newly created
 *                            array
 *          preassigns      - Preassigns, used for the newly created array
 *
 * @warning This function will crash if the underlying array of the bounds
 *          cannot be resolved.
 *          Ensure with-loop has with-ids before calling this function.
 ******************************************************************************/
void
WLUTupdateBoundNthDim (node **bound, size_t dimension,
                       node *new_scalar_avis, node **vardecs, node **preassigns)
{
    node *bound_array;
    bool bound_is_id;

    DBUG_ENTER ();

    DBUG_ASSERT (bound != NULL, "bound may not be null!");
    DBUG_ASSERT (*bound != NULL, "bound may not point to null!");
    DBUG_ASSERT (NODE_TYPE (*bound) == N_array || NODE_TYPE (*bound) == N_id,
                 "bound must be an n_id or n_array but is \"%s\"!",
                 NODE_TEXT (*bound));
    DBUG_ASSERT (new_scalar_avis != NULL, "new_scalar_avis may not be null!");
    DBUG_ASSERT (vardecs != NULL, "Vardecs may not be null!");
    DBUG_ASSERT (preassigns != NULL, "Preassigns may not be null!");

    bound_is_id = NODE_TYPE (*bound) == N_id;
    bound_array = WLUTfindArrayForBound (*bound);
    DBUG_ASSERT (bound_array != NULL,
                 "The array belonging to the bound could not be found!");

    if (bound_is_id) {
        // For an N_id, we don't know if there are other references to the
        // array, so we create a copy to replace the id later
        bound_array = DUPdoDupNode (bound_array);
    }

    // Replace the nth element of bound_array.
    ARRAY_AELEMS (bound_array) = TCputNthExprs (
        dimension,
        ARRAY_AELEMS (bound_array),
        TBmakeId (new_scalar_avis));

    if (bound_is_id) {
        // Replace the id with a newly generated id for the modified array.
        *bound = FREEdoFreeTree (*bound);
        *bound = TBmakeId (FLATGexpression2Avis (bound_array, vardecs,
                                                 preassigns, NULL));
    }
    DBUG_RETURN ();
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
 *              el' = guard (el, p0);
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
 *              el' = guard (el, p0);
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
