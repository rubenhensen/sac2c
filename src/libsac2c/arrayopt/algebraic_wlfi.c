/*
 * $Id: algebraic_wlfi.c 16030 2009-04-09 19:40:34Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup awlfi Algebraic With-Loop Folding Inference
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
 * @brief Algebraic With-Loop Folding (AWLF)
 *        AWLF performs WLF on some arrays that are not foldable
 *        by WLF. Features of AWLF are described in algebraic_wlf.c
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
 *        This phase inserts _attachextrema "intersection expressions"
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
 * @file algebraic_wlfi.c
 *
 * Prefix: AWLFI
 *
 *****************************************************************************/
#include "algebraic_wlfi.h"
#include "algebraic_wlf.h"

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
#include "check.h"
#include "ivextrema.h"
#include "phase.h"

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
    bool awlfoldablefoldee; /* foldeeWL may be legally foldable. */
                            /* (If index sets prove to be OK)     */
    bool onefundef;         /* fundef-based traversal */
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
#define INFO_AWLFOLDABLEFOLDEE(n) ((n)->awlfoldablefoldee)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)

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
    INFO_AWLFOLDABLEFOLDEE (result) = FALSE;
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

/******************************************************************************
 *
 * function:
 *   node *AWLFIdoAlgebraicWithLoopFoldingOneFunction(node *arg_node)
 *
 * @brief Global entry point of Algebraic With-Loop folding
 *        Applies Algebraic WL folding to a fundef.
 *
 *****************************************************************************/
node *
AWLFIdoAlgebraicWithLoopFoldingOneFunction (node *arg_node)
{
    DBUG_ENTER ("AWLFIdoAlgebraicWithLoopFoldingOneFunction");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 ("AWLFIdoAlgebraicWithLoopFoldingOneFunction called for non-fundef"));

    TRAVpush (TR_awlfi);
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
 * @fn
 *                                  node **preassigns, node *restypeavis)
 *
 *   @brief  TRUE if the N_with does not contain a default N_part.
 *
 *   @param  node *arg_node: an N_with node.
 *   @return Boolean.
 *
 ******************************************************************************/
static bool
noDefaultPartition (node *arg_node)
{
    node *partn;
    bool z = TRUE;

    DBUG_ENTER ("noDefaultPartition");

    partn = WITH_PART (arg_node);

    while ((NULL != partn) && z) {
        z = z & (N_default != NODE_TYPE (PART_GENERATOR (partn)));
        partn = PART_NEXT (partn);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIflattenExpression(node *arg_node, node **vardecs,
 *                                  node **preassigns, node *restypeavis)
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
 *   @param  node *arg_node: a node to be flattened.
 *           node **vardecs: a pointer to a vardecs chain that
 *                           will have a new vardec appended to it.
 *           node **preassigns: a pointer to a preassigns chain that
 *                           will have a new assign appended to it.
 *           node *restypeavis: an N_avis with the same type as TMP.
 *
 *   @return node *node:      N_avis node for flattened node
 *
 ******************************************************************************/
node *
AWLFIflattenExpression (node *arg_node, node **vardecs, node **preassigns,
                        node *restypeavis)
{
    node *avis;
    node *nas;

    DBUG_ENTER ("AWLFIflattenExpression");

    avis = TBmakeAvis (TRAVtmpVar (), TYcopyType (AVIS_TYPE (restypeavis)));
    *vardecs = TBmakeVardec (avis, *vardecs);
    nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), arg_node), NULL);
    *preassigns = TCappendAssign (*preassigns, nas);
    AVIS_SSAASSIGN (avis) = nas;
    if (isSAAMode ()) {
        AVIS_DIM (avis) = DUPdoDupTree (AVIS_DIM (restypeavis));
        AVIS_SHAPE (avis) = DUPdoDupTree (AVIS_SHAPE (restypeavis));
    }
    DBUG_PRINT ("AWLFI",
                ("AWLFIflattenExpression generated assign for %s", AVIS_NAME (avis)));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilderOne( node *arg_node, info *arg_info,
 *                                      node *foldeepart, int boundnum,
 *                                      node *ivminmax,
 *                                      node *genshpavis)
 *
 * @brief Build a pair of expressions for intersecting the bounds of
 *        a single foldeeWL partition with folderWL index set, of the form:
 *
 *           iv' = (k*iv) + ivoffset;
 *           sel( iv', foldeeWL)
 *
 *        The intersection calculation is made slightly more complex
 *        by the fact that iv' has shape of:
 *         GENERATOR_BOUND1(foldeepart) ++ cellshape(foldeepart)
 *        Instead of computing this, we take the prefix of iv'.
 *
 *        We determine the intersection of the folderWL iv'
 *        index set with the foldeeWL's partition bounds this way:
 *
 *          genshp = _shape_A_(GENERATOR_BOUND1( foldee));
 *
 *          ivmin = _take_SxV( genshp, AVIS_MINVAL( iv'));
 *          intlo = _max_VxV_( AVIS_MINVAL( ivmin), GENERATOR_BOUND1(foldee));
 *
 *          ivmax = _take_SxV( genshp, AVIS_MAXVAL( iv');
 *          ivmax' = _add_VxS_( ivmax, 1);
 *          inthi  = _min_VxV_( ivmax', GENERATOR_BOUND2(foldee));
 *          iv'' = _attachextrema_(iv', ivmax', intlo, inthi);
 *          sel( iv'', foldeeWL)
 *
 *        where int1 evaluates to the lower bound of the intersection,
 *        and   int2 evaluates to the upper bound of the intersection.
 *
 *        The ivmaxpref calculation adjusts AVIS_MAXVAL to match
 *        the canonical generator bounds. This is used as
 *        shown above, but the value glued to the guard is
 *        also needed by FindMatchingPart in AWLF.
 *
 * @params arg_node: the _sel_VxA_( idx, foldeeWL)
 * @params foldeepart: An N_part of the foldeeWL.
 * @params arg_info.
 * @params boundnum: 1 for bound1,     or 2 for bound2
 * @parame bounder: AVIS_MINVAL( idx) or ( AVIS_MAXVAL( idx) + 1)
 * @param  genshpavis: the N_avis for the above genshp computation.
 *
 * @return An N_avis pointing to an N_exprs for the two intersect expressions.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilderOne (node *arg_node, info *arg_info, node *foldeepart, int boundnum,
                           node *ivminmax, node *genshpavis)
{
    node *boundee;
    node *folderpart;
    node *expn;
    node *idxavis;
    node *idxassign;
    node *resavis;
    node *takeres;
    node *takeresavis;

    DBUG_ENTER ("IntersectBoundsBuilderOne");

    folderpart = INFO_PART (arg_info);

    /* The indexing expression used in the sel(iv, foldee). */
    idxavis = ID_AVIS (PRF_ARG1 (arg_node));
    idxassign = AVIS_SSAASSIGN (idxavis);

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

    takeres = TCmakePrf2 (F_take_SxV, TBmakeId (genshpavis), TBmakeId (ivminmax));

    takeresavis = AWLFIflattenExpression (takeres, &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGNS (arg_info), genshpavis);

    expn = TCmakePrf2 ((boundnum == 1) ? F_max_VxV : F_min_VxV, TBmakeId (takeresavis),
                       DUPdoDupTree (boundee));
    resavis = AWLFIflattenExpression (expn, &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info), genshpavis);

    DBUG_RETURN (resavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilder( node *arg_node, info *arg_info,
 *                                   node *foldeepart, int boundnum,
 *                                   node *ivminmax)
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
 * @params boundnum: 1 for bound1,        2 for bound2
 * @params ivminmax: AVIS_MINVAL( idx) or (AVIS_MAXVAL( idx) + 1)
 * @return An N_exprs node containing the (two*#foldee partitions) intersect expressions.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilder (node *arg_node, info *arg_info, node *foldeeid, int boundnum,
                        node *ivminmax)
{
    node *expn = NULL;
    node *partn;
    node *foldeeassign;
    node *foldeewl;
    node *curavis;
    node *genshp;
    node *genshpavis;
    node *gen;

    DBUG_ENTER ("IntersectBoundsBuilder");

    foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (foldeeid)));
    foldeewl = LET_EXPR (foldeeassign);

    /* Emit:  genshp = _shape_A_(GENERATOR_BOUND1( foldee)); */
    gen = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (foldeewl)));
    genshp = TCmakePrf2 (F_idx_shape_sel, TBmakeNum (0), DUPdoDupTree (gen));
    genshpavis = AWLFIflattenExpression (genshp, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info), ID_AVIS (gen));

    partn = WITH_PART (foldeewl);
    while (NULL != partn) {
        curavis = IntersectBoundsBuilderOne (arg_node, arg_info, partn, boundnum,
                                             ivminmax, genshpavis);
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));
        partn = PART_NEXT (partn);
    }

    DBUG_RETURN (expn);
}

/** <!--********************************************************************-->
 *
 * @fn node *attachIntersectCalc( node *arg_node, info *arg_info)
 *
 * @brief  We are looking at the N_prf for:
 *
 *            z = _sel_VxA_( idx, foldeeWL);
 *         and idx now has extrema attached to it. We
 *         are now in a position to compute the intersection
 *         between idx's index set and that of the foldeeWL
 *         partitions.
 *
 *         We create new idx' from idx, to hold the result
 *         of the intersect computations that we build here.
 *
 *   Replace:
 *
 *   z = _sel_(idx, foldeeWL);
 *
 *   by
 *
 *   minv0 = _max_VxV( AVIS_MINVAL( idx)    , GENERATOR_BOUND1( foldeepart)
 *   maxv0 = _min_VxV( AVIS_MAXVAL( idx) + 1, GENERATOR_BOUND2( foldeepart)
 *   ... ( above 2 lines for each partition)
 *
 *   iv' = _attachintersect( iv, ivmax, minv0, maxv0, minv1, maxv1,...);
 *   z = _sel_(iv', foldeeWL);
 *
 *   We have to compute ivmax as the AVIS_MAXVAL( iv) + 1, because
 *   extrema are exact, and generator bounds are 1 higher.
 *
 *   This function also appends vardecs to the INFO chain,
 *   and assigns to the preassigns chain.
 *
 * @params arg_node: the N_prf node of the sel() operation.
 * @return A pointer to the newly created N_id node for iv'.
 *
 *****************************************************************************/

static node *
attachIntersectCalc (node *arg_node, info *arg_info)
{
    node *ivavis;
    node *ivassign;
    int ivshape;
    node *ivid;
    node *ividprime;
    node *ivmax;
    node *lbicalc;
    node *ubicalc;
    node *foldeewl;
    node *args;
    node *idxavis;

    DBUG_ENTER ("attachIntersectCalc");

    DBUG_PRINT ("AWLFI", ("Inserting attachextrema computations"));

    /* Generate expressions for lower-bound intersection and
     * upper-bound intersection calculation.
     */
    ivid = PRF_ARG1 (arg_node);
    foldeewl = PRF_ARG2 (arg_node);

    /* Convert exact extrema upper bound into N_generator form */
    idxavis = ID_AVIS (ivid);
    ivmax
      = IVEXIadjustExtremaBound (AVIS_MAXVAL (idxavis), arg_info, 1,
                                 &INFO_VARDECS (arg_info), &INFO_PREASSIGNS (arg_info));

    lbicalc
      = IntersectBoundsBuilder (arg_node, arg_info, foldeewl, 1, AVIS_MINVAL (idxavis));
    ubicalc = IntersectBoundsBuilder (arg_node, arg_info, foldeewl, 2, ivmax);
    args = TCappendExprs (TBmakeExprs (DUPdoDupTree (ivid),
                                       TBmakeExprs (TBmakeId (ivmax), NULL)),
                          lbicalc);
    args = TCappendExprs (args, ubicalc);

    ivshape = SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (ivid))));
    ivavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (ivid))),
                         TYcopyType (AVIS_TYPE (ID_AVIS (ivid))));

    INFO_VARDECS (arg_info) = TBmakeVardec (ivavis, INFO_VARDECS (arg_info));
    ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivavis, NULL),
                                        TBmakePrf (F_attachintersect, args)),
                             NULL);
    INFO_PREASSIGNS (arg_info) = TCappendAssign (INFO_PREASSIGNS (arg_info), ivassign);
    ividprime = TBmakeId (ivavis);
    AVIS_SSAASSIGN (ivavis) = ivassign;

    if (isSAAMode ()) {
        AVIS_DIM (ivavis) = DUPdoDupTree (AVIS_DIM (ID_AVIS (ivid)));
        AVIS_SHAPE (ivavis) = DUPdoDupTree (AVIS_SHAPE (ID_AVIS (ivid)));
    }

    global.optcounters.awlfi_insert++;

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
 *        are determined by the AWLF phase later on.
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
 *             This is not strictly needed; it is only there
 *             to avoid potentially computing the same
 *             foldeeWL element more than once. FIXME: wl_needcount.c
 *             changes will relax this and restriction and base it
 *             on foldeeWL element computation cost.
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
 *        The expressions hanging from the attachextrema inserted
 *        by attachExtremaCalc are intended to determine if this
 *        requirement is met. CF, CVP, and other optimizations
 *        should simplify those expressions and give AWLF
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

        DBUG_PRINT ("AWLFI", ("FoldeeWL:%s: WITH_REFERENCED_FOLD=%d",
                              AVIS_NAME (foldeeavis), WITH_REFERENCED_FOLD (foldeewl)));

        if ((NODE_TYPE (foldeeassign) == N_let)
            && (NODE_TYPE (LET_EXPR (foldeeassign)) == N_with)
            && (WITHOP_NEXT (WITH_WITHOP (LET_EXPR (foldeeassign))) == NULL)
            && (noDefaultPartition (LET_EXPR (foldeeassign)))
            && ((NODE_TYPE (WITH_WITHOP (LET_EXPR (foldeeassign))) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (LET_EXPR (foldeeassign))) == N_modarray))) {
            z = TRUE;
        }
    }

    if (z) {
        DBUG_PRINT ("AWLFI",
                    ("Foldee WL %s is suitable for folding.", AVIS_NAME (foldeeavis)));
    } else {
        DBUG_PRINT ("AWLFI", ("Foldee WL %s is not suitable for folding.",
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
 *        We deem the prf foldable if idx derives directly from
 *        an _attachintersect_, or if idx has extrema.
 *
 *        This may not be enough to guarantee foldability, but
 *        without extrema, we're stuck.
 *
 * @param _sel_VxA_( idx, foldeeWL) arg_node.
 *
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
    bool z;

    DBUG_ENTER ("checkFolderFoldable");

    idx = PRF_ARG1 (arg_node);
    idxavis = ID_AVIS (idx);
    /* idx has extrema or F_attachintersect */
    z = ((NULL != AVIS_MINVAL (idxavis)) && (NULL != AVIS_MAXVAL (idxavis)))
        || isPrfArg1AttachIntersect (arg_node);
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
#ifdef FIXME
    node *folderwl;
    node *foldeewl;
    node *b1;
    node *b2;
#endif // FIXME
    bool z;

    DBUG_ENTER ("checkBothFoldable");

    z = TRUE; /* We no longer care about generator bounds.
               * All folding checks are now based on the
               * intersection between idx index set and
               * foldeeWL partition intersection generator
               * limits, catenated to cell shape.
               */
#ifdef FIXME
    foldeewl = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (ID_AVIS (PRF_ARG2 (arg_node)))));
    folderwl = INFO_FOLDERWL (arg_info);

    b1 = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (folderwl)));
    b2 = GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (foldeewl)));
    z = SHcompareShapes (TYgetShape (AVIS_TYPE (ID_AVIS (b1))),
                         TYgetShape (AVIS_TYPE (ID_AVIS (b2))));
    if (z) {
        DBUG_PRINT ("AWLFI", ("FolderWL & FoldeeWL %s generator shapes match.",
                              AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
    } else {
        DBUG_PRINT ("AWLFI", ("FolderWL & FoldeeWL %s generator shapes do not match.",
                              AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
    }
#endif // FIXME
    DBUG_RETURN (z);
}
/** <!--********************************************************************-->
 *
 * @fn node *AWLFIfundef(node *arg_node, info *arg_info)
 *
 * @brief applies AWLFI to a given fundef.
 *
 *****************************************************************************/
node *
AWLFIfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;

    DBUG_ENTER ("AWLFIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("AWLFI", ("Algebraic-With-Loop-Folding Inference in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        arg_info = MakeInfo (arg_node);
        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_awlfi);

        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
            /* If new vardecs were made, append them to the current set */
            if (INFO_VARDECS (arg_info) != NULL) {
                BLOCK_VARDEC (FUNDEF_BODY (arg_node))
                  = TCappendVardec (INFO_VARDECS (arg_info),
                                    BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
                INFO_VARDECS (arg_info) = NULL;
            }

            INFO_ONEFUNDEF (arg_info) = old_onefundef;
        }

        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("AWLFI", ("Algebraic-With-Loop-Folding Inference in %s %s ends",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));
    }
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), NULL);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node AWLFIassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, foldee).
 *
 *****************************************************************************/
node *
AWLFIassign (node *arg_node, info *arg_info)
{

    DBUG_ENTER ("AWLFIassign");

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
 * @fn node *AWLFIwith( node *arg_node, info *arg_info)
 *
 * @brief applies AWLFI to a with-loop in a top-down manner.
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
AWLFIwith (node *arg_node, info *arg_info)
{
    info *old_arg_info;

    DBUG_ENTER ("AWLFIwith");

    old_arg_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));
    INFO_LEVEL (arg_info) = INFO_LEVEL (old_arg_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_arg_info);
    INFO_FOLDERWL (arg_info) = arg_node;

    DBUG_PRINT ("AWLFI", ("Resetting WITH_REFERENCED_FOLDERWL, etc."));
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
 * @fn node *AWLFIcode( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
AWLFIcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIcode");

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);
    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIpart( node *arg_node, info *arg_info)
 *
 * @brief Traverse each partition of a WL.
 *
 *****************************************************************************/
node *
AWLFIpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIpart");

    INFO_PART (arg_info) = arg_node;
    PART_CODE (arg_node) = TRAVdo (PART_CODE (arg_node), arg_info);
    INFO_PART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIids( node *arg_node, info *arg_info)
 *
 * @brief set current With-Loop level as ids defDepth attribute
 *
 *****************************************************************************/
node *
AWLFIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIids");

    AVIS_DEFDEPTH (IDS_AVIS (arg_node)) = INFO_LEVEL (arg_info);
    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *  (cloned from SSAWLI)
 *
 * function:
 *   node *AWLFIid(node *arg_node, info *arg_info)
 *
 * description:
 *   If this Id is a reference to a WL (N_with) we want to increment
 *   the number of references to the potential foldeeWL (WITH_REFERENCED_FOLD).
 *   The WL node has to be found via avis_ssaassign backlink.
 *
 *   We want to end up with WITH_REFERENCED_FOLD counting only
 *   references from a single WL. Hence, the checking on
 *   WITH_REFERENCED_FOLDERWL
 *   AWLF will disallow folding if WITH_REFERENCED_FOLD != AVIS_NEEDCOUNT.
 *
 ******************************************************************************/
node *
AWLFIid (node *arg_node, info *arg_info)
{
    node *assignn;
    node *foldeewl;

    DBUG_ENTER ("AWLFIid");
    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
#ifdef NOISY
    DBUG_PRINT ("AWLFI", ("AWLFIid looking at %s", AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));
    foldeewl = ((NULL != assignn) && (N_with == NODE_TYPE (ASSIGN_RHS (assignn))))
                 ? ASSIGN_RHS (assignn)
                 : NULL;

    if ((NULL != foldeewl) && (NULL == WITH_REFERENCED_FOLDERWL (foldeewl))) {
        /* First reference to this WL. */
        WITH_REFERENCED_FOLDERWL (foldeewl) = INFO_FOLDERWL (arg_info);
        WITH_REFERENCED_FOLD (foldeewl) = 0;
        DBUG_PRINT ("AWLFI", ("AWLFIid found first reference to %s",
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
        DBUG_PRINT ("AWLFI",
                    ("AWLFIid incrementing WITH_REFERENCED_FOLD(%s) = %d",
                     AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (foldeewl)));
    } else {
#ifdef NOISY
        DBUG_PRINT ("AWLFI", ("AWLFIid %s is not defined by a WL",
                              AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine all _sel_VxA_(idx, foldeeWL) primitives to see if
 *   we may be able to fold foldeeWL here, assuming that
 *   the _sel_ is within a potential folderWL.
 *
 *   We don't find out if all the conditions for folding can
 *   be met until this phase completes, so AWLF makes the
 *   final decision on folding.
 *
 *   When we do encounter an eligible sel() operation,
 *   with extrema available on PRF_ARG1, we construct
 *   an intersect computation between the now-available index
 *   set of idx, and each partition of the foldeeWL.
 *   These are attached to the sel() via an F_attachintersect
 *   guard.
 *
 *****************************************************************************/
node *
AWLFIprf (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ("AWLFIprf");

    if ((INFO_PART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)) {

        INFO_AWLFOLDABLEFOLDEE (arg_info) = checkFolderFoldable (arg_node, arg_info)
                                            && checkFoldeeFoldable (arg_node, arg_info)
                                            && checkBothFoldable (arg_node, arg_info);

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* Maybe attach intersect calculations now. */
        if ((INFO_AWLFOLDABLEFOLDEE (arg_info))
            && (!isPrfArg1AttachIntersect (arg_node))) {
            z = attachIntersectCalc (arg_node, arg_info);
            FREEdoFreeNode (PRF_ARG1 (arg_node));
            PRF_ARG1 (arg_node) = z;
            DBUG_PRINT ("AWLFI", ("AWLFIprf inserted F_attachintersect at _sel_VxA_"));
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIcond(node *arg_node, info *arg_info)
 *
 * description:
 *   traverse conditional parts in the given order.
 *
 ******************************************************************************/
node *
AWLFIcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIcond");

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENINSTR (arg_node) = TRAVdo (COND_THENINSTR (arg_node), arg_info);
    COND_ELSEINSTR (arg_node) = TRAVdo (COND_ELSEINSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFImodarray(node *arg_node, info *arg_info)
 *
 * description:
 *   if op is modarray( foldee_wl), we increment
 *   WITH_REFERENCED_FOLD for the foldee.
 *
 ******************************************************************************/
node *
AWLFImodarray (node *arg_node, info *arg_info)
{
    node *foldeewlid;
    node *foldeeavis;
    node *foldeeassign;
    node *foldeewl;

    DBUG_ENTER ("AWLFImodarray");

    arg_node = TRAVcont (arg_node, arg_info);

    if (N_modarray == NODE_TYPE (arg_node)) {
        foldeewlid = MODARRAY_ARRAY (arg_node);
        foldeeavis = ID_AVIS (foldeewlid);
        foldeeassign = ASSIGN_INSTR (AVIS_SSAASSIGN (foldeeavis));
        foldeewl = LET_EXPR (foldeeassign);
        (WITH_REFERENCED_FOLD (foldeewl))++;
        DBUG_PRINT ("AWLFI", ("AWLFImodarray: WITH_REFERENCED_FOLD(%s) = %d",
                              AVIS_NAME (foldeeavis), WITH_REFERENCED_FOLD (foldeewl)));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIlet(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIlet");
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIblock(node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIblock");
    BLOCK_INSTR (arg_node) = TRAVdo (BLOCK_INSTR (arg_node), arg_info);
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIavis(node *arg_node, info *arg_info)
 *
 * description:
 *    Zero the AVIS_NEEDCOUNT values before traversing any code.
 *
 ******************************************************************************/
node *
AWLFIavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIavis");

    AVIS_NEEDCOUNT (arg_node) = 0;
    AVIS_ISWLFOLDED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIfuncond( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIfuncond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIfuncond");

    FUNCOND_IF (arg_node) = TRAVopt (FUNCOND_IF (arg_node), arg_info);
    FUNCOND_THEN (arg_node) = TRAVopt (FUNCOND_THEN (arg_node), arg_info);
    FUNCOND_ELSE (arg_node) = TRAVopt (FUNCOND_ELSE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIwhile( node *arg_node, info *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
AWLFIwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("AWLFIwhile");

    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding inference -->
 *****************************************************************************/