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
#include "inferneedcounters.h"
#include "wl_cost_check.h"
#include "set_withloop_depth.h"
#include "wl_needcount.h"
#include "deadcoderemoval.h"
#include "indexvectorutils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;
    node *lhs;
    node *consumerpart;
    node *wlprojection1;            /* lower bound of WL proj */
    node *wlprojection2;            /* upper bound of WL proj */
    node *withcode;                 /* WITH_CODE from N_with */
    intersect_type_t intersecttype; /* intersect type. see enum */
    lut_t *lut;                     /* LUT for renaming */
    lut_t *foldlut;                 /* LUT for renames during fold */
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
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_CONSUMERPART(n) ((n)->consumerpart)
#define INFO_WLPROJECTION1(n) ((n)->wlprojection1)
#define INFO_WLPROJECTION2(n) ((n)->wlprojection2)
#define INFO_WITHCODE(n) ((n)->withcode)
#define INFO_INTERSECTTYPE(n) ((n)->intersecttype)
#define INFO_LUT(n) ((n)->lut)
#define INFO_FOLDLUT(n) ((n)->foldlut)
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
    INFO_PREASSIGNS (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_CONSUMERPART (result) = NULL;
    INFO_WLPROJECTION1 (result) = NULL;
    INFO_WLPROJECTION2 (result) = NULL;
    INFO_WITHCODE (result) = NULL;
    INFO_INTERSECTTYPE (result) = INTERSECT_unknown;
    INFO_LUT (result) = NULL;
    INFO_FOLDLUT (result) = NULL;
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
    info *arg_info;

    DBUG_ENTER ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "called with non-fundef node");

    arg_info = MakeInfo (arg_node);
    INFO_FOLDLUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_cubsl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    INFO_FOLDLUT (arg_info) = LUTremoveLut (INFO_FOLDLUT (arg_info));
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
 * @param   arg_node: An F_noteintersect null-intersection entry.
 *
 * @result  IsNullIntersect: TRUE if any element of the
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
 * @param  arg_node: An F_noteintersect WLINTERSECT1PART entry.
 * @param  cwlb1, cwlb2: Consumer-WL partition bounds
 * @param  cwlproj1, cwlproj2: Consumer-WL inverse projection bounds
 *
 * @result  isInt1Part: TRUE if ALL elements of the
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
#if 0
    bool z2 = FALSE; // DEBUG FIXME
#endif
    constant *con;

    DBUG_ENTER ();

    con = COaST2Constant (arg_node);
    if (NULL != con) {
        z = COisTrue (con, TRUE);
        COfreeConstant (con);
    }

#if 0
    // FIXME
    z2 = (TULSisValuesMatch (cwlb1, cwlproj1) && TULSisValuesMatch (cwlb2, cwlproj2));
#endif
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
 * @param noteint: An F_noteintersect N_prf node
 *        intersectListNo: which PWL partition number we are looking at
 *        arg_info: your basic arg_info
 *
 * @result:
 *
 *****************************************************************************/
static void
SetWLProjections (node *noteint, size_t intersectListNo, info *arg_info)
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
 * @fn static GetNthPart( size_t partno, node *npart)
 *
 * @brief Get npart[partno] for npart.
 *
 * @param  partno: non_negative size_t partition index
 * @param  npart: A WITH_PART node.
 *
 * @result: The partno'th N_part of npart.
 *
 *****************************************************************************/
static node *
GetNthPart (size_t partno, node *npart)
{
    node *z;

    DBUG_ENTER ();

    z = npart;
    while (partno != 0) {
        z = PART_NEXT (z);
        DBUG_ASSERT (z != 0, "partn[partno] index errro");
        partno--;
    }

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
 * @param  idx: N_id node of _noteintersect() list.
 * @param  producerWLGenerator: PART_GENERATOR of producerWL
 * @param  cwlp: consumer WL partition, or NULL if naked consumer
 * @param  arg_info: Your basic arg_info
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
    size_t intersectListNo;
    size_t intersectListLim;
    node *cwlpb1;
    node *cwlpb2;
    bool cwlpstepok;
    bool cwlpwidthok;
    node *intersect1Part;
    bool int1part;
    bool notint1part;
    bool intnull;
    bool notintnull;
#ifdef BROKEN
    int noteintinsertcycle;
#endif

    DBUG_ENTER ();

    noteint = AWLFIfindNoteintersect (idx);
#ifdef BROKEN
    noteintinsertcycle = PRF_NOTEINTERSECTINSERTIONCYCLE (noteint);
#endif
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
                     || (SCSisConstantOne (GENERATOR_STEP (producerWLGenerator)));
        cwlpwidthok = (NULL == GENERATOR_WIDTH (producerWLGenerator))
                      || (SCSisConstantOne (GENERATOR_WIDTH (producerWLGenerator)));
    }

    intersectListNo = 0;
    while (((INTERSECT_unknown == z) || (INTERSECT_null == z))
           && (intersectListNo < intersectListLim)) {

        DBUG_PRINT ("Started check of partition #%zu", intersectListNo);

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

#ifdef BROKEN // This definitely breaks a few things:
            /* Slicing and exact intersect criteria */
            // After four kicks at the AWLFI can, give up and slice.
            if (((global.cycle_counter - noteintinsertcycle) > 3)
                && (AWLFIisHasInverseProjection (proj1))
                && (AWLFIisHasInverseProjection (proj2))) {
                DBUG_PRINT ("Blind slicing cube at cycle %d", global.cycle_counter);
                // sac2c codingtimeformul.sac -v1  -doawlf -nowlf -noctz
                // gives wrong answers.
                // Also, takeAKSunknownAKDVector.sac goes to cube-slicing
                // heaven, never quite getting the intersect right
                // for AWLF.

                z = INTERSECT_sliceneeded;
                SetWLProjections (noteint, intersectListNo, arg_info);
            }
#endif /* BROKEN */

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

            if (int1part && (NULL == cwlpb1) && global.optimize.doscwlf) {
                DBUG_PRINT ("Naked consumer detected");
                z = INTERSECT_exact;
                SetWLProjections (noteint, intersectListNo, arg_info);
            }
        } else {
            DBUG_PRINT ("Generator field(s) changed underfoot");
            z = INTERSECT_null;
        }
        DBUG_PRINT ("Finished check of partition #%zu", intersectListNo);
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
 * @param:
 *
 * @result: A pointer to a character string constant.
 *
 *****************************************************************************/
static char *
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
 * @param  arg_node: the N_prf of the sel(idx, pwl).
 * @param  cwlp: the consumerWL partition containing arg_node,
 *           or NULL, if we have a naked consumer.
 * @param  pwl: the N_with of the pwl.
 * @param  arg_info: Either arg_info or NULL.
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
    int producerPartno = 0;
#ifndef DBUG_OFF
    int intPartno = -1;
    char *nm;
#endif

    DBUG_ENTER ();
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node), "expected N_prf arg_node");
    DBUG_ASSERT (N_with == NODE_TYPE (pwl), "expected N_with pwl");

    idx = PRF_ARG1 (arg_node); /* idx of _sel_VxA_( idx, pwl) */
    // idxassign = AVIS_SSAASSIGN (ID_AVIS (idx));
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
#ifndef DBUG_OFF
            intPartno = producerPartno;
#endif
        }
        producerWLPart = PART_NEXT (producerWLPart);
        producerPartno++;
    }

#ifndef DBUG_OFF
    if (NULL != arg_info) {
        nm = AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)));
    } else {
        nm = "?";
    }
#endif

    DBUG_PRINT ("match type is (%s) for intPartno %d of PWL=%s, CWL=%s",
                IntersectTypeName (z), intPartno,
                AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))), nm);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *populateFoldLut( node *arg_node, info *arg_info, shape *shp)
 *
 * @brief Generate a clone name for a WITHID.
 *        Populate one element of a look up table with
 *        said name and its original, which we will use
 *        to do renames in the copied PWL code block.
 *        See caller for description. Basically,
 *        we have a PWL with generator of this form:
 *
 *    PWL = with {
 *         ( . <= iv=[i,j] <= .) : _sel_VxA_( iv, AAA);
 *
 *        We want to perform renames in the PWL code block as follows:
 *
 *        iv --> iv'
 *        i  --> i'
 *        j  --> j'
 *
 * @param: arg_node: one N_avis node of the PWL generator (e.g., iv),
 *                   to serve as iv for above assigns.
 *         arg_info: your basic arg_info.
 *         shp:      the shape descriptor of the new LHS.
 *
 * @result: New N_avis node, e.g, iv'.
 *          Side effect: mapping iv -> iv' entry is now in LUT.
 *                       New vardec for iv'.
 *
 *****************************************************************************/
static node *
populateFoldLut (node *arg_node, info *arg_info, shape *shp)
{
    node *navis;

    DBUG_ENTER ();

    /* Generate a new LHS name for WITHID_VEC/IDS */
    navis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (arg_node))), shp));

    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_FOLDLUT (arg_info), arg_node, navis);

    DBUG_PRINT ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                AVIS_NAME (arg_node), AVIS_NAME (navis));

    DBUG_RETURN (navis);
}

/** <!--********************************************************************-->
 *
 * @ fn static node *makeIdxAssigns( node *arg_node, node *pwlpart)
 *
 * @brief This function does setup for renaming PWL index vector elements
 *        to correspond to their CWL brethren.
 *
 *        For a PWL partition, with generator:
 *
 *        (. <= iv=[i,j] < .)
 *
 *        and a consumer _sel_VxA_( idx, PWL),
 *
 *        we generate an N_assign chain of this form:
 *
 *        iv = idx;
 *        k0 = [0];
 *        i  = _sel_VxA_( k0, idx);
 *        k1 = [1];
 *        j  = _sel_VxA_( k1, idx);
 *
 *        Also, iv, i, j, k are placed into the LUT.
 *
 * @param  arg_node: An N_assign node.
 *
 * @result: an N_assign chain as above.
 *
 *****************************************************************************/
static node *
makeIdxAssigns (node *arg_node, info *arg_info, node *pwlpart)
{
    node *z = NULL;
    node *ids;
    node *narray;
    node *idxavis;
    node *navis;
    node *nass;
    node *lhsids;
    node *lhsavis;
    node *sel;
    node *args;
    int k;

    DBUG_ENTER ();

    ids = WITHID_IDS (PART_WITHID (pwlpart));
    args = LET_EXPR (ASSIGN_STMT (arg_node));
    idxavis = IVUToffset2Vect (args, &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info), NULL, pwlpart);
    DBUG_ASSERT (NULL != idxavis, "Could not rebuild iv for _sel_VxA_(iv, PWL)");

    k = 0;

    while (NULL != ids) {
        /* Build k0 = [k]; */
        /* First, the k */
        narray = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
        navis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));

        nass = TBmakeAssign (TBmakeLet (TBmakeIds (navis, NULL), narray), NULL);
        AVIS_SSAASSIGN (navis) = nass;
        z = TCappendAssign (nass, z);
        INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));

        lhsavis = populateFoldLut (IDS_AVIS (ids), arg_info, SHcreateShape (0));
        DBUG_PRINT ("created %s = _sel_VxA_(%d, %s)", AVIS_NAME (lhsavis), k,
                    AVIS_NAME (idxavis));

        sel = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                       TCmakePrf2 (F_sel_VxA, TBmakeId (navis),
                                                   TBmakeId (idxavis))),
                            NULL);
        z = TCappendAssign (z, sel);
        AVIS_SSAASSIGN (lhsavis) = sel;
        ids = IDS_NEXT (ids);
        k++;
    }

    /* Now generate iv = idx; */
    lhsids = WITHID_VEC (PART_WITHID (pwlpart));
    lhsavis = populateFoldLut (IDS_AVIS (lhsids), arg_info, SHcreateShape (1, k));
    z = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), TBmakeId (idxavis)), z);
    AVIS_SSAASSIGN (lhsavis) = z;
    DBUG_PRINT ("makeIdxAssigns created %s = %s)", AVIS_NAME (lhsavis),
                AVIS_NAME (idxavis));
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BypassNoteintersect( node *arg_node)
 *
 * @brief Bypass the  F_noteintersect that might precede the
 *        sel( iv, PWL).
 *
 *        "might", because we may have SimpleComposition happening.
 *
 * @param N_assign for _idx_sel() or _sel_VxA_()
 * @result Rebuilt arg_node.
 *
 *****************************************************************************/
static node *
BypassNoteintersect (node *arg_node)
{
    node *z = NULL;
    pattern *pat;
    node *expr;
    node *exprni;

    DBUG_ENTER ();

    expr = LET_EXPR (ASSIGN_STMT (arg_node));

    exprni = AWLFIfindNoteintersect (PRF_ARG1 (expr));
    if (NULL != exprni) {
        DBUG_PRINT ("Insertion cycle was %d, folding cycle is %d",
                    PRF_NOTEINTERSECTINSERTIONCYCLE (exprni), global.cycle_counter);
    }

    pat = PMprf (1, PMAisPrf (F_noteintersect), 2, PMvar (1, PMAgetNode (&z), 0),
                 PMskip (0));
    if (PMmatchFlat (pat, PRF_ARG1 (expr))) {
        PRF_ARG1 (expr) = FREEdoFreeNode (PRF_ARG1 (expr));
        PRF_ARG1 (expr) = DUPdoDupNode (z);
    }

    pat = PMfree (pat);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn static node *FindMarkedSelAssignParent( node *assgn)
 *
 * @param  assgn: The N_assign chain for the current N_part/N_code block
 *
 * @brief: Find the N_assign node that has the marked sel() primitive
 *         as its RHS, and return its predecessor. We do this
 *         because we are going to replace the sel() N_assign.
 *         If N_assign nodes were doubly linked, we wouldn't need this
 *         function.
 *
 * @result: The relevant N_assign node.
 *          If not found, crash. If more than one found in the block,
 *          crash.
 *
 *****************************************************************************/
static node *
FindMarkedSelAssignParent (node *assgn)
{
    node *z = NULL;
    node *prevassgn = NULL;

    DBUG_ENTER ();

    while (NULL != assgn) {
        if ((N_let == NODE_TYPE (ASSIGN_STMT (assgn)))
            && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_STMT (assgn))))
            && ((F_sel_VxA == PRF_PRF (LET_EXPR (ASSIGN_STMT (assgn))))
                || ((F_idx_sel == PRF_PRF (LET_EXPR (ASSIGN_STMT (assgn))))))
            && (PRF_ISFOLDNOW (LET_EXPR (ASSIGN_STMT (assgn))))) {

            DBUG_ASSERT (NULL == z, "More than one marked sel() found in N_part");
            z = prevassgn;
        }
        prevassgn = assgn;
        assgn = ASSIGN_NEXT (assgn);
    }

    DBUG_ASSERT (NULL != z, "No marked sel() found in N_part");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *performFold( ... )
 *
 * @brief
 *   In
 *    pwl = with...  elb = _sel_VxA_(iv=[i,j], AAA) ...
 *    consumerWL = with...  elc = _sel_VxA_(idx, pwl) ...
 *
 *   Replace, in the consumerWL:
 *     elc = _sel_VxA_( idx, pwl)
 *   by
 *     iv = idx;
 *     i = _sel_VxA_([0], idx);
 *     j = _sel_VxA_([1], idx);
 *
 *     {code block from pwl, with SSA renames}
 *
 *     tmp = PWL)
 *     elc = tmp;
 *
 * @param cwlpart: cwl N_part
 * @param partno: pwl partition number that will be folded
 * @param PWL: N_part node of pwl.
 *
 * @result: new cwlpart with pwl partno folded into it.
 *
 *****************************************************************************/
static node *
performFold (node *cwlpart, size_t partno, info *arg_info)
{
    node *pwlblock;
    node *newpblock;
    node *newsel;
    node *idxassigns;
    node *cellexpr;
    node *assgn;
    node *passgn;
    node *pwl;
    node *pwlpart;
    node *z = NULL;
    node *cwlass;

    DBUG_ENTER ();

    DBUG_PRINT ("Replacing code block in CWL=%s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    cwlass = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (cwlpart)));
    passgn = FindMarkedSelAssignParent (cwlass);
    assgn = ASSIGN_NEXT (passgn);
    PRF_ISFOLDNOW (LET_EXPR (ASSIGN_STMT (assgn))) = FALSE;
    pwl = AWLFIfindWlId (PRF_ARG2 (LET_EXPR (ASSIGN_STMT (assgn))));
    if (NULL != pwl) {
        pwl = AWLFIfindWL (pwl); /* Now the N_with */
    }

    pwlpart = GetNthPart (partno, WITH_PART (pwl));

    /* Generate iv=[i,j] assigns, then do renames. */
    idxassigns = makeIdxAssigns (BypassNoteintersect (assgn), arg_info, pwlpart);
    cellexpr = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (pwlpart))));

    pwlpart = IVEXCdoIndexVectorExtremaCleanupPartition (pwlpart, arg_info);
    pwlblock = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (pwlpart)));

    if (NULL != pwlblock) {
        /* Remove extrema from old block and recompute everything */
        newpblock = DUPdoDupTreeLutSsa (pwlblock, INFO_FOLDLUT (arg_info),
                                        INFO_FUNDEF (arg_info));
    } else {
        /* If PWL code block is empty, don't duplicate code block.
         * If the cell is non-scalar, we still need a _sel_() to pick
         * the proper cell element.
         */
        newpblock = NULL;
    }

    cellexpr = (node *)LUTsearchInLutPp (INFO_FOLDLUT (arg_info), cellexpr);
    newsel = TBmakeId (cellexpr);

    LUTremoveContentLut (INFO_FOLDLUT (arg_info));

    // Rebuild the new cwlpart's code block:
    // We started with:  assignA -> sel -> assgnC
    // and end up with:  assignA -> idxassigns -> newpblock -> assgnC
    ASSIGN_NEXT (passgn) = NULL; // assignA
    z = TCappendAssign (cwlass, idxassigns);
    z = TCappendAssign (z, newpblock);

    FREEdoFreeNode (LET_EXPR (ASSIGN_STMT (assgn))); // Kill the old sel()
    LET_EXPR (ASSIGN_STMT (assgn)) = newsel;         // Now folded wl result
    z = TCappendAssign (z, assgn);
    BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (cwlpart))) = z;

    global.optcounters.awlf_expr += 1;

    DBUG_RETURN (cwlpart);
}

/** <!--********************************************************************-->
 *
 * @fn node *CloneCode( arg_node, arg_info)
 *
 * @brief Clone a WL code block, renaming any LHS definitions.
 *
 * @param arg_node: an N_code
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
    z = DUPdoDupNodeLutSsa (arg_node, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));
    CODE_INC_USED (z); /* DUP gives us Used=0 */

    z = IVEXCdoIndexVectorExtremaCleanupPartition (z, NULL);

    /* append new code block to N_code chain */
    INFO_WITHCODE (arg_info) = TCappendCode (INFO_WITHCODE (arg_info), z);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildSubcube(...)
 *
 * @param arg_node: an N_part of the consumerWL.
 * @param arg_info: your basic info node
 * @param lb: the generator lower bound for the new partition
 * @param ub: the generator upper bound for the new partition
 * @param step: the generator step for the new partition
 * @param width: the generator width for the new partition
 * @param withid: the WITHID for the new partition
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
    // FIXME: need to check current bounds against new bounds!
    // and maybe is1Part and/or isNull

    genn = TBmakeGenerator (F_wl_le, F_wl_lt, DUPdoDupNode (lb), DUPdoDupNode (ub),
                            DUPdoDupNode (step), DUPdoDupNode (width));
    clone = CloneCode (PART_CODE (arg_node), arg_info);
    newpart = TBmakePart (clone, DUPdoDupNode (withid), genn);

    DBUG_RETURN (newpart);
}

/** <!--********************************************************************-->
 *
 * @fn static node *BuildSubcubes(...)
 *
 * @param arg_node: an N_part of the consumerWL.
 * @param arg_info: your basic info node
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
    node *newcwlpart;
    size_t partno;
    size_t partlim;
    node *lb;
    node *ub;
    node *noteintersect;
    node *step;
    node *width;
    node *withid;
    pattern *patlb;
    pattern *patub;

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

    if (partlim == 1) { // Do not slice simple compositions
        newpartns = performFold (DUPdoDupNode (arg_node), partno, arg_info);
    } else {
        DBUG_PRINT ("Slicing partition %s into %zu pieces",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))), partlim);
        while (partno < partlim) {
            PMmatchFlat (patlb, TCgetNthExprsExpr (WLPROJECTION1 (partno),
                                                   PRF_ARGS (noteintersect)));
            PMmatchFlat (patub, TCgetNthExprsExpr (WLPROJECTION2 (partno),
                                                   PRF_ARGS (noteintersect)));
            newcwlpart = BuildSubcube (arg_node, arg_info, lb, ub, step, width, withid);
            newcwlpart = performFold (newcwlpart, partno, arg_info);
            newpartns = TCappendPart (newpartns, newcwlpart);
            partno++;
        }
    }
    global.optcounters.cubsl_expr++;

    patlb = PMfree (patlb);
    patub = PMfree (patub);

    DBUG_RETURN (newpartns);
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
        INFO_FOLDLUT (arg_info) = INFO_FOLDLUT (oldarginfo);

        /* cubeslicer (or others) may leave around old WL. That will confuse
         * WLNC, which is driven by vardecs, NOT by function body traversal.
         */
        arg_node = DCRdoDeadCodeRemoval (arg_node);
        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_awlfi);
        arg_node = WLNCdoWLNeedCount (arg_node);

        arg_node = WLCCdoWLCostCheck (arg_node);
        arg_node = SWLDdoSetWithloopDepth (arg_node);

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
        && (NULL != INFO_PREASSIGNS (arg_info))) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
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
    WITH_CODE (arg_node) = INFO_WITHCODE (arg_info);
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
            PART_NEXT (arg_node) = NULL;
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
    DBUG_PRINT ("Start looking at N_let %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_PRINT ("Finished looking at N_let %s",
                AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));

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
    int noteintinsertcycle;

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
            noteintinsertcycle = PRF_NOTEINTERSECTINSERTIONCYCLE (noteint);
            INFO_NOTEINTERSECT (arg_info) = noteint;
            INFO_INTERSECTTYPE (arg_info)
              = CUBSLfindMatchingPart (arg_node, INFO_CONSUMERPART (arg_info), pwl,
                                       arg_info, &INFO_PRODUCERPART (arg_info));
            if ((INTERSECT_exact != INFO_INTERSECTTYPE (arg_info)) && (NULL != noteint)
                && ((global.cycle_counter - noteintinsertcycle) > 3)
                && (AWLFIisHasAllInverseProjections (noteint))) {
                DBUG_ASSERT (!INFO_CUTNOW (arg_info), "CUTNOW error");
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
