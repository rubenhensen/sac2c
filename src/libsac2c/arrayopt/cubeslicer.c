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
#include "check.h"
#include "with_loop_utilities.h"
#include "gdb_utils.h"
#include "symbolic_constant_simplification.h"
#include "flattengenerators.h"
#include "tree_utils.h"
#include "compare_tree.h"
#include "narray_utilities.h"
#include "new_typecheck.h"

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
    node *lhs;
    node *consumerpart;
    node *wlprojection1;            /* lower bound of WL proj */
    node *wlprojection2;            /* upper bound of WL proj */
    node *withcode;                 /* WITH_CODE from N_with */
    intersect_type_t intersecttype; /* intersect type. see enum */
    lut_t *lut;                     /* LUT for renaming */
    node *producerpart;             /* PWL part matching CWL */
    bool cutnow;                    /* Cut this CWL partn now */
    bool isfoldnow;                 /* Fold this partition now. */
    node *noteintersect;            /* The relevant noteintersect in the N_part */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNSWITH(n) ((n)->preassignswith)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_CONSUMERPART(n) ((n)->consumerpart)
#define INFO_WLPROJECTION1(n) ((n)->wlprojection1)
#define INFO_WLPROJECTION2(n) ((n)->wlprojection2)
#define INFO_WITHCODE(n) ((n)->withcode)
#define INFO_INTERSECTTYPE(n) ((n)->intersecttype)
#define INFO_LUT(n) ((n)->lut)
#define INFO_PRODUCERPART(n) ((n)->producerpart)
#define INFO_CUTNOW(n) ((n)->cutnow)
#define INFO_ISFOLDNOW(n) ((n)->isfoldnow)
#define INFO_NOTEINTERSECT(n) ((n)->noteintersect)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNSWITH (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_CONSUMERPART (result) = NULL;
    INFO_WLPROJECTION1 (result) = NULL;
    INFO_WLPROJECTION2 (result) = NULL;
    INFO_WITHCODE (result) = NULL;
    INFO_INTERSECTTYPE (result) = INTERSECT_unknown;
    INFO_LUT (result) = NULL;
    INFO_PRODUCERPART (result) = NULL;
    INFO_CUTNOW (result) = FALSE;
    INFO_ISFOLDNOW (result) = FALSE;
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
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "CUBSLdoAlgebraicWithLoopFoldingCubeSlicing called for non-fundef");

    TRAVpush (TR_cubsl);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

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
 *   node *RemoveSuperfluousCodes(node *wln)
 *
 * description:
 *   remove all unused N_code nodes of the given WL.
 *
 *  This was stolen from SSAWLF. We should move it into tree/somewhere,
 *  but I am waiting for word back from Clemens and Bodo.
 *
 ******************************************************************************/

static node *
RemoveSuperfluousCodes (node *wln)
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
static bool
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

    z = (fa == fb)
        || (((NULL != fa) && (NULL != fb)) && (PMmatchFlatSkipExtrema (pata, fa))
            && (PMmatchFlatSkipExtrema (patb, fb)) && (TULSisValuesMatch (fa, fb)));
    pata = PMfree (pata);
    patb = PMfree (patb);

    if ((NULL != fa) && (NULL != fb)) {
        if (z) {
            DBUG_PRINT ("matched");
        } else {
            DBUG_PRINT ("did not match");
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static isNullIntersect( node *arg_node)
 * @fn static isNotNullIntersect( node *arg_node)
 *
 * @brief Functions for determining Null intersection of two WLs
 *
 * @params arg_node: An F_noteintersect null-intersection entry.
 *
 * @result: IsNullIntersect: TRUE if any element of the
 *          intersection between the index vector set and producerWL
 *          partition is empty.
 *
 *          Otherwise, FALSE. Slicing may or may not be required.
 *          And, we may not yet know if the intersect is, or is not, NULL.
 *          I.e., do not make decisions based on a FALSE result here.
 *
 *          IsNotNullIntersect: TRUE if all elements of the
 *          intersection of index vector set and producerWL
 *          are non-null. Else FALSE.
 *
 *
 *****************************************************************************/
static bool
isNullIntersect (node *arg_node)
{
    bool z;

    DBUG_ENTER ();

    z = NAUTisMemberArray (TRUE, arg_node);

    DBUG_RETURN (z);
}

static bool
isNotNullIntersect (node *arg_node)
{
    bool z = FALSE;
    constant *con;

    DBUG_ENTER ();

    con = COaST2Constant (arg_node);
    if (NULL != con) {
        z = COisFalse (con, TRUE);
        COfreeConstant (con);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static isInt1Part( node *arg_node)
 * @fn static isNotInt1Part( node *arg_node)
 *
 * @brief Functions for determining if WL intersection does not
 *        cross PWL partition boundaries.
 *
 * @params arg_node: An F_noteintersect WLINTERSECT1PART entry.
 *         cwlb1, cwlb2: Consumer-WL partition bounds
 *         cwlproj1, cwlproj2: Consumer-WL inverse projection bounds
 *
 * @result: isInt1Part: TRUE if ALL elements of the
 *          intersection of the index vector set and producerWL
 *          lie within a single partition of the PWL.
 *
 *          Also TRUE if bounds and projections match.
 *
 *          Else FALSE.
 *
 *          IsNotInt1Part: TRUE if ANY element of the
 *          intersection of the index vector set and producerWL
 *          crosses a partition of the PWL.
 *          Else FALSE.
 *
 *****************************************************************************/
static bool
isInt1Part (node *arg_node, node *cwlb1, node *cwlb2, node *cwlproj1, node *cwlproj2)
{
    bool z = FALSE;
    bool z2 = FALSE; // DEBUG FIXME
    constant *con;

    DBUG_ENTER ();

    con = COaST2Constant (arg_node);
    if (NULL != con) {
        z = COisTrue (con, TRUE);
        COfreeConstant (con);
    }

    int fixthis;
    z2 = (TULSisValuesMatch (cwlb1, cwlproj1) && TULSisValuesMatch (cwlb2, cwlproj2));
    if (!z) {
        DBUG_PRINT ("someone is confused");
        // z = z || z2;
    }

    DBUG_RETURN (z);
}

static bool
isNotInt1Part (node *arg_node)
{
    bool z;

    DBUG_ENTER ();

    z = NAUTisMemberArray (FALSE, arg_node);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static void SetWLProjections(...)
 *
 * @brief Side-effect setter for findIntersection
 *
 * @params noteint: An F_noteintersect N_prf node
 *         intersectListNo: which PWL partition number we are looking at
 *         arg_info: your basic arg_info
 *
 * @result:
 *
 *****************************************************************************/
static void
SetWLProjections (node *noteint, int intersectListNo, info *arg_info)
{

    DBUG_ENTER ();

    if (NULL != arg_info) {
        INFO_WLPROJECTION1 (arg_info)
          = TCgetNthExprsExpr (WLPROJECTION1 (intersectListNo), PRF_ARGS (noteint));
        INFO_WLPROJECTION2 (arg_info)
          = TCgetNthExprsExpr (WLPROJECTION2 (intersectListNo), PRF_ARGS (noteint));
    }

    DBUG_RETURN ();
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
 *         cwlp: consumer WL partition, or NULL if naked consumer
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
    node *pwlB1Original;
    node *pwlB2Original;
    node *cwlpg;
    node *bnd;
    node *proj1 = NULL;
    node *proj2 = NULL;
    node *noteint;
    pattern *pat;
    int intersectListNo;
    int intersectListLim;
    node *cwlpb1;
    node *cwlpb2;
    bool cwlpstepok;
    bool cwlpwidthok;
    node *intersect1Part;
    bool int1part;
    bool notint1part;
    bool intnull;
    bool notintnull;
    int noteintinsertcycle;

    DBUG_ENTER ();

    noteint = AWLFIfindNoteintersect (idx);
    noteintinsertcycle = PRF_NOTEINTERSECTINSERTIONCYCLE (noteint);
    intersectListLim = (TCcountExprs (PRF_ARGS (noteint)) - WLFIRST) / WLEPP;
    pat = PMarray (1, PMAgetNode (&bnd), 1, PMskip (0));
    if (NULL != cwlp) { /* support for naked consumer - no CWL */
        cwlpg = PART_GENERATOR (cwlp);
        cwlpb1 = GENERATOR_BOUND1 (cwlpg);
        cwlpb2 = GENERATOR_BOUND2 (cwlpg);
        cwlpstepok = matchGeneratorField (GENERATOR_STEP (producerWLGenerator),
                                          GENERATOR_STEP (cwlpg));
        cwlpwidthok = matchGeneratorField (GENERATOR_WIDTH (producerWLGenerator),
                                           GENERATOR_WIDTH (cwlpg));
    } else { /* For naked consumer with constant or extrema'd iv,
              * STEP and WIDTH are 1, and any non-null intersect is good.
              */
        cwlpb1 = NULL;
        cwlpb2 = NULL;
        cwlpstepok = (NULL == GENERATOR_STEP (producerWLGenerator))
                     || (SCSmatchConstantOne (GENERATOR_STEP (producerWLGenerator)));
        cwlpwidthok = (NULL == GENERATOR_WIDTH (producerWLGenerator))
                      || (SCSmatchConstantOne (GENERATOR_WIDTH (producerWLGenerator)));
    }

    intersectListNo = 0;
    while (((INTERSECT_unknown == z) || (INTERSECT_null == z))
           && (intersectListNo < intersectListLim)) {

        DBUG_PRINT ("Started check of partition #%d", intersectListNo);

        proj1 = TCgetNthExprsExpr (WLPROJECTION1 (intersectListNo), PRF_ARGS (noteint));
        proj2 = TCgetNthExprsExpr (WLPROJECTION2 (intersectListNo), PRF_ARGS (noteint));

        pwlB1Original
          = TCgetNthExprsExpr (WLBOUND1ORIGINAL (intersectListNo), PRF_ARGS (noteint));
        pwlB2Original
          = TCgetNthExprsExpr (WLBOUND2ORIGINAL (intersectListNo), PRF_ARGS (noteint));
        intersect1Part
          = TCgetNthExprsExpr (WLINTERSECTION1PART (intersectListNo), PRF_ARGS (noteint));

        if ((matchGeneratorField (pwlB1Original, GENERATOR_BOUND1 (producerWLGenerator)))
            && (matchGeneratorField (pwlB2Original,
                                     GENERATOR_BOUND2 (producerWLGenerator)))
            && (cwlpstepok && cwlpwidthok)) {
            DBUG_PRINT ("Producer partition appears unchanged");

            int1part = isInt1Part (intersect1Part, cwlpb1, cwlpb2, proj1, proj2);
            notint1part = isNotInt1Part (intersect1Part);
            intnull
              = isNullIntersect (TCgetNthExprsExpr (WLINTERSECTIONNULL (intersectListNo),
                                                    PRF_ARGS (noteint)));
            notintnull = isNotNullIntersect (
              TCgetNthExprsExpr (WLINTERSECTIONNULL (intersectListNo),
                                 PRF_ARGS (noteint)));

            /* Slicing and exact intersect criteria */
            // After four kicks at the AWLFI can, give up and slice.
            if (((global.cycle_counter - noteintinsertcycle) > 3)
                && (AWLFIisHasInverseProjection (proj1))
                && (AWLFIisHasInverseProjection (proj2))) {
                DBUG_PRINT ("Blind slicing cube at cycle %d", global.cycle_counter);
#define BROKEN
#ifdef BROKEN // This definitely breaks a few things:
              // sac2c codingtimeformul.sac -v1  -doawlf -nowlf -noctz
              // gives wrong answers.
              // Also, takeAKSunknownAKDVector.sac goes to cube-slicing
              // heaven, never quite getting the intersect right
              // for AWLF.

                z = INTERSECT_sliceneeded;
                SetWLProjections (noteint, intersectListNo, arg_info);
#endif BROKEN
            }

            if (intnull) {
                DBUG_PRINT ("Null intersect");
                z = INTERSECT_null;
            }

            if (int1part) {
                z = INTERSECT_exact;
                DBUG_PRINT ("exact intersect and not NULL");
                SetWLProjections (noteint, intersectListNo, arg_info);
            }

            if (notint1part && notintnull) {
                DBUG_PRINT ("slice needed");
                z = INTERSECT_sliceneeded;
                SetWLProjections (noteint, intersectListNo, arg_info);
            }

            if (int1part && (NULL == cwlpb1)) {
                DBUG_PRINT ("Naked consumer detected");
                z = INTERSECT_exact;
                SetWLProjections (noteint, intersectListNo, arg_info);
            }
        } else {
            DBUG_PRINT ("Generator field(s) changed underfoot");
            z = INTERSECT_null;
        }
        DBUG_PRINT ("Finished check of partition #%d", intersectListNo);
        intersectListNo++;
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn
 *
 * @brief Return a string that describes the intersect type
 *
 * @params:
 *
 * @result: A pointer to a character string constant.
 *
 *****************************************************************************/
static __attribute__ ((unused)) char *
IntersectTypeName (intersect_type_t itype)
{
    char *z;

    DBUG_ENTER ();

    switch (itype) {
    case INTERSECT_null:
        z = "INTERSECT_null";
        break;
    case INTERSECT_exact:
        z = "INTERSECT_exact";
        break;
    case INTERSECT_sliceneeded:
        z = "INTERSECT_sliceneeded";
        break;
    case INTERSECT_notnull:
        z = "INTERSECT_notnull";
        break;
    case INTERSECT_unknown:
        z = "INTERSECT_unknown";
        break;
    default:
        z = "INTERSECT is confused";
        break;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 *
 * @fn node * CUBSLfindMatchingPart(...
 *
 * @brief Search for a pwl partition that is the best
 *        match to the consumerWLPart index set.
 *
 *        The search comprises two steps:
 *
 *        1. Find a partition in the pwl that has bounds
 *           that match those in the F_noteintersect bounds, if
 *           such still exists.
 *           This search is required because other optimizations
 *           may have split a partition, reordered partitions
 *           within the pwl, or deleted the partition
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
 *            b. ConsumerWL index set is subset of pwl,
 *               or matches exactly.
 *               Folding is trivial, using the intersect data in
 *               the _noteintersect.
 *
 *            c. ConsumerWL index set is superset of pwl.
 *               Folding is possible, but the ConsumerWL partition
 *               must be split into two or three partitions.
 *
 * @params arg_node: the N_prf of the sel(idx, pwl).
 *         cwlp: the consumerWL partition containing arg_node,
 *           or NULL, if we have a naked consumer.
 *         pwl: the N_with of the pwl.
 *         arg_info: Either arg_info or NULL.
 *
 * @result: The type of match we found; NULL if none is found.
 *          We also set INFO_PRODUCERPART, if a match is found.
 *
 *****************************************************************************/
intersect_type_t
CUBSLfindMatchingPart (node *arg_node, node *cwlp, node *pwl, info *arg_info,
                       node **producerpart)
{
    node *producerWLGenerator;
    node *producerWLPart;
    intersect_type_t z = INTERSECT_unknown;
    intersect_type_t intersecttype = INTERSECT_unknown;
    node *idx;
    node *idxassign;
    node *idxparent;
    int producerPartno = 0;
    int intPartno = -1;
    node *noteint;
    char *nm;

    DBUG_ENTER ();
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node), "expected N_prf arg_node");
    DBUG_ASSERT (N_with == NODE_TYPE (pwl), "expected N_with pwl");

    idx = PRF_ARG1 (arg_node); /* idx of _sel_VxA_( idx, pwl) */
    noteint = AWLFIfindNoteintersect (idx);
    idxassign = AVIS_SSAASSIGN (ID_AVIS (idx));
    idxparent = LET_EXPR (ASSIGN_STMT (idxassign));
    if (NULL != arg_info) {
        INFO_WLPROJECTION1 (arg_info) = NULL;
        INFO_WLPROJECTION2 (arg_info) = NULL;
    }

    producerWLPart = WITH_PART (pwl);
    (*producerpart) = NULL;

    /* Find pwl partition, if any, that matches the extrema of iv */
    while (producerWLPart != NULL) {
        producerWLGenerator = PART_GENERATOR (producerWLPart);
        intersecttype = FindIntersection (idx, producerWLGenerator, cwlp, arg_info);
        if (intersecttype > z) {
            (*producerpart) = producerWLPart; /* Note best match */
            z = intersecttype;
            intPartno = producerPartno;
        }
        producerWLPart = PART_NEXT (producerWLPart);
        producerPartno++;
    }

    if (NULL != arg_info) {
        nm = AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)));
    } else {
        nm = "?";
    }

    DBUG_PRINT ("match type is (%s) for intPartno %d of PWL=%s, CWL=%s",
                IntersectTypeName (z), intPartno,
                AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))), nm);

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
 *          INFO_LUT remains with the new names, because we need
 *          it for the matching F_noteintersect rename of WLIVAVIS
 *
 *****************************************************************************/
static node *
CloneCode (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ();

    DBUG_ASSERT (1 == CODE_USED (arg_node), "CODE_USED confusion3");
    LUTremoveContentLut (INFO_LUT (arg_info));
    z = DUPdoDupTreeLutSsa (arg_node, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));
    CODE_INC_USED (z); /* DUP gives us Used=0 */

    z = IVEXCdoIndexVectorExtremaCleanupPartition (z, NULL);

    /* prepend new code block to N_code chain */
    CODE_NEXT (z) = CODE_NEXT (INFO_WITHCODE (arg_info));
    CODE_NEXT (INFO_WITHCODE (arg_info)) = z;

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildSubcube(...)
 *
 * @params arg_node: an N_part of the consumerWL.
 *         arg_info: your basic info node
 *         lb: the generator lower bound for the new partition
 *         ub: the generator upper bound for the new partition
 *         step: the generator step for the new partition
 *         width: the generator width for the new partition
 *         withid: the WITHID for the new partition
 *
 * @result: Build new partition from arg_node and new bounds.
 *          New N_code node is also built.
 *
 * @brief From a consumerWL partition, construct one N_part.
 *
 *****************************************************************************/
static node *
BuildSubcube (node *arg_node, info *arg_info, node *lb, node *ub, node *step, node *width,
              node *withid)
{
    node *clone;
    node *newpart;
    node *genn;

    DBUG_ENTER ();
    int fixme; // need to check current bounds against new bounds!
               // and maybe is1Part and/or isNull

    genn = TBmakeGenerator (F_wl_le, F_wl_lt, DUPdoDupNode (lb), DUPdoDupNode (ub),
                            DUPdoDupNode (step), DUPdoDupNode (width));
    clone = CloneCode (PART_CODE (arg_node), arg_info);
    newpart = TBmakePart (clone, DUPdoDupNode (withid), genn);

    DBUG_RETURN (newpart);
}

/** <!--********************************************************************-->
 *
 * @fn static node *FindMarkedSelAssign( node *assgn)
 *
 * @params assgn: The N_assign chain for the current N_part/N_code block
 *
 * @brief: The N_assign node that has the marked sel() primitive
 *          as its RHS
 *
 * @result: The relevant N_assign node.
 *          If not found, crash. If more than one found in the block,
 *          crash.
 *
 *****************************************************************************/
static node *
FindMarkedSelAssign (node *assgn)
{
    node *z = NULL;

    DBUG_ENTER ();

    while (NULL != assgn) {
        if ((N_let == NODE_TYPE (ASSIGN_STMT (assgn)))
            && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_STMT (assgn))))
            && ((F_sel_VxA == PRF_PRF (LET_EXPR (ASSIGN_STMT (assgn))))
                || ((F_idx_sel == PRF_PRF (LET_EXPR (ASSIGN_STMT (assgn))))))
            && (PRF_ISFOLDNOW (LET_EXPR (ASSIGN_STMT (assgn))))) {

            DBUG_ASSERT (NULL == z, "More than one marked sel() found in N_part");
            z = assgn;
        }
        assgn = ASSIGN_NEXT (assgn);
    }

    DBUG_ASSERT (NULL != z, "No marked sel() found in N_part");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildNewNoteintersect( newpart, partno, noteintersect);
 *
 * @params newpart: A newly minted CWL N_part, resulting from cube slicing
 *         partno: the index into noteintersect of the PWL partition
 *         that newpart matches.
 *         nodeintersect: The N_prf node for the F_noteintersect
 *
 * @result: The amended newpart
 *
 * @brief Locate the sel() primitive that is to be folded, and
 *        create a new F_noteintersect for it, containing only
 *        one entry, for PWL partno.
 *
 *        We start with this:
 *
 *         selassign =  { F_idx_sel( iv, pwl) : selnext}
 *
 *        and produce this:
 *
 *          selassign = { F_noteintersect( iv,...) : newassign}
 *          newassign = { F_idx_sel( selassign, pwl) : selnext}
 *
 *
 *****************************************************************************/
static node *
BuildNewNoteintersect (node *newpart, int partno, node *noteintersect, info *arg_info)
{
    node *z;
    node *ni;
    node *assgn;
    node *niavis;
    ntype *restype;
    node *nilet;
    node *selprf;
    node *iv;
    node *selassgn;
    node *selavis;
    node *onepart;

    DBUG_ENTER ();

    ni = DUPdoDupTreeLutSsa (noteintersect, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));
    // Mark the relevant partition as having a one-partition intersect,
    // so that it will immediately AWLF.
    onepart = TCgetNthExprsExpr (WLINTERSECTION1PART (partno), PRF_ARGS (ni));
    onepart = SCSmakeTrue (onepart);
    onepart = FLATGexpression2Avis (onepart, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNSWITH (arg_info), NULL);
    PRF_ARGS (ni)
      = TCputNthExprs (WLINTERSECTION1PART (partno), PRF_ARGS (ni), TBmakeId (onepart));

    // Stick new F_noteintersect before sel().
    assgn = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (newpart)));
    assgn = FindMarkedSelAssign (assgn);

    // We have to make the N_assign for the sel() become the
    // N_assign for the F_noteintersect, then link that to the
    // new N_assign, which we fill with the sel().
    restype = TYcopyType (AVIS_TYPE (ID_AVIS (PRF_ARG1 (noteintersect))));
    niavis = TBmakeAvis (TRAVtmpVar (), restype);
    INFO_VARDECS (arg_info) = TBmakeVardec (niavis, INFO_VARDECS (arg_info));
    nilet = TBmakeLet (TBmakeIds (niavis, NULL), ni);

    // Make the sel() use the F_noteintersect result as PRF_ARG1
    selprf = LET_EXPR (ASSIGN_STMT (assgn));
    iv = DUPdoDupNode (PRF_ARG1 (selprf));
    PRF_ARG1 (selprf) = FREEdoFreeNode (PRF_ARG1 (selprf));
    PRF_ARG1 (selprf) = TBmakeId (niavis);

    // Unmark the sel() N_prf.
    PRF_ISFOLDNOW (selprf) = FALSE;

    // Make the F_noteintersect use the sel()'s PRF_ARG1 as its PRF_ARG1
    PRF_ARG1 (ni) = FREEdoFreeNode (PRF_ARG1 (ni));
    PRF_ARG1 (ni) = iv;

    // Splice new F_noteintersect into the N_assign chain.
    selassgn = TBmakeAssign (ASSIGN_STMT (assgn), ASSIGN_NEXT (assgn)); // F_sel()
    ASSIGN_STMT (assgn) = nilet;                                        // F_noteintersect
    AVIS_SSAASSIGN (niavis) = assgn;
    ASSIGN_NEXT (assgn) = selassgn;
    selavis = IDS_AVIS (LET_IDS (ASSIGN_STMT (selassgn)));
    AVIS_SSAASSIGN (selavis) = selassgn;

    z = newpart;

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildSubcubes(...)
 *
 * @params arg_node: an N_part of the consumerWL.
 *         arg_info: your basic info node
 *
 * @result: New N_part node, or NULL (if this is simple composition)
 *          New N_code nodes are also built.
 *
 * @brief From a consumerWL partition, construct one N_part
 *        per noteintersect segment.
 *
 *****************************************************************************/
static node *
BuildSubcubes (node *arg_node, info *arg_info)
{
    node *newpartns = NULL;
    node *newpart;
    int partno;
    int partlim;
    node *lb;
    node *ub;
    node *noteintersect;
    node *step;
    node *width;
    node *withid;
    pattern *patlb;
    pattern *patub;
    node *assgn;

    DBUG_ENTER ();

    DBUG_ASSERT (N_part == NODE_TYPE (arg_node), "Expected N_part");

    patlb = PMarray (1, PMAgetNode (&lb), 1, PMskip (0));
    patub = PMarray (1, PMAgetNode (&ub), 1, PMskip (0));

    noteintersect = INFO_NOTEINTERSECT (arg_info);
    partlim = (TCcountExprs (PRF_ARGS (noteintersect)) - WLFIRST) / WLEPP;
    partno = 0;

    step = GENERATOR_STEP (PART_GENERATOR (arg_node));
    width = GENERATOR_WIDTH (PART_GENERATOR (arg_node));
    withid = PART_WITHID (arg_node);

    if (partlim != 1) { // Do not slice simple compositions
        DBUG_PRINT ("Slicing partition %s into %d pieces",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), partlim);
        while (partno < partlim) {
            PMmatchFlat (patlb, TCgetNthExprsExpr (WLPROJECTION1 (partno),
                                                   PRF_ARGS (noteintersect)));
            PMmatchFlat (patub, TCgetNthExprsExpr (WLPROJECTION2 (partno),
                                                   PRF_ARGS (noteintersect)));
            // Next two lines are poor man's assertion
            assgn = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (arg_node)));
            assgn = FindMarkedSelAssign (assgn);

            newpart = BuildSubcube (arg_node, arg_info, lb, ub, step, width, withid);
            newpart = BuildNewNoteintersect (newpart, partno, noteintersect, arg_info);
            newpartns = TCappendPart (newpartns, newpart);
            partno++;
        }
    } else {
        // Simple composition merely gets marked for folding
        newpartns = DUPdoDupNode (arg_node);
        newpartns = BuildNewNoteintersect (newpartns, partno, noteintersect, arg_info);
    }

    patlb = PMfree (patlb);
    patub = PMfree (patub);

    global.optcounters.cubsl_expr++;

    DBUG_RETURN (newpartns);
}

/** <!--********************************************************************-->
 *
 * @fn bool
 *
 * @brief: Predicate for presence of at least one exact intersect in arg_node
 *
 * @param: arg_node - an F_noteintersect.
 *
 * @result: TRUE if arg_node has at least one exact intersect present
 *
 *****************************************************************************/
static bool
IntersectExactPresent (node *arg_node)
{
    bool z = FALSE;
    int intersectListLim;
    int intersectListNo;
    node *onepart;
    node *isnull;

    DBUG_ENTER ();

    intersectListNo = 0;
    intersectListLim
      = (NULL != arg_node) ? (TCcountExprs (PRF_ARGS (arg_node)) - WLFIRST) / WLEPP : 0;

    while ((!z) && (intersectListNo < intersectListLim)) {
        onepart = TCgetNthExprsExpr (WLINTERSECTION1PART (intersectListNo),
                                     PRF_ARGS (arg_node));
        isnull
          = TCgetNthExprsExpr (WLINTERSECTIONNULL (intersectListNo), PRF_ARGS (arg_node));
        z = SCSmatchConstantOne (onepart);
        // DEADCODE && SCSmatchConstantZero( isnull);
        intersectListNo++;
    }

    DBUG_RETURN (z);
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

    info *oldarginfo;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("Algebraic-With-Loop-Folding Cube Slicing in %s %s begins",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));

        oldarginfo = arg_info;
        arg_info = MakeInfo (arg_node);
        INFO_LUT (arg_info) = LUTgenerateLut ();

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /* If new vardecs were made, append them to the current set */
            if (INFO_VARDECS (arg_info) != NULL) {
                BLOCK_VARDECS (FUNDEF_BODY (arg_node))
                  = TCappendVardec (INFO_VARDECS (arg_info),
                                    BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
                INFO_VARDECS (arg_info) = NULL;
            }

            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }

        INFO_LUT (arg_info) = LUTremoveLut (INFO_LUT (arg_info));
        arg_info = FreeInfo (arg_info);
        arg_info = oldarginfo;

        DBUG_PRINT ("Algebraic-With-Loop-Folding Cube Slicing in %s %s ends",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));
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
 *        Short-circuit further checks of this block if we
 *        have found a partition to slice.
 *
 *
 *****************************************************************************/
node *
CUBSLassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

    DBUG_ASSERT (!INFO_CUTNOW (arg_info), "more cutnow confusion");
    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if ((N_let == NODE_TYPE (ASSIGN_STMT (arg_node)))
        && (N_with == NODE_TYPE (LET_EXPR (ASSIGN_STMT (arg_node))))
        && (NULL != INFO_PREASSIGNSWITH (arg_info))) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWITH (arg_info), arg_node);
        INFO_PREASSIGNSWITH (arg_info) = NULL;
    }

    /*
     * Top-down traversal, unless we have found a partn to slice.
     */
    if (!INFO_CUTNOW (arg_info)) {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

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
    arg_node = RemoveSuperfluousCodes (arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse a partition of a WL.
 *
 * NB. In order for slicing to occur, the WL projections must have
 * been moved out of the WL by LIR. If they have not been moved,
 * then the projections have some sort of dependence on locals
 * within the WL, and a crash will happen down the line, due
 * to value error on those locals.
 *
 *****************************************************************************/
node *
CUBSLpart (node *arg_node, info *arg_info)
{
    node *oldconsumerpart;
    node *oldwlprojection1;
    node *oldwlprojection2;
    intersect_type_t oldintersecttype;
    node *oldnoteintersect;
    node *newparts;
    node *newnode;

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
    oldwlprojection1 = INFO_WLPROJECTION1 (arg_info);
    INFO_WLPROJECTION1 (arg_info) = NULL;
    oldwlprojection2 = INFO_WLPROJECTION2 (arg_info);
    INFO_WLPROJECTION2 (arg_info) = NULL;
    oldnoteintersect = INFO_NOTEINTERSECT (arg_info);
    INFO_NOTEINTERSECT (arg_info) = NULL;

    DBUG_ASSERT (!INFO_CUTNOW (arg_info), "cutnow confusion");

    DBUG_PRINT ("traversing code block for %s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVopt (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    DBUG_PRINT ("back from traversing code block for %s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
    DBUG_PRINT ("CWL partition %s intersect type is %s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                IntersectTypeName (INFO_INTERSECTTYPE (arg_info)));

    // blind slicing.
    if (INFO_CUTNOW (arg_info)) {
        DBUG_ASSERT (1 == CODE_USED (PART_CODE (arg_node)), "CODE_USED confusion");
        newparts = BuildSubcubes (arg_node, arg_info);
        if (NULL != newparts) { // We may not have done any work
            newnode = TCappendPart (newparts, PART_NEXT (arg_node));
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = newnode;
            DBUG_ASSERT (1 == CODE_USED (PART_CODE (arg_node)), "CODE_USED confusion2");
        }
    }

    INFO_CONSUMERPART (arg_info) = oldconsumerpart;
    INFO_INTERSECTTYPE (arg_info) = oldintersecttype;
    INFO_WLPROJECTION1 (arg_info) = oldwlprojection1;
    INFO_WLPROJECTION2 (arg_info) = oldwlprojection2;
    INFO_CUTNOW (arg_info) = FALSE;
    INFO_NOTEINTERSECT (arg_info) = oldnoteintersect;

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
 *        If we find a suitable node, mark things so that
 *        we do not look any further into this N_part, but
 *        instead go off and slice the partition.
 *
 *****************************************************************************/
node *
CUBSLprf (node *arg_node, info *arg_info)
{
    node *pwl = NULL;
    node *pwlid = NULL;
    node *noteint = NULL;

    DBUG_ENTER ();

    if ((F_sel_VxA == PRF_PRF (arg_node)) || (F_idx_sel == PRF_PRF (arg_node))) {
        DBUG_PRINT ("Looking at %s =_sel_VxA_( iv, X)",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
        PRF_ISFOLDNOW (arg_node) = FALSE;
        noteint = AWLFIfindNoteintersect (PRF_ARG1 (arg_node));
        pwlid = AWLFIfindWlId (PRF_ARG2 (arg_node));
        pwl = AWLFIfindWL (pwlid);
    }

    /* If cwl intersect information is stale, due to cubeslicing of
     * pwl, discard it and start over */
    if ((NULL != noteint) && (NULL != pwl)) {
        if (!AWLFIisValidNoteintersect (noteint, pwlid)) {
            noteint = AWLFIdetachNoteintersect (noteint);
            FREEdoFreeNode (PRF_ARG1 (arg_node));
            PRF_ARG1 (arg_node) = noteint;
            DBUG_PRINT ("Discarded invalid F_noteintersect for cwl=%s, pwl=%s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (ID_AVIS (pwlid)));
            pwl = NULL;
        } else {
            /* producerWL may be entirely gone now, perhaps
             * due to it being copyWL, etc.
             * Or, it may have been sliced, so partition bounds differ from
             * what they were originally.
             *
             * Side effect of call is to set INFO_CONSUMERPART and
             * INFO_INTERSECTTYPE.
             */
            INFO_NOTEINTERSECT (arg_info) = noteint;
            INFO_INTERSECTTYPE (arg_info)
              = CUBSLfindMatchingPart (arg_node, INFO_CONSUMERPART (arg_info), pwl,
                                       arg_info, &INFO_PRODUCERPART (arg_info));
            if ((INTERSECT_exact != INFO_INTERSECTTYPE (arg_info)) && (NULL != noteint)
                && (AWLFIisHasAllInverseProjections (noteint))) {
                INFO_CUTNOW (arg_info) = TRUE;
                PRF_ISFOLDNOW (arg_node) = TRUE;
                DBUG_PRINT ("Marked for slicing: %s=sel( iv, %s)",
                            AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
            }
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding cube slicing -->
 *****************************************************************************/

#undef DBUG_PREFIX
