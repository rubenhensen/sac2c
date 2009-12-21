/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cubes Algebraic With-Loop Folding Cube Slicer
 *
 * @terminology:
 *
 * @brief
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
#include "dbug.h"
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
    bool onefundef; /* fundef-based traversal */
    node *lhs;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

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

    DBUG_ENTER ("CUBSLdoAlgebraicWithLoopFoldingCubeSlicing");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 ("CUBSLdoAlgebraicWithLoopFoldingCubeSlicing called for non-fundef"));

    arg_info = MakeInfo (arg_node);
    INFO_ONEFUNDEF (arg_info) = TRUE;

    TRAVpush (TR_cubsl);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

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

/** <!--********************************************************************-->
 *
 * @fn static
 * void MarkPartitionSliceNeeded( node *consumerpart,
 *                                node *intersectb1, node *intersectb2,
 *                                node *idxbound1, idxbound2,
 *                                info *arg_info);
 *
 * @brief Possibly mark current consumerWL partition as requiring slicing.
 *
 *        The partition requires slicing if:
 *          - intersects and bounds are N_array nodes.
 *          - the intersect is not NULL.
 *          - the intersects and bounds do not match.
 *            [Since we only get called when this is the case,
 *             they are guaranteed not to match.]
 *
 * @params: consumerpart: consumerWL partition that may need slicing
 *          intersectb1: lower bound of intersection between consumerWL
 *                       sel operation index set and producerWL partition
 *                       bounds.
 *          intersectb2: upper bound of same.
 *          idxbound1:   lower bound of consumerWL sel operation index set.
 *          idxboundi2:  lower bound of same.
 *          arg_info:    your basic arg_info node.
 *
 * @result: None.
 *
 * @note: It may happen that more than one partition in the consumerWL
 *        requires slicing.
 *
 *****************************************************************************/
static void
MarkPartitionSliceNeeded (node *consumerpart, node *intersectb1, node *intersectb2,
                          node *idxbound1, node *idxbound2, info *arg_info)
{
    DBUG_ENTER ("MarkPartitionSliceNeeded");
    if ((N_array == NODE_TYPE (intersectb1)) && (N_array == NODE_TYPE (intersectb2))
        && (N_array == NODE_TYPE (idxbound1)) && (N_array == NODE_TYPE (idxbound2))) {

        DBUG_PRINT ("AWLF", ("FIXME: check for NULL intersection"));
#ifdef CRUD
        INFO_INTERSECTB1 (arg_info) = intersectb1;
        INFO_INTERSECTB2 (arg_info) = intersectb2;
        INFO_IDXBOUND1 (arg_info) = idxbound1;
        INFO_IDXBOUND2 (arg_info) = idxbound2;
#endif // CRUD
    }

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn static node *AppendPart(node *partz, node *newpart)
 *
 * @params: partz: an N_part, the current chain of new partitions
 *          newpart: an N_part, a new partition to be added to the chain.
 *
 * @brief: append newpart to partz.

 *****************************************************************************/
static node *
AppendPart (node *partz, node *newpart)
{
    DBUG_ENTER ("AppendPart");

    if (NULL == partz) {
        partz = newpart;
    } else {
        PART_NEXT (partz) = newpart;
    }

    DBUG_RETURN (partz);
}

/** <!--********************************************************************-->
 *
 * @fn static node *PartitionSlicer(...)
 *
 * @params partn: an N_part of the consumerWL.
 *         idx: an N_array, representing the intersect of the
 *              consumerWL index set and a producerWL partition.
 *         idx must be the same shape as the partn generators.
 *         d: the axis which we are going to slice. e.g., for matrix,
 *            d = 0 --> slice rows
 *            d = 1 --> slice columns
 *            etc.
 *
 * @result: 1-3 N_part nodes, depending on the value of idx.
 *
 * @brief Slice a WL partition into 1-3 partitions.
 *
 * We have a WL partition, partn, and an intersection index set, idx,
 * for the partition that is smaller than the partition. We wish
 * to slice partn into sub-partitions, in order that AWLF
 * can operate on the sub-partition(s).
 *
 * idx is known to lie totally within partn, as it arises from
 * the WL index set intersection bounds.
 *
 * In the simplest situation, there are three possible
 * cases of intersect. The rectangle represents partn;
 * the xxxx's represent the array covered by idx.
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
 *  For cases alpha and gamma, we split partn into two parts.
 *  For case beta, we split it into three parts. In each case,
 *  one of the partitions is guaranteed to match idx.
 *
 * Because the intersection is multi-dimensional, we perform
 * the splitting on one axis at a time. Hence, we may end up
 * with each axis generating 1-3 new partitions for each
 * partition it gets as input.
 *
 * Here are the cases for splitting along axis 1 (columns) in a rank-2 array;
 * an x denotes partI:
 *
 * alpha   alpha  alpha
 *
 * x..     .x.    ..x
 * ...     ...    ...
 * ...     ...    ...
 *
 *
 * beta    beta   beta
 * ...     ...    ...
 * x..     .x.    ..x
 * ...     ...    ...
 *
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
 * Note: Re the question of when to perform partition slicing.
 *       I'm not sure, but let's start here:
 *       We want to avoid a situation in which we slice
 *       a partition before we know that slicing is required.
 *       These are the requirements:
 *
 *        - The consumerWL index set is an N_array.
 *        - The intersect of the consumerWL index set with
 *          the producerWL partition bounds is:
 *            . non-empty  (or we are looking at a total mismatch)
 *            . not an exact match (because it could fold as is).
 *
 *****************************************************************************/
static node *
PartitionSlicer (node *partn, node *lb, node *ub, int d, info *arg_info)
{
    node *partz = NULL;
    node *newpart = NULL;
    node *lbpart;
    node *ubpart;
    node *step;
    node *width;
    node *withid;
    node *newlb;
    node *newub;
    node *genn;
    node *coden;
    node *ilb;
    node *iub;
    node *plb;
    node *pub;

    DBUG_ENTER ("PartitionSlicer");

    DBUG_ASSERT (N_part == NODE_TYPE (partn), "Partition Slicer expected N_part partn");
    DBUG_ASSERT (N_array == NODE_TYPE (lb), "Partition Slicer expected N_array lb");
    DBUG_ASSERT (N_array == NODE_TYPE (ub), "Partition Slicer expected N_array ub");
    lbpart = GENERATOR_BOUND1 (PART_GENERATOR (partn));
    ubpart = GENERATOR_BOUND2 (PART_GENERATOR (partn));
    step = GENERATOR_STEP (PART_GENERATOR (partn));
    width = GENERATOR_WIDTH (PART_GENERATOR (partn));
    coden = PART_CODE (partn);
    withid = PART_WITHID (partn);

    ilb = TCgetNthExprs (d, lb);
    iub = TCgetNthExprs (d, ub);
    plb = TCgetNthExprs (d, lbpart);
    pub = TCgetNthExprs (d, ubpart);

    /* Cases beta, gamma need partA */
    if (ilb != plb) { /*           this compare may have to be fancier */
        newlb = DUPdoDupTree (lbpart);
        newlb = WLSflattenBound (newlb, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        newub = DUPdoDupTree (ubpart);
        /* newub[d] = iub */
        EXPRS_EXPR (newub) = FREEdoFreeTree (EXPRS_EXPR (newub));
        EXPRS_EXPR (newub) = DUPdoDupTree (EXPRS_EXPR (iub));
        newub = WLSflattenBound (newub, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupTree (step),
                                DUPdoDupTree (width));
        newpart = TBmakePart (coden, DUPdoDupTree (withid), genn);
        CODE_INC_USED (coden);
        partz = AppendPart (partz, newpart);
    }

    /* All cases need partI */
    newlb = DUPdoDupTree (lb);
    newlb
      = WLSflattenBound (newlb, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info));
    newub = DUPdoDupTree (ub);
    newub
      = WLSflattenBound (newub, &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info));

    genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupTree (step),
                            DUPdoDupTree (width));
    newpart = TBmakePart (coden, DUPdoDupTree (withid), genn);
    CODE_INC_USED (coden);
    partz = AppendPart (partz, newpart);

    /* Case alpha, beta need partC */
    if (iub != pub) { /* this compare may have to be fancier */
        newlb = DUPdoDupTree (lb);
        newlb = WLSflattenBound (newlb, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        newub = DUPdoDupTree (ubpart);
        newub = WLSflattenBound (newub, &INFO_VARDECS (arg_info),
                                 &INFO_PREASSIGNS (arg_info));

        genn = TBmakeGenerator (F_wl_le, F_wl_lt, newlb, newub, DUPdoDupTree (step),
                                DUPdoDupTree (width));
        newpart = TBmakePart (coden, DUPdoDupTree (withid), genn);
        CODE_INC_USED (coden);
        partz = AppendPart (partz, newpart);
    }
    DBUG_RETURN (partz);
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

    DBUG_ENTER ("CUBSLfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("CUBSL", ("Algebraic-With-Loop-Folding Cube Slicing in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

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

        DBUG_PRINT ("CUBSL", ("Algebraic-With-Loop-Folding Cube Slicing in %s %s ends",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));
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
 *****************************************************************************/
node *
CUBSLassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CUBSLassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
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
 * @brief applies CUBSL to a with-loop in a top-down manner.
 *
 *****************************************************************************/
node *
CUBSLwith (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("CUBSLwith");

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CUBSLcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUBSLcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
CUBSLpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUBSLpart");

    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ("CUBSLlet");

    INFO_LHS (arg_info) = LET_IDS (arg_node);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUBSLprf( node *arg_node, info *arg_info)
 *
 * @brief Look for suitable _sel_VxA_( iv, X) nodes in this WL partition.
 *
 *****************************************************************************/
node *
CUBSLprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CUBSLprf");

    if (F_sel_VxA == PRF_PRF (arg_node) && isPrfArg1AttachIntersect (arg_node)) {
        DBUG_PRINT ("CUBSL", (" %s =_sel-VxA_( iv, X) has attachintersect",
                              AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)))));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding cube slicing -->
 *****************************************************************************/
