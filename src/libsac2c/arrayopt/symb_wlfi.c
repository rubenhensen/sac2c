/*
 * $Id: symb_wlf.c 16030 2009-04-09 19:40:34Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup swlf Extended (Symbolic) With-Loop Folding Inference
 *
 * @terminology:
 *        FoldeeWL, or foldee: The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the foldeeWL.
 *
 *        FolderWL: the WL that will absorb the block(s) from
 *        the foldeeWL. In the example below, B is the folderWL.
 *
 *        Intersection expression: the expression that
 *        specifies the set of index vectors in one WL partition
 *        that are present in a partition of another WL.
 *
 * @brief Extended With-Loop Folding
 *        EWLF performs WLF on some arrays that are not foldable
 *        by WLF. Features of extended WLF are described in symb_wlf.c
 *
 *        This phase begins by computing AVIS_NEEDCOUNT for
 *        all N_id nodes. It then makes a depth-first traversal
 *        of the function, computing WL_REFERENCED_FOLD(foldeeWL)++
 *        for all sel() operations in WLs that are of the form:
 *
 *             _sel_VxA_(iv, foldeeWL)
 *
 *        where foldeeWL is the result of another WL.
 *        Only one folderWL is allowed to contribute to the
 *        total. The intent here is to prevent foldeeWL
 *        computations of the same result element
 *        from being folded into several WLs, thereby
 *        increasing the total amount of computation required.
 *        EWLF will permit the folding to occur if, among
 *        other requirements:
 *
 *         AVIS_NEEDCOUNT(foldeeWL) == WL_REFERENCED_FOLD(foldeeWL)
 *
 *        This phase inserts _dataflowguard "intersection expressions"
 *        before certain _sel_(iv, X) statements. Specifically, iv must
 *        be a function of the WITH_ID(folderWL),
 *        and X must be the result of a foldeeWL.
 *
 *        An intersection expression provides the information required
 *        to cut a folderWL partition so that its index set
 *        is entirely contained with the index set of
 *        one partition of the foldeeWL.
 *
 *        Once the cutting has been performed, folding of the
 *        foldeeWL into the folderWL can be performed directly,
 *        by replacing the above _sel_(iv, X) expression with
 *        the code from the foldeeWL partition, and performing
 *        appropriate renames of WITH_IDs.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file symb_wlfi.c
 *
 * Prefix: SWLFI
 *
 *****************************************************************************/
#include "symb_wlfi.h"
#include "symb_wlf.h"

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
#include "globals.h"
#include "wl_cost_check.h"
#include "wl_needcount.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"

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
    node *part;             /* The current folderWL partition */
    node *folderwl;         /* The current folderWL N_with */
    int level;              /* The current nesting level of WLs. This
                             * is used to ensure that an index expression
                             * refers to an earlier WL in the same code
                             * block, rather than to a WL within this
                             * WL. I think...
                             */
    bool swlfoldablefoldee; /* foldeeWL may be legally foldable. */
                            /* (If index sets prove to be OK)     */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_PART(n) ((n)->part)
#define INFO_FOLDERWL(n) ((n)->folderwl)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_SWLFOLDABLEFOLDEE(n) ((n)->swlfoldablefoldee)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PART (result) = NULL;
    INFO_FOLDERWL (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_SWLFOLDABLEFOLDEE (result) = FALSE;

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
 *   node *SWLFIdoSymbolicWithLoopFoldingModule(node *module)
 *
 * description:
 *   Applies symbolic WL folding to a module.
 *
 *****************************************************************************/
node *
SWLFIdoSymbolicWithLoopFoldingModule (node *arg_node)
{

    DBUG_ENTER ("SWLFIdoSymbolicWithLoopFoldingModule");

    TRAVpush (TR_swlfi);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIdoSymbolicWithLoopFolding( node *fundef)
 *
 * @brief global entry point of symbolic With-Loop folding
 *
 * @param fundef Fundef-Node to apply SWLF.
 *
 * @return optimized fundef
 *
 *****************************************************************************/
node *
SWLFIdoSymbolicWithLoopFolding (node *arg_node)
{
    DBUG_ENTER ("SWLFIdoSymbolicWithLoopFolding");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 ("SWLFIdoSymbolicWithLoopFolding called for non-fundef node"));

    TRAVpush (TR_swlfi);
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

/** <!--********************************************************************-->
 *
 * @fn node *flattenExpression(node *arg_node, info *arg_info, node *bounder)
 *
 *   @brief  Flattens the expression at arg_node.
 *           E.g., if the expression is:
 *
 *            _max_VxV_(a, b);
 *
 *          it will look like this on the way out:
 *           TYPETHINGY  TMP;
 *            ...
 *
 *            TMP = _max_VxV_(a, b);
 *            TMP
 *
 *
 *   @param  node *arg_node: a node to be flattened.
 *           info *arg_info:
 *           node *bounder: an N_id with the same type as TMP.
 *
 *   @return node *node:      N_id node for flattened node
 ******************************************************************************/
static node *
flattenExpression (node *arg_node, info *arg_info, node *bounder)
{
    node *res;
    node *avis;
    node *nas;
    node *prf;
    node *id;

    DBUG_ENTER ("flattenExpression");
    res = arg_node;
    switch (NODE_TYPE (arg_node)) {

    case N_exprs:
        prf = EXPRS_EXPR (arg_node);
        DBUG_ASSERT (N_prf == NODE_TYPE (prf),
                     "flattenExpression expected N_prf in expression");
        avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (ID_AVIS (bounder))));
        INFO_VARDECS (arg_info) = TBmakeVardec (avis, INFO_VARDECS (arg_info));
        nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), DUPdoDupTree (prf)), NULL);
        INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), nas);
        AVIS_SSAASSIGN (avis) = nas;
        AVIS_DIM (avis) = DUPdoDupTree (AVIS_DIM (ID_AVIS (bounder)));
        AVIS_SHAPE (avis) = DUPdoDupTree (AVIS_SHAPE (ID_AVIS (bounder)));
        id = TBmakeId (avis);
        DBUG_PRINT ("SWLFI",
                    ("flattenExpression generated assign for %s", AVIS_NAME (avis)));
        FREEdoFreeTree (arg_node);
        res = id;

    case N_id:
        break;

    default:
        DBUG_PRINT ("SWLFI", ("flattenExpression missed a case."));
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilderOne( node *arg_node, info *arg_info,
 *                                      node *foldeepart, int boundnum)
 *
 * @brief Build a pair of expressions for intersecting the bounds of
 *        a single foldeeWL partition with folderWL index set, of the form:
 *
 *           iv' = (k*iv) + ivoffset;
 *           sel( iv', foldeeWL)
 *
 *        The expression for simplification by the compiler.
 *        We determine the intersection of the folderWL iv'
 *        index set with the foldeeWL's  partition bounds this way:
 *
 *          int1 = _max_VxV_( AVIS_MINVAL( iv'),
 *                            GENERATOR_BOUND1(foldee));
 *
 *          int2 = _min_VxV_( AVIS_MAXVAL( iv'),
 *                            GENERATOR_BOUND2(foldee));
 *          iv'' = _dataflowguard(iv', int1, int2);
 *          sel( iv'', foldeeWL)
 *
 *        where int1 evaluates to the lower bound of the intersection,
 *        and   int2 evaluates to the upper bound of the intersection.
 *
 * @params arg_node: the _sel_VxA_( idx, foldeeWL)
 * @params foldeepart: An N_part of the foldeeWL.
 * @params arg_info.
 * @params boundnum: 1 for bound1, 2 for bound2
 * @return An N_exprs containing the two intersect expressions.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilderOne (node *arg_node, info *arg_info, node *foldeepart, int boundnum)
{
    node *boundee;
    node *bounder;
    node *folderpart;
    node *expn;
    node *idxavis;
    node *idxassign;
    node *bid;

    DBUG_ENTER ("IntersectBoundsBuilderOne");

    folderpart = INFO_PART (arg_info);

    /* The indexing expression used in the sel(iv, foldee). */
    idxavis = ID_AVIS (PRF_ARG1 (arg_node));
    idxassign = AVIS_SSAASSIGN (idxavis);

    /* AVIS_MINVAL and AVIS_MAXVAL are guaranteed to exist if we get here */
    bounder = (boundnum == 1) ? AVIS_MINVAL (idxavis) : AVIS_MAXVAL (idxavis);
    boundee = (boundnum == 1) ? GENERATOR_BOUND1 (PART_GENERATOR (foldeepart))
                              : GENERATOR_BOUND2 (PART_GENERATOR (foldeepart));

    DBUG_ASSERT (N_id == NODE_TYPE (boundee),
                 "IntersectBoundsBuilderOne expected N_id WL-generator boundee");

    /* Now, replace the folderWL iv by the appropriate bound.
     * If we started with
     *
     *   _sel_VxA_((k*iv) + offset, foldee)
     *
     * We will end up with:
     *
     *   (k*folderbound1) + offset
     * and
     *   (k*folderbound2) + offset
     */
    bid = TBmakeId (bounder);
    expn = TBmakeExprs (TCmakePrf2 ((boundnum == 1) ? F_max_VxV : F_min_VxV, bid,
                                    DUPdoDupTree (boundee)),
                        NULL);
    expn = flattenExpression (expn, arg_info, bid);

    DBUG_RETURN (expn);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilder( node *arg_node, info *arg_info,
 *                                   node *foldeepart, int boundnum)
 *
 * @brief Build a set of expressions for intersecting the bounds of
 *        all foldeeWL partitions with folderWL index set, of the form:
 *
 *           sel((k*iv) + ivoffset, foldee)
 *
 *        We traverse all foldeeWL partitions, building a set of
 *        intersect calcuations for each of them.
 *
 * @params arg_node: The _sel_VxA( idx, foldeeWL).
 * @params foldeeid: The N_id created by the foldeeWL.
 * @params arg_info.
 * @params boundnum: 1 for bound1, 2 for bound2
 * @return An N_exprs node containing the (two*#foldee partitions) intersect expressions.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilder (node *arg_node, info *arg_info, node *foldeeid, int boundnum)
{
    node *expn = NULL;
    node *partn;
    node *foldeeassign;
    node *foldeewl;
    node *curid;

    DBUG_ENTER ("IntersectBoundsBuilder");

    foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (foldeeid)));
    foldeewl = LET_EXPR (foldeeassign);
    partn = WITH_PART (foldeewl);

    while (NULL != partn) {
        curid = IntersectBoundsBuilderOne (arg_node, arg_info, partn, boundnum);
        expn = TCappendExprs (expn, TBmakeExprs (curid, NULL));
        partn = PART_NEXT (partn);
    }

    DBUG_RETURN (expn);
}

/** <!--********************************************************************-->
 *
 * @fn node *createNewIV( node *arg_node, info *arg_info)
 *
 * @brief Create new iv' from iv.
 *        Also, create a guard expression:
 *         iv' = _dataflowguard_( iv, TRUE);
 *        Our caller will replace the selection:
 *         z = _sel_(iv, foldeeWL);
 *        by
 *         z = _sel_(iv', foldeeWL);
 *
 *        This function also appends a vardec to the INFO chain,
 *        and an assign to the preassigns chain.
 *
 * @params arg_node: the N_prf node of the sel() operation.
 * @return A pointer to the newly created N_id node for iv'.
 *
 *****************************************************************************/

static node *
createNewIV (node *arg_node, info *arg_info)
{
    node *ivavis;
    node *ivassign;
    int ivshape;
    node *ivid;
    node *ividprime;
    node *lbicalc;
    node *ubicalc;
    node *foldeewl;
    node *args;

    DBUG_ENTER ("createNewIV");
    DBUG_PRINT ("SWLFI", ("Inserting dataflow guard"));

    /* Generate expressions for lower-bound intersection and
     * upper-bound intersection calculation.
     */

    if (!isPrfArg1DataFlowGuard (arg_node)) {
        ivid = PRF_ARG1 (arg_node);
        foldeewl = PRF_ARG2 (arg_node);
        lbicalc = IntersectBoundsBuilder (arg_node, arg_info, foldeewl, 1);
        ubicalc = IntersectBoundsBuilder (arg_node, arg_info, foldeewl, 2);
        args = TCappendExprs (TBmakeExprs (DUPdoDupTree (ivid), NULL), lbicalc);
        args = TCappendExprs (args, ubicalc);

        ivshape = SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (ivid))));
        ivavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (ivid))),
                             TYcopyType (AVIS_TYPE (ID_AVIS (ivid))));

        INFO_VARDECS (arg_info) = TBmakeVardec (ivavis, INFO_VARDECS (arg_info));
        ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivavis, NULL),
                                            TBmakePrf (F_dataflowguard, args)),
                                 NULL);
        INFO_PREASSIGNS (arg_info)
          = TCappendAssign (INFO_PREASSIGNS (arg_info), ivassign);
        ividprime = TBmakeId (ivavis);
        AVIS_SSAASSIGN (ivavis) = ivassign;
    }

    DBUG_RETURN (ividprime);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkFoldeeFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(idx, foldee), contained
 *        within a folderWL. We want to determine if
 *        foldee is a WL that is a possible candidate for having some
 *        partition of itself folded into the folderWL that
 *        contains the _sel_ expression.
 *
 *        This function concerns itself only with the characteristics
 *        of the entire foldeeWL: partition-dependent characteristics
 *        are determined by the SWLF phase later on.
 *
 *        The requirements for folding are:
 *
 *           - foldeeWL is the result of a WL.
 *
 *           - foldeeWL operator is a genarray or modarray.
 *
 *           - foldeeWL is a single-operator WL.
 *
 *           - foldeeWL has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *
 *           - foldeeWL is referenced only by the folderWL.
 *
 *           - foldeeWL has a DEFDEPTH value (which I don't understand yet).
 *
 *           - folderWL and foldeeWL generator bounds are
 *             the same shape.
 *
 *        There is an added requirement, that the index set of
 *        the folderWL partition match, or be a subset, of the
 *        foldeeWL partition index set.
 *
 *        The expressions hanging from the dataflowguard inserted
 *        by createNewIV are intended to determine if this
 *        requirement is met. CF, CVP, and other optimizations
 *        should simplify those expressions and give SWLF
 *        the information it needs to determine if the index
 *        set requirements are met.
 *
 *
 * @param _sel_VxA_( idx, foldeeWL)
 * @result If some partition of the foldeeWL may be a legal
 *         candidate for folding into the folderWL, return true.
 *         Else false.
 *
 *****************************************************************************/
static bool
checkFoldeeFoldable (node *arg_node, info *arg_info)
{
    node *foldeeavis;
    node *foldeewlid;
    node *foldeewl;
    node *foldeeassign;
    node *rhs;
    bool z = FALSE;

    DBUG_ENTER ("checkFoldeeFoldable");

    foldeewlid = PRF_ARG2 (arg_node);
    foldeeavis = ID_AVIS (foldeewlid);
    rhs = AVIS_SSAASSIGN (foldeeavis);
    if ((NULL != rhs) && (N_with == NODE_TYPE (ASSIGN_RHS (rhs)))) {
        foldeewl = ASSIGN_RHS (rhs);
        foldeeassign = ASSIGN_INSTR (rhs);

        DBUG_PRINT ("SWLFI", ("FoldeeWL:%s: WITH_REFERENCED_FOLD=%d",
                              AVIS_NAME (foldeeavis), WITH_REFERENCED_FOLD (foldeewl)));

        if ((NODE_TYPE (foldeeassign) == N_let)
            && (NODE_TYPE (LET_EXPR (foldeeassign)) == N_with)
            && (WITHOP_NEXT (WITH_WITHOP (LET_EXPR (foldeeassign))) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (LET_EXPR (foldeeassign))) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (LET_EXPR (foldeeassign))) == N_modarray))) {
            z = TRUE;
        }
    }

    if (z) {
        DBUG_PRINT ("SWLFI",
                    ("Foldee WL %s is suitable for folding.", AVIS_NAME (foldeeavis)));
    } else {
        DBUG_PRINT ("SWLFI", ("Foldee WL %s is not suitable for folding.",
                              AVIS_NAME (foldeeavis)));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkFolderFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(idx, foldee), contained
 *        within a folderWL. We want to determine if
 *        the folderWL is acceptable to have something folded into it.
 *
 *        We require that idx have a non-NULL AVIS_MINVAL & AVIS_MAXVAL.
 *        This may not be enough to guarantee foldability, but
 *        without them, we're stuck.
 *
 *        These WITHID matches are foldable:
 *
 *         foldeeWL WITHID_VEC == folderWL index set extrema
 *
 * @param _sel_VxA_( idx, foldeeWL) arg_node.
 * @result True if the folderWL (and the indexing expression)
 *         are acceptable for having another WL folded into it,
 *         else false.
 *
 *****************************************************************************/
static bool
checkFolderFoldable (node *arg_node, info *arg_info)
{
    node *idx = NULL;
    node *idxavis;
    bool z = FALSE;

    DBUG_ENTER ("checkFolderFoldable");

    idx = PRF_ARG1 (arg_node);
    idxavis = ID_AVIS (idx);
    z = (NULL != AVIS_MINVAL (idxavis)) && (NULL != AVIS_MAXVAL (idxavis));
    if (z) {
        DBUG_PRINT ("SWLFI", ("FolderWL %s extrema found: WL may be foldable.",
                              AVIS_NAME (idxavis)));
    } else {
        DBUG_PRINT ("SWLFI", ("FolderWL %s extrema not found: WL is not foldable.",
                              AVIS_NAME (idxavis)));
    }
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkBothFoldable( node *arg_node, info *arg_info)
 *
 * @brief Make any early checks we can that may show that
 *        the foldeeWL and folderWL are not foldable.
 *
 *        At present, we only check that the generator bounds
 *        of both WLs are of the same shape.
 *
 *        We presume that earlier phases have ensured that
 *        the BOUND1 and BOUND2 lengths are the same for each partition.
 *
 * @param _sel_VxA_( idx, foldeeWL) arg_node.
 * @result True if the folderWL and foldeeWL
 *         have no problems being folded (yet).
 *
 *****************************************************************************/
static bool
checkBothFoldable (node *arg_node, info *arg_info)
{
    node *folderwl;
    node *foldeewl;
    node *b1;
    node *b2;
    bool z;

    DBUG_ENTER ("checkBothFoldable");

    foldeewl = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))));
    folderwl = INFO_FOLDERWL (arg_info);

    b1 = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (folderwl)));
    b2 = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (foldeewl)));
    z = SHcompareShapes (TYgetShape (AVIS_TYPE (ID_AVIS (b1))),
                         TYgetShape (AVIS_TYPE (ID_AVIS (b2))));
    if (z) {
        DBUG_PRINT ("SWLFI", ("FolderWL & FoldeeWL %s generator shapes match.",
                              AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
    } else {
        DBUG_PRINT ("SWLFI", ("FolderWL & FoldeeWL %s generator shapes do not match.",
                              AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
    }
    DBUG_RETURN (z);
}
/** <!--********************************************************************-->
 *
 * @fn node *SWLFIfundef(node *arg_node, info *arg_info)
 *
 * @brief applies SWLFI to a given fundef.
 *
 *****************************************************************************/
node *
SWLFIfundef (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SWLFIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("SWLFI", ("Symbolic-With-Loop-Folding Inference in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TRUE);
        arg_info = MakeInfo (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            INFO_VARDECS (arg_info) = BLOCK_VARDEC (FUNDEF_BODY (arg_node));

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            BLOCK_VARDEC (FUNDEF_BODY (arg_node)) = INFO_VARDECS (arg_info);
            INFO_VARDECS (arg_info) = NULL;
        }

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("SWLFI", ("Symbolic-With-Loop-Folding Inference in %s %s ends",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    }
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node SWLFIassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, foldee).
 *
 *****************************************************************************/
node *
SWLFIassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("SWLFIassign");

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
 * @fn node *SWLFIwith( node *arg_node, info *arg_info)
 *
 * @brief applies SWLFI to a with-loop in a top-down manner.
 *
 *        When we return here, we will have counted all the
 *        references to any potential foldeeWL that we
 *        may want to deal with. We already know the foldeeWL
 *        reference count, because that's computed at entry to
 *        this phase. Hence, we are then in a position to
 *        determine if the fold will be legal.
 *
 *****************************************************************************/
node *
SWLFIwith (node *arg_node, info *arg_info)
{
    info *old_arg_info;

    DBUG_ENTER ("SWLFIwith");

    old_arg_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_arg_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_arg_info);
    INFO_FOLDERWL (arg_info) = arg_node;

    DBUG_PRINT ("SWLFI", ("Resetting WITH_REFERENCED_FOLDERWL, etc."));
    WITH_REFERENCED_FOLD (arg_node) = 0;
    WITH_REFERENCED_FOLDERWL (arg_node) = NULL;
    WITH_REFERENCES_FOLDED (arg_node) = 0;

    WITH_CODE (arg_node) = TRAVopt (WITH_CODE (arg_node), arg_info);
    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    INFO_VARDECS (old_arg_info) = INFO_VARDECS (arg_info);

    arg_info = FreeInfo (arg_info);
    arg_info = old_arg_info;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
SWLFIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
SWLFIpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIpart");

    INFO_PART (arg_info) = arg_node;
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_PART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop level as ids defDepth attribute
 *
 *****************************************************************************/
node *
SWLFIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIids");

    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *  (cloned from SSAWLI)
 *
 * function:
 *   node *SWLFIid(node *arg_node, info *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_with) we want to increment
 *   the number of references to the potential foldeeWL (WITH_REFERENCED_FOLD).
 *   The WL node has to be found via avis_ssaassign backlink.
 *
 *   We want to end up with WITH_REFERENCED_FOLD counting only
 *   references from a single WL. Hence, the checking on
 *   WITH_REFERENCED_FOLDERWL
 *   SWLF will disallow folding if WITH_REFERENCED_FOLD != AVIS_NEEDCOUNT.
 *
 ******************************************************************************/
node *
SWLFIid (node *arg_node, info *arg_info)
{
    node *assignn;
    node *foldeewl;

    DBUG_ENTER ("SWLFIid");
    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
#ifdef NOISY
    DBUG_PRINT ("SWLFI", ("WLIid looking at %s", AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    foldeewl = ((NULL != assignn) && (N_with == NODE_TYPE (ASSIGN_RHS (assignn))))
                 ? ASSIGN_RHS (assignn)
                 : NULL;

    if ((NULL != foldeewl) && (NULL == WITH_REFERENCED_FOLDERWL (foldeewl))) {
        /* First reference to this WL. */
        WITH_REFERENCED_FOLDERWL (foldeewl) = INFO_FOLDERWL (arg_info);
        WITH_REFERENCED_FOLD (foldeewl) = 0;
        DBUG_PRINT ("SWLFI", ("WLIid found first reference to %s",
                              AVIS_NAME (ID_AVIS (arg_node))));
    }

    /*
     * arg_node describes a WL, so
     * WITH_REFERENCED_FOLD(foldeeWL) may have to be
     * incremented
     */
    if ((NULL != foldeewl) && (NULL != INFO_FOLDERWL (arg_info))
        && (WITH_REFERENCED_FOLDERWL (foldeewl) == INFO_FOLDERWL (arg_info))) {
        (WITH_REFERENCED_FOLD (foldeewl)) += 1;
        DBUG_PRINT ("SWLFI",
                    ("WLIid incrementing WITH_REFERENCED_FOLD(%s) = %d",
                     AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (foldeewl)));
    } else {
#ifdef NOISY
        DBUG_PRINT ("SWLFI",
                    ("WLIid %s is not defined by a WL", AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SWLFIprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all _sel_VxA_(iv, foldeeWL) primitives to see if
 *   we may be able to fold foldeeWL here, assuming that
 *   the _sel_ is within a potential folderWL.
 *
 *   We don't find out if all the conditions for folding can
 *   be met until this phase completes, so SWLF makes the
 *   final decision on folding.
 *
 *****************************************************************************/
node *
SWLFIprf (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ("SWLFIprf");

    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)) {

        INFO_SWLFOLDABLEFOLDEE (arg_info) = checkFolderFoldable (arg_node, arg_info)
                                            && checkFoldeeFoldable (arg_node, arg_info)
                                            && checkBothFoldable (arg_node, arg_info);

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* Replace iv by iv' */
        if (INFO_SWLFOLDABLEFOLDEE (arg_info)) {
            z = createNewIV (arg_node, arg_info);
            /* FIXME why doesn't this work???FreeDoFreeNode( PRF_ARG1( arg_node)); */
            PRF_ARG1 (arg_node) = z;
            DBUG_PRINT ("SWLFI", ("SWLFIprf Inserting F_dataflowguard at _sel_VxA_"));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFIcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
SWLFIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIcond");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFImodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   if op is modarray( foldee_wl), we increment
 *   WITH_REFERENCED_FOLD for the foldee.
 *
 ******************************************************************************/
node *
SWLFImodarray (node *arg_node, info *arg_info)
{
    node *foldeewlid;
    node *foldeeavis;
    node *foldeeassign;
    node *foldeewl;

    DBUG_ENTER ("SWLFImodarray");

    arg_node = TRAVcont (arg_node, arg_info);

    if (N_modarray == NODE_TYPE (arg_node)) {
        foldeewlid = MODARRAY_ARRAY (arg_node);
        foldeeavis = ID_AVIS (foldeewlid);
        foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (foldeeavis));
        foldeewl = LET_EXPR (foldeeassign);
        (WITH_REFERENCED_FOLD (foldeewl))++;
        DBUG_PRINT ("SWLFI", ("WLImodarray: WITH_REFERENCED_FOLD(%s) = %d",
                              AVIS_NAME (foldeeavis), WITH_REFERENCED_FOLD (foldeewl)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFIlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   FIXME: Probably not needed???
 ******************************************************************************/
node *
SWLFIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIlet");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFIblock(node *arg_node, info *arg_info)
 *
 * description:
 *
 *   FIXME: Probably not needed???
 ******************************************************************************/
node *
SWLFIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIblock");
    BLOCK_VARDEC (arg_node) = TRAVopt (BLOCK_VARDEC (arg_node), arg_info);
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SWLFIavis(node *arg_node, info *arg_info)
 *
 * description:
 *    Zero the AVIS_NEEDCOUNT values before traversing any code.
 *
 ******************************************************************************/
node *
SWLFIavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SWLFIavis");

    AVIS_NEEDCOUNT (arg_node) = 0;
    AVIS_ISWLFOLDED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Symbolic with loop folding -->
 *****************************************************************************/
