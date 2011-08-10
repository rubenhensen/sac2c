/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cubes Algebraic With-Loop Folding Cube Slicer
 *
 * @terminology:
 *
 * @brief This traversal uses information in each F_noteintersect
 *        node as the basis for making a decision about slicing
 *        each consumerWL partition.
 *
 *        The order of slicing is bottom-to-top, so as to slice
 *        consumerWLs before their producerWLs.
 *        Similarly, the order is innermost-to-outermost WL.
 *
 *        Each consumerWL partition may generate one, two, or three
 *        slices per axis.
 *
 * @notes We traverse PART_CODES within each WL partition,
 *        seeking evidence that this is a consumerWL partition.
 *
 *        Since a given partition may have several, potentially
 *        conflicting intersect specifications in it, once we
 *        have sliced a partition, we examine the new partitions,
 *        and delete any F_noteintersect that does not have an
 *        exact match on the new partition. The idea here is that
 *        we do not know if the unmatched intersections represent
 *        valid slices-in-waiting.
 *        Further AWLFI traversals will rebuild those F_noteintersect
 *        calls with up-to-date partition bounds, and perform additional
 *        slicing, as required.
 *
 *        Locating Producer-WL partition from _noteintersect() information:
 *        A producer-WL partition may change its order within its WL,
 *        or it may be deleted entirely (e.g., by WLSIMP).
 *        The FindMatchingPartition function performs this task.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file cubeslicer.c
 *
 * Prefix: CUBSL
 *
 *****************************************************************************/
#include "cubeslicer.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"

#define DBUG_PREFIX "CUBSL"
#include "debug.h"

#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "wls.h"
#include "algebraic_wlf.h"
#include "algebraic_wlfi.h"
#include "ivexcleanup.h"
#include "compare_tree.h"
#include "check.h"
#include "gdb_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassignswith;
    bool onefundef; /* fundef-based traversal */
    node *lhs;
    node *consumerpart;
    node *wlprojection1;            /* lower bound of WL proj */
    node *wlprojection2;            /* upper bound of WL proj */
    node *withcode;                 /* WITH_CODE from N_with */
    intersect_type_t intersecttype; /* intersect type. see enum */
    lut_t *lut;                     /* LUT for renaming */
    node *noteintersect;            /* F_noteintersect node */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNSWITH(n) ((n)->preassignswith)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_CONSUMERPART(n) ((n)->consumerpart)
#define INFO_WLPROJECTION1(n) ((n)->wlprojection1)
#define INFO_WLPROJECTION2(n) ((n)->wlprojection2)
#define INFO_WITHCODE(n) ((n)->withcode)
#define INFO_INTERSECTTYPE(n) ((n)->intersecttype)
#define INFO_LUT(n) ((n)->lut)
#define INFO_NOTEINTERSECT(n) ((n)->noteintersect)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSWITH (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_CONSUMERPART (result) = NULL;
    INFO_WLPROJECTION1 (result) = NULL;
    INFO_WLPROJECTION2 (result) = NULL;
    INFO_WITHCODE (result) = NULL;
    INFO_INTERSECTTYPE (result) = INTERSECT_unknown;
    INFO_LUT (result) = NULL;
    INFO_NOTEINTERSECT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node * CUBSLdoAlgebraicWithLoopFoldingCubeSlicing( node *arg_node)
 *
 * @brief Global entry point of Algebraic With-Loop folding cube slicer.
 *
 *****************************************************************************/
node *
CUBSLdoAlgebraicWithLoopFoldingCubeSlicing (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "CUBSLdoAlgebraicWithLoopFoldingCubeSlicing called for non-fundef");

    arg_info = MakeInfo (arg_node);
    INFO_ONEFUNDEF (arg_info) = TRUE;
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_cubsl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *CheckForSuperfluousCodes(node *wln)
 *
 * description:
 *   remove all unused N_code nodes of the given WL.
 *
 *  This was stolen from SSAWLF. We should move it into tree/somewhere,
 *  but I am waiting for word back from Clemens and Bodo.
 *
 ******************************************************************************/

static node *
CheckForSuperfluousCodes (node *wln)
{
    node **tmp;

    DBUG_ENTER ();

    tmp = &WITH_CODE (wln);
    while (*tmp) {
        if (!CODE_USED ((*tmp))) {
            *tmp = FREEdoFreeNode (*tmp);
        } else {
            tmp = &CODE_NEXT ((*tmp));
        }
    }

    DBUG_RETURN (wln);
}

/** <!--********************************************************************-->
 *
 * @fn bool matchValues( node *fa, node *fb)
 *
 * @brief Match two AVIS nodes. This is to catch the case
 *        where one is an N_vardec and the other is an N_arg node.
 *
 * @param - fa and fb are N_avis nodes.
 *
 * @return Boolean TRUE if both fields are the same or can
 *          be shown to represent the same value.
 *
 *****************************************************************************/
static bool
matchValues (node *fa, node *fb)
{
    constant *fac;
    constant *fbc;
    bool z;

    DBUG_ENTER ();

    /* If one field is local and the other is a function argument,
     * we can have both AKV, but they do not share the same N_vardec.
     */
    z = (fa == fb);
    if ((!z) && (NULL != fa) && (NULL != fb)) {
        fac = COaST2Constant (fa);
        fbc = COaST2Constant (fb);
        z = (NULL != fac) && (NULL != fbc) && COcompareConstants (fac, fbc);
        fac = (NULL != fac) ? COfreeConstant (fac) : NULL;
        fbc = (NULL != fbc) ? COfreeConstant (fbc) : NULL;
    }

    if ((NULL != fa) && (NULL != fb)) {
        if (z) {
            DBUG_PRINT ("Values match");
        } else {
            DBUG_PRINT ("Values do not match");
        }
    }

#ifdef FIXME // bug??
    DBUG_ASSERT (FALSE == z, "this is supposed to be dead code!");
#endif // FIXME // bug??
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool matchGeneratorField( node *fa, node *fb)
 *
 * @brief Attempt to match corresponding N_id/N_array fields of
 *        two generators (BOUND, WIDTH, STEP),
 *        such as GENERATOR_BOUND1( wla) and GENERATOR_BOUND1( wlb).
 *        Fields "match" if they meet any of the following criteria:
 *         - both fields are NULL.
 *         - both fields refer to the same name.
 *         - both fields have a common ancestor in their SSAASSIGN chains.
 *         - fa can be shown to be the shape vector for foldewl.
 *
 * @param - fa is a GENERATOR_BOUND2 or similar generator field for
 *          the consumerWL.
 *          fb is a GENERATOR_BOUND2 or similar generator field for
 *          the producerWL.
 *
 * @return Boolean TRUE if both fields are the same or can
 *          be shown to represent the same shape vector.
 *
 *****************************************************************************/
bool
matchGeneratorField (node *fa, node *fb)
{
    node *fav = NULL;
    node *fbv = NULL;
    pattern *pata;
    pattern *patb;
    bool z;

    DBUG_ENTER ();

    pata = PMarray (1, PMAgetNode (&fav), 1, PMskip (0));
    patb = PMarray (1, PMAgetNode (&fbv), 1, PMskip (0));

    z = fa == fb;
    if ((!z)
        && (((NULL != fa) && (NULL != fb)) && (PMmatchFlatSkipExtrema (pata, fa))
            && (PMmatchFlatSkipExtrema (patb, fb)))) {
        z = CMPT_EQ == CMPTdoCompareTree (fav, fbv);
    }
    PMfree (pata);
    PMfree (patb);

    if ((!z) && (NULL != fa) && (NULL != fb)) {

        z = matchValues (fa, fb);
    }

    if ((NULL != fa) && (NULL != fb)) {
        if (z) {
            DBUG_PRINT ("matchGeneratorField matched");
        } else {
            DBUG_PRINT ("matchGeneratorField did not match");
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static intersect_type_t isNullIntersect( node *arg_node)
 *
 * @brief Function for determining Null intersection of two WLs
 *
 * @params arg_node: A F_noteintersect null intersection entry.
 *
 * @result: INTERSECT_null if intersection of index vector set and producerWL
 *          partition is empty.
 *          INTERSECT_notnull is the intersection is known to be non-empty.
 *          Otherwise, INTERSECT_unknown.
 *
 *****************************************************************************/
intersect_type_t
isNullIntersect (node *arg_node)
{
    intersect_type_t z = INTERSECT_unknown;
    constant *con;

    DBUG_ENTER ();

    con = COaST2Constant (arg_node);
    if (NULL != con) {
        z = (COisFalse (con, TRUE)) ? INTERSECT_notnull : INTERSECT_null;
        COfreeConstant (con);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static bool isExactIntersect( node *cbound1, node *intersectb1,
 *                                   node *cbound2, node *intersectb2)
 *
 * @brief Predicate for exact partition intersection.
 *
 * @params cbound1, cbound2: bounds of consumerWL partition
 *         intersectb1, intersectb2: intersection of producerWL partition
 *                                   and consumer WL idx extrema, normalize
 *                                   back to consumerWL bounds
 * @result: INTERSECT_exact if intersection is exact;
 *          INTERSECT_unknown if we are unable to find N_array bounds
 *          for all arguments;
 *          else INTERSECT_sliceneeded, on the assumption that
 *          we know the intersect is non-NULL.
 *
 *****************************************************************************/
static bool
isExactIntersect (node *cbound1, node *intersectb1, node *cbound2, node *intersectb2)
{
    pattern *pat;
    node *arr = NULL;
    node *c1;
    node *c2;
    node *i1;
    node *i2;
    intersect_type_t z = INTERSECT_unknown;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));

    PMmatchFlat (pat, cbound1);
    c1 = arr;
    PMmatchFlat (pat, cbound2);
    c2 = arr;
    PMmatchFlat (pat, intersectb1);
    i1 = arr;
    PMmatchFlat (pat, intersectb2);
    i2 = arr;

    if ((NULL != c1) && (NULL != c2) && (NULL != i1) && (NULL != i2)) {
        z = (matchGeneratorField (c1, i1)) && (matchGeneratorField (c2, i2))
              ? INTERSECT_exact
              : INTERSECT_nonexact;
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static intersect_type_t FindIntersection(
 *
 * @brief Search the _noteintersect_() list to see if there is an intersection
 *        that could match the current producerWL partition.
 *        Side effect is to set INFO_INTERSECTBOUND1/2( arg_info).
 *
 * @params idx: N_id node of _noteintersect() list.
 *         producerWLGenerator: PART_GENERATOR of producerWL
 *         cwlp: consumer WL partition.
 *         arg_info: Your basic arg_info
 *
 * @result: Type of WL intersection between cwlp and
 *          current producerWL partition.
 *
 *****************************************************************************/
static intersect_type_t
FindIntersection (node *idx, node *producerWLGenerator, node *cwlp, info *arg_info)
{
    intersect_type_t z = INTERSECT_unknown;
    node *producerWLBound1Original;
    node *producerWLBound2Original;
    node *consumerWLGenerator;
    node *bnd;
    node *proj1 = NULL;
    node *proj2 = NULL;
    node *noteint;
    pattern *pat;
    intersect_type_t nullIntersect;
    int intersectListNo;
    int intersectListLim;

    DBUG_ENTER ();

    intersectListNo = 0;

    /* NB. We assume noteintersect immediately precedes idx ! */
    noteint = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (idx))));
    DBUG_ASSERT (F_noteintersect == PRF_PRF (noteint), "Expected F_noteintersect");
    intersectListLim = TCcountExprs (PRF_ARGS (noteint));
    intersectListLim = (intersectListLim - 1) / WLEPP;
    pat = PMarray (1, PMAgetNode (&bnd), 1, PMskip (0));
    consumerWLGenerator = PART_GENERATOR (cwlp);
    if (NULL != arg_info) { /* Different callers! */
        INFO_WLPROJECTION1 (arg_info) = NULL;
        INFO_WLPROJECTION2 (arg_info) = NULL;
        INFO_NOTEINTERSECT (arg_info) = NULL;
    }

    while (((INTERSECT_unknown == z) || (INTERSECT_null == z))
           && (intersectListNo < intersectListLim)) {

        proj1 = TCgetNthExprsExpr (WLPROJECTION1 (intersectListNo), PRF_ARGS (noteint));
        proj2 = TCgetNthExprsExpr (WLPROJECTION2 (intersectListNo), PRF_ARGS (noteint));

        producerWLBound1Original
          = TCgetNthExprsExpr (WLBOUND1ORIGINAL (intersectListNo), PRF_ARGS (noteint));
        producerWLBound2Original
          = TCgetNthExprsExpr (WLBOUND2ORIGINAL (intersectListNo), PRF_ARGS (noteint));
        nullIntersect = isNullIntersect (
          TCgetNthExprsExpr (WLINTERSECTIONNULL (intersectListNo), PRF_ARGS (noteint)));

        if ((N_generator == NODE_TYPE (consumerWLGenerator))
            && (N_generator == NODE_TYPE (producerWLGenerator))
            && (matchGeneratorField (producerWLBound1Original,
                                     GENERATOR_BOUND1 (producerWLGenerator)))
            && (matchGeneratorField (producerWLBound2Original,
                                     GENERATOR_BOUND2 (producerWLGenerator)))
            && (matchGeneratorField (GENERATOR_STEP (producerWLGenerator),
                                     GENERATOR_STEP (consumerWLGenerator)))
            && (matchGeneratorField (GENERATOR_WIDTH (producerWLGenerator),
                                     GENERATOR_WIDTH (consumerWLGenerator)))) {
            DBUG_PRINT ("All generator fields match");
            switch (nullIntersect) {

            default:
                DBUG_ASSERT (FALSE, "null intersect confusion");
                break;

            case INTERSECT_null:
                z = INTERSECT_null;
                DBUG_PRINT ("Null intersect");
                break;

            case INTERSECT_notnull:
#ifdef FIXME // This is bad. N_array elements might be local to WL */
                /* FIXME clean up this code to use better PM, ala CF */
                bnd = NULL;
                PMmatchFlat (pat, proj1);
                proj1 = bnd;
                bnd = NULL;
                PMmatchFlat (pat, proj2);
                proj2 = bnd;
#endif //  FIXME // This is bad. N_array elements might be local to WL */

                if (NULL != arg_info) { /* Different callers! */
                    INFO_WLPROJECTION1 (arg_info) = proj1;
                    INFO_WLPROJECTION2 (arg_info) = proj2;
                    INFO_NOTEINTERSECT (arg_info) = noteint;
                }

                z = isExactIntersect (GENERATOR_BOUND1 (consumerWLGenerator), proj1,
                                      GENERATOR_BOUND2 (consumerWLGenerator), proj2);
                if (INTERSECT_exact == z) {
                    DBUG_PRINT ("Intersect suitable for exact AWLF");
                } else {
                    DBUG_PRINT ("Intersect suitable for sliced AWLF");
                }
                break;

            case INTERSECT_unknown:
                DBUG_PRINT ("intersection unknown");
                z = INTERSECT_unknown;
                break;
            }
        } else {
            DBUG_PRINT ("Generator field mismatch");
            z = INTERSECT_null;
        }
        intersectListNo++;
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 *
 * @fn node * CUBSLfindMatchingPart(...
 *
 * @brief Search for a producerWL partition that matches the
 *        consumerWLPart index set, or that is a superset thereof.
 *
 *        The search comprises two steps:
 *
 *        1. Find a partition in the producerWL that has bounds
 *           that match those in the F_noteintersect bounds, if
 *           such still exists.
 *           This search is required because other optimizations
 *           may have split a partition, reordered partitions
 *           within the producerWL, or deleted the partition
 *           entirely.
 *
 *           If we are unable to find a matching partition, we
 *           have to discard the consumerWLPart's intersect
 *           computation data, and redo it.
 *
 *        2. If we do find a matching partition, there are three
 *           types of intersection possible:
 *
 *            a. Null - no intersection, so no folding is possible.
 *
 *            b. ConsumerWL index set is subset of producerWL,
 *               or matches exactly.
 *               Folding is trivial, using the intersect data in
 *               the _noteintersect.
 *
 *            c. ConsumerWL index set is superset of producerWL.
 *               Folding is possible, but the ConsumerWL partition
 *               must be split into two or three partitions.
 *
 * @params arg_node: the N_prf of the sel(idx, producerWL).
 *         consumerpart: the consumerWL partition containing arg_node.
 *         producerWL: the N_with of the producerWL.
 *         arg_info: Either arg_info or NULL.
 *
 * @result: The address of the matching producerWL partition, if any.
 *          NULL if none is found.
 *
 *****************************************************************************/
node *
CUBSLfindMatchingPart (node *arg_node, intersect_type_t *itype, node *consumerpart,
                       node *producerWL, info *arg_info)
{
    node *producerWLGenerator;
    node *producerWLPart;
    node *z = NULL;
    node *idx;
    node *idxassign;
    node *idxparent;
    intersect_type_t intersecttype = INTERSECT_unknown;
    char *typ;
    int producerPartno = 0;

    DBUG_ENTER ();
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node), "expected N_prf arg_node");
    DBUG_ASSERT (N_with == NODE_TYPE (producerWL), "expected N_with producerWL");
    DBUG_ASSERT (N_part == NODE_TYPE (consumerpart), "expected N_part consumerpart");

    idx = PRF_ARG1 (arg_node); /* idx of _sel_VxA_( idx, producerWL) */
    idxassign = AVIS_SSAASSIGN (ID_AVIS (idx));
    idxparent = LET_EXPR (ASSIGN_INSTR (idxassign));

    producerWLPart = WITH_PART (producerWL);

    /* Find producerWL partition, if any, that matches the extrema of iv */
    while (((INTERSECT_unknown == intersecttype) || (INTERSECT_null == intersecttype))
           && (producerWLPart != NULL)) {
        producerWLGenerator = PART_GENERATOR (producerWLPart);
        intersecttype
          = FindIntersection (idx, producerWLGenerator, consumerpart, arg_info);

        switch (intersecttype) {
        default:
        case INTERSECT_unknown:
        case INTERSECT_null:
            typ = "NULL";
            break;

        case INTERSECT_exact:
            typ = "exact";
            z = producerWLPart;
            (*itype) = intersecttype;
            break;

        case INTERSECT_nonexact:
            typ = "slice needed";
            z = producerWLPart;
            (*itype) = intersecttype;
            break;
        }
        producerWLPart = PART_NEXT (producerWLPart);
        producerPartno++;
    }

    DBUG_PRINT ("Referent match type is (%s) for producerPartno %d", typ, producerPartno);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *CloneCode( arg_node, arg_info)
 *
 * @brief Clone a WL code block, renaming any LHS definitions.
 *
 * @params arg_node: an N_code
 *
 * @result: a cloned and renamed N_code
 *
 *****************************************************************************/
static node *
CloneCode (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ();

    if ((N_empty == NODE_TYPE (arg_node))) {
        DBUG_ASSERT (FALSE, "this looks funny.");
        z = NULL;
    } else {
        z = DUPdoDupTreeLutSsa (arg_node, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));
        CODE_INC_USED (z); /* DUP gives us Used=0 */
    }

    LUTremoveContentLut (INFO_LUT (arg_info));
    z = IVEXCdoIndexVectorExtremaCleanupPartition (z, NULL);

    /* prepend new code block to N_code chain */
    CODE_NEXT (z) = CODE_NEXT (INFO_WITHCODE (arg_info));
    CODE_NEXT (INFO_WITHCODE (arg_info)) = z;

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *PartitionSlicerOneAxis(...)
 *
 * @params consumerWLpartn: an N_part of the consumerWL.
 *         lb: An N_array, presenting the lower bound of the intersect
 *             between the consumerWL index set and a producerWL's
 *             partition bounds.
 *         ub: Same as lb, except the upper bound.
 *         axis: the axis which we are going to slice. e.g., for matrix,
 *            axis = 0 --> slice rows
 *            axis = 1 --> slice columns
 *            etc.
 *
 * @result: 1-3 N_part nodes.
 *
 * @brief Slice a WL partition into 1-3 partitions.
 *
 * We have a WL partition, consumerWLpartn, and bounds for
 * an intersection index set for the partition
 * that is smaller than the partition. We wish
 * to slice consumerWLpartn into sub-partitions, in order that AWLF
 * can operate on the sub-partition(s).
 *
 * In the simplest situation, there are three possible
 * cases of intersect. The rectangle represents consumerWLpartn;
 * the xxxx's represent the array covered by lb, ub.
 *
 *   alpha        beta        gamma
 *  __________   __________  _________
 *  |xxxxxxxxx| | partA   | | partA   |
 *  |xxpartIxx| |         | |         |
 *  |         | |xxxxxxxxx| |         |
 *  |         | |xxpartIxx| |         |
 *  | partC   | |         | |xxxxxxxxx|
 *  |         | | partC   | |xxxxxxxxx|
 *  |_________| |_________| |xxpartIxx|
 *
 *  For cases alpha and gamma, we split consumerWLpartn into two parts.
 *  For case beta, we split it into three parts.
 *
 * Because the intersection is multi-dimensional, we perform
 * the splitting on one axis at a time. Hence, we may end up
 * with each axis generating 1-3 new partitions for each
 * partition it gets as input.
 *
 * Here are the cases for splitting along axis 1 (columns) in a rank-2 array;
 * an x denotes partI:
 *
 *         Case 1:
 * alpha   alpha  alpha
 *
 * x..     .x.    ..x
 * ...     ...    ...
 * ...     ...    ...
 *
 *
 *         Case 2:
 * alpha   alpha  alpha
 * beta    beta   beta
 * ...     ...    ...
 * x..     .x.    ..x
 * ...     ...    ...
 *
 *
 *         Case 3:
 * alpha   alpha  alpha
 * gamma  gamma   gamma
 * ...    ...     ...
 * ...    ...     ...
 * x..    .x.     ..x
 *
 *
 *
 * Note: This code bears some resemblance to that of CutSlices.
 *       This is simpler, because we know more
 *       about the index set intersection.
 *
 * Note: These are the slicing requirements:
 *
 *        - The consumerWL index set is an N_array.
 *        - The intersect of the consumerWL index set with
 *          the producerWL partition bounds:
 *            . is non-empty  (or we are looking at a total mismatch)
 *            . is not an exact match (because it could fold as is).
 *            . has an associated set of AVIS_WITHIDS entries that
 *              are all known constants.
 *
 *****************************************************************************/
static node *
PartitionSlicerOneAxis (node *consumerWLpartn, node *lb, node *ub, int axis,
                        info *arg_info)
{
    node *partz = NULL;
    node *newpart = NULL;
    node *partlb = NULL;
    node *partub = NULL;
    node *step;
    node *width;
    node *withid;
    node *newlb;
    node *newub;
    node *genn;
    node *ilba;
    node *iuba;
    node *plba;
    node *puba;
    node *plb;
    node *pub;
    node *clone;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ();

    DBUG_ASSERT (N_part == NODE_TYPE (consumerWLpartn),
                 "expected N_part consumerWLpartn");
    DBUG_ASSERT (N_array == NODE_TYPE (lb), "expected N_array lb");
    DBUG_ASSERT (N_array == NODE_TYPE (ub), "expected N_array ub");

    plb = GENERATOR_BOUND1 (PART_GENERATOR (consumerWLpartn));
    pub = GENERATOR_BOUND2 (PART_GENERATOR (consumerWLpartn));
    pat1 = PMarray (1, PMAgetNode (&partlb), 1, PMskip (0));
    pat2 = PMarray (1, PMAgetNode (&partub), 1, PMskip (0));

    if (!((PMmatchFlat (pat1, plb)) && (PMmatchFlat (pat2, pub)))) {
        DBUG_ASSERT (FALSE, "Expected N_array generators");
    }
    step = GENERATOR_STEP (PART_GENERATOR (consumerWLpartn));
    width = GENERATOR_WIDTH (PART_GENERATOR (consumerWLpartn));
    withid = PART_WITHID (consumerWLpartn);

    ilba = TCgetNthExprsExpr (axis, ARRAY_AELEMS (lb));
    iuba = TCgetNthExprsExpr (axis, ARRAY_AELEMS (ub));
    plba = TCgetNthExprsExpr (axis, ARRAY_AELEMS (partlb));
    puba = TCgetNthExprsExpr (axis, ARRAY_AELEMS (partub));

    /* Cases beta, gamma need partA */
    if (CMPT_NEQ == CMPTdoCompareTree (ilba, plba)) {
        DBUG_PRINT ("Constructing partition A for %s",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        newlb = DUPdoDupNode (partlb);
        newub = DUPdoDupNode (partub);
        ARRAY_AELEMS (newub)
          = TCputNthExprs (axis, ARRAY_AELEMS (newub), DUPdoDupNode (ilba));
        genn
          = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupNode (step), NULL);
        clone = CloneCode (PART_CODE (consumerWLpartn), arg_info);
        newpart = TBmakePart (clone, DUPdoDupNode (withid), genn);
        partz = TCappendPart (partz, newpart);
    }

    /* All cases need partI or original node */
    DBUG_PRINT ("Constructing partition I for %s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
    newlb = DUPdoDupNode (partlb);
    ARRAY_AELEMS (newlb)
      = TCputNthExprs (axis, ARRAY_AELEMS (newlb), DUPdoDupNode (ilba));
    newub = DUPdoDupNode (partub);
    ARRAY_AELEMS (newub)
      = TCputNthExprs (axis, ARRAY_AELEMS (newub), DUPdoDupNode (iuba));
    genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupNode (step), NULL);
    clone = CloneCode (PART_CODE (consumerWLpartn), arg_info);
    newpart = TBmakePart (clone, DUPdoDupNode (withid), genn);
    partz = TCappendPart (partz, newpart);

    /* Case alpha, beta need partC */
    if (CMPT_NEQ == CMPTdoCompareTree (iuba, puba)) {
        DBUG_PRINT ("Constructing partition C for %s",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        newlb = DUPdoDupNode (partlb);
        newub = DUPdoDupNode (partub);
        ARRAY_AELEMS (newlb)
          = TCputNthExprs (axis, ARRAY_AELEMS (newlb), DUPdoDupNode (iuba));
        genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupNode (step),
                                DUPdoDupNode (width));
        clone = CloneCode (PART_CODE (consumerWLpartn), arg_info);
        newpart = TBmakePart (clone, DUPdoDupNode (withid), genn);
        partz = TCappendPart (partz, newpart);
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (partz);
}

/** <!--********************************************************************-->
 *
 * @fn static node *PartitionSlicer(...)
 *
 * @params arg_node: an N_part of the consumerWL.
 *         arg_info: your basic info node
 *         lb: an N_array, representing the lower-bound intersect of the
 *              consumerWL index set and a producerWL partition.
 *              lb must be the same shape as the partn generators.
 *         ub: Same as lb, but for upper-bound intersect.
 *
 * @result: 1-3 N_part nodes, per dimension of the WL generator.
 *          New N_code nodes are also built.
 *
 * @brief Slice a consumerWL partition into as many partitions as required.
 *
 *
 *****************************************************************************/
static node *
PartitionSlicer (node *arg_node, info *arg_info, node *lb, node *ub)
{
    node *newpartns = NULL;
    node *curpartn;
    node *p;
    pattern *pat;
    node *arr;
    node *lbarr;
    node *ubarr;
    int axes;
    int axis;

    DBUG_ENTER ();

    DBUG_ASSERT (N_part == NODE_TYPE (arg_node), "Expected N_part");

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));
    PMmatchFlat (pat, lb);
    lbarr = arr;
    PMmatchFlat (pat, ub);
    ubarr = arr;

    axes = SHgetUnrLen (ARRAY_FRAMESHAPE (lbarr));

    /* We start with one N_part, but each axis may add more N_parts. */

    for (axis = 0; axis < axes; axis++) {
        DBUG_PRINT ("Slicing partition %s on axis %d",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), axis);
        curpartn = arg_node;
        newpartns = NULL;
        while (NULL != curpartn) {
            p = PartitionSlicerOneAxis (curpartn, lbarr, ubarr, axis, arg_info);
            newpartns = TCappendPart (newpartns, p);
            curpartn = PART_NEXT (curpartn);
        }

        arg_node = FREEdoFreeTree (arg_node);
        arg_node = newpartns;
    }

    pat = PMfree (pat);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLfundef(node *arg_node, info *arg_info)
 *
 * @brief applies CUBSL to a given fundef.
 *
 *****************************************************************************/
node *
CUBSLfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("Algebraic-With-Loop-Folding Cube Slicing in %s %s begins",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));

        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /* If new vardecs were made, append them to the current set */
            if (INFO_VARDECS (arg_info) != NULL) {
                BLOCK_VARDEC (FUNDEF_BODY (arg_node))
                  = TCappendVardec (INFO_VARDECS (arg_info),
                                    BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
                INFO_VARDECS (arg_info) = NULL;
            }

            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

            INFO_ONEFUNDEF (arg_info) = old_onefundef;
        }

        DBUG_PRINT ("Algebraic-With-Loop-Folding Cube Slicing in %s %s ends",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));
    }

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node CUBSLassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, foldee).
 *
 *        Also prepends newly generated WL bounds before the
 *        WL they are associated with.
 *
 *****************************************************************************/
node *
CUBSLassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if ((N_let == NODE_TYPE (ASSIGN_INSTR (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))))
        && (NULL != INFO_PREASSIGNSWITH (arg_info))) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
        INFO_PREASSIGNSWITH (arg_info) = NULL;
    }

    /*
     * Top-down traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLwith( node *arg_node, info *arg_info)
 *
 * @brief applies CUBSL to a with-loop.
 *        We replace the current WITH_CODE chain with
 *        the old chain amended with new partition codes.
 *
 *        We work from innermost WL to outermost, which I
 *        think is correct, but am not positive.
 *
 *****************************************************************************/
node *
CUBSLwith (node *arg_node, info *arg_info)
{
    node *oldwithcode;
    intersect_type_t oldintersecttype;

    DBUG_ENTER ();

    oldintersecttype = INFO_INTERSECTTYPE (arg_info);
    INFO_INTERSECTTYPE (arg_info) = INTERSECT_unknown;
    oldwithcode = INFO_WITHCODE (arg_info);
    INFO_WITHCODE (arg_info) = WITH_CODE (arg_node);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
    DBUG_ASSERT (WITH_CODE (arg_node) == INFO_WITHCODE (arg_info),
                 "N_code list has been tangled");
    INFO_INTERSECTTYPE (arg_info) = oldintersecttype;
    INFO_WITHCODE (arg_info) = oldwithcode;
    arg_node = CheckForSuperfluousCodes (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 * In order for slicing to occur, the WL projections must have
 * been moved out of the WL by LIR. If they have not been moved,
 * then the projections have some sort of dependence on locals
 * within the WL.
 *
 *****************************************************************************/
node *
CUBSLpart (node *arg_node, info *arg_info)
{
    node *oldconsumerpart;
    node *partnext;
    intersect_type_t oldintersecttype;

    DBUG_ENTER ();

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_PRINT ("traversing partition for %s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
    DBUG_ASSERT (INTERSECT_unknown == INFO_INTERSECTTYPE (arg_info),
                 "partition confusion");

    oldconsumerpart = INFO_CONSUMERPART (arg_info);
    INFO_CONSUMERPART (arg_info) = arg_node;
    oldintersecttype = INFO_INTERSECTTYPE (arg_info);
    INFO_INTERSECTTYPE (arg_info) = INTERSECT_unknown;

    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVopt (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    if ((INTERSECT_nonexact == INFO_INTERSECTTYPE (arg_info))
        && (NULL != INFO_WLPROJECTION1 (arg_info))
        && (AWLFIisHasInverseProjection (INFO_WLPROJECTION1 (arg_info)))
        && (!AWLFIisIdsMemberPartition (INFO_WLPROJECTION1 (arg_info), arg_node))
        && (NULL != INFO_WLPROJECTION2 (arg_info))
        && (AWLFIisHasInverseProjection (INFO_WLPROJECTION2 (arg_info)))
        && (!AWLFIisIdsMemberPartition (INFO_WLPROJECTION2 (arg_info), arg_node))) {

        partnext = PART_NEXT (arg_node);
        PART_NEXT (arg_node) = NULL;
        arg_node = PartitionSlicer (arg_node, arg_info, INFO_WLPROJECTION1 (arg_info),
                                    INFO_WLPROJECTION2 (arg_info));
        PART_NEXT (arg_node) = TCappendPart (PART_NEXT (arg_node), partnext);
    }

    DBUG_PRINT ("Partition %s intersect type is %d",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                INFO_INTERSECTTYPE (arg_info));
    INFO_CONSUMERPART (arg_info) = oldconsumerpart;
    INFO_INTERSECTTYPE (arg_info) = oldintersecttype;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLlet( node *arg_node, info *arg_info)
 *
 * @brief Save the LHS name for debugging
 *
 *****************************************************************************/
node *
CUBSLlet (node *arg_node, info *arg_info)
{
    node *oldlhs;

    DBUG_ENTER ();

    oldlhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);
#ifdef VERBOSE
    DBUG_PRINT ("Start looking at %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
#endif // VERBOSE

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

#ifdef VERBOSE
    DBUG_PRINT ("Finished looking at %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
#endif // VERBOSE

    INFO_LHS (arg_info) = oldlhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLprf( node *arg_node, info *arg_info)
 *
 * @brief Look for suitable _sel_VxA_( iv, X) nodes in this WL partition.
 *        Leave cube-slicing info in arg_info.
 *
 *****************************************************************************/
node *
CUBSLprf (node *arg_node, info *arg_info)
{
    node *producerPart;
    node *producerWL;

    DBUG_ENTER ();

    if ((F_sel_VxA == PRF_PRF (arg_node)) && (INFO_CONSUMERPART (arg_info) != NULL)
        && (AWLFIisHasNoteintersect (arg_node))) {
        DBUG_PRINT ("Looking at %s =_sel_VxA_( iv, X)",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        producerWL = AWLFIfindWlId (PRF_ARG2 (arg_node));
        producerWL = AWLFIgetWlWith (producerWL);

        /* producerWL may be entirely gone now, perhaps
         * due to it being copyWL, etc.
         * Or, it may have been sliced, so partition bounds differ from
         * what they were originally.
         *
         * Side effect of call is to set INFO_CONSUMERPART.
         */
        if (NULL != producerWL) {
            producerPart
              = CUBSLfindMatchingPart (arg_node, &INFO_INTERSECTTYPE (arg_info),
                                       INFO_CONSUMERPART (arg_info), producerWL,
                                       arg_info);
            if (NULL != producerPart) {
                DBUG_PRINT ("CUBSLprf found producerPart");
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding cube slicing -->
 *****************************************************************************/

#undef DBUG_PREFIX
