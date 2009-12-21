/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup awlf Algebraic With-Loop Folding
 *
 * @terminology:
 *        ProducerWL: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the producerWL.
 *
 *        ConsumerWL: the WL that will absorb the block(s) from
 *        the producerWL. In the example below, B is the consumerWL.
 *
 * @brief Algebraic With-Loop Folding
 *        This performs WLF on AKD arrays, not foldable by WLF.
 *
 *        The features of AWLF are:
 *
 *            - Ability to fold arrays whose shapes are not
 *              known statically. Specifically, if the index
 *              set of the consumerWL is known to be identical to,
 *              or a subset of, the index set of the producerWL,
 *              folding will occur.
 *
 *           - the producerWL must have an SSAASSIGN.
 *
 *           - the producerWL must have an empty code block (so that the cost
 *             must have of duplicating it is zero), or it must have
 *             a NEEDCOUNT of 1.
 *             There must be no other references to the producerWL.
 *             If CVP and friends have done their job, this should
 *             not be a severe restriction.
 *
 *             This code includes a little trick to convert a
 *             modarray( producerWL) into a genarray( shape( producerWL)).
 *             This happens after any folding is performed.
 *             Hence, there is a kludge to
 *             allow a NEEDCOUNT of 2 if there is a modarray consumerWL
 *             present.
 *
 *           - the producerWL must have a DEPDEPTH value of 1??
 *             I think the idea here is to ensure that we do not
 *             pull a producerWL inside a loop, which could cause
 *             increased computation.
 *
 *           - The consumerWL must refer to the producerWL via
 *              _sel_VxA_(idx, producerWL)
 *             and idx must be the consumerWL's WITHID, or a
 *             linear function thereof.
 *
 *           - The producerWL operator is a genarray or modarray
 *             (NO folds, please).
 *
 *           - The WL is a single-operator WL. (These should have been
 *             eliminated by phase 10, I think.)
 *
 *           - The index set of the consumerWL's partition matches
 *             the generator of the producerWL, or is a subset
 *             of it. Note that this implies that
 *             the WL-bounds are the same length, which
 *             may not be the case. Consider this example:
 *
 *               x = with([0] <= iv < [n]) ... : [1,2,3,4]; NB. x is int[.,.]
 *               z = with([0,0], <= iv < shape(x)) :  x[iv];
 *
 *             The bounds of x are int[1], while the bounds of z are int[2].
 *             It is important that AWLFI not generate a partition
 *             intersection expression such as:
 *
 *              _awlf = _max_VxV_( bound1(x), bound1(z));
 *
 *             because TUP will get very confused about the length
 *             error. As I did...
 *
 * An example for Algebraic With-Loop Folding is given below:
 *
 * <pre>
 *  1  A = with( iv)             NB. Foldee WL
 *  2          ( lb <= iv < ub) {
 *  3        <block_a>
 *  4      } : val;
 *  5      genarray( shp);
 *  6
 *  7  ...
 *  8
 *  9  B = with( jv)               NB. Folder WL
 * 10          ( lb <= jv < ub) {
 * 11        <block_b1>
 * 12        ael = sel( jv, A);
 * 13        <block_b2>
 * 14      } : -
 * 15      genarray( shp);
 * </pre>
 *
 *   is transformed into
 *
 * <pre>
 *  1  A = with( iv)
 *  2          ( lb <= iv < ub) {
 *  3        <block_a>
 *  4      } : val;
 *  5      genarray( shp);
 *  6
 *  7  ...
 *  8
 *  9  B = with( jv)
 * 10          ( lb <= jv < ub) {
 * 11        <block_b1>
 * 12        <block_a>, with references to iv renamed to jv.
 * 13        ael = new id of val;
 * 14        <block_b2>
 * 15      } : -
 * 16      genarray( shp);
 * </pre>
 *
 * Then lines 1-5 are removed by DCR(Dead Code Removal).
 *
 *  TODO:
 *   1. At present, AWLF does not occur unless ALL references
 *      to the producerWL are in the consumerWL, or unless the
 *      producerWL has an empty code block. Here is a possible extension
 *      to allow small computations to be folded into several
 *      consumerWL:
 *
 *      Introduce a cost function into WLNC. The idea here is
 *      to provide a crude measure of the cost of computing
 *      a single WL result element. We start by giving
 *      each primitive a cost:
 *        F_xxx_SxS_:  1
 *        F_xxx_SxV_:  infinity
 *        F_xxx_VxS_:  infinity
 *        F_xxx_VxV_:  infinity
 *        N_ap:        infinity
 *        N_with:      infinity
 *        etc.
 *
 *      The wl_needcount code will sum the cost of the code
 *      in each WL. Hmm. The cost must be associated with the
 *      WL-partition.
 *
 *      If the producerWL is otherwise ripe for folding, we allow
 *      the fold to occur if the cost is less than some threshold,
 *      even if the references to the producerWL occur in several
 *      consumerWL.
 *
 *   2. Permit WLF through reshape operations.
 *      The idea here is that, if the consumerWL and producerWL
 *      operating in wlidx mode, then
 *      the folding can be done, because neither WL cares about
 *      the shape of the producerWL result, just that they have
 *      identical element counts.
 *
 *      A bit more thought here might give a nice way to
 *      extend this to the case where only one WL is operating
 *      in wlidx mode.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file algebraic_wlf.c
 *
 * Prefix: AWLF
 *
 *****************************************************************************/
#include "algebraic_wlf.h"
#include "algebraic_wlfi.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"
#include "dbug.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "free.h"
#include "LookUpTable.h"
#include "globals.h"
#include "wl_cost_check.h"
#include "wl_needcount.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "new_types.h"
#include "phase.h"
#include "check.h"
#include "wls.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *part;
    /* This is the current partition in the consumerWL. */
    node *wl;
    /* This is the current consumerWL. */
    int level;
    /* This is the current nesting level of WLs */
    node *awlfoldableProducerWLpart;
    lut_t *lut;
    /* This is the WITH_ID renaming lut */
    node *vardecs;
    node *preassigns;
    node *intersectb1;
    node *intersectb2;
    node *idxbound1;
    node *idxbound2;
    bool onefundef;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PART(n) ((n)->part)
#define INFO_WL(n) ((n)->wl)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_AWLFOLDABLEPRODUCERWLPART(n) ((n)->awlfoldableProducerWLpart)
#define INFO_LUT(n) ((n)->lut)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_INTERSECTB1(n) ((n)->intersectb1)
#define INFO_INTERSECTB2(n) ((n)->intersectb2)
#define INFO_IDXBOUND1(n) ((n)->idxbound1)
#define INFO_IDXBOUND2(n) ((n)->idxbound2)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_PART (result) = NULL;
    INFO_WL (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_AWLFOLDABLEPRODUCERWLPART (result) = NULL;
    INFO_LUT (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_INTERSECTB1 (result) = NULL;
    INFO_INTERSECTB2 (result) = NULL;
    INFO_IDXBOUND1 (result) = NULL;
    INFO_IDXBOUND2 (result) = NULL;
    INFO_ONEFUNDEF (result) = FALSE;

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

/** <!--********************************************************************-->
 *
 * @fn node *AWLFdoAlgebraicWithLoopFoldingOneFunction( node *fundef)
 *
 * @brief global entry point of Algebraic With-Loop folding
 *
 * @param N_fundef apply AWLF.
 *
 * @return optimized N_fundef
 *
 *****************************************************************************/
node *
AWLFdoAlgebraicWithLoopFoldingOneFunction (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("AWLFdoAlgebraicWithLoopFoldingOneFunction");

    arg_info = MakeInfo (NULL);
    INFO_ONEFUNDEF (arg_info) = TRUE;
    INFO_LUT (arg_info) = LUTgenerateLut ();

    TRAVpush (TR_awlf);
    arg_node = TRAVopt (arg_node, arg_info);
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
 * @fn bool isPrfArg1AttachIntersect( node *arg_node)
 *
 * @brief Predicate to check if arg1 of this N_prf has
 *        an F_attachintersect guard attached to it.
 *
 * @param arg_node: an N_prf
 *
 * @return Boolean TRUE if PRF_ARG2 is an F_attachintersect.
 *
 *****************************************************************************/
bool
isPrfArg1AttachIntersect (node *arg_node)
{
    node *arg1;
    node *assgn;
    bool z = FALSE;

    DBUG_ENTER ("isPrfArg1AttachIntersect");

    arg1 = PRF_ARG1 (arg_node);
    DBUG_ASSERT (N_id == NODE_TYPE (arg1),
                 "isPrfArg1AttachIntersect expected N_id as PRF_ARG1");
    assgn = AVIS_SSAASSIGN (ID_AVIS (arg1));
    if ((NULL != assgn) && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn))))
        && (F_attachintersect == PRF_PRF (LET_EXPR (ASSIGN_INSTR (assgn))))) {
        z = TRUE;
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool isPrfArg1AttachExtrema( node *arg_node)
 *
 * @brief Predicate to check if arg1 of this N_prf is an F_attachextrema op
 *  OR if arg1 is an F_attachintersect, and its PRF_ARG1 is an F_attachextrema.
 *  This is VERY brittle code, and we should find a more robust way
 *  to do this. FIXME.
 *  Like a PM of some sort...
 *
 * @param arg_node: an N_prf
 *
 * @return Boolean TRUE if PRF_ARG2 is an F_attachextrema.
 *
 *****************************************************************************/
bool
isPrfArg1AttachExtrema (node *arg_node)
{
    node *arg1;
    node *assgn;
    node *assgn2;
    node *prf;
    node *prf2;
    bool z = FALSE;

    DBUG_ENTER ("isPrfArg1AttachExtrema");

    arg1 = PRF_ARG1 (arg_node);
    DBUG_ASSERT (N_id == NODE_TYPE (arg1),
                 "isPrfArg1AttachExtrema expected N_id as PRF_ARG1");
    assgn = AVIS_SSAASSIGN (ID_AVIS (arg1));
    if ((NULL != assgn) && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn))))) {

        prf = LET_EXPR (ASSIGN_INSTR (assgn));
        if ((F_attachextrema == PRF_PRF (prf))) {
            z = TRUE;
        } else {
            assgn2 = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (prf)));
            if ((NULL != assgn)
                && (N_prf == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assgn2))))) {
                prf2 = LET_EXPR (ASSIGN_INSTR (assgn2));
                if ((F_attachintersect == PRF_PRF (LET_EXPR (ASSIGN_INSTR (assgn2))))) {
                    z = TRUE;
                }
            }
        }
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn bool matchGeneratorField( node *fa, node *fb, node *producerWL)
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
    constant *fafs = NULL;
    constant *fbfs = NULL;
    constant *fac;
    constant *fbc;
    bool z;

    DBUG_ENTER ("matchGeneratorField");

    z = (fa == fb); /* SAA should do it this way most of the time */

    if ((!z) && (NULL != fa) && (NULL != fb)
        && (PMO (PMOarray (&fafs, &fav, fa)) && (PMO (PMOarray (&fbfs, &fbv, fb))))) {
        z = (fav == fbv);
    }
    fafs = (NULL != fafs) ? COfreeConstant (fafs) : fafs;
    fbfs = (NULL != fbfs) ? COfreeConstant (fbfs) : fbfs;

    /* If one field is local and the other is a function argument,
     * we can have both AKV, but they will not be merged, at
     * last in saacyc today. If that ever gets fixed, we can
     * remove this check. This is a workaround for a performance
     * problem in sac/apex/buildv/buildv.sac
     */
    if ((!z) && (NULL != fa) && (NULL != fb) && TYisAKV (AVIS_TYPE (ID_AVIS (fa)))
        && TYisAKV (AVIS_TYPE (ID_AVIS (fb)))) {
        fac = COaST2Constant (fa);
        fbc = COaST2Constant (fa);
        z = COcompareConstants (fac, fbc);
        fac = COfreeConstant (fac);
        fbc = COfreeConstant (fbc);
    }

    if ((NULL != fa) && (NULL != fb)) {
        if (z) {
            DBUG_PRINT ("AWLF", ("matchGeneratorField %s and %s matched",
                                 AVIS_NAME (ID_AVIS (fa)), AVIS_NAME (ID_AVIS (fb))));
        } else {
            DBUG_PRINT ("AWLF", ("matchGeneratorField %s and %s did not match",
                                 AVIS_NAME (ID_AVIS (fa)), AVIS_NAME (ID_AVIS (fb))));
        }
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node * ExtractNthItem
 *
 * @brief Extract the Nth item of the attach_intersect information
 *
 * @params itemno: the Nth item number.
 * @params idx: the index vector for the _sel_VxA( idx, producerWL).
 *
 * @result: The item itself.
 *
 *****************************************************************************/
static node *
ExtractNthItem (int itemno, node *idx)
{
    node *bnd;
    node *dfg;
    node *val = NULL;

    DBUG_ENTER ("ExtractNthWLIntersection");

    dfg = AVIS_SSAASSIGN (ID_AVIS (idx));
    dfg = LET_EXPR (ASSIGN_INSTR (dfg));
    DBUG_ASSERT ((F_attachintersect == PRF_PRF (dfg)),
                 ("ExtractNthItem wanted F_attachintersect as idx parent"));
    bnd = TCgetNthExprsExpr (itemno, PRF_ARGS (dfg));

    if (!(PMO (PMOlastVarGuards (&val, bnd)))) {
        DBUG_ASSERT (FALSE, ("ExtractNthItem could not find var!"));
    }

    DBUG_RETURN (val);
}

/* expressions per partition are: bound1, bound2, intlo, inthi, intNull */
#define WLBOUND1ORIGINAL(partno) (1 + (5 * partno))
#define WLBOUND2ORIGINAL(partno) (1 + (5 * partno) + 1)
#define WLINTERSECTION(partno, boundno) (1 + 2 + (5 * partno) + boundno)
#define WLINTERSECTIONNULL(partno) (1 + 4 + (5 * partno))

/** <!--********************************************************************-->
 *
 *
 * @fn node * FindMatchingPart(...
 *
 * @brief Search for a producerWL partition that matches the
 *        consumerWLPart index set, or that is a superset thereof.
 *
 *        The search comprises two steps:
 *
 *        1. Find a partition in the producerWL that has bounds
 *           that match those in the F_attachintersect bounds, if
 *           such still exists.
 *           This search is required because other optimizations
 *           may have split a partition, and/or reordered partitions
 *           within the producerWL.
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
 *            b. ConsumerWL is subset of producerWL, or matches exactly.
 *               Folding is trivial, using the intersect data in
 *               the _attach_intersect_.
 *
 *            c. ConsumerWL is superset of producerWL.
 *               Folding is possible, but the ConsumerWL partition
 *               must be split into 2 or three partitions.
 *
 * @params arg_node: the N_prf of the sel(idx, producerWL).
 *         consumerpart: the consumerWL partition containing arg_node.
 *         producerWL: the N_with of the producerWL.
 *
 * @result: The address of the matching prducerWL partition, if any.
 *          NULL if none is found.
 *
 *****************************************************************************/
static node *
FindMatchingPart (node *arg_node, info *arg_info, node *consumerpart, node *producerWL)
{
    node *consumerWLGenerator;
    node *producerWLGenerator;
    node *producerWLPart;
    node *intersectb1;
    node *intersectb2;
    node *idx;
    node *idxbound1;
    node *idxbound2;
    node *idxassign;
    node *idxparent;
    node *producerWLBound1Original;
    node *producerWLBound2Original;
    bool matched = FALSE;
    node *nullIntersect;
    int producerPartno = 0;

    DBUG_ENTER ("FindMatchingPart");
    DBUG_ASSERT (N_prf == NODE_TYPE (arg_node),
                 ("FindMatchingPart expected N_prf arg_node"));
    DBUG_ASSERT (N_with == NODE_TYPE (producerWL),
                 ("FindMatchingPart expected N_with producerWL"));
    DBUG_ASSERT (N_part == NODE_TYPE (consumerpart),
                 ("FindMatchingPart expected N_part consumerpart"));

    idx = PRF_ARG1 (arg_node); /* idx of _sel_VxA_( idx, producerWL) */
    idxassign = AVIS_SSAASSIGN (ID_AVIS (idx));
    DBUG_ASSERT (NULL != idxassign, ("FindMatchingPart found NULL SSAASSIGN"));
    idxparent = LET_EXPR (ASSIGN_INSTR (idxassign));
    DBUG_ASSERT (F_attachintersect == PRF_PRF (idxparent),
                 ("FindMatchingPart expected F_attachintersect as idx parent"));

    consumerWLGenerator = PART_GENERATOR (consumerpart);
    producerWLPart = WITH_PART (producerWL);

    /* Find matching producerWL partition, if any */
    while ((!matched) && producerWLPart != NULL) {
        producerWLGenerator = PART_GENERATOR (producerWLPart);

        idxbound1 = AVIS_MINVAL (ID_AVIS (PRF_ARG1 (idxparent)));
        idxbound2 = AVIS_MAXVAL (ID_AVIS (PRF_ARG1 (idxparent)));
        intersectb1 = ID_AVIS (ExtractNthItem (WLINTERSECTION (producerPartno, 0), idx));
        intersectb2 = ID_AVIS (ExtractNthItem (WLINTERSECTION (producerPartno, 1), idx));

        if ((NULL != idxbound1) && (NULL != intersectb1)) {
            DBUG_PRINT (
              "AWLF",
              ("producerPartno #%d matching producer idxbound1 %s, intersectb1 %s",
               producerPartno, AVIS_NAME (idxbound1), AVIS_NAME (intersectb1)));
        }
        if ((NULL != idxbound2) && (NULL != intersectb2)) {
            DBUG_PRINT (
              "AWLF",
              ("producerPartno #%d matching producer idxbound2 %s, intersectb2 %s",
               producerPartno, AVIS_NAME (idxbound2), AVIS_NAME (intersectb2)));
        }

        producerWLBound1Original
          = ExtractNthItem (WLBOUND1ORIGINAL (producerPartno), idx);
        producerWLBound2Original
          = ExtractNthItem (WLBOUND2ORIGINAL (producerPartno), idx);
        nullIntersect = ExtractNthItem (WLINTERSECTIONNULL (producerPartno), idx);

        matched = ((N_generator == NODE_TYPE (consumerWLGenerator))
                   && (N_generator == NODE_TYPE (producerWLGenerator))
                   && (matchGeneratorField (producerWLBound1Original,
                                            GENERATOR_BOUND1 (consumerWLGenerator)))
                   && (matchGeneratorField (producerWLBound2Original,
                                            GENERATOR_BOUND2 (consumerWLGenerator)))
                   && (matchGeneratorField (GENERATOR_STEP (producerWLGenerator),
                                            GENERATOR_STEP (consumerWLGenerator)))
                   && (matchGeneratorField (GENERATOR_WIDTH (producerWLGenerator),
                                            GENERATOR_WIDTH (consumerWLGenerator))));

        if (matched) {
            DBUG_PRINT ("AWLF", ("All generator fields match"));
            matched =
              /*
              ( !nullIntersect)                                   &&
              */
              (idxbound1 == intersectb1) && (idxbound2 == intersectb2);
        }

        if (!matched) {
            producerWLPart = PART_NEXT (producerWLPart);
            producerPartno++;
        }
    }

    if (matched) {
        DBUG_PRINT ("AWLF", ("FindMatchingPart referents match producerPartno %d",
                             producerPartno));
    } else {
        producerWLPart = NULL;
        DBUG_PRINT ("AWLF", ("FindMatchingPart does not match"));
    }

    DBUG_RETURN (producerWLPart);
}

/** <!--********************************************************************-->
 *
 * @fn bool isEmptyPartitionCodeBlock( node *partn)
 *
 * @brief Predicate for finding N_part node with no code block.
 * @param N_part
 * @result TRUE if code block is empty
 *
 *****************************************************************************/
static bool
isEmptyPartitionCodeBlock (node *partn)
{
    bool z;

    DBUG_ENTER ("isEmptyPartitionCodeBlock");

    z = (N_empty == NODE_TYPE (BLOCK_INSTR (CODE_CBLOCK (PART_CODE (partn)))));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkAWLFoldable( node *arg_node, info *arg_info,
 * node *consumerWLPart, int level)
 *
 * @brief check if _sel_VxA_(idx, prducerWL), appearing in folder WL
 *        partition, part, is foldable into the folder WL.
 *        Most checks have already been made by AWLFI.
 *        Here, we check that the generators match,
 *        and that the only references to the producerWL result are
 *        in the consumerWL.
 *
 * @param _sel_VxA_( idx, prducerWL)
 * @param consumerWLPart: The partition into which we would like to
 *        fold this sel().
 * @param level
 * @result If the producerWL is foldable into the consumerWL, return the
 *         N_part of the producerWL that should be used for the fold; else NULL.
 *
 *****************************************************************************/
static node *
checkAWLFoldable (node *arg_node, info *arg_info, node *consumerWLPart, int level)
{
    node *producerWLid;
    node *foldeeavis;
    node *foldeeassign;
    node *producerWL;
    node *foldeepart = NULL;

    DBUG_ENTER ("checkAWLFoldable");

    producerWLid = PRF_ARG2 (arg_node);
    foldeeavis = ID_AVIS (producerWLid);
    if ((NULL != AVIS_SSAASSIGN (foldeeavis))
        && (AVIS_DEFDEPTH (foldeeavis) + 1 == level)) {
        foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (producerWLid)));
        if (NODE_TYPE (LET_EXPR (foldeeassign)) == N_with) {
            producerWL = LET_EXPR (foldeeassign);
            DBUG_PRINT (
              "AWLF",
              ("consumerWL %s: producerWL AVIS_NEEDCOUNT=%d, AVIS_WL_NEEDCOUNT=%d",
               AVIS_NAME (foldeeavis), AVIS_NEEDCOUNT (foldeeavis),
               AVIS_WL_NEEDCOUNT (foldeeavis)));
            foldeepart
              = FindMatchingPart (arg_node, arg_info, consumerWLPart, producerWL);
            /* Allow fold if needcounts match OR if foldeepartition
             * has empty code block. This is a crude cost function:
             * We should allow "cheap" prducerWL partitions to fold.
             * E.g., toi(iota(N)), but I'm in a hurry...
             */
            if ((NULL != foldeepart)
                && ((AVIS_NEEDCOUNT (foldeeavis) != AVIS_WL_NEEDCOUNT (foldeeavis)))
                && (!isEmptyPartitionCodeBlock (foldeepart))) {
                foldeepart = NULL;
            }
        }
    } else {
        DBUG_PRINT ("AWLF", ("WL %s will never fold. AVIS_DEFDEPTH: %d, lavel: %d",
                             AVIS_NAME (foldeeavis), AVIS_DEFDEPTH (foldeeavis), level));
    }

    if (NULL != foldeepart) {
        AVIS_ISWLFOLDED (foldeeavis) = TRUE;
        DBUG_PRINT ("AWLF", ("WL %s will be folded.", AVIS_NAME (foldeeavis)));
    } else {
        DBUG_PRINT ("AWLF", ("WLs %s will not be folded.", AVIS_NAME (foldeeavis)));
    }

    DBUG_RETURN (foldeepart);
}

/** <!--********************************************************************-->
 *
 * @fn static node *populateLut( node *arg_node, info *arg_info, shape *shp)
 *
 * @brief Generate a clone name for a WITHID.
 *        Populate one element of a look up table with
 *        said name and its original, which we will use
 *        to do renames in the copied WL code block.
 *        See caller for description. Basically,
 *        we have a producerWL with generator of this form:
 *
 *    producerWL = with {
 *         ( . <= iv=[i,j] <= .) : _sel_VxA_( iv, AAA);
 *
 *        We want to perform renames in the producerWL code block as follows:
 *
 *        iv --> iv'
 *        i  --> i'
 *        j  --> j'
 *
 * @param: arg_node: one N_avis node of the producerWL generator (e.g., iv),
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
populateLut (node *arg_node, info *arg_info, shape *shp)
{
    node *navis;

    DBUG_ENTER ("populateLut");

    /* Generate a new LHS name for WITHID_VEC/IDS */
    navis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (arg_node)),
                        TYmakeAKS (TYcopyType (TYgetScalar (AVIS_TYPE (arg_node))), shp));

    if (isSAAMode ()) {
        AVIS_DIM (navis) = DUPdoDupTree (AVIS_DIM (arg_node));
        AVIS_SHAPE (navis) = DUPdoDupTree (AVIS_SHAPE (arg_node));
    }
    INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));
    LUTinsertIntoLutP (INFO_LUT (arg_info), arg_node, navis);

    DBUG_PRINT ("AWLF", ("Inserted WITHID_VEC into lut: oldname: %s, newname %s",
                         AVIS_NAME (arg_node), AVIS_NAME (navis)));

    DBUG_RETURN (navis);
}

/** <!--********************************************************************-->
 *
 * @ fn static node *makeIdxAssigns( node *arg_node, node *foldeePart)
 *
 * @brief for a prducerWL partition, with generator:
 *        (. <= iv=[i,j] < .)
 *        and a folder _sel_VxA_( idx, producerWL),
 *
 *        generate an N_assigns chain of this form:
 *
 *        iv = idx;
 *        k0 = [0];
 *        i  = _sel_VxA_( k0, idx);
 *        k1 = [1];
 *        j  = _sel_VxA_( k1, idx);
 *
 *        Then, iv, i, j will all be SSA-renamed by the caller.
 *
 * @result: an N_assign chain as above.
 *
 *****************************************************************************/
static node *
makeIdxAssigns (node *arg_node, info *arg_info, node *foldeePart)
{
    node *z = NULL;
    node *ids;
    node *narray;
    node *idxid;
    node *navis;
    node *nass;
    node *lhsids;
    node *lhsavis;
    node *sel;
    int k;

    DBUG_ENTER ("makeIdxAssigns");

    ids = WITHID_IDS (PART_WITHID (foldeePart));
    idxid = PRF_ARG1 (LET_EXPR (ASSIGN_INSTR (arg_node)));
    k = 0;

    while (NULL != ids) {
        /* Build k0 = [k]; */
        /* First, the k */
        narray = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
        navis = TBmakeAvis (TRAVtmpVar (),
                            TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, 1)));
        if (isSAAMode ()) {
            AVIS_DIM (navis) = TBmakeNum (1);
            AVIS_SHAPE (navis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (1), NULL));
        }

        nass = TBmakeAssign (TBmakeLet (TBmakeIds (navis, NULL), narray), NULL);
        AVIS_SSAASSIGN (navis) = nass;
        z = TCappendAssign (nass, z);
        INFO_VARDECS (arg_info) = TBmakeVardec (navis, INFO_VARDECS (arg_info));

        lhsavis = populateLut (IDS_AVIS (ids), arg_info, SHcreateShape (0));
        DBUG_PRINT ("AWLF", ("makeIdxAssigns created %s = _sel_VxA_(%d, %s)",
                             AVIS_NAME (lhsavis), k, AVIS_NAME (ID_AVIS (idxid))));

        sel = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL),
                                       TCmakePrf2 (F_sel_VxA, TBmakeId (navis),
                                                   DUPdoDupNode (idxid))),
                            NULL);
        z = TCappendAssign (z, sel);
        AVIS_SSAASSIGN (lhsavis) = sel;

        if (isSAAMode ()) {
            AVIS_DIM (lhsavis) = TBmakeNum (0);
            AVIS_SHAPE (lhsavis) = TCmakeIntVector (NULL);
        }

        ids = IDS_NEXT (ids);
        k++;
    }

    /* Now generate iv = idx; */
    lhsids = WITHID_VEC (PART_WITHID (foldeePart));
    lhsavis = populateLut (IDS_AVIS (lhsids), arg_info, SHcreateShape (1, k));
    z = TBmakeAssign (TBmakeLet (TBmakeIds (lhsavis, NULL), DUPdoDupNode (idxid)), z);
    AVIS_SSAASSIGN (lhsavis) = z;
    DBUG_PRINT ("AWLF", ("makeIdxAssigns created %s = %s)", AVIS_NAME (lhsavis),
                         AVIS_NAME (ID_AVIS (idxid))));

    if (isSAAMode ()) {
        AVIS_DIM (lhsavis) = TBmakeNum (1);
        AVIS_SHAPE (lhsavis) = TCmakeIntVector (TBmakeExprs (TBmakeNum (k), NULL));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *doAWLFreplace( ... )
 *
 * @brief
 *   In
 *    producerWL = with...  elb = _sel_VxA_(iv=[i,j], AAA) ...
 *    consumerWL = with...  elc = _sel_VxA_(idx, producerWL) ...
 *
 *   Replace, in the consumerWL:
 *     elc = _sel_VxA_( idx, producerWL)
 *   by
 *     iv = idx;
 *     i = _sel_VxA_([0], idx);
 *     j = _sel_VxA_([1], idx);
 *
 *     {code block from producerWL, with SSA renames}
 *
 *     tmp = producerWL)
 *     elc = tmp;
 *
 * @params
 *    arg_node: N_assign for the sel()
 *    fundef: N_fundef node, so we can insert new avis node for
 *            temp assign being made here.
 *    producerWLPart: N_part node of producerWL.
 *
 *    folder: N_part node of folder.
 *****************************************************************************/
static node *
doAWLFreplace (node *arg_node, node *fundef, node *producerWLPart, node *consumerWLPart,
               info *arg_info)
{
    node *oldblock;
    node *newblock;
    node *newavis;
    node *idxassigns;
    node *expravis;

    DBUG_ENTER ("doAWLFreplace");

    oldblock = BLOCK_INSTR (CODE_CBLOCK (PART_CODE (producerWLPart)));

    /* Generate iv=[i,j] assigns, then do renames. */
    idxassigns = makeIdxAssigns (arg_node, arg_info, producerWLPart);

    /* If producerWL is empty, don't do any code substitutions.
     * Just replace sel(iv, producerWL) by iv.
     */
    newblock
      = (N_empty == NODE_TYPE (oldblock))
          ? NULL
          : DUPdoDupTreeLutSsa (oldblock, INFO_LUT (arg_info), INFO_FUNDEF (arg_info));

    expravis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (PART_CODE (producerWLPart))));
    newavis = LUTsearchInLutPp (INFO_LUT (arg_info), expravis);

    LUTremoveContentLut (INFO_LUT (arg_info));

    /**
     * replace the code
     */
    FREEdoFreeNode (LET_EXPR (ASSIGN_INSTR (arg_node)));
    LET_EXPR (ASSIGN_INSTR (arg_node)) = TBmakeId (newavis);
    if (NULL != newblock) {
        arg_node = TCappendAssign (newblock, arg_node);
    }

    arg_node = TCappendAssign (idxassigns, arg_node);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *AWLFfundef(node *arg_node, info *arg_info)
 *
 * @brief applies AWLF to a given fundef.
 *
 *****************************************************************************/
node *
AWLFfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("AWLFfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("AWLF", ("Algebraic With-Loop folding in %s %s begins",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));

        INFO_FUNDEF (arg_info) = arg_node;

        arg_node = WLNCdoWLNeedCount (arg_node);
        arg_node = WLCCdoWLCostCheck (arg_node);

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /* If new vardecs were made, append them to the current set */
        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDEC (FUNDEF_BODY (arg_node))
              = TCappendVardec (INFO_VARDECS (arg_info),
                                BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
            INFO_VARDECS (arg_info) = NULL;
        }

        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        INFO_ONEFUNDEF (arg_info) = old_onefundef;

        DBUG_PRINT ("AWLF", ("Algebraic With-Loop folding in %s %s ends",
                             (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                             FUNDEF_NAME (arg_node)));
    }
    INFO_FUNDEF (arg_info) = NULL;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node AWLFassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, producerWL).
 *
 *****************************************************************************/
node *
AWLFassign (node *arg_node, info *arg_info)
{
    node *foldableProducerPart;

    DBUG_ENTER ("AWLFassign");

#ifdef VERBOSE
    DBUG_PRINT ("AWLF", ("Traversing N_assign"));
#endif // VERBOSE
    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    foldableProducerPart = INFO_AWLFOLDABLEPRODUCERWLPART (arg_info);
    INFO_AWLFOLDABLEPRODUCERWLPART (arg_info) = NULL;
    DBUG_ASSERT ((NULL == INFO_PREASSIGNS (arg_info)),
                 "AWLFassign INFO_PREASSIGNS not NULL");

    /*
     * Top-down traversal
     */
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    /*
     * Append the new cloned block
     */
    if (NULL != foldableProducerPart) {
        arg_node = doAWLFreplace (arg_node, INFO_FUNDEF (arg_info), foldableProducerPart,
                                  INFO_PART (arg_info), arg_info);

        global.optcounters.awlf_expr += 1;
    }

    if (NULL != INFO_PREASSIGNS (arg_info)) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFwith( node *arg_node, info *arg_info)
 *
 * @brief applies AWLF to a with-loop in a top-down manner.
 *
 *****************************************************************************/
node *
AWLFwith (node *arg_node, info *arg_info)
{
    node *nextop;
    node *genop;
    node *folderop;
    node *foldeeshape;
    info *old_info;

    DBUG_ENTER ("AWLFwith");

    DBUG_PRINT ("AWLF", ("Examining N_with"));
    old_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_WL (arg_info) = arg_node;
    INFO_LUT (arg_info) = INFO_LUT (old_info);
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_info);
    INFO_PREASSIGNS (arg_info) = INFO_PREASSIGNS (old_info);

    WITH_REFERENCED_FOLDERWL (arg_node) = NULL;
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    /* Try to replace modarray(producerWL) by genarray(shape(producerWL)).
     * This has no effect on the producerWL itself, but is needed to
     * eliminate the reference to producerWL, so it can be removed by DCR.
     *
     * If the producerWL has been folded, its result will have
     * AVIS_FOLDED set. Since there are, by the definition of
     * folding, no other references to the producerWL, we can
     * blindly replace the modarray by the genarray.
     */
    folderop = WITH_WITHOP (arg_node);
    if ((N_modarray == NODE_TYPE (folderop))
        && (NULL != AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (folderop))))
        && (TRUE == AVIS_ISWLFOLDED (ID_AVIS (MODARRAY_ARRAY (folderop))))) {
        foldeeshape = AVIS_SHAPE (ID_AVIS (MODARRAY_ARRAY (folderop)));
        genop = TBmakeGenarray (DUPdoDupTree (foldeeshape), NULL);
        GENARRAY_NEXT (genop) = MODARRAY_NEXT (folderop);
        nextop = FREEdoFreeNode (folderop);
        WITH_WITHOP (arg_node) = genop;
        DBUG_PRINT ("AWLF", ("Replacing modarray by genarray"));
    }

    INFO_WL (old_info) = NULL;
    INFO_VARDECS (old_info) = INFO_VARDECS (arg_info);
    INFO_PREASSIGNS (old_info) = INFO_PREASSIGNS (arg_info);
    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
AWLFcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFcode");

    DBUG_PRINT ("AWLF", ("Traversing N_code"));
    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
AWLFpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFpart");

    DBUG_PRINT ("AWLF", ("Traversing N_part"));
    INFO_PART (arg_info) = arg_node;
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_PART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop level as ids defDepth attribute
 *
 *****************************************************************************/
node *
AWLFids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFids");

#ifdef VERBOSE
    DBUG_PRINT ("AWLF", ("Traversing N_ids"));
#endif // VERBOSE
    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all _sel_VxA_( idx, producerWL)  primitives to see if
 *   the _sel_VxA_ is contained inside a WL, and that idx has
 *   intersect information attached to it.
 *   If so, producerWL may be a candidate for folding into this WL.
 *
 *   If idx does not have an SSAASSIGN, it means idx is a WITHID.
 *   If so, we will visit here again, after extrema have been
 *   attached to idx, and idx renamed.
 *
 *****************************************************************************/
node *
AWLFprf (node *arg_node, info *arg_info)
{
    node *arg1;

    DBUG_ENTER ("AWLFprf");

#ifdef VERBOSE
    DBUG_PRINT ("AWLF", ("Traversing N_prf"));
#endif // VERBOSE
    arg1 = PRF_ARG1 (arg_node);
    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (isPrfArg1AttachIntersect (arg_node))) {
        INFO_AWLFOLDABLEPRODUCERWLPART (arg_info)
          = checkAWLFoldable (arg_node, arg_info, INFO_PART (arg_info),
                              INFO_LEVEL (arg_info));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *AWLFcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
AWLFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFcond");

    DBUG_PRINT ("AWLF", ("Traversing N_cond"));
    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFfuncond( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFfuncond");

    DBUG_PRINT ("AWLF", ("Traversing N_funcond"));
    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFwhile");

    DBUG_PRINT ("AWLF", ("Traversing N_while"));
    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding -->
 *****************************************************************************/
