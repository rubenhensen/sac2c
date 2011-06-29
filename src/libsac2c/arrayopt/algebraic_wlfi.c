/*
 *        one partition of the producerWL.
 * $Id: algebraic_wlfi.c 16030 2009-04-09 19:40:34Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup awlfi Algebraic With-Loop Folding Inference
 *
 * @terminology:
 *        ProducerWL:
 *        The WL that will no longer
 *        exist after this phase completes. In the example
 *        below, A is the producerWL.
 *
 *        ConsumerWL:
 *        the WL that will absorb the block(s) from
 *        the producerWL. In the example below, B is the ConsumerWL.
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
 *        all N_id nodes. This is done as a cost-function
 *        heuristic to prevent the same producerWL element
 *        from being computed repeatedly.
 *        [ We could develop a better cost function. For example,
 *          iota(N) has a cost of zero, because the producerWL
 *          element cost is just an access to the WITH_ID.]
 *
 *        The phase then makes a depth-first traversal
 *        of the function, computing WL_REFERENCED_FOLD(producerWL)++
 *        for all sel() operations in WLs that are of the form:
 *
 *             _sel_VxA_(iv, producerWL)
 *
 *        where producerWL is the result of another WL.
 *        At present, only one consumerWL is allowed to contribute to the
 *        total. AWLF will permit the folding to occur if, among
 *        other requirements:
 *
 *         AVIS_NEEDCOUNT(producerWL) == WL_REFERENCED_FOLD(producerWL)
 *
 *        This phase inserts _attachextrema "intersection expressions"
 *        before _sel_(iv, X) statements, if iv is a function of
 *        the WITH_ID(consumerWL) and X is the result of a producerWL.
 *
 *        An intersection expression provides the information required
 *        to cut a consumerWL partition so that its index set
 *        is entirely contained with the index set of
 *        one partition of the producerWL.
 *
 *        Once the cutting, if needed, has been performed, folding of the
 *        producerWL into the consumerWL can be performed directly,
 *        by replacing the above _sel_(iv, X) expression with
 *        the code from the producerWL partition, and performing
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
#include "tree_utils.h"
#include "shape.h"
#include "check.h"
#include "ivextrema.h"
#include "phase.h"
#include "namespaces.h"
#include "deserialize.h"
#include "wls.h"
#include "SSAWithloopFolding.h"
#include "new_typecheck.h"
#include "ivexpropagation.h"
#include "string.h"
#include "constant_folding.h"
#include "variable_propagation.h"
#include "ElimSubDiv.h"
#include "arithmetic_simplification.h"
#include "associative_law.h"
#include "distributive_law.h"
#include "UndoElimSubDiv.h"
#include "inlining.h"
#include "elim_alpha_types.h"
#include "elim_bottom_types.h"
#include "insert_symb_arrayattr.h"
#include "dispatchfuncalls.h"
#include "SSACSE.h"
#include "cubeslicer.h"
#include "prfunroll.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *fundef;
    node *vardecs;
    node *preassigns;        /* These go above current statement */
    node *preassignswl;      /* These go above the consumerWL */
    node *consumerwlpart;    /* The current consumerWL partition */
    node *consumerwl;        /* The current consumerWL N_with */
    node *consumerwllhs;     /* The current consumerWL LHS */
    node *producerwl;        /* The producerWL N_with for this PWL */
    node *producerwllhs;     /* The producerWL LHS for this consumerWL */
    node *let;               /* The N_let node */
    node *withids;           /* the WITHID_IDS entry for an iv element */
    int level;               /* The current nesting level of WLs. This
                              * is used to ensure that an index expression
                              * refers to an earlier WL in the same code
                              * block, rather than to a WL within this
                              * WL. I think...
                              */
    bool producerWLFoldable; /* producerWL may be legally foldable. */
                             /* (If index sets prove to be OK)     */
    bool onefundef;          /* fundef-based traversal */
    bool nofinverse;         /* no intersect to CWL mapping found */
    bool finverseswap;       /* If TRUE, must swp min/max */
    bool finverseintroduced; /* If TRUE, most simplify F-inverse */
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_PREASSIGNS(n) ((n)->preassigns)
#define INFO_PREASSIGNSWL(n) ((n)->preassignswl)
#define INFO_CONSUMERWLPART(n) ((n)->consumerwlpart)
#define INFO_CONSUMERWL(n) ((n)->consumerwl)
#define INFO_CONSUMERWLLHS(n) ((n)->consumerwllhs)
#define INFO_PRODUCERWL(n) ((n)->producerwl)
#define INFO_PRODUCERWLLHS(n) ((n)->producerwllhs)
#define INFO_LET(n) ((n)->let)
#define INFO_WITHIDS(n) ((n)->withids)
#define INFO_LEVEL(n) ((n)->level)
#define INFO_PRODUCERWLFOLDABLE(n) ((n)->producerWLFoldable)
#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_NOFINVERSE(n) ((n)->nofinverse)
#define INFO_FINVERSESWAP(n) ((n)->finverseswap)
#define INFO_FINVERSEINTRODUCED(n) ((n)->finverseintroduced)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PREASSIGNSWL (result) = NULL;
    INFO_CONSUMERWLPART (result) = NULL;
    INFO_CONSUMERWL (result) = NULL;
    INFO_CONSUMERWLLHS (result) = NULL;
    INFO_PRODUCERWL (result) = NULL;
    INFO_PRODUCERWLLHS (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_WITHIDS (result) = NULL;
    INFO_LEVEL (result) = 0;
    INFO_PRODUCERWLFOLDABLE (result) = TRUE;
    INFO_ONEFUNDEF (result) = FALSE;
    INFO_NOFINVERSE (result) = FALSE;
    INFO_FINVERSESWAP (result) = FALSE;
    INFO_FINVERSEINTRODUCED (result) = FALSE;

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
 *   node *AWLFIdoAlgebraicWithLoopFolding(node *arg_node)
 *
 * @brief Global entry point of Algebraic With-Loop folding
 *        Applies Algebraic WL folding to a fundef.
 *
 *****************************************************************************/
node *
AWLFIdoAlgebraicWithLoopFolding (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("AWLFIdoAlgebraicWithLoopFolding");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 ("AWLFIdoAlgebraicWithLoopFoldingOneFunction called for non-fundef"));

    arg_info = MakeInfo (arg_node);
    INFO_ONEFUNDEF (arg_info) = TRUE;

    DSinitDeserialize (global.syntax_tree);

    TRAVpush (TR_awlfi);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    DSfinishDeserialize (global.syntax_tree);

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
 * function: node *SimplifySymbioticExpression
 *
 * description: Code to simplify symbiotic expression for
 *              AWLF intersect calculation.
 *
 *              We perform inlining to bring sacprelude code into
 *              this function, then repeatedly invoke a small
 *              set of optimizations until we reach a fix point
 *              or give up.
 *
 * @params  arg_node: an N_fundef node.
 *
 * @result: an updated N_fundef node.
 *
 ******************************************************************************/
static node *
SimplifySymbioticExpression (node *arg_node, info *arg_info)
{
    int i;
    int ct;
    int countINL = 0;
    int countCSE = 0;
    int countTUP = 0;
    int countCF = 0;
    int countVP = 0;
    int countAS = 0;
    int countAL = 0;
    int countDL = 0;

    DBUG_ENTER ("SimplifySymbioticExpression");

    DBUG_PRINT ("SSE", ("Entering opt micro-cycle for %s %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                        FUNDEF_NAME (arg_node)));
    /* Loop over optimizers until we reach a fix point or give up */
    for (i = 0; i < global.max_optcycles; i++) {
        ct = i;

        /* Invoke each opt */
        if (global.optimize.doinl) {
            countINL = global.optcounters.inl_fun;
            arg_node = INLdoInlining (arg_node);
        }
        if (global.optimize.dosaa) {
            arg_node = ISAAdoInsertShapeVariables (arg_node);
        }
        if (global.optimize.docse) {
            countCSE = global.optcounters.cse_expr;
            arg_node = CSEdoCommonSubexpressionElimination (arg_node);
        }
        if (global.optimize.dotup) {
            countTUP = global.optcounters.tup_upgrades;
            arg_node = NTCdoNewTypeCheck (arg_node);
        }
        if (global.optimize.dotup) {
            arg_node = EATdoEliminateAlphaTypes (arg_node);
        }
        if (global.optimize.dotup) {
            arg_node = EBTdoEliminateBottomTypes (arg_node);
        }
        if (TRUE) {
            arg_node = DFCdoDispatchFunCalls (arg_node);
        }
        if (global.optimize.docf) {
            arg_node = CFdoConstantFolding (arg_node);
            countCF = global.optcounters.cf_expr;
        }
        if (global.optimize.dovp) {
            arg_node = VPdoVarPropagation (arg_node);
            countVP = global.optcounters.vp_expr;
        }
        if (global.optimize.dosde) {
            arg_node = ESDdoElimSubDiv (arg_node);
        }
        if (global.optimize.doas) {
            arg_node = ASdoArithmeticSimplification (arg_node);
            countAS = global.optcounters.as_expr;
        }
        if (global.optimize.docse) {
            arg_node = CSEdoCommonSubexpressionElimination (arg_node);
        }
        if (global.optimize.docf) {
            arg_node = CFdoConstantFolding (arg_node);
            countCF = global.optcounters.cf_expr;
        }
        if (global.optimize.doas) {
            countAL = global.optcounters.al_expr;
            arg_node = ALdoAssocLawOptimization (arg_node);
        }
        if (global.optimize.dodl) {
            countDL = global.optcounters.dl_expr;
            arg_node = DLdoDistributiveLawOptimization (arg_node);
        }

        DBUG_PRINT ("SSE", ("INL=%d, CSE=%d, TUP=%d, CF=%d, VP=%d, AS=%d, AL=%d, DL=%d\n",
                            (global.optcounters.inl_fun - countINL),
                            (global.optcounters.cse_expr - countCSE),
                            (global.optcounters.tup_upgrades - countTUP),
                            (global.optcounters.cf_expr - countCF),
                            (global.optcounters.vp_expr - countVP),
                            (global.optcounters.as_expr - countAS),
                            (global.optcounters.al_expr - countAL),
                            (global.optcounters.dl_expr - countDL)));

        if (/* Fix point check */
            (countINL == global.optcounters.inl_fun)
            && (countCSE == global.optcounters.cse_expr)
            && (countTUP == global.optcounters.tup_upgrades)
            && (countCF == global.optcounters.cf_expr)
            && (countVP == global.optcounters.vp_expr)
            && (countAS == global.optcounters.as_expr)
            && (countAL == global.optcounters.al_expr)
            && (countDL == global.optcounters.dl_expr)) {
            i = global.max_optcycles;
        }
    }
    DBUG_PRINT ("SSE", ("Stabilized at iteration %d", ct));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *getNthWithids(...)
 *
 * @brief: A Withids entry starts off as an N_array, such
 *         as:  [ j, i], where j and i are WITHID_IDS
 *         entries. However, if the withids entry matches
 *         the WL's WITHID_IDS exactly, then CSE will
 *         replace it by the WITHID_VEC. This code is
 *         intended to deal with both cases, so when
 *         you write getNthWithids( 1, [j, i]), you get i.
 *
 * @param: arg_node - the AVIS_WITHIDS of interest.
 *         n: the element # we want to fetch.
 *
 * @result: N_avis node for withids or NULL.
 *
 *****************************************************************************/
static node *
getNthWithids (int n, node *arg_node, info *arg_info)
{
    node *z = NULL;
    node *partwithid;
    node *withids;
    pattern *pat;

    DBUG_ENTER ("getNthWithids");

    pat = PMany (1, PMAgetNode (&withids), 1, PMskip (0));
    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        if (N_array == NODE_TYPE (withids)) {
            z = TCgetNthExprsExpr (n, ARRAY_AELEMS (withids));
            z = ID_AVIS (z);
        } else { /* withids is WITHID_VEC. Get WITHID_IDS */
            partwithid = PART_WITHID (INFO_CONSUMERWLPART (arg_info));
            if (ID_AVIS (withids) == IDS_AVIS (WITHID_VEC (partwithid))) {
                withids = WITHID_IDS (partwithid);
                z = TCgetNthIds (n, withids);
            }
        }
    }

    pat = PMfree (pat);

    DBUG_RETURN (z);
}

#ifdef DEADCODE // FIXME
/******************************************************************************
 *
 * function: node *isAllWithidsPresent( node *arg_node)
 *
 * description: Predicate for determining if an N_id node
 *              has all its AVIS_WITHID elements present, for
 *              non-constant elements.
 *
 * @params  arg_node: an N_id node.
 *
 * @result: A boolean, TRUE if AVIS_WITHID are present for all
 *          non-constant elements.
 *
 ******************************************************************************/
static bool
isAllWithidsPresent (node *arg_node)
{
    node *exprs;
    node *avis;
    node *arr;
    pattern *pat;
    bool z = FALSE;

    DBUG_ENTER ("isAllWithidsPresent");

    /* Check that we have WITHIDS for all non-constant array elements.
     * All N_array elements must be N_id nodes.
     */

    pat = PMarray (1, PMAgetNode (&arr), 1, PMskip (0));
    if (PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG1 (arg_node))) {
        exprs = ARRAY_AELEMS (arr);
        z = (NULL != exprs);
        while (z && (NULL != exprs)) {
            if (N_id == NODE_TYPE (EXPRS_EXPR (exprs))) {
                avis = ID_AVIS (EXPRS_EXPR (exprs));
                z = z
                    && ((NULL != AVIS_WITHIDS (avis))
                        || COisConstant (EXPRS_EXPR (exprs)));
            } else {
                z = FALSE;
            }
            exprs = EXPRS_NEXT (exprs);
        }
    }

    pat = PMfree (pat);

    DBUG_RETURN (z);
}
#endif //  DEADCODE  //FIXME

/** <!--********************************************************************-->
 *
 * @fn int FindPrfParent2(...)
 *
 * @brief: Find parent node -- the one with a WITHID_IDS,
 *         of a dyadic scalar function
 *
 * @param: arg_node - an N_prf or N_id or N_num ...  of interest.
 *         arg_info - your basic arg_info node
 *
 * @result: 0 = Not found
 *          1 = PRF_ARG1 traces back to a WITHID_IDS
 *          2 = PRF_ARG2 traces back to a WITHID_IDS
 *
 *****************************************************************************/
static int
FindPrfParent2 (node *arg_node, info *arg_info)
{
    int z = 0;
    node *withidids;
    node *arg = NULL;
    ;
    pattern *pat;
    int tcindex = -1;

    DBUG_ENTER ("FindPrfParent2");

    withidids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
    pat = PMany (1, PMAgetNode (&arg), 0);

    switch (NODE_TYPE (arg_node)) {
    case N_prf:

        tcindex = TClookupIdsNode (withidids, ID_AVIS (PRF_ARG2 (arg_node)));
        if (-1 != tcindex) {
            z = 2;
        }

        tcindex = TClookupIdsNode (withidids, ID_AVIS (PRF_ARG1 (arg_node)));
        if (-1 != tcindex) {
            z = 1;
        }

        if ((0 == z) && (0 != FindPrfParent2 (PRF_ARG1 (arg_node), arg_info))) {
            z = 1;
        }
        if ((0 == z) && (0 != FindPrfParent2 (PRF_ARG2 (arg_node), arg_info))) {
            z = 2;
        }
        break;

    case N_id:
        if ((PMmatchFlatSkipExtremaAndGuards (pat, arg_node))
            && (N_id == NODE_TYPE (arg))) {
            tcindex = TClookupIdsNode (withidids, ID_AVIS (arg));
            if (-1 != tcindex) {
                z = 1;
            }
        }
        break;

    default:
        break;
    }

    if ((z != 0) && (-1 != tcindex)) {
        INFO_WITHIDS (arg_info) = TCgetNthIds (tcindex, withidids);
    }

    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisHasNoteintersect( node *arg_node)
 *
 * @brief: Predicate for presence of F_noteintersect on sel() statement.
 *
 * @param: arg_node - an F_sel_VxA_ N_prf node.
 *
 * @result: TRUE if  arg_node has a F_noteintersect associated with it.
 *
 *****************************************************************************/
bool
AWLFIisHasNoteintersect (node *arg_node)
{
    node *prf = NULL;
    pattern *pat;
    bool z;

    DBUG_ENTER ("AWLFIisHasNoteintersect");

    pat = PMprf (1, PMAgetNode (&prf), 0);
    z = (PMmatchFlat (pat, PRF_ARG1 (arg_node))) && (F_noteintersect == PRF_PRF (prf));
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool isHasValidNoteintersect( node *arg_node, info *arg_info)
 *
 * @brief: Predicate for presence of valid F_noteintersect on sel() statement.
 *
 * @param: arg_node - an F_sel_VxA_ N_prf node.
 *
 * @result: TRUE if  arg_node has a valid F_noteintersect associated with it.
 *          By "valid", we mean that the PWL and the F_noteintersect
 *          have identical bounds and identical PWL partition counts.
 *          A mismatch can arise when we have a code sequence such as:
 *
 *            A = WL( ...);
 *            B = WL( A);
 *            C = WL( B);
 *
 *          and A is folded into B AFTER B has been cube-sliced.
 *          This invalidates the F_noteintersect previously generated
 *          within C.
 *
 *          For now, we merely check that the partition count for
 *          B (the PWL) matches the partition count for the F_noteintersect
 *          for C (the CWL).
 *
 *****************************************************************************/
static bool
isHasValidNoteintersect (node *arg_node, info *arg_info)
{
    node *prf = NULL;
    pattern *pat;
    bool z;
    int nexprs;
    int npart;

    DBUG_ENTER ("isHasValidNoteintersect");

    pat = PMprf (1, PMAgetNode (&prf), 0);
    z = (PMmatchFlat (pat, PRF_ARG1 (arg_node))) && (F_noteintersect == PRF_PRF (prf));
    if (z) {
        nexprs = (TCcountExprs (PRF_ARGS (prf)) - 1) / WLEPP;
        npart = TCcountParts (WITH_PART (INFO_PRODUCERWL (arg_info)));
        z = (nexprs == npart);
    }

    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *detachNoteintersect( node *arg_node)
 *
 * @brief:
 *
 * @param: arg_node - an F_sel_VxA_ N_prf node.
 *
 * @result:  same F_sel op, but with PRF_ARG1 replaced.
 *
 *****************************************************************************/
static node *
detachNoteintersect (node *arg_node)
{
    node *prf = NULL;
    pattern *pat;
    node *z;

    DBUG_ENTER ("detachNoteintersect");

    z = arg_node;
    pat = PMprf (1, PMAgetNode (&prf), 0);
    if ((PMmatchFlat (pat, PRF_ARG1 (arg_node))) && (F_noteintersect == PRF_PRF (prf))) {
        FREEdoFreeNode (PRF_ARG1 (arg_node));
        PRF_ARG1 (arg_node) = DUPdoDupNode (PRF_ARG1 (prf));
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool isHasInverseProjection( node *arg_node)
 *
 * @brief: Predicate for presence of inverse projection of arg_node
 *
 * @param: arg_node - an F_intersect WLPROJECTION1/2 entry.
 *
 * @result: TRUE if  arg_node has an inverse projection
 *          associated with it.
 *
 *****************************************************************************/
static bool
isHasInverseProjection (node *arg_node)
{
    bool z = TRUE;
    constant *co;

    DBUG_ENTER ("isHasinverseProjection");

#define NOINVERSEPROJECTION -1
    co = COaST2Constant (arg_node);
    if (NULL != co) {
        z = (NOINVERSEPROJECTION != COconst2Int (co));
        co = COfreeConstant (co);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjectionScalar(...)
 *
 * @brief Chase one element of an N_array back to its WITHID_IDS,
 *        if possible.
 *
 * @params:  fn: The current expression we are tracing,
 *           An N_id or an N_num.
 *           arg_info: Your basic arg_info node.
 *           ivp: The current inverse projection, iv'
 *
 * @result: An N_avis node that gives the result of the F-inverse mapping
 *          function to take us from iv'->iv, or
 *          a N_num or else NULL, if no such node can be found.
 *
 *          Side effect: Set INFO_WITHIDS, if possible.
 *
 *****************************************************************************/
static node *
BuildInverseProjectionScalar (node *fn, info *arg_info, node *ivp)
{
    node *z = NULL;
    int markiv;
    node *xarg;
    node *ivarg;
    node *id1;
    node *id2;
    node *resavis;
    node *ids;
    node *assgn;
    node *idx = NULL;
    node *rhs;
    node *withidids;
    node *ipavis;
    int tcindex;
    prf nprf;

    pattern *pat;

    DBUG_ENTER ("BuildInverseProjectionScalar");

    if (N_num == NODE_TYPE (fn)) {
        z = DUPdoDupNode (fn);
    } else {
        if (N_id == NODE_TYPE (fn)) {
            ipavis = ID_AVIS (fn);
            DBUG_PRINT ("AWLFI", ("Tracing %s", AVIS_NAME (ipavis)));
            pat = PMany (1, PMAgetNode (&idx), 0);
            if (PMmatchFlatSkipExtremaAndGuards (pat, fn)) {
                switch (NODE_TYPE (idx)) {
                case N_id:
                    /* Check for WITHID_IDS */
                    withidids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
                    tcindex = TClookupIdsNode (withidids, ID_AVIS (idx));
                    if (-1 != tcindex) {
                        DBUG_PRINT ("AWLFI",
                                    ("Found %s as source of iv'=%s",
                                     AVIS_NAME (ID_AVIS (idx)), AVIS_NAME (ipavis)));
                        INFO_WITHIDS (arg_info) = TCgetNthIds (tcindex, withidids);
                        if (N_num == NODE_TYPE (ivp)) {
                            z = AWLFIflattenExpression (DUPdoDupTree (ivp),
                                                        &INFO_VARDECS (arg_info),
                                                        &INFO_PREASSIGNSWL (arg_info),
                                                        TYmakeAKS (TYmakeSimpleType (
                                                                     T_int),
                                                                   SHcreateShape (0)));
                        } else {
                            DBUG_ASSERT (N_id == NODE_TYPE (ivp), "Expected N_id ivp");
                            z = ID_AVIS (ivp);
                        }
                    } else {
                        /* Vanilla variable */
                        rhs = AVIS_SSAASSIGN (ID_AVIS (idx));
                        DBUG_PRINT ("AWLFI", ("We lost the trail."));
                        z = NULL;
                    }
                    break;

                case N_prf:
                    DBUG_ASSERT ((N_id == NODE_TYPE (ivp)), "Expected N_id ivp");
                    switch (PRF_PRF (idx)) {
                    case F_add_SxS:
                        /* iv' = ( iv + x);   -->  iv = ( iv' - x);
                         * iv' = ( x  + iv);  -->  iv = ( iv' - x);
                         */
                        markiv = FindPrfParent2 (idx, arg_info);
                        if (0 != markiv) {
                            ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                            xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                            DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                            DBUG_ASSERT (N_id == NODE_TYPE (ivarg),
                                         "Expected N_id ivarg");
                            resavis = TBmakeAvis (TRAVtmpVarName ("tisadd"),
                                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                                             SHcreateShape (0)));
                            INFO_VARDECS (arg_info)
                              = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                            ids = TBmakeIds (resavis, NULL);
                            assgn
                              = TBmakeAssign (TBmakeLet (ids,
                                                         TCmakePrf2 (F_sub_SxS,
                                                                     TBmakeId (
                                                                       ID_AVIS (ivp)),
                                                                     TBmakeId (
                                                                       ID_AVIS (xarg)))),
                                              NULL);
                            INFO_PREASSIGNSWL (arg_info)
                              = TCappendAssign (INFO_PREASSIGNSWL (arg_info), assgn);
                            AVIS_SSAASSIGN (resavis) = assgn;
                            z = BuildInverseProjectionScalar (ivarg, arg_info,
                                                              TBmakeId (resavis));
                        }
                        break;

                    case F_sub_SxS:
                        /* Case 1: iv' = ( iv - x);   -->  iv = ( iv' + x);
                         * Case 2: iv' = ( x - iv);   -->  iv = ( x - iv');
                         *         Also, must swap minval/maxval.
                         */
                        resavis = TBmakeAvis (TRAVtmpVarName ("tissub"),
                                              TYmakeAKS (TYmakeSimpleType (T_int),
                                                         SHcreateShape (0)));
                        INFO_VARDECS (arg_info)
                          = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                        markiv = FindPrfParent2 (idx, arg_info);
                        if (0 != markiv) {
                            ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                            xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                            DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                            DBUG_ASSERT (N_id == NODE_TYPE (ivarg),
                                         "Expected N_id ivarg");
                            switch (markiv) {
                            case 1:
                                nprf = F_add_SxS;
                                id1 = TBmakeId (ID_AVIS (ivp));
                                id2 = TBmakeId (ID_AVIS (xarg));
                                break;

                            case 2:
                                nprf = F_sub_SxS;
                                id1 = TBmakeId (ID_AVIS (xarg));
                                id2 = TBmakeId (ID_AVIS (ivp));
                                INFO_FINVERSESWAP (arg_info)
                                  = !INFO_FINVERSESWAP (arg_info);
                                break;

                            default:
                                nprf = F_add_SxS;
                                id1 = NULL;
                                id2 = NULL;
                                DBUG_ASSERT (FALSE, "ivarg confusion");
                            }

                            ids = TBmakeIds (resavis, NULL);
                            assgn = TBmakeAssign (TBmakeLet (ids,
                                                             TCmakePrf2 (nprf, id1, id2)),
                                                  NULL);
                            INFO_PREASSIGNSWL (arg_info)
                              = TCappendAssign (INFO_PREASSIGNSWL (arg_info), assgn);
                            AVIS_SSAASSIGN (resavis) = assgn;
                            z = BuildInverseProjectionScalar (ivarg, arg_info,
                                                              TBmakeId (resavis));
                        }
                        break;

                    case F_mul_SxS:
                        markiv = FindPrfParent2 (idx, arg_info);
                        DBUG_ASSERT (FALSE, "Coding time for F_mul_SxS_");
                        if (COisConstant (PRF_ARG2 (idx))) {
                        }
                        break;

                    default:
                        DBUG_ASSERT (FALSE, "N_prf not recognized");
                        break;
                    }
                    break;

                case N_num:
                    DBUG_PRINT ("AWLFI", ("Found integer as source of iv'=%s",
                                          AVIS_NAME (ipavis)));
                    z = ipavis;
                    break;

                case N_array:
                    DBUG_ASSERT (1 == SHgetUnrLen (ARRAY_FRAMESHAPE (idx)),
                                 "Expected 1-element N_array");
                    DBUG_ASSERT (FALSE, "We are confused");
                    break;

                default:
                    DBUG_ASSERT (FALSE, ("Cannot chase iv'"));
                    break;
                }
            }
            pat = PMfree (pat);
        }
    }

    DBUG_ASSERT ((NULL == z) || N_avis == NODE_TYPE (z), "failed to gen avis");
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *PermuteIntersectElements( node *intr, node *zwithids,  node *zarr,
 *                                     info *arg_info)
 *
 * @brief: Permute and/or merge
 *
 * @params: intr: an N_exprs chain of an intersect calculation
 *          Its length matches that of iv in the sel( iv, producerWL)
 *          in the consumerWL.
 *
 *          zwithids: an N_ids chain, of the same shape as
 *          intr, comprising the WITHID_IDS related to the
 *          corresponding element of intr.
 *
 *          zarr: a writable copy of the resulting N_array's N_exprs list.
 *
 *          arg_info: your basic arg_info node.
 *
 *
 * @result: The permuted and/or confluenced N_exprs chain.
 *          Its length matches that of the WITHID_IDS in the
 *          consumerWL.
 *
 *          Effectively, this code performs, in the absence
 *          of duplicate zwithids entries:
 *
 *            z[ withidsids iota zwithids] = intr;
 *
 *          If there are duplicates, we insert min/max ops... FIXME
 *
 *****************************************************************************/
static node *
PermuteIntersectElements (node *intr, node *zwithids, node *zarr, info *arg_info)
{
    node *z;
    node *ids;
    int shpz;
    int shpids;
    int shpintr;
    int i;
    int idx;
    node *hole;
    node *el;

    DBUG_ENTER ("PermuteIntersectElements");

    z = zarr;
    ids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
    shpids = TCcountIds (ids);
    shpintr = TCcountExprs (intr);
    hole = IVEXImakeIntScalar (NOINVERSEPROJECTION, &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNSWL (arg_info));

    for (i = 0; i < shpids; i++) {
        z = TCputNthExprs (i, z, TBmakeId (hole));
    }

    for (i = 0; i < shpintr; i++) {
        idx = TClookupIdsNode (ids, TCgetNthIds (i, zwithids));
        DBUG_ASSERT (hole == ID_AVIS (TCgetNthExprsExpr (idx, z)),
                     "Time to code confluence stuff");
        z = TCputNthExprs (idx, z, TCgetNthExprsExpr (i, intr));
    }

    for (i = 0; i < shpids; i++) {
        el = TCgetNthExprsExpr (i, z);
        DBUG_ASSERT ((N_id != NODE_TYPE (el)) || (hole != ID_AVIS (el)),
                     "No holes wanted!");
    }

    shpz = TCcountExprs (z);
    DBUG_ASSERT (shpz == shpids, "Wrong boundary intersect shape");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjectionOne( node *arg_node, info *arg_info,
 *                                      node( *ivprime,
 *                                      node *intr)
 *
 * @brief   For a consumerWL with iv as WITHID_IDS,
 *          we have code of the form:
 *
 *             iv' = F( iv);
 *             el = producerWL( iv');
 *
 *         We are given iv' lb/ub as the WL intersection
 *         between a partition of the producerWL and the consumerWL.
 *
 *         This function generates code to compute F_1,
 *         the inverse of F, and applies it to iv', to compute
 *         a new iv, which is the bound of the WL intersection
 *         in the consumerWL space. I.e.,
 *
 *            iv = F_1( iv');
 *
 * @params: arg_node is an F_noteintersect node.
 *          ivprime: The iv' N_array node.
 *          intr: the WLintersect N_array node for lb or ub.
 *
 * @result: An N_avis node, which represents the result of
 *          mapping the WLintersect extrema back to consumerWL space,
 *          and performing any axis transposition and/or confluence,
 *          so that said result shape matches the shape of the
 *          consumerWL generators.
 *
 *          If we are unable to compute the inverse, we
 *          return NULL. This may occur if, e.g., we multiply
 *          iv by an unknown value, k. If we cannot show
 *          that k is non-zero, we do not have an inverse.
 *
 *****************************************************************************/
static node *
BuildInverseProjectionOne (node *arg_node, info *arg_info, node *ivprime, node *intr)
{
    node *z = NULL;
    node *zwithids = NULL;
    node *zw = NULL;
    node *iprime;
    node *ziavis;
    node *zarr;
    node *lb;
    node *el;
    node *withids;
    ntype *typ;
    pattern *pat;
    int dim;

    int ivindx;
    DBUG_ENTER ("BuildInverseProjectionOne");

    DBUG_ASSERT (N_array == NODE_TYPE (ivprime), "Expected N_array ivprime");
    DBUG_ASSERT (N_array == NODE_TYPE (intr), "Expected N_array intr");

    dim = SHgetUnrLen (ARRAY_FRAMESHAPE (ivprime));
    INFO_WITHIDS (arg_info) = NULL;

    for (ivindx = 0; ivindx < dim; ivindx++) {
        iprime = TCgetNthExprsExpr (ivindx, ARRAY_AELEMS (ivprime));
        el = TCgetNthExprsExpr (ivindx, ARRAY_AELEMS (intr));

        withids = getNthWithids (ivindx, AVIS_WITHIDS (ID_AVIS (PRF_ARG1 (arg_node))),
                                 arg_info);
        if (NULL != withids) {
            INFO_FINVERSESWAP (arg_info) = FALSE;
            ziavis = BuildInverseProjectionScalar (iprime, arg_info, el);
            if (NULL != ziavis) {
                if (N_avis == NODE_TYPE (ziavis)) {
                    AVIS_FINVERSESWAP (ziavis) = INFO_FINVERSESWAP (arg_info);
                    ziavis = TBmakeId (ziavis);
                }

                z = TCappendExprs (z, TBmakeExprs (ziavis, NULL));
                zwithids = TCappendIds (zwithids, TBmakeIds (withids, NULL));
                DBUG_ASSERT (NULL != INFO_WITHIDS (arg_info), ("No withids found"));
                zw = TCappendIds (zw, TBmakeIds (INFO_WITHIDS (arg_info), NULL));
            } else {
                DBUG_PRINT ("AWLFI", ("Failed to find inverse map for N_array"));
                ivindx = dim;
                z = (NULL != z) ? FREEdoFreeTree (z) : z;
                zwithids = (NULL != zwithids) ? FREEdoFreeTree (zwithids) : zwithids;
            }
        }
    }

    if (NULL != z) {
        lb = GENERATOR_BOUND1 (PART_GENERATOR (INFO_CONSUMERWLPART (arg_info)));
        pat = PMarray (1, PMAgetNode (&zarr), 1, PMskip (0));
        if (PMmatchFlat (pat, lb)) {
            if (CMPT_EQ == CMPTdoCompareTree (zwithids, zw)) {
                DBUG_PRINT ("AWLFI", ("they match"));
            } else {
                DBUG_PRINT ("AWLFI", ("they do not match"));
                DBUG_ASSERT (FALSE, ("Failing..."));
            }
            zarr = DUPdoDupTree (zarr); /* Make result shape match generator shape */
            z = PermuteIntersectElements (z, zw, ARRAY_AELEMS (zarr), arg_info);
            ARRAY_AELEMS (zarr) = z;
            typ = AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)));
            z = AWLFIflattenExpression (zarr, &INFO_VARDECS (arg_info),
                                        &INFO_PREASSIGNSWL (arg_info), TYcopyType (typ));
        }
        pat = PMfree (pat);
    }

    if (NULL != z) {
        global.optcounters.awlfi_expr += 1;
        DBUG_ASSERT (N_avis == NODE_TYPE (z), "Expected N_avis z");
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjections( node *arg_node, info *arg_info)
 *
 * @brief Given an F_noteintersect node,
 *        examine each set of intersects to compute the inverse projection
 *        of the intersection of the producerWL partition bounds
 *        with the consumerWL's index vector, iv', back into
 *        the consumerWL's partition bounds.
 *
 * @params: arg_node, arg_info.
 *
 * @result: Updated F_noteintersect node.
 *
 *          We may return non-updated node if we can not
 *          compute the inverse projection.
 *          This may occur if, e.g., we multiply
 *          iv by an unknown value, k. If we cannot show
 *          that k is non-zero, we do not have an inverse.
 *          We also have multiple calls to BuildInverseProjection,
 *          one per PWL axis, and a failure on any axis is cause
 *          for failure.
 *
 *****************************************************************************/
static bool
MatchExpr (node *arg, node *expr)
{
    bool z;

    DBUG_ENTER ("MatchExpr");
    z = (arg == expr);
    DBUG_RETURN (z);
}

static node *
BuildInverseProjections (node *arg_node, info *arg_info)
{
    node *zlb = NULL;
    node *zub = NULL;
    pattern *pat1;
    pattern *pat2;
    int numpart;
    int curpart;
    int curelidxlb;
    int curelidxub;
    node *ivprime;
    bool swaplb = FALSE;
    bool swapub = FALSE;
    node *tmp;
    node *intr;
    node *nlet;

    DBUG_ENTER ("BuildInverseProjections");

    numpart = (TCcountExprs (PRF_ARGS (arg_node)) / WLEPP);
    pat1 = PMarray (1, PMAgetNode (&intr), 1, PMskip (0));
    pat2 = PMarray (1, PMAgetNode (&ivprime), 1, PMskip (0));

    if (PMmatchFlat (pat2, PRF_ARG1 (arg_node))) {

        /* Iterate across intersects */
        for (curpart = 0; curpart < numpart; curpart++) {
            curelidxlb = WLPROJECTION1 (curpart);
            if (!isHasInverseProjection (
                  TCgetNthExprsExpr (curelidxlb, PRF_ARGS (arg_node)))) {
                intr = TCgetNthExprsExpr (WLINTERSECTION1 (curpart), PRF_ARGS (arg_node));
                if (PMmatchFlat (pat1, intr)) {
                    zlb = BuildInverseProjectionOne (arg_node, arg_info, ivprime, intr);
                    swaplb = INFO_FINVERSESWAP (arg_info);
                }
            }
            curelidxub = WLPROJECTION2 (curpart);
            if (!isHasInverseProjection (
                  TCgetNthExprsExpr (curelidxub, PRF_ARGS (arg_node)))) {
                intr = TCgetNthExprsExpr (WLINTERSECTION2 (curpart), PRF_ARGS (arg_node));
                intr = IVEXPadjustExtremaBound (ID_AVIS (intr), arg_info, -1,
                                                &INFO_VARDECS (arg_info),
                                                &INFO_PREASSIGNSWL (arg_info), "bip1");
                nlet = TCfilterAssignArg (MatchExpr, AVIS_SSAASSIGN (intr),
                                          &INFO_PREASSIGNSWL (arg_info));
                INFO_PREASSIGNSWL (arg_info)
                  = TCappendAssign (INFO_PREASSIGNSWL (arg_info), nlet);
                intr = TBmakeId (intr);
                if (PMmatchFlat (pat1, intr)) {
                    zub = BuildInverseProjectionOne (arg_node, arg_info, ivprime, intr);
                    swapub = INFO_FINVERSESWAP (arg_info);
                }
            }

            /* If we have both new bounds, update the F_intersect */
            if ((NULL != zlb) && (NULL != zub)) {
                DBUG_ASSERT ((swaplb == swapub), ("Swap confusion"));
                if (swaplb) {
                    tmp = zlb;
                    zlb = zub;
                    zub = tmp;
                }
                zlb = TBmakeId (zlb);
                PRF_ARGS (arg_node)
                  = TCputNthExprs (curelidxlb, PRF_ARGS (arg_node), zlb);
                zub = IVEXPadjustExtremaBound (zub, arg_info, 1, &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGNSWL (arg_info), "bip2");
                zub = TBmakeId (zub);
                PRF_ARGS (arg_node)
                  = TCputNthExprs (curelidxub, PRF_ARGS (arg_node), zub);
            }
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIgetWlWith( node *arg_node)
 *
 * @brief Given an N_id, return its N_with node, or NULL, if arg_node
 *        was not created by a WL.
 *        This skips over, e.g., afterguards in the way.
 *
 * @params: arg_node
 * @result: The N_with node of the WL
 *
 *****************************************************************************/
node *
AWLFIgetWlWith (node *arg_node)
{
    node *wl = NULL;
    node *z = NULL;
    pattern *pat;

    DBUG_ENTER ("AWLFIgetWlWith");

    pat = PMwith (1, PMAgetNode (&wl), 0);
    if (PMmatchFlatWith (pat, arg_node)) {
        z = wl;
    }

    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIfindWlId( node *arg_node)
 *
 * @brief: Determine if N_id arg_node was created by a WL.
 *
 * @param: arg_node: an N_id node
 *
 * @return: The N_id that created the WL, or NULL if the N_id node
 *          was not created by a WL, or if the WL generators
 *          are not N_array nodes.
 *
 *****************************************************************************/
node *
AWLFIfindWlId (node *arg_node)
{
    node *wlid = NULL;
    node *wl;
    node *z = NULL;
    pattern *pat;

    DBUG_ENTER ("AWLFIfindWlId");

    pat = PMvar (1, PMAgetNode (&wlid), 0);
    if (PMmatchFlatSkipGuards (pat, arg_node)) {
        wl = AWLFIgetWlWith (wlid);
        if (NULL != wl) {
            DBUG_PRINT ("AWLFI",
                        ("Found WL:%s: WITH_REFERENCED_FOLD=%d",
                         AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (wl)));
            z = wlid;
        }
    } else {
        DBUG_PRINT ("AWLFI", ("Did not find WL:%s", AVIS_NAME (ID_AVIS (arg_node))));
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool noDefaultPartition( node *arg_node)
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
 *                                  node **preassigns, ntype *restype)
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
 *   NB: It is the caller's responsibility to DUP the arg_node,
 *       if required.
 *
 *   @param  node *arg_node: a node to be flattened.
 *           node **vardecs: a pointer to a vardecs chain that
 *                           will have a new vardec appended to it.
 *           node **preassigns: a pointer to a preassigns chain that
 *                           will have a new assign appended to it.
 *           node *restype:  the ntype of TMP.
 *
 *   @return node *node:      N_avis node for flattened node
 *
 ******************************************************************************/
node *
AWLFIflattenExpression (node *arg_node, node **vardecs, node **preassigns, ntype *restype)
{
    node *avis;
    node *nas;

    DBUG_ENTER ("AWLFIflattenExpression");

    if (N_id == NODE_TYPE (arg_node)) {
        avis = ID_AVIS (arg_node);
    } else {
        avis = TBmakeAvis (TRAVtmpVar (), restype);
        *vardecs = TBmakeVardec (avis, *vardecs);
        nas = TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), arg_node), NULL);
        *preassigns = TCappendAssign (*preassigns, nas);
        AVIS_SSAASSIGN (avis) = nas;
        DBUG_PRINT ("AWLFI", ("Generated assign for %s", AVIS_NAME (avis)));
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilderOne( node *arg_node, info *arg_info,
 *                                      node *producerwlPart, int boundnum,
 *                                      node *ivminmax)
 *
 * @brief Build a pair of expressions for intersecting the bounds of
 *        a single producerWL partition with consumerWL index set, of the form:
 *
 *           iv' = (k*iv) + ivoffset;
 *           z = sel( iv', producerWL)
 *
 *        We determine the intersection of the consumerWL iv'
 *        index set with the producerWL's partition bounds this way:
 *
 *          genshp = _shape_A_(GENERATOR_BOUND1( producerwlPart));
 *
 *          (The next few lines compute:
 *           intlo = _max_VxV_( AVIS_MIN( iv'),
 *                              GENERATOR_BOUND1(producerwlPart));
 *
 *          but the optimizers are unable to compute common
 *          expressions such as:
 *
 *           _max_VxV_( iv, iv + 1)
 *
 *          Hence, we use the following:
 *
 *           xl = AVIS_MIN( iv');
 *           yl = GENERATOR_BOUND1( producerwlPart);
 *           d  = _sub_VxV_( xl, yl);
 *           zero = 0;
 *           p = _gt_VxS_( d, zero);
 *           p0intlo = _mesh_( p, xl, yl);
 *           )
 *
 *          ( Similar treatment for _min_VxV_ as above:
 *
 *          xh = AVIS_MAX( iv');
 *          yh = GENERATOR_BOUND2( producerwlPart);
 *          d'  = _sub_VxV_( xh, yh);
 *          p' = _lt_VxS_( d', zero);
 *          p0inthi = _mesh_( p', xh, yh);
 *          )
 *
 *          iv'' = _noteintersect(iv',
 *                                 p0bound1, p0bound2, p0intlo, p0inthi, p0int,
 *                                 p1bound1, p1bound2, p1intlo, p1inthi, p1int,
 *                                 ...);
 *          z = sel( iv'', producerWL)
 *
 *        where int1 evaluates to the lower bound of the intersection,
 *        and   int2 evaluates to the upper bound of the intersection
 *        and p0, p1... are the partitions of the producerWL.
 *
 *
 *        NB. ALL lower bounds precede all upper bounds in the
 *            _attach_extrema_ arguments.
 *
 *        NB. We need the producerWL partition bounds for at least
 *            two reasons: a producerWL partition may be split
 *            between the time we build this code and the time
 *            we look at the answer. When we find a potential
 *            hit, we have to go look up the partition bounds
 *            in the producerWL again, to ensure that the requisite
 *            partition still exists. Also, even if the partitions
 *            remain unchanged in size, their order may be
 *            shuffled, so we have to search for the right one.
 *
 *        NB. The null intersect computation is required so that
 *            we can distinguish it from non-null intersects,
 *             for the cases where cube slicing will be required.
 *
 * @params arg_node: the _sel_VxA_( idx, producerWL)
 * @params arg_info.
 * @params producerPart: An N_part of the producerWL.
 * @params boundnum: 1 for bound1,     or 2 for bound2
 * @params ivmin: AVIS_MIN( idx)
 * @params ivmax: AVIS_MAX( idx)
 *
 * @return An N_avis pointing to an N_exprs for the two intersect expressions,
 *         or NULL if we were unable to compute the inverse projection.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilderOne (node *arg_node, info *arg_info, node *producerPart,
                           int boundnum, node *ivmin, node *ivmax)
{
    node *pg;
    node *resavis;
    node *fncall;
    pattern *pat;
    node *gen = NULL;
    node *mmx;
    node *z;
    char *fun;
    int shp;

    DBUG_ENTER ("IntersectBoundsBuilderOne");

    pg = (boundnum == 1) ? GENERATOR_BOUND1 (PART_GENERATOR (producerPart))
                         : GENERATOR_BOUND2 (PART_GENERATOR (producerPart));

    pat = PMarray (1, PMAgetNode (&gen), 0);
    PMmatchFlatSkipExtrema (pat, pg);
    shp = SHgetUnrLen (ARRAY_FRAMESHAPE (gen));

    fun = (1 == boundnum) ? "partitionIntersectMax" : "partitionIntersectMin";
    mmx = (1 == boundnum) ? ID_AVIS (ivmin) : ID_AVIS (ivmax);

    DBUG_ASSERT (N_array == NODE_TYPE (gen), "Expected N_array gen");
    gen = WLSflattenBound (DUPdoDupTree (gen), &INFO_VARDECS (arg_info),
                           &INFO_PREASSIGNS (arg_info));

    fncall = DSdispatchFunCall (NSgetNamespace ("sacprelude"), fun,
                                TCcreateExprsChainFromAvises (2, gen, mmx));
    resavis = AWLFIflattenExpression (fncall, &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      TYmakeAKS (TYmakeSimpleType (T_int),
                                                 SHcreateShape (1, shp)));
    z = TUscalarizeVector (resavis, &INFO_PREASSIGNS (arg_info),
                           &INFO_VARDECS (arg_info));

    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectNullComputationBuilder( node *idxavismin,
 *                                            node *idxavismax,
 *                                            node *bound1, node *bound2,
 *                                            info *arg_info)
 *
 * @brief:  Emit symbiotic expression to determine if intersection
 *          of index vector set and partition bounds is null.
 *
 * @params: idxavismin: AVIS_MIN( consumerWL partition index vector)
 * @params: idxavismax: AVIS_MAX( consumerWL partition index vector)
 * @params: bound1: N_avis of GENERATOR_BOUND1 of producerWL partition.
 * @params: bound2: N_avis of GENERATOR_BOUND2 of producerWL partition.
 * @params: arg_info: your basic arg_info node
 *
 * @result: N_avis node of generated computation's boolean result.
 *
 *****************************************************************************/

static node *
IntersectNullComputationBuilder (node *idxavismin, node *idxavismax, node *bound1,
                                 node *bound2, info *arg_info)
{
    node *fncall;
    node *resavis;
    int shp;

    DBUG_ENTER ("IntersectNullComputationBuilder");

    DBUG_ASSERT (N_avis == NODE_TYPE (bound1), "Expected N_avis bound1");
    DBUG_ASSERT (N_avis == NODE_TYPE (bound2), "Expected N_avis bound2");

    fncall = DSdispatchFunCall (NSgetNamespace ("sacprelude"), "isPartitionIntersectNull",
                                TCcreateExprsChainFromAvises (4, ID_AVIS (idxavismin),
                                                              ID_AVIS (idxavismax),
                                                              bound1, bound2));

    shp = SHgetUnrLen (TYgetShape (AVIS_TYPE (bound1)));
    resavis = AWLFIflattenExpression (fncall, &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      TYmakeAKS (TYmakeSimpleType (T_bool),
                                                 SHcreateShape (1, shp)));

    DBUG_RETURN (resavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilder( node *arg_node, info *arg_info,
 *                                   node *producerwlPart, node *ivavis)
 *
 * @brief Build a set of expressions for intersecting the bounds of
 *        all producerWL partitions with consumerWL index set, of the form:
 *
 *           sel((k*iv) + ivoffset, producerWL)
 *
 *        We traverse all producerWL partitions, building a set of
 *        intersect calcuations for each of them.
 *
 * @params arg_node: The _sel_VxA( iv, producerWL).
 * @params arg_info.
 * @params boundnum: 1 for bound1,        2 for bound2
 * @params ivavis: the N_avis of the index vector used by the sel() operation.
 * @return An N_exprs node containing the ( 2 * # producerwlPart partitions)
 *         intersect expressions, in the form:
 *           p0bound1, p0bound2, p0intlo, p0inthi, p0nullint,
 *           p1bound1, p1bound2, p1intlo, p1inthi, p1nullint,
 *           ...
 *         If we are unable to produce an inverse mapping function
 *         for the CWL index vector, NULL.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilder (node *arg_node, info *arg_info, node *ivavis)
{
    node *expn = NULL;
    node *partn;
    node *minarr;
    node *maxarr;
    node *curavis;
    node *avismin;
    node *avismax;
    node *g1;
    node *g2;
    node *gen1 = NULL;
    node *gen2 = NULL;
    node *minex;
    node *maxex;
    node *minel;
    node *maxel;
    node *hole;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ("IntersectBoundsBuilder");

    partn = WITH_PART (INFO_PRODUCERWL (arg_info));
    pat1 = PMarray (1, PMAgetNode (&gen1), 0);
    pat2 = PMarray (1, PMAgetNode (&gen2), 0);

    while (NULL != partn) {
        g1 = GENERATOR_BOUND1 (PART_GENERATOR (partn));
        if (PMmatchFlatSkipExtrema (pat1, g1)) {
            g1 = gen1;
        }
        g1 = WLSflattenBound (DUPdoDupTree (g1), &INFO_VARDECS (arg_info),
                              &INFO_PREASSIGNSWL (arg_info));

        g2 = GENERATOR_BOUND2 (PART_GENERATOR (partn));
        if (PMmatchFlatSkipExtrema (pat2, g2)) {
            g2 = gen2;
        }
        g2 = WLSflattenBound (DUPdoDupTree (g2), &INFO_VARDECS (arg_info),
                              &INFO_PREASSIGNSWL (arg_info));

        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (g1), NULL));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (g2), NULL));

        avismin = IntersectBoundsBuilderOne (arg_node, arg_info, partn, 1,
                                             AVIS_MIN (ivavis), AVIS_MAX (ivavis));
        DBUG_ASSERT (NULL != avismin, "Bobbo is confused. No null, please");

        avismax = IntersectBoundsBuilderOne (arg_node, arg_info, partn, 2,
                                             AVIS_MIN (ivavis), AVIS_MAX (ivavis));
        DBUG_ASSERT (NULL != avismax, "Bobbo is confused. No null, please");

        /* Swap (some) elements of avismin and avismax, due to
         * subtractions of the form (k - iv) in the index vector
         * We have to move down the scalarized version of avismin/avismax
         * as a following swap would otherwise result in value error.
         */
        minarr = DUPdoDupTree (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (avismin))));
        avismin = AWLFIflattenExpression (minarr, &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGNS (arg_info),
                                          TYcopyType (AVIS_TYPE (avismin)));
        maxarr = DUPdoDupTree (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (avismax))));
        avismax = AWLFIflattenExpression (maxarr, &INFO_VARDECS (arg_info),
                                          &INFO_PREASSIGNS (arg_info),
                                          TYcopyType (AVIS_TYPE (avismax)));

        minarr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (avismin)));
        maxarr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (avismax)));
        minex = ARRAY_AELEMS (minarr);
        maxex = ARRAY_AELEMS (maxarr);
        while (NULL != minex) {
            minel = EXPRS_EXPR (minex);
            maxel = EXPRS_EXPR (maxex);
            DBUG_ASSERT (AVIS_FINVERSESWAP (ID_AVIS (minel))
                           == AVIS_FINVERSESWAP (ID_AVIS (maxel)),
                         "Expected matching swap values");
            if (AVIS_FINVERSESWAP (ID_AVIS (minel))) {
                DBUG_PRINT ("AWLFI",
                            ("Swapping F-inverse %s and %s", AVIS_NAME (ID_AVIS (minel)),
                             AVIS_NAME (ID_AVIS (maxel))));
                AVIS_FINVERSESWAP (ID_AVIS (minel)) = FALSE;
                AVIS_FINVERSESWAP (ID_AVIS (maxel)) = FALSE;
                EXPRS_EXPR (minex) = maxel;
                EXPRS_EXPR (maxex) = minel;
            }
            /* Denormalize AVIS_MAX */
            maxel = EXPRS_EXPR (maxex);
            maxel = IVEXPadjustExtremaBound (ID_AVIS (maxel), arg_info, 1,
                                             &INFO_VARDECS (arg_info),
                                             &INFO_PREASSIGNS (arg_info), "ibb1");
            FREEdoFreeNode (EXPRS_EXPR (maxex));
            EXPRS_EXPR (maxex) = TBmakeId (maxel);

            minex = EXPRS_NEXT (minex);
            maxex = EXPRS_NEXT (maxex);
            /* We have to move avismax after the normalization computation. */
            INFO_PREASSIGNS (arg_info)
              = TUmoveAssign (avismax, INFO_PREASSIGNS (arg_info));
        }

        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (avismin), NULL));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (avismax), NULL));

        curavis = IntersectNullComputationBuilder (AVIS_MIN (ivavis), AVIS_MAX (ivavis),
                                                   g1, g2, arg_info);
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));

        /* Reserve room for inverse projections */
        hole = IVEXImakeIntScalar (NOINVERSEPROJECTION, &INFO_VARDECS (arg_info),
                                   &INFO_PREASSIGNS (arg_info));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (hole), NULL));
        expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (hole), NULL));

        partn = PART_NEXT (partn);
    }
#ifdef FIXME // THis is probably garbage */
    if (INFO_NOFINVERSE (arg_info)) {
        DBUG_ASSERT (FALSE, "Bobbo has to write nullifier");
    }
#endif // FIXME // THis is probably garbage */

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (expn);
}

/** <!--********************************************************************-->
 *
 * @fn node *attachIntersectCalc( node *arg_node, info *arg_info)
 *
 * @brief  We are looking at the N_prf for:
 *
 *            z = _sel_VxA_( iv, producerWL);
 *         within the consumerWL, and idx now has extrema attached to it.
 *         We are now in a position to compute the intersection
 *         between idx's index set and that of the producerWL
 *         partitions.
 *
 *         We create new iv' from idx, to hold the result
 *         of the intersect computations that we build here.
 *
 *      See IntersectBoundsBuilderOne for details.
 *
 * @return: The N_avis for the newly created iv' node.
 *         If we are unable to compute the inverse mapping function
 *         from the WL intersection to the CWL partition bounds,
 *         we return ivavis.
 *
 *****************************************************************************/

static node *
attachIntersectCalc (node *arg_node, info *arg_info)
{
    node *ivavis;
    node *ivpavis;
    node *ivassign;
    int ivshape;
    node *intersectcalc;
    node *args;

    DBUG_ENTER ("attachIntersectCalc");

    DBUG_PRINT ("AWLFI", ("Inserting attachextrema computations"));

    /* Generate expressions for lower-bound intersection and
     * upper-bound intersection calculation.
     */
    ivavis = ID_AVIS (PRF_ARG1 (arg_node));
    intersectcalc = IntersectBoundsBuilder (arg_node, arg_info, ivavis);
    if (NULL != intersectcalc) {
        args = TBmakeExprs (TBmakeId (ivavis), NULL);
        args = TCappendExprs (args, intersectcalc); /* WLINTERSECT1/2 */

        ivshape = SHgetUnrLen (TYgetShape (AVIS_TYPE (ivavis)));
        ivpavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ivavis)),
                              TYeliminateAKV (AVIS_TYPE (ivavis)));

        INFO_VARDECS (arg_info) = TBmakeVardec (ivpavis, INFO_VARDECS (arg_info));
        ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivpavis, NULL),
                                            TBmakePrf (F_noteintersect, args)),
                                 NULL);
        INFO_PREASSIGNS (arg_info)
          = TCappendAssign (INFO_PREASSIGNS (arg_info), ivassign);
        AVIS_SSAASSIGN (ivpavis) = ivassign;

        PART_ISCONSUMERPART (INFO_CONSUMERWLPART (arg_info)) = TRUE;
        INFO_FINVERSEINTRODUCED (arg_info) = TRUE;
    } else {
        ivpavis = ivavis;
        INFO_PRODUCERWLFOLDABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (ivpavis);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisSingleOpWL( node *arg_node)
 *
 * @brief: predicate for determining if node is single-op WL
 *
 * @param: arg_node: an N_with
 *
 * @return: TRUE if only one result from WL
 *
 *****************************************************************************/
bool
AWLFIisSingleOpWL (node *arg_node)
{
    bool z;

    DBUG_ENTER ("AWLFIisSingleOpWL");

    switch (NODE_TYPE (WITH_WITHOP (arg_node))) {
    default:
        z = FALSE;
        DBUG_ASSERT (FALSE, "WITHOP confusion");
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
 * @fn bool checkProducerWLFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(idx, producerWL), contained
 *        within a consumerWL. We want to determine if
 *        producerWL is a WL that is a possible candidate for having some
 *        partition of itself folded into the consumerWL that
 *        contains the _sel_ expression.
 *
 *        This function concerns itself only with the characteristics
 *        of the entire producerWL: partition-dependent characteristics
 *        are determined by the AWLF phase later on.
 *
 *        The requirements for folding are:
 *
 *           - producerWL is the result of a WL.
 *
 *           - producerWL operator is a genarray or modarray.
 *
 *           - producerWL is a single-operator WL.
 *
 *           - producerWL has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *
 *           - producerWL is referenced only by the consumerWL.
 *             This is not strictly needed; it is only there
 *             to avoid potentially computing the same
 *             producerWL element more than once. FIXME: wl_needcount.c
 *             changes will relax this and restriction and base it
 *             on producerWL element computation cost.
 *
 *           - producerWL has a DEFDEPTH value (which I don't understand yet).
 *
 *           - consumerWL and producerWL generator bounds are
 *             the same shape.
 *
 *        There is an added requirement, that the index set of
 *        the consumerWL partition match, or be a subset, of the
 *        producerWL partition index set.
 *
 *        The expressions hanging from the attachextrema inserted
 *        by attachExtremaCalc are intended to determine if this
 *        requirement is met. CF, CVP, and other optimizations
 *        should simplify those expressions and give AWLF
 *        the information it needs to determine if the index
 *        set requirements are met.
 *
 *
 * @param _sel_VxA_( idx, producerWL)
 * @result If some partition of the producerWL may be a legal
 *         candidate for folding into the consumerWL, return true.
 *         Else false.
 *
 *****************************************************************************/
static bool
checkProducerWLFoldable (node *arg_node, info *arg_info)
{
    node *p;
    bool z;

    DBUG_ENTER ("checkProducerWLFoldable");

    p = INFO_PRODUCERWL (arg_info);
    z = NULL != p;
    if (z) {
        z = (AWLFIisSingleOpWL (p)) && (noDefaultPartition (p))
            && (WITHOP_NEXT (WITH_WITHOP (p)) == NULL)
            && ((NODE_TYPE (WITH_WITHOP (p)) == N_genarray)
                || (NODE_TYPE (WITH_WITHOP (p)) == N_modarray));

        if (z) {
            DBUG_PRINT ("AWLFI",
                        ("ProducerWL:%s is suitable for folding; WITH_REFERENCED_FOLD=%d",
                         AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))),
                         WITH_REFERENCED_FOLD (p)));
        } else {
            DBUG_PRINT ("AWLFI", ("ProducerWL %s is not suitable for folding.",
                                  AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node)))));
        }
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkConsumerWLFoldable( node *arg_node, info *arg_info)
 *
 * @brief We are looking at _sel_VxA_(iv, producerWL), contained
 *        within a consumerWL. We want to determine if
 *        the consumerWL is acceptable to have something folded into it.
 *
 *        We deem the prf foldable if iv directly from
 *        an _attachintersect_, or if iv has extrema.
 *        We also require that iv has a non-NULL AVIS_WITHIDS,
 *        so that we can trace back the iv to its withids,
 *        and thereby construct an inverse mapping function.
 *
 *        This may not be enough to guarantee foldability, but
 *        without extrema, we're stuck.
 *
 * @param _sel_VxA_( iv, producerWL) arg_node.
 *
 * @result True if the consumerWL (and the indexing expression)
 *         are acceptable for having another WL folded into it,
 *         else false.
 *
 *****************************************************************************/
static bool
checkConsumerWLFoldable (node *arg_node, info *arg_info)
{
    node *iv = NULL;
    node *ivavis;
    bool z;

    DBUG_ENTER ("checkConsumerWLFoldable");

    iv = PRF_ARG1 (arg_node);
    ivavis = ID_AVIS (iv);
    /* iv has extrema or F_noteintersect */
    z = ((NULL != AVIS_MIN (ivavis)) && (NULL != AVIS_MAX (ivavis)) &&
#ifdef FIXME // DEBUG
         (isAllWithidsPresent (arg_node)) &&
#else  // FIXME // DEBUG
         (TRUE) &&
#endif //  FIXME // DEBUG
         (NULL != INFO_CONSUMERWL (arg_info)))
        || AWLFIisHasNoteintersect (arg_node);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool checkBothFoldable( node *arg_node, info *arg_info)
 *
 * @brief Make any early checks we can that may show that
 *        the producerWL and consumerWL are not foldable.
 *
 *        We check that the shape of the generator bound of the producerWL
 *        matches the shape of the index vector in the consumerWL.
 *
 * @param N_prf: _sel_VxA_( idx, producerWL) arg_node.
 * @result True if the consumerWL and producerWL
 *         shapes are conformable for folding.
 *
 *****************************************************************************/
static bool
checkBothFoldable (node *arg_node, info *arg_info)
{
    node *bp;
    node *bc;
    node *pwl;
    int lev;
    int xrhob;
    int xrhoc;
    bool z = FALSE;
    pattern *pat;

    DBUG_ENTER ("checkBothFoldable");

    pwl = INFO_PRODUCERWL (arg_info);
    pat = PMarray (1, PMAgetNode (&bp), 1, PMskip (0));
    if ((NULL != pwl)
        && (PMmatchFlat (pat, GENERATOR_BOUND1 (PART_GENERATOR (WITH_PART (pwl)))))) {
        xrhob = SHgetUnrLen (ARRAY_FRAMESHAPE (bp));
        bc = AVIS_MIN (ID_AVIS (PRF_ARG1 (arg_node)));
        if (NULL != bc) {
            xrhoc = SHgetUnrLen (TYgetShape (AVIS_TYPE (ID_AVIS (bc))));
            z = xrhob == xrhoc;
        }
    }

    /* Both producerWL and consumerWL must be at same nesting level */
    lev = 1 + AVIS_DEFDEPTH (ID_AVIS (INFO_PRODUCERWLLHS (arg_info)));
    z = z && (lev == INFO_LEVEL (arg_info));

    if (z) {
        DBUG_PRINT ("AWLFI", ("Producer with-loop %s is foldable into %s",
                              AVIS_NAME (ID_AVIS (INFO_PRODUCERWLLHS (arg_info))),
                              AVIS_NAME (INFO_CONSUMERWLLHS (arg_info))));
    } else {
        DBUG_PRINT ("AWLFI", ("Producer with-loop %s is not foldable into %s",
                              AVIS_NAME (ID_AVIS (INFO_PRODUCERWLLHS (arg_info))),
                              AVIS_NAME (INFO_CONSUMERWLLHS (arg_info))));
    }

    pat = PMfree (pat);

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
    int optctr;

    DBUG_ENTER ("AWLFIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("AWLFI", ("Algebraic-With-Loop-Folding Inference in %s %s begins",
                              (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                              FUNDEF_NAME (arg_node)));

        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;
        optctr = global.optcounters.awlfi_expr;

        arg_node = INFNCdoInferNeedCountersOneFundef (arg_node, TR_awlfi);

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
        }

        if (global.optcounters.awlfi_expr != optctr) {
            arg_node = SimplifySymbioticExpression (arg_node, arg_info);
        }

        INFO_ONEFUNDEF (arg_info) = old_onefundef;

        DBUG_PRINT ("AWLFI", ("Algebraic-With-Loop-Folding Inference in %s %s ends",
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
 * @fn node AWLFIassign( node *arg_node, info *arg_info)
 *
 * @brief performs a top-down traversal.
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, producerWL).
 *
 *****************************************************************************/
node *
AWLFIassign (node *arg_node, info *arg_info)
{
    node *let;
    node *oldpreassigns;

    DBUG_ENTER ("AWLFIassign");

    oldpreassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    INFO_PREASSIGNS (arg_info) = oldpreassigns;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    let = ASSIGN_INSTR (arg_node);
    if ((N_let == NODE_TYPE (let)) && (N_with == NODE_TYPE (LET_EXPR (let)))
        && (INFO_PREASSIGNSWL (arg_info) != NULL)) {
        arg_node = TCappendAssign (INFO_PREASSIGNSWL (arg_info), arg_node);
        INFO_PREASSIGNSWL (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIwith( node *arg_node, info *arg_info)
 *
 * @brief applies AWLFI to a with-loop in a top-down manner.
 *
 *        When we return here, we will have counted all the
 *        references to any potential producerWL that we
 *        may want to deal with. We already know the producerWL
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
    INFO_CONSUMERWL (arg_info) = arg_node;
    INFO_ONEFUNDEF (arg_info) = INFO_ONEFUNDEF (old_arg_info);
    INFO_FINVERSEINTRODUCED (arg_info) = INFO_FINVERSEINTRODUCED (old_arg_info);

    DBUG_PRINT ("AWLFI", ("Resetting WITH_REFERENCED_CONSUMERWL, etc."));
    WITH_REFERENCED_FOLD (arg_node) = 0;
    WITH_REFERENCED_CONSUMERWL (arg_node) = NULL;
    WITH_REFERENCES_FOLDED (arg_node) = 0;

    WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);

    INFO_VARDECS (old_arg_info) = INFO_VARDECS (arg_info);
    INFO_FINVERSEINTRODUCED (old_arg_info) = INFO_FINVERSEINTRODUCED (arg_info);
    INFO_PREASSIGNSWL (old_arg_info) = INFO_PREASSIGNSWL (arg_info);

    arg_info = FreeInfo (arg_info);
    arg_info = old_arg_info;

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

    INFO_CONSUMERWLPART (arg_info) = arg_node;
    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVdo (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    INFO_CONSUMERWLPART (arg_info) = NULL;

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
 *   the number of references to the potential producerWL (WITH_REFERENCED_FOLD).
 *   We want to end up with WITH_REFERENCED_FOLD counting only
 *   references from a single WL. Hence, the checking on
 *   WITH_REFERENCED_CONSUMERWL
 *   AWLF will disallow folding if WITH_REFERENCED_FOLD != AVIS_NEEDCOUNT.
 *
 ******************************************************************************/
node *
AWLFIid (node *arg_node, info *arg_info)
{
    node *p;

    DBUG_ENTER ("AWLFIid");
    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
#ifdef NOISY
    DBUG_PRINT ("AWLFI", ("AWLFIid looking at %s", AVIS_NAME (ID_AVIS (arg_node))));
#endif // NOISY
    p = INFO_CONSUMERWL (arg_info);
    if ((NULL != p) && (NULL == WITH_REFERENCED_CONSUMERWL (p))) {
        /* First reference to this WL. */
        WITH_REFERENCED_CONSUMERWL (p) = INFO_CONSUMERWL (arg_info);
        WITH_REFERENCED_FOLD (p) = 0;
        DBUG_PRINT ("AWLFI", ("AWLFIid found first reference to %s",
                              AVIS_NAME (ID_AVIS (arg_node))));
    }

    /*
     * arg_node describes a WL, so
     * WITH_REFERENCED_FOLD( p) may have to be
     * incremented
     */
    if ((NULL != p) && (NULL != INFO_CONSUMERWL (arg_info))
        && (WITH_REFERENCED_CONSUMERWL (p) == INFO_CONSUMERWL (arg_info))) {
        (WITH_REFERENCED_FOLD (p))++;
        DBUG_PRINT ("AWLFI", ("AWLFIid incrementing WITH_REFERENCED_FOLD(%s) = %d",
                              AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (p)));
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
 *   Examine all _sel_VxA_(idx, producerWL) primitives to see if
 *   we may be able to fold producerWL here, assuming that
 *   the _sel_ is within a potential consumerWL.
 *
 *   We don't find out if all the conditions for folding can
 *   be met until this phase completes, so AWLF makes the
 *   final decision on folding.
 *
 *   When we do encounter an eligible sel() operation,
 *   with extrema available on PRF_ARG1, we construct
 *   an intersect computation between the now-available index
 *   set of idx, and each partition of the producerWL.
 *   These are attached to the sel() via an F_attachintersect
 *   guard.
 *
 *****************************************************************************/
node *
AWLFIprf (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ("AWLFIprf");

    if ((INFO_CONSUMERWLPART (arg_info) != NULL) && (PRF_PRF (arg_node) == F_sel_VxA)
        && (NODE_TYPE (PRF_ARG1 (arg_node)) == N_id)
        && (NODE_TYPE (PRF_ARG2 (arg_node)) == N_id)) {

        INFO_PRODUCERWLLHS (arg_info) = AWLFIfindWlId (PRF_ARG2 (arg_node));
        INFO_PRODUCERWL (arg_info) = AWLFIgetWlWith (INFO_PRODUCERWLLHS (arg_info));
        INFO_PRODUCERWLFOLDABLE (arg_info)
          = checkConsumerWLFoldable (arg_node, arg_info)
            && checkProducerWLFoldable (arg_node, arg_info)
            && checkBothFoldable (arg_node, arg_info);

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* Maybe detach invalid intersect calculations now. */
        if ((INFO_PRODUCERWLFOLDABLE (arg_info)) && (AWLFIisHasNoteintersect (arg_node))
            && (!isHasValidNoteintersect (arg_node, arg_info))) {
            arg_node = detachNoteintersect (arg_node);
        }

        /* Maybe attach intersect calculations now. */
        if ((INFO_PRODUCERWLFOLDABLE (arg_info))
            && (!AWLFIisHasNoteintersect (arg_node))) {
            z = attachIntersectCalc (arg_node, arg_info);
            if (z != ID_AVIS (PRF_ARG1 (arg_node))) {
                FREEdoFreeNode (PRF_ARG1 (arg_node));
                PRF_ARG1 (arg_node) = TBmakeId (z);
                DBUG_PRINT (
                  "AWLFI",
                  ("AWLFIprf inserted F_attachintersect into cwl=%s at _sel_VxA_",
                   AVIS_NAME (INFO_CONSUMERWLLHS (arg_info))));
            }
        }
    }

    if (F_noteintersect == PRF_PRF (arg_node)) {
        /* Project partitionIntersectMin/Max data back from PWL to CWL */
        arg_node = BuildInverseProjections (arg_node, arg_info);
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
 *   if op is modarray( producerWL), we increment
 *   WITH_REFERENCED_FOLD for the producerWL.
 *
 ******************************************************************************/
node *
AWLFImodarray (node *arg_node, info *arg_info)
{
    node *wl;

    DBUG_ENTER ("AWLFImodarray");

    arg_node = TRAVcont (arg_node, arg_info);

    if (N_modarray == NODE_TYPE (arg_node)) {
        INFO_PRODUCERWL (arg_info) = AWLFIfindWlId (MODARRAY_ARRAY (arg_node));
        wl = INFO_PRODUCERWL (arg_info);
        (WITH_REFERENCED_FOLD (wl))++;
        DBUG_PRINT ("AWLFI", ("AWLFImodarray: WITH_REFERENCED_FOLD(%s) = %d",
                              AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (arg_node))),
                              WITH_REFERENCED_FOLD (wl)));
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
    node *oldlet;
    node *oldconsumerwlname;

    DBUG_ENTER ("AWLFIlet");

    oldconsumerwlname = INFO_CONSUMERWLLHS (arg_info);
    INFO_CONSUMERWLLHS (arg_info) = IDS_AVIS (LET_IDS (arg_node));
    oldlet = INFO_LET (arg_info);
    INFO_LET (arg_info) = arg_node;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LET (arg_info) = oldlet;
    INFO_CONSUMERWLLHS (arg_info) = oldconsumerwlname;

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
