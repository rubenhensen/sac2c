/** <!--********************************************************************-->
 *
 * @defgroup awlfi Algebraic With-Loop Folding Inference
 *
 * @terminology:
 *        PWL:
 *        The WL that will no longer exist after AWLF completes. In the example
 *        below, A is the producerWL.
 *
 *        CWL:
 *        the WL that will absorb the block(s) from
 *        the producerWL. In the example below, B is the CWL.
 *
 *        We also allow the consumer sel() not to be a WL,
 *        so that expressions such as:  (iota(1000000)[42] will
 *        not generate array temps. This is the "nake consumer" case.
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
#include "traverse_optcounter.h"
#include "node_basic.h"
#include "print.h"

#define DBUG_PREFIX "AWLFI"
#include "debug.h"

#include "traverse.h"
#include "str.h"
#include "memory.h"
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
#include "reorder_equalityprf_arguments.h"
#include "transform_gtge_to_ltle.h"
#include "ElimSubDiv.h"
#include "UndoElimSubDiv.h"
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
#include "loop_invariant_removal.h"
#include "withloop_invariant_removal.h"
#include "cubeslicer.h"
#include "prfunroll.h"
#include "flattengenerators.h"
#include "indexvectorutils.h"
#include "deadcoderemoval.h"
#include "with_loop_utilities.h"
#include "set_withloop_depth.h"
#include "symbolic_constant_simplification.h"
#include "polyhedral_utilities.h"

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
    node *consumerwlids;     /* The current consumerWL N_ids */
    node *producerwl;        /* The producerWL N_with for this PWL */
    node *producerwllhs;     /* The producerWL LHS for this consumerWL */
    node *let;               /* The N_let node */
    node *withids;           /* the WITHID_IDS entry for an iv element */
    int defdepth;            /* The current nesting level of WLs. This
                              * is used to ensure that an index expression
                              * refers to an earlier WL in the same code
                              * block, rather than to a WL within this
                              * WL.
                              */
    bool producerWLFoldable :1; /* producerWL may be legally foldable. */
                             /* (If index sets prove to be OK)     */
    bool nofinverse :1;         /* no intersect to CWL mapping found */
    bool finverseswap :1;       /* If TRUE, must swp min/max */
    bool finverseintroduced :1; /* If TRUE, most simplify F-inverse */
    node *zwithids;          /* zwithids for GENERATOR_BOUNDs */
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
#define INFO_CONSUMERWLIDS(n) ((n)->consumerwlids)
#define INFO_PRODUCERWL(n) ((n)->producerwl)
#define INFO_PRODUCERWLLHS(n) ((n)->producerwllhs)
#define INFO_LET(n) ((n)->let)
#define INFO_WITHIDS(n) ((n)->withids)
#define INFO_DEFDEPTH(n) ((n)->defdepth)
#define INFO_PRODUCERWLFOLDABLE(n) ((n)->producerWLFoldable)
#define INFO_NOFINVERSE(n) ((n)->nofinverse)
#define INFO_FINVERSESWAP(n) ((n)->finverseswap)
#define INFO_FINVERSEINTRODUCED(n) ((n)->finverseintroduced)
#define INFO_ZWITHIDS(n) ((n)->zwithids)

static info *
MakeInfo (node *fundef)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = fundef;
    INFO_VARDECS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_PREASSIGNSWL (result) = NULL;
    INFO_CONSUMERWLPART (result) = NULL;
    INFO_CONSUMERWL (result) = NULL;
    INFO_CONSUMERWLIDS (result) = NULL;
    INFO_PRODUCERWL (result) = NULL;
    INFO_PRODUCERWLLHS (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_WITHIDS (result) = NULL;
    INFO_DEFDEPTH (result) = 0;
    INFO_PRODUCERWLFOLDABLE (result) = TRUE;
    INFO_NOFINVERSE (result) = FALSE;
    INFO_FINVERSESWAP (result) = FALSE;
    INFO_FINVERSEINTRODUCED (result) = FALSE;
    INFO_ZWITHIDS (result) = NULL;

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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Called for non-fundef node");

    arg_info = MakeInfo (arg_node);

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
 *              As an example of why this is needed, look at
 *              apex/iotan/iotan.sac when compiled without it!
 *
 * @param  arg_node: an N_fundef node.
 *
 * @result an updated N_fundef node.
 *
 * I do not like having this function present, but have no better
 * ideas at present.
 *
 ******************************************************************************/
static node *
SimplifySymbioticExpression (node *arg_node, info *arg_info)
{
    int i = 0;
#ifndef DBUG_OFF
    int ct = 0;
#endif
    bool done = false;

    TOC_SETUP(14, COUNT_DLIR, COUNT_WLIR, COUNT_INL,
              COUNT_CSE, COUNT_TUP, COUNT_CF,
              COUNT_VP, COUNT_REA, COUNT_AS,
              COUNT_AL, COUNT_DL, COUNT_ESD,
              COUNT_UESD, COUNT_DCR)

    TOC_SETCOUNTER (COUNT_WLIR, global.optcounters.wlir_expr)
    TOC_SETCOUNTER (COUNT_CSE, global.optcounters.cse_expr)
    TOC_SETCOUNTER (COUNT_ESD, global.optcounters.esd_expr)

    DBUG_ENTER ();

    if (global.optimize.dosse) {
        DBUG_PRINT_TAG ("SSE", "Entering opt micro-cycle for %s %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                        FUNDEF_NAME (arg_node));
        /* Loop over optimizers until we reach a fix point or give up */
        for (i = 0; i < global.max_optcycles; i++) {
#ifndef DBUG_OFF
            ct = i;
#endif

            DBUG_PRINT_TAG ("SSE", "Cycle iteration %d (fun %s %s) begins.", i,
                            (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                            FUNDEF_NAME (arg_node));

#ifndef DBUG_OFF
            if (global.check_frequency >= 4) {
                arg_node = PHrunConsistencyChecks (arg_node);
            }
#endif

            /* Invoke each opt */
            TOC_RUNOPT_TAG ("SSE", "DLIR", global.optimize.dodlir, COUNT_DLIR,
                            global.optcounters.dlir_expr, arg_node, DLIRdoLoopInvariantRemoval);
            TOC_RUNOPT_TAG ("SSE", "WLIR", global.optimize.dowlir, COUNT_WLIR,
                            global.optcounters.wlir_expr, arg_node, WLIRdoLoopInvariantRemoval);
            TOC_RUNOPT_TAG ("SSE", "INL", global.optimize.doinl, COUNT_INL, global.optcounters.inl_fun,
                            arg_node, INLdoInlining);
            TOC_RUNOPT_TAG ("SSE", "ISAA", global.optimize.dosaa, TOC_IGNORE, 0, arg_node,
                            ISAAdoInsertShapeVariables);
            TOC_RUNOPT_TAG ("SSE", "CSE", global.optimize.docse, COUNT_CSE, global.optcounters.cse_expr,
                            arg_node, CSEdoCommonSubexpressionElimination);
            TOC_RUNOPT_TAG ("SSE", "NTC", global.optimize.dotup, COUNT_TUP, global.optcounters.tup_upgrades,
                            arg_node, NTCdoNewTypeCheck);
            TOC_RUNOPT_TAG ("SSE", "EAT", global.optimize.dotup, TOC_IGNORE, 0, arg_node,
                            EATdoEliminateAlphaTypes);
            TOC_RUNOPT_TAG ("SSE", "EBT", global.optimize.dotup, TOC_IGNORE, 0, arg_node,
                            EBTdoEliminateBottomTypes);
            TOC_RUNOPT_TAG ("SSE", "DFC", TRUE, TOC_IGNORE, 0,  arg_node, DFCdoDispatchFunCalls);
            TOC_RUNOPT_TAG ("SSE", "CF", global.optimize.docf, COUNT_CF, global.optcounters.cf_expr,
                            arg_node, CFdoConstantFolding);
            TOC_RUNOPT_TAG ("SSE", "VP", global.optimize.dovp, COUNT_VP, global.optcounters.vp_expr,
                            arg_node, VPdoVarPropagation);
            TOC_RUNOPT_TAG ("SSE", "REA", global.optimize.dorea, COUNT_REA, global.optcounters.rea_expr,
                            arg_node, REAdoReorderEqualityprfArguments);
            TOC_RUNOPT_TAG ("SSE", "TGTL", global.optimize.dotgtl, COUNT_REA, global.optcounters.tgtl_expr,
                            arg_node, TGTLdoTransformGtgeToLtle);
            TOC_RUNOPT_TAG ("SSE", "ESD", global.optimize.dosde, COUNT_ESD, global.optcounters.esd_expr,
                            arg_node, ESDdoElimSubDiv);
            TOC_RUNOPT_TAG ("SSE", "AS", global.optimize.doas, COUNT_AS, global.optcounters.as_expr,
                            arg_node, ASdoArithmeticSimplification);
            TOC_RUNOPT_TAG ("SSE", "CF", global.optimize.docf, COUNT_CF, global.optcounters.cf_expr,
                            arg_node, CFdoConstantFolding);
            TOC_RUNOPT_TAG ("SSE", "CSE", global.optimize.docse, TOC_IGNORE, 0, arg_node,
                            CSEdoCommonSubexpressionElimination);
            TOC_RUNOPT_TAG ("SSE", "AL", global.optimize.doal, COUNT_AL, global.optcounters.al_expr,
                            arg_node, ALdoAssocLawOptimization);
            TOC_RUNOPT_TAG ("SSE", "DL", global.optimize.dodl, COUNT_DL, global.optcounters.dl_expr,
                            arg_node, DLdoDistributiveLawOptimization);
            TOC_RUNOPT_TAG ("SSE", "UESD", global.optimize.dosde, COUNT_UESD, global.optcounters.uesd_expr,
                            arg_node, UESDdoUndoElimSubDiv);
            TOC_RUNOPT_TAG ("SSE", "DCR", global.optimize.dodcr, COUNT_DCR,
                            global.optcounters.dead_var + global.optcounters.dead_expr, arg_node,
                            DCRdoDeadCodeRemoval);

            /* We do not count DCR, as it's merely for cleanup */
            DBUG_PRINT_TAG ("SSE",
                            "DLIR= %zu, WLIR= %zu, INL=%zu, CSE=%zu, TUP=%zu, CF=%zu, VP=%zu, "
                            "AS=%zu, AL=%zu, DL=%zu, "
                            "ESD=%zu, UESD=%zu, DCR=%zu",
                            (global.optcounters.dlir_expr - TOC_GETCOUNTER (COUNT_DLIR)),
                            (global.optcounters.wlir_expr - TOC_GETCOUNTER (COUNT_WLIR)),
                            (global.optcounters.inl_fun - TOC_GETCOUNTER (COUNT_INL)),
                            (global.optcounters.cse_expr - TOC_GETCOUNTER (COUNT_CSE)),
                            (global.optcounters.tup_upgrades - TOC_GETCOUNTER (COUNT_TUP)),
                            (global.optcounters.cf_expr - TOC_GETCOUNTER (COUNT_CF)),
                            (global.optcounters.vp_expr - TOC_GETCOUNTER (COUNT_VP)),
                            (global.optcounters.as_expr - TOC_GETCOUNTER (COUNT_AS)),
                            (global.optcounters.al_expr - TOC_GETCOUNTER (COUNT_AL)),
                            (global.optcounters.dl_expr - TOC_GETCOUNTER (COUNT_DL)),
                            /* The following are not for some reason in the fixpoint check
                               below: */
                            (global.optcounters.esd_expr - TOC_GETCOUNTER (COUNT_ESD)),
                            (global.optcounters.uesd_expr - TOC_GETCOUNTER (COUNT_UESD)),
                            ((global.optcounters.dead_var + global.optcounters.dead_expr)
                             - TOC_GETCOUNTER (COUNT_DCR)));

            /* Fix point check */
            TOC_COMPARE_RANGE (COUNT_DLIR, COUNT_DL, done)

            if (done) {
                i = global.max_optcycles;
            }
        }
        DBUG_PRINT_TAG ("SSE", "Stabilized at iteration %d for function %s", ct,
                        FUNDEF_NAME (arg_node));

    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn int AWLFIfindPrfParent2(...)
 *
 * @brief: Find parent node -- the one with a WITHID_IDS,
 *         of a dyadic scalar function
 *
 * @param: arg_node - an N_prf or N_id or N_num ...  of interest.
 *         withidids  - the WITH_IDS of the consumerWL partition of interesst.
 *         withid - the address of the withids we found, if any.
 *
 * @result: 0 = Not found
 *          1 = PRF_ARG1 traces back to a WITHID_IDS
 *          2 = PRF_ARG2 traces back to a WITHID_IDS
 *
 *****************************************************************************/
int
AWLFIfindPrfParent2 (node *arg_node, node *withidids, node **withid)
{
    int z = 0;
    node *arg = NULL;
    ;
    pattern *pat;
    size_t tcindex = 0;
    bool isIdsMember = false;
    node *id;

    DBUG_ENTER ();

    if (NULL != withidids) {
        pat = PMany (1, PMAgetNode (&arg), 0);

        switch (NODE_TYPE (arg_node)) {
        case N_prf:

            if (NULL != PRF_EXPRS2 (arg_node)) {
                tcindex = TClookupIdsNode (withidids, ID_AVIS (PRF_ARG2 (arg_node)), &isIdsMember);
                if (isIdsMember) {
                    z = 2;
                }
            }

            id = PRF_ARG1 (arg_node); /* Stupid _type_conv_() has N_type
                                       * as PRF_ARG1.
                                       */
            if (N_id == NODE_TYPE (id)) {
                tcindex = TClookupIdsNode (withidids, ID_AVIS (id), &isIdsMember);
                if (isIdsMember) {
                    z = 1;
                }
            }

            if ((0 == z) && (N_id == NODE_TYPE (id))
                && (0 != AWLFIfindPrfParent2 (id, withidids, withid))) {
                z = 1;
            }
            if ((0 == z) && (NULL != PRF_EXPRS2 (arg_node))
                && (0 != AWLFIfindPrfParent2 (PRF_ARG2 (arg_node), withidids, withid))) {
                z = 2;
            }
            break;

        case N_id:
            if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
                if (N_id == NODE_TYPE (arg)) {
                    tcindex = TClookupIdsNode (withidids, ID_AVIS (arg), &isIdsMember);
                    if (isIdsMember) {
                        z = 1;
                    }
                } else {
                    z = AWLFIfindPrfParent2 (arg, withidids, withid);
                }
            }
            break;

        default:
            break;
        }

        if ((NULL != withid) && (z != 0) && (isIdsMember)) {
            *withid = TCgetNthIds (tcindex, withidids);
        }

        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIfindNoteintersect( node *arg_node)
 *
 * @brief: Search for F_noteintersect on the iv of a sel(iv, X) function.
 *
 * @param: arg_node - a PRF_ARG node.
 *
 * @result: N_prf node of F_noteintersect, or NULL, if it
 *          does not exist.
 *
 *****************************************************************************/
node *
AWLFIfindNoteintersect (node *arg_node)
{
    node *z = NULL;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMprf (2, PMAisPrf (F_noteintersect), PMAgetNode (&z), 0);
    PMmatchFlat (pat, arg_node);
    pat = PMfree (pat);

    DBUG_ASSERT ((NULL == z) || (N_prf == NODE_TYPE (z)), "did not find N_prf");

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
    bool z;

    DBUG_ENTER ();

    DBUG_ASSERT ((F_idx_sel == PRF_PRF (arg_node)) || (F_sel_VxA == PRF_PRF (arg_node)),
                 "Expected sel/idx_sel");
    z = (NULL != AWLFIfindNoteintersect (PRF_ARG1 (arg_node)));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisValidNoteintersect()
 *
 * @brief: Predicate for presence of valid F_noteintersect on sel() statement.
 *
 * @param: arg_node - a consumerWL F_noteintersect N_prf node.
 *         pwld - the producer WL N_id referenced by the consumerWL.
 *
 * @result: TRUE if arg_node is a valid F_noteintersect.
 *          By "valid", we SHOULD mean that the PWL and the F_noteintersect
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
 *          However, for now, we check that the partition count for
 *          B (the PWL) matches the partition count for the F_noteintersect
 *          for C (the CWL).
 *
 *          We also check that the pwlid is, in fact, the WL referred to
 *          by the F_noteintersect.
 *
 *****************************************************************************/
bool
AWLFIisValidNoteintersect (node *arg_node, node *pwlid)
{
    bool z;
    size_t nexprs;
    size_t npart;

    DBUG_ENTER ();

    z = ((NULL != pwlid) && (N_prf == NODE_TYPE (arg_node))
         && (F_noteintersect == PRF_PRF (arg_node))
         && (ID_AVIS (pwlid)
             == ID_AVIS (TCgetNthExprsExpr (WLPRODUCERWL, PRF_ARGS (arg_node)))));

    if (z) {
        nexprs = (TCcountExprs (PRF_ARGS (arg_node)) - WLFIRST) / WLEPP;
        npart = TCcountParts (WITH_PART (AWLFIfindWL (pwlid)));
        z = (nexprs == npart);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIdetachNoteintersect( node *arg_node)
 *
 * @brief:  Discard the F_noteintersect. This just bypasses
 *          it, but DCR will remove it.
 *
 * @param: arg_node - an F_noteintersect
 *
 * @result: PRF_ARG1 of the F_noteintersect
 *
 *****************************************************************************/
node *
AWLFIdetachNoteintersect (node *arg_node)
{
    node *z;

    DBUG_ENTER ();

    DBUG_ASSERT (F_noteintersect == PRF_PRF (arg_node), "Expected F_intersect");
    z = DUPdoDupNode (PRF_ARG1 (arg_node));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *FakeUpConstantExtremum( node *elem, info *arg_info, int emax)
 *
 * @brief: If elem is a constant (AKV), generate the appropriate
 *         normalized constant extremum for it. Else NULL.
 *
 * @param: elem: the avis for an index vector or index scalar.
 *               or an N_num.
 *
 * @result: an N_avis for AVIS_MIN or AVIS_MAX for the constant, or NULL
 *          if elem is not constant.
 *
 *****************************************************************************/
static node *
FakeUpConstantExtremum (node *elem, info *arg_info, bool emax)
{
    constant *elminco = NULL;
    node *elavis = NULL;
    node *el;

    DBUG_ENTER ();

    elminco = COaST2Constant (elem);
    if (NULL != elminco) {
        el = COconstant2AST (elminco);
        elminco = COfreeConstant (elminco);
        elavis = FLATGexpression2Avis (el, &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGNS (arg_info), NULL);
        if (emax) { // normalize maxval
            elavis = IVEXPadjustExtremaBound (elavis, 1, &INFO_VARDECS (arg_info),
                                              &INFO_PREASSIGNS (arg_info), "fakecon");
        }
    }

    DBUG_RETURN (elavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *GenerateMinMaxForArray( node *ivavis, info *arg_info, bool emax)
 *
 * @brief: Fake up AVIS_MIN/MAX N_array for naked consumer and for
 *         non-in-block elements of other consumers.
 *
 *         We do NOT alter the AVIS_MIN/MAX values of ivavis or its
 *         N_array elements, as they may be used later.
 *
 *         Extrema for each element, elem, are built as follows:
 *           - constants: faked, as elem and elem+1
 *           - non-constants, not defined in block: elem and elem+1
 *           - defined-in-block with extrema: extrema are used directly
 *           - defined-in-block, with missing extrema: elem and/or elem+1
 *
 * @param: ivavis: an N_avis for an N_array, iv from _sel_VxA_( iv, PWL)
 *         emax: TRUE for AVIS_MAX, FALSE for AVIS_MIN
 *
 * @result: The generated AVIS_MIN/MAX for the N_array, unless we can't
 *          generate one, in which case we return NULL.
 *
 *          NB. results are NORMALIZED.
 *
 *
 *
 *****************************************************************************/
static node *
GenerateMinMaxForArray (node *ivavis, info *arg_info, bool emax)
{
    node *elem;
    node *aelems;
    node *narr = NULL;
    pattern *pat;
    node *ivid = NULL;
    node *exprs = NULL;
    node *exavis = NULL;
    bool badnews = FALSE;
    node *newarr = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (NULL != ivavis, "Must have non-NULL ivavis");
    ivid = TBmakeId (ivavis);
    pat = PMarray (1, PMAgetNode (&narr), 1, PMskip (0));
    if ((PMmatchFlat (pat, ivid))) {
        aelems = ARRAY_AELEMS (narr);
        while ((NULL != aelems) && !badnews) {
            exavis = NULL;
            elem = EXPRS_EXPR (aelems);
            aelems = EXPRS_NEXT (aelems);

            /* If constant element, fake up extrema for it */
            exavis = FakeUpConstantExtremum (elem, arg_info, emax);

            /* Non-constant, defined in this block, & extremum present: normal AWLF */
            if ((NULL == exavis)
                && (SWLDisDefinedInThisBlock (ID_AVIS (elem), INFO_DEFDEPTH (arg_info)))
                && (!emax) && (IVEXPisAvisHasMin (ID_AVIS (elem)))) {
                exavis = ID_AVIS (AVIS_MIN (ID_AVIS (elem))); // copy min
            }

            if ((NULL == exavis)
                && (SWLDisDefinedInThisBlock (ID_AVIS (elem), INFO_DEFDEPTH (arg_info)))
                && (emax) && (IVEXPisAvisHasMax (ID_AVIS (elem)))) {
                exavis = ID_AVIS (AVIS_MAX (ID_AVIS (elem))); // copy max
            }

            /* Non-constant, defined in other block. Fake up value. */
            if ((NULL == exavis)
                && (!SWLDisDefinedInThisBlock (ID_AVIS (elem),
                                               INFO_DEFDEPTH (arg_info)))) {
                exavis = ID_AVIS (elem); /* Not defined in this block. */
                if (emax) {
                    exavis = IVEXPadjustExtremaBound (exavis, 1, /* normalize */
                                                      &INFO_VARDECS (arg_info),
                                                      &INFO_PREASSIGNS (arg_info),
                                                      "nonconminmax");
                }
            }

            if (NULL == exavis) {
                /* Defined in this block, but
                 * no extrema yet. Try again later.
                 */
                badnews = TRUE;
            }
            if (!badnews) {
                exprs = TCappendExprs (exprs, TBmakeExprs (TBmakeId (exavis), NULL));
            }
        }

        if (badnews) {
            exprs = (NULL != exprs) ? FREEdoFreeTree (exprs) : NULL;
            DBUG_PRINT ("Could not build fake extrema for %s", AVIS_NAME (ivavis));
        } else {
            newarr = DUPdoDupTree (narr);
            if (NULL != ARRAY_AELEMS (newarr)) { // [:int] avoidance
                ARRAY_AELEMS (newarr) = FREEdoFreeTree (ARRAY_AELEMS (newarr));
                ARRAY_AELEMS (newarr) = exprs;
            }
            DBUG_PRINT ("Built fake extrema for %s", AVIS_NAME (ivavis));
        }
    }
    pat = PMfree (pat);
    ivid = FREEdoFreeNode (ivid);

    DBUG_RETURN (newarr);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisHasAllInverseProjections( node *arg_node)
 *
 * @brief: Predicate for presence of inverse projection of arg_node
 *
 * @param: arg_node - an F_noteintersect.
 *
 * @result: TRUE if arg_node has all inverse projections present.
 *
 *****************************************************************************/
bool
AWLFIisHasAllInverseProjections (node *arg_node)
{
    bool z = TRUE;
    size_t intersectListLim;
    size_t intersectListNo;
    node *proj1;
    node *proj2;

    DBUG_ENTER ();

    z = NULL != arg_node;
    intersectListNo = 0;
    intersectListLim = z ? (TCcountExprs (PRF_ARGS (arg_node)) - WLFIRST) / WLEPP : 0;

    while (z && (intersectListNo < intersectListLim)) {
        proj1 = TCgetNthExprsExpr (WLPROJECTION1 (intersectListNo), PRF_ARGS (arg_node));
        proj2 = TCgetNthExprsExpr (WLPROJECTION2 (intersectListNo), PRF_ARGS (arg_node));
        z = z && AWLFIisHasInverseProjection (proj1);
        z = z && AWLFIisHasInverseProjection (proj2);
        intersectListNo++;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisHasInverseProjection( node *arg_node)
 *
 * @brief: Predicate for presence of inverse projection of arg_node
 *
 * @param: arg_node - an F_noteintersect WLPROJECTION1/2 entry.
 *
 * @result: TRUE if arg_node has an inverse projection
 *          associated with it.
 *
 *****************************************************************************/
bool
AWLFIisHasInverseProjection (node *arg_node)
{
    bool z = TRUE;
    constant *co;

    DBUG_ENTER ();

#define NOINVERSEPROJECTION (-666)
    /* NOINVERSEPROJECTION is just a highly visible number that
     * is not a legal index
     */
    if (NULL != arg_node) {
        co = IVUTiV2Constant (arg_node);
        if (NULL != co) {
            z = (NOINVERSEPROJECTION != COconst2Int (co));
            co = COfreeConstant (co);
        }
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
 * @param  iprime: The current expression we are tracing,
 * @param  An N_id or an N_num.
 *           Or, it can be the N_avis of a WITHID_IDS node.
 *
 * @param  arg_info: Your basic arg_info node.
 *
 * @param  lbub[ ivindx]: The current inverse projection.
 *                This is normalized, a la WL bounds.
 *                If this is a recursive call, lbub is scalar, and
 *                is an N_avis.
 *
 * @result An N_avis node that gives the result of the F-inverse mapping
 *          function to take us from iv'->iv, or
 *          a N_num or else NULL, if no such node can be found.
 *
 *          Side effect: Set INFO_WITHIDS, if possible.
 *
 *****************************************************************************/

static node *
FlattenLbubel (node *lbub, size_t ivindx, info *arg_info)
{
    node *lbubelavis;
    node *lbubel;

    DBUG_ENTER ();

    if (N_avis == NODE_TYPE (lbub)) {
        lbubelavis = lbub;
    } else {
        lbubel = TCgetNthExprsExpr (ivindx, ARRAY_AELEMS (lbub));
        if (N_num == NODE_TYPE (lbubel)) {
            lbubelavis
              = FLATGexpression2Avis (DUPdoDupTree (lbubel), &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info), NULL);
        } else {
            lbubelavis = ID_AVIS (lbubel);
        }
    }

    DBUG_RETURN (lbubelavis);
}

static node *
BuildInverseProjectionScalar (node *iprime, info *arg_info, node *lbub, size_t ivindx)
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
    node *withidids;
    node *ipavis;
    size_t tcindex;
    bool isIdsMember;
    prf nprf;

    pattern *pat;

    DBUG_ENTER ();

    DBUG_PRINT ("Building inverse projection scalar");
    INFO_WITHIDS (arg_info) = NULL;
    switch (NODE_TYPE (iprime)) {
    default:
        DBUG_UNREACHABLE ("unexpected iprime NODE_TYPE");
        break;

    case N_num:
        z = DUPdoDupNode (iprime);
        break;

    case N_avis: /* iprime is WITHID_IDS - self-inverse */
        z = iprime;
        break;

    case N_id:
        ipavis = ID_AVIS (iprime);
        DBUG_PRINT ("Tracing %s", AVIS_NAME (ipavis));
        pat = PMany (1, PMAgetNode (&idx), 0);

        /* If we want to find withids, we have to skip BOTH extrema
         * and guards. E.g., in twopartoffsetWLAKD.sac
         */
        if (PMmatchFlatSkipExtremaAndGuards (pat, iprime)) {
            withidids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));

            switch (NODE_TYPE (idx)) {
            case N_id:
                tcindex = TClookupIdsNode (withidids, ID_AVIS (idx), &isIdsMember);
                if (isIdsMember) {
                    DBUG_PRINT ("Found %s as source of iv'=%s", AVIS_NAME (ID_AVIS (idx)),
                                AVIS_NAME (ipavis));
                    INFO_WITHIDS (arg_info) = TCgetNthIds (tcindex, withidids);
                    z = FlattenLbubel (lbub, ivindx, arg_info);
                } else {
                    /* Vanilla variable */
                    DBUG_PRINT ("We lost the trail.");
                    z = NULL;
                }
                break;

            case N_prf:
                switch (PRF_PRF (idx)) {
                case F_add_SxS:
                    /* iv' = ( iv + x);   -->  iv = ( iv' - x);
                     * iv' = ( x  + iv);  -->  iv = ( iv' - x);
                     */
                    markiv
                      = AWLFIfindPrfParent2 (idx, withidids, &INFO_WITHIDS (arg_info));
                    if (0 != markiv) {
                        ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                        xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                        DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                        DBUG_ASSERT (N_id == NODE_TYPE (ivarg), "Expected N_id ivarg");
                        resavis = TBmakeAvis (TRAVtmpVarName ("tisadd"),
                                              TYmakeAKS (TYmakeSimpleType (T_int),
                                                         SHcreateShape (0)));
                        INFO_VARDECS (arg_info)
                          = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                        ids = TBmakeIds (resavis, NULL);
                        assgn = TBmakeAssign (
                          TBmakeLet (ids,
                                     TCmakePrf2 (F_sub_SxS,
                                                 TBmakeId (FlattenLbubel (lbub, ivindx,
                                                                          arg_info)),
                                                 TBmakeId (ID_AVIS (xarg)))),
                          NULL);
                        INFO_PREASSIGNS (arg_info)
                          = TCappendAssign (INFO_PREASSIGNS (arg_info), assgn);
                        AVIS_SSAASSIGN (resavis) = assgn;
                        z = BuildInverseProjectionScalar (ivarg, arg_info, resavis,
                                                          ivindx);
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
                    markiv
                      = AWLFIfindPrfParent2 (idx, withidids, &INFO_WITHIDS (arg_info));
                    if (0 != markiv) {
                        ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                        xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                        DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                        DBUG_ASSERT (N_id == NODE_TYPE (ivarg), "Expected N_id ivarg");
                        switch (markiv) {
                        case 1:
                            nprf = F_add_SxS;
                            id1 = TBmakeId (FlattenLbubel (lbub, ivindx, arg_info));
                            id2 = TBmakeId (ID_AVIS (xarg));
                            break;

                        case 2:
                            nprf = F_sub_SxS;
                            id1 = TBmakeId (ID_AVIS (xarg));
                            id2 = TBmakeId (FlattenLbubel (lbub, ivindx, arg_info));
                            INFO_FINVERSESWAP (arg_info) = !INFO_FINVERSESWAP (arg_info);
                            break;

                        default:
                            nprf = F_add_SxS;
                            id1 = NULL;
                            id2 = NULL;
                            DBUG_UNREACHABLE ("ivarg confusion");
                        }

                        ids = TBmakeIds (resavis, NULL);
                        assgn
                          = TBmakeAssign (TBmakeLet (ids, TCmakePrf2 (nprf, id1, id2)),
                                          NULL);
                        INFO_PREASSIGNS (arg_info)
                          = TCappendAssign (INFO_PREASSIGNS (arg_info), assgn);
                        AVIS_SSAASSIGN (resavis) = assgn;
                        z = BuildInverseProjectionScalar (ivarg, arg_info, resavis,
                                                          ivindx);
                    }
                    break;

                case F_mul_SxS:
                    /* iv' = ( iv *  x);  -->  iv = ( iv' / x);
                     * iv' = ( x  * iv);  -->  iv = ( iv' / x);
                     */
                    markiv
                      = AWLFIfindPrfParent2 (idx, withidids, &INFO_WITHIDS (arg_info));
                    if (0 != markiv) {
                        ivarg = (2 == markiv) ? PRF_ARG2 (idx) : PRF_ARG1 (idx);
                        xarg = (2 == markiv) ? PRF_ARG1 (idx) : PRF_ARG2 (idx);
                        DBUG_ASSERT (N_id == NODE_TYPE (xarg), "Expected N_id xarg");
                        DBUG_ASSERT (N_id == NODE_TYPE (ivarg), "Expected N_id ivarg");
                        // Check for multiply by zero, just in case.
                        if (SCSisConstantZero (xarg)) {
                            DBUG_PRINT ("multiply by zero has no inverse");
                        } else {
                            resavis = TBmakeAvis (TRAVtmpVarName ("tismul"),
                                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                                             SHcreateShape (0)));
                            INFO_VARDECS (arg_info)
                              = TBmakeVardec (resavis, INFO_VARDECS (arg_info));
                            ids = TBmakeIds (resavis, NULL);
                            assgn = TBmakeAssign (
                              TBmakeLet (ids, TCmakePrf2 (F_div_SxS,
                                                          TBmakeId (
                                                            FlattenLbubel (lbub, ivindx,
                                                                           arg_info)),
                                                          TBmakeId (ID_AVIS (xarg)))),
                              NULL);
                            INFO_PREASSIGNS (arg_info)
                              = TCappendAssign (INFO_PREASSIGNS (arg_info), assgn);
                            AVIS_SSAASSIGN (resavis) = assgn;
                            z = BuildInverseProjectionScalar (ivarg, arg_info, resavis,
                                                              ivindx);
                        }
                    }
                    break;

                default:
                    if (!PMMisInGuards (PRF_PRF (idx))) {
                        /* idx may be something like an _idx_sel() that
                         * will disappear soon, due to CF */
                        DBUG_PRINT ("N_prf not recognized");
                        break;
                    } else { /* Guard may get removed in later saacyc */
                        DBUG_PRINT ("Skipping guard N_prf");
                        z = NULL;
                        break;
                    }
                }
                break;

            case N_num:
                DBUG_PRINT ("Found integer as source of iv'=%s", AVIS_NAME (ipavis));
                z = ipavis;
                break;

            case N_array:
                DBUG_ASSERT (1 == SHgetUnrLen (ARRAY_FRAMESHAPE (idx)),
                             "Expected 1-element N_array");
                DBUG_UNREACHABLE ("We are confused");
                break;

            default:
                DBUG_UNREACHABLE ("Cannot chase iv'");
                break;
            }
        }
        pat = PMfree (pat);
        break;
    }

    DBUG_PRINT ("Finished building inverse projection scalar");
    DBUG_ASSERT ((NULL == z) || (N_avis == NODE_TYPE (z)) || (N_num == NODE_TYPE (z)),
                 "failed to gen inverse");
    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIflattenScalarNode( node *arg_node, info *arg_info)
 *
 * @brief: Flatten a scalar node, if not already flattened.
 *
 * @param  arg_node: an N_num or N_id node
 * @param  arg_info: your basic arg_info node.
 *
 * @result  N_avis for possibly flattened node
 *
 *****************************************************************************/
node *
AWLFIflattenScalarNode (node *arg_node, info *arg_info)
{
    node *z;

    DBUG_ENTER ();

    if (N_num == NODE_TYPE (arg_node)) {
        z = FLATGexpression2Avis (DUPdoDupNode (arg_node), &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info), NULL);

    } else {
        DBUG_ASSERT (N_id == NODE_TYPE (arg_node), "Expected N_id");
        z = ID_AVIS (arg_node);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildAxisConfluence(...)
 *
 * @brief: Generate code to perform max/min on WL-intersection of
 *         two axes that are confluent. E.g., if the CWL is extracting
 *         the major diagonal from a 2-D PWL with two partitions,
 *         we may have something like:  _sel_VxA_( [i, i+1], PWL).
 *
 *         In that case, we take the maximum of the minimum bound
 *         and the minimum of the maximum bound.
 *
 * @param: idx: index into result of this element.
 *
 * @param   zarr: the N_array result we are overwriting.
 *
 * @param   zelnew: The inverse intersect, of the shape of the CWL bounds,
 *          as an element of the ARRAY_AELEMS N_exprs chain.
 *
 * @param   bndel: Current element of the generator bound.
 *                 This is used to determine whether to
 *                 overwrite current result element or if we need min/max.
 *
 * @param   boundnum: 0 if we are computing BOUND1,
 *                    1 if we are computing BOUND2
 *
 * @param   arg_info: your basic arg_info node.
 *
 * @result If zarr[ idx] is not used yet:
 *
 *              zarr[ idx] = zelnew;
 *
 *         If zarr[idx] is used, there is confluence, so we have:
 *
 *           zarr[ idx] = Max( zarr[ idx], zelnew) if 0=boundnum
 *           zarr[ idx] = Min( zarr[ idx], zelnew) if 1=boundnum
 *
 *****************************************************************************/
static node *
BuildAxisConfluence (node *zarr, size_t idx, node *zelnew, node *bndel, int boundnum,
                     info *arg_info)
{

    node *zprime;
    node *zelcur;
    const char *fn;
    node *fncall;
    node *newavis;
    node *curavis;

    DBUG_ENTER ();

    zelcur = TCgetNthExprsExpr (idx, zarr);
    if (CMPT_EQ == CMPTdoCompareTree (zelcur, bndel)) { /* not used yet */
        zprime = TCputNthExprs (idx, zarr, TBmakeId (ID_AVIS (zelnew)));
    } else {
        if (CMPT_EQ == CMPTdoCompareTree (zelcur, zelnew)) { /* No change */
            zprime = zarr;
        } else { /* confluence */
            fn = (0 == boundnum) ? "partitionMax" : "partitionMin";
            newavis = AWLFIflattenScalarNode (zelnew, arg_info);
            curavis = AWLFIflattenScalarNode (zelcur, arg_info);
            fncall
              = DSdispatchFunCall (NSgetNamespace (global.preludename), fn,
                                   TCcreateExprsChainFromAvises (2, curavis, newavis));
            zprime = FLATGexpression2Avis (fncall, &INFO_VARDECS (arg_info),
                                           &INFO_PREASSIGNS (arg_info),
                                           TYmakeAKS (TYmakeSimpleType (T_int),
                                                      SHcreateShape (0)));
            zprime = TCputNthExprs (idx, zarr, TBmakeId (zprime));
        }
    }

    DBUG_RETURN (zprime);
}

/** <!--********************************************************************-->
 *
 * @fn node *PermuteIntersectElements( node *zelu, node *zwithids,
                                       info *arg_info, int boundnum)
 *
 * @brief: Permute and/or merge inverse intersection elements, to
 *         construct CUBSL argument.
 *
 * @param zelu: an N_exprs chain of an intersect calculation
 *          Its length matches that of iv in the sel( iv, producerWL)
 *          in the consumerWL.
 *          These are in denormalized form.
 *
 * @param   zwithids: an N_ids chain, of the same shape as
 *          zelu, comprising the WITHID_IDS related to the
 *          corresponding element of zelu.
 *
 * @param   arg_info: your basic arg_info node.
 *
 * @param   boundnum: 0 if we are computing BOUND1,
 *                    1 if we are computing BOUND2
 *
 * @result  The permuted and/or confluenced N_avis for an
 *          N_exprs chain
 *          whose length matches that of the consumerWL GENERATOR_BOUND.
 *
 *          Effectively, this code performs, in the absence
 *          of duplicate zwithids entries:
 *
 *            zarr[ withidids iota zwithids] = zelu;
 *
 *          If there are duplicates, we insert min/max ops to handle
 *          the axis confluence
 *
 *          If there is no CWL, then we can not have any permutation,
 *          so the result is zelu.
 *
 *****************************************************************************/
static node *
PermuteIntersectElements (node *zelu, node *zwithids, info *arg_info, int boundnum)
{
    node *ids;
    size_t shpz;
    size_t shpids;
    size_t shpzelu;
    size_t i;
    size_t idx;
    bool isIdsMember;
    pattern *pat;
    node *bndarr = NULL;
    node *zarr;
    node *z;
    node *zelnew;
    size_t xrho = 0;
    node *bndel;
    ntype *typ;

    DBUG_ENTER ();

    if (NULL == INFO_CONSUMERWLPART (arg_info)) {
        xrho = TCcountExprs (zelu);
        DBUG_ASSERT (xrho >= 1, "Expected non-empty N_exprs node");
        typ = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0));
        z = TBmakeArray (typ, SHcreateShape (1, xrho), zelu);
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info),
                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                             SHcreateShape (1, xrho)));
    } else {
        z = PART_GENERATOR (INFO_CONSUMERWLPART (arg_info));
        if (0 == boundnum) {
            z = GENERATOR_BOUND1 (z);
        } else {
            z = GENERATOR_BOUND2 (z);
        }

        if (N_array == NODE_TYPE (z)) {
            xrho = SHgetUnrLen (ARRAY_FRAMESHAPE (z));
            z = FLATGexpression2Avis (DUPdoDupNode (z), &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      TYmakeAKS (TYmakeSimpleType (T_int),
                                                 SHcreateShape (1, xrho)));
        } else {
            z = ID_AVIS (z);
        }

        if (1 == boundnum) { /* Denormalize BOUND2 */
            z = IVEXPadjustExtremaBound (z, -1, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info), "pie");
        }

        z = TBmakeId (z);

        pat = PMarray (1, PMAgetNode (&bndarr), 1, PMskip (0));
        if (!PMmatchFlat (pat, z)) {
            DBUG_UNREACHABLE ("Expected N_array bounds");
        }
        DBUG_ASSERT (N_exprs == NODE_TYPE (zelu), "Expected N_exprs zelu");

        zarr = DUPdoDupTree (ARRAY_AELEMS (bndarr));

        shpz = TCcountExprs (zarr);
        ids = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
        shpids = TCcountIds (ids);
        DBUG_ASSERT (shpz == shpids, "Wrong boundary intersect shape");
        shpzelu = TCcountExprs (zelu);

        for (i = 0; i < shpzelu; i++) {
            idx = TClookupIdsNode (ids, TCgetNthIds (i, zwithids), &isIdsMember);
            if (isIdsMember) { /* skip places where idx is a constant, etc. */
                             /* E.g., sel( [ JJ, 2], PWL);                */
                zelnew = TCgetNthExprsExpr (i, zelu);
                bndel = TCgetNthExprsExpr (idx, ARRAY_AELEMS (bndarr));
                zarr = BuildAxisConfluence (zarr, idx, zelnew, bndel, boundnum, arg_info);
            }
        }

        z = DUPdoDupNode (bndarr);
        FREEdoFreeTree (ARRAY_AELEMS (z));
        ARRAY_AELEMS (z) = zarr;
        z = FLATGexpression2Avis (z, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info),
                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                             SHcreateShape (1, xrho)));

        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjectionOne(...)
 *
 * @brief   For a consumerWL with iv as WITHID_IDS,
 *          we have code of the form:
 *
 *             iv' = F( iv);
 *             el = producerWL( iv');
 *
 *         We are given iv' and lbub as the WL intersection
 *         between a partition of the producerWL and the consumerWL.
 *
 *         This function generates code to compute F_1,
 *         the inverse of F, and applies it to iv', to compute
 *         a new iv, which is the bound of the WL intersection
 *         in the consumerWL space. I.e.,
 *
 *            iv = F_1( iv');
 *
 * @param   arg_node is an F_noteintersect node.
 *
 * @param   arriv: The iv' N_array node, or the WITHID_VEC for the
 *                 consumerWL.
 *
 * @param   lbub: the WLintersect N_array node for lb or ub.
 *                This is denormalized, so that ub and lb are
 *                treated identically. I.e., if we have this WL generator:
 *                  ( [0] <= iv < [50])
 *                then we have these bounds:
 *                                 lb   ub
 *                  Normalized:    [0]  [50]
 *                  Denormlized:   [0]  [49]
 *
 * @result  An N_exprs node, which represents the result of
 *          mapping the WLintersect extrema back to consumerWL space,
 *          for those elements where we can do so.
 *
 *          If we are unable to compute the inverse, we
 *          return NULL. This may occur if, e.g., we multiply
 *          iv by an unknown value, k. If we cannot show
 *          that k is non-zero, we do not have an inverse.
 *
 *****************************************************************************/
static node *
BuildInverseProjectionOne (node *arg_node, info *arg_info, node *arriv, node *lbub)
{
    node *z = NULL;
    node *zw = NULL;
    node *iprime;
    node *ziavis;
    size_t dim;

    size_t ivindx;
    DBUG_ENTER ();

    dim = SHgetUnrLen (ARRAY_FRAMESHAPE (lbub));
    if (N_array != NODE_TYPE (arriv)) {
        DBUG_ASSERT (ID_AVIS (arriv)
                       == IDS_AVIS (
                            WITHID_VEC (PART_WITHID (INFO_CONSUMERWLPART (arg_info)))),
                     "arriv not WITHIDS_VEC!");
        arriv = WITHID_IDS (PART_WITHID (INFO_CONSUMERWLPART (arg_info)));
        dim = TCcountIds (arriv);
    }

    INFO_WITHIDS (arg_info) = NULL;

    for (ivindx = 0; ivindx < dim; ivindx++) {
        ziavis = NULL;
        if (N_array == NODE_TYPE (arriv)) {
            iprime = TCgetNthExprsExpr (ivindx, ARRAY_AELEMS (arriv));
        } else {
            iprime = TCgetNthIds (ivindx, arriv);
        }

        INFO_FINVERSESWAP (arg_info) = FALSE;
        ziavis = BuildInverseProjectionScalar (iprime, arg_info, lbub, ivindx);
        if (NULL != ziavis) {
            if (N_avis == NODE_TYPE (ziavis)) {
                AVIS_FINVERSESWAP (ziavis) = INFO_FINVERSESWAP (arg_info);
                ziavis = TBmakeId (ziavis);
            }

            z = TCappendExprs (z, TBmakeExprs (ziavis, NULL));
            zw = TCappendIds (zw, TBmakeIds (INFO_WITHIDS (arg_info), NULL));
        }
    }

    if (NULL != z) {
        global.optcounters.awlfi_expr += 1;
        INFO_ZWITHIDS (arg_info) = zw;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *BuildInverseProjections()
 *
 * @brief Given an F_noteintersect node,
 *        examine each set of intersects to compute the inverse projection
 *        of the intersection of the producerWL partition bounds
 *        with the consumerWL's index vector, iv', back into
 *        the consumerWL's partition bounds.
 *
 * @param  arg_node, arg_info.
 *
 * @result Updated F_noteintersect node.
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
 *          This section of code operates with denormalized extrema.
 *
 *****************************************************************************/
static bool
MatchExpr (node *arg, node *expr)
{
    bool z;

    DBUG_ENTER ();
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
    pattern *pat3;
    pattern *pat4;
    size_t numpart;
    size_t curpart;
    size_t curelidxlb;
    size_t curelidxub;
    bool swaplb = FALSE;
    bool swapub = FALSE;
    node *tmp;
    node *intrlb = NULL;
    node *intrub = NULL;
    node *arrlb; /* Denormalized */
    node *arrub; /* Denormalized */
    node *nlet;
    node *zwlb = NULL;
    node *zwub = NULL;
    node *zel = NULL;
    node *zeu = NULL;
    node *arriv = NULL;
    node *ivid;

    DBUG_ENTER ();

    numpart = (TCcountExprs (PRF_ARGS (arg_node)) - WLFIRST) / WLEPP;
    pat1 = PMarray (1, PMAgetNode (&arrlb), 1, PMskip (0));
    pat2 = PMarray (1, PMAgetNode (&arrub), 1, PMskip (0));

    pat3 = PMarray (1, PMAgetNode (&arriv), 1, PMskip (0));

    pat4 = PMany (1, PMAgetNode (&arriv), 0);

    /* ivid is either iv from sel(iv, PWL) or rebuilt value of same */
    ivid = TCgetNthExprsExpr (WLIVAVIS, PRF_ARGS (arg_node));
    /* Guard-skipping for the benefit of Bug #525. */
    if ((PMmatchFlatSkipGuards (pat3, ivid)) || (PMmatchFlat (pat4, ivid))) {
        /* Iterate across intersects */
        for (curpart = 0; curpart < numpart; curpart++) {
            curelidxlb = WLPROJECTION1 (curpart);
            curelidxub = WLPROJECTION2 (curpart);
            DBUG_PRINT ("Building inverse projection for %s, partition #%zu",
                        AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))), curpart);
            intrlb = TCgetNthExprsExpr (WLINTERSECTION1 (curpart), PRF_ARGS (arg_node));
            intrub = TCgetNthExprsExpr (WLINTERSECTION2 (curpart), PRF_ARGS (arg_node));

            // Denormalize upper bound before performing inverse projection.
            // Part of the reason for this is that we may swap bounds later on.
            intrub
              = IVEXPadjustExtremaBound (ID_AVIS (intrub), -1, &INFO_VARDECS (arg_info),
                                         &INFO_PREASSIGNS (arg_info), "biptop");
            intrub = TBmakeId (intrub);

            /* If naked consumer, inverse projection is identity */
            if (NULL == INFO_CONSUMERWLPART (arg_info)) {
                PRF_ARGS (arg_node) = TCputNthExprs (curelidxlb, PRF_ARGS (arg_node),
                                                     TBmakeId (ID_AVIS (intrlb)));
                PRF_ARGS (arg_node) = TCputNthExprs (curelidxub, PRF_ARGS (arg_node),
                                                     TBmakeId (ID_AVIS (intrub)));
            }

            if ((!AWLFIisHasInverseProjection (
                  TCgetNthExprsExpr (curelidxlb, PRF_ARGS (arg_node))))
                && (!AWLFIisHasInverseProjection (
                     TCgetNthExprsExpr (curelidxub, PRF_ARGS (arg_node))))) {
                if (!PMmatchFlat (pat2, intrub)) {
                    DBUG_UNREACHABLE ("lost the N_array for %s",
                                      AVIS_NAME (ID_AVIS (intrub)));
                }

                if ((PMmatchFlat (pat1, intrlb)) && (PMmatchFlat (pat2, intrub))
                    && (!WLUTisIdsMemberPartition (intrlb,
                                                   INFO_CONSUMERWLPART (arg_info)))
                    && (!WLUTisIdsMemberPartition (intrub,
                                                   INFO_CONSUMERWLPART (arg_info)))) {
                    zel = BuildInverseProjectionOne (arg_node, arg_info, arriv, arrlb);
                    zwlb = INFO_ZWITHIDS (arg_info);
                    swaplb = INFO_FINVERSESWAP (arg_info);

                    nlet
                      = TCfilterAssignArg (MatchExpr, AVIS_SSAASSIGN (ID_AVIS (intrub)),
                                           &INFO_PREASSIGNS (arg_info));
                    INFO_PREASSIGNS (arg_info)
                      = TCappendAssign (INFO_PREASSIGNS (arg_info), nlet);

                    zeu = BuildInverseProjectionOne (arg_node, arg_info, arriv, arrub);
                    zwub = INFO_ZWITHIDS (arg_info);
                    swapub = INFO_FINVERSESWAP (arg_info);
                }
            }

            if (NULL != INFO_CONSUMERWLPART (arg_info)) {

                /* If we have both new bounds, update the F_noteintersect */
                if ((NULL != zel) && (NULL != zeu)) {
                    DBUG_ASSERT (swaplb == swapub, "Swap confusion");
                    DBUG_ASSERT (N_exprs == NODE_TYPE (zel), "Expected N_exprs zel");
                    DBUG_ASSERT (N_exprs == NODE_TYPE (zeu), "Expected N_exprs zeu");
                    if (swaplb) {
                        //  DBUG_UNREACHABLE ("time2 code");
                        tmp = zel;
                        zel = zeu;
                        zeu = tmp;
                    }

                    DBUG_PRINT ("Building axis permute & confluence for %s, partn #%zu",
                                AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))), curpart);
                    zlb = PermuteIntersectElements (zel, zwlb, arg_info, 0);

                    zub = PermuteIntersectElements (zeu, zwub, arg_info, 1);
                    zub = IVEXPadjustExtremaBound (zub, 1, &INFO_VARDECS (arg_info),
                                                   &INFO_PREASSIGNS (arg_info), "bip5");

                    PRF_ARGS (arg_node)
                      = TCputNthExprs (curelidxlb, PRF_ARGS (arg_node), TBmakeId (zlb));
                    PRF_ARGS (arg_node)
                      = TCputNthExprs (curelidxub, PRF_ARGS (arg_node), TBmakeId (zub));
                }
            }
        }
        zel = NULL;
        zeu = NULL;
    } else {
        DBUG_UNREACHABLE ("Could not find N_array for %s",
                          AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
    }

    DBUG_PRINT ("Done b inverse projection for %s",
                AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIfindWL( node *arg_node)
 *
 * @brief Given an N_id, return its N_with node, or NULL, if arg_node
 *        was not created by a WL.
 *
 * @param  arg_node, perhaps
 * @result The N_with node of the WL
 *          NULL if arg_node is not an N_id
 *
 *****************************************************************************/
node *
AWLFIfindWL (node *arg_node)
{
    node *wl = NULL;
    node *z = NULL;
    pattern *pat;

    DBUG_ENTER ();

    if ((NULL != arg_node) && (N_id == NODE_TYPE (arg_node))) {
        pat = PMwith (1, PMAgetNode (&wl), 0);
        if (PMmatchFlatWith (pat, arg_node)) {
            z = wl;
        }

        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIfindWlId( node *arg_node)
 *
 * @brief: Determine if N_id arg_node was created by a WL.
 *
 * @param: arg_node: an N_id node, perhaps
 *
 * @return: The N_id that created the WL, or NULL if the N_id node
 *          was not created by a WL, or if the WL generators
 *          are not N_array nodes.
 *
 *          NULL if arg_node is not an N_id.
 *
 *****************************************************************************/
node *
AWLFIfindWlId (node *arg_node)
{
    node *wlid = NULL;
    node *wl;
    node *z = NULL;
    pattern *pat;

    DBUG_ENTER ();

    if (N_id == NODE_TYPE (arg_node)) {
        pat = PMvar (1, PMAgetNode (&wlid), 0);
        if (PMmatchFlatSkipGuards (pat, arg_node)) {
            wl = AWLFIfindWL (wlid);
            if (NULL != wl) {
                DBUG_PRINT ("Found WL:%s: WITH_REFERENCED_FOLD=%d",
                            AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (wl));
                z = wlid;
            }
        } else {
            DBUG_PRINT ("Did not find WL:%s", AVIS_NAME (ID_AVIS (arg_node)));
        }
        pat = PMfree (pat);
    }

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

    DBUG_ENTER ();

    partn = WITH_PART (arg_node);

    while ((NULL != partn) && z) {
        z = z & (N_default != NODE_TYPE (PART_GENERATOR (partn)));
        partn = PART_NEXT (partn);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn AWLFItakeDropIv( ...)
 *
 *   @brief Perform take on ivmin/ivmax, iv... This is required
 *          in the case where the PWL generates non-scalar cells.
 *
 *   @param: takect - the result shape/take count
 *   @param: arg_node - ivmin or ivmax.
 *   @return the flattened, possibly shortened, ivmin/ivmax.
 *
 ******************************************************************************/
node *
AWLFItakeDropIv (int takect, size_t dropct, node *arg_node, node **vardecs,
                 node **preassigns)
{
    node *z;
    node *arr = NULL;
    node *zavis;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&arr), 0);
    PMmatchFlatSkipExtrema (pat, arg_node);
    DBUG_ASSERT (N_array == NODE_TYPE (arr), "Expected N_array ivmin/ivmax");

    if (takect != SHgetUnrLen (ARRAY_FRAMESHAPE (arr))) {
        z = TCtakeDropExprs (takect, dropct, ARRAY_AELEMS (arr));
        z = DUPdoDupTree (z);
        z = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arr)), SHcreateShape (1, takect), z);
    } else {
        z = DUPdoDupTree (arr);
    }

    zavis = FLATGexpression2Avis (z, vardecs, preassigns,
                                  TYmakeAKS (TYmakeSimpleType (T_int),
                                             SHcreateShape (1, takect)));
    pat = PMfree (pat);

    DBUG_RETURN (zavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectBoundsBuilderOne( node *arg_node, info *arg_info,
 *                                      node *producerwlPart, int boundnum,
 *                                      node *ivmin, node *ivmax)
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
 *           ...
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
 *        NB. We need the producerWL partition bounds in the noteintersect
 *            for at least two reasons: a producerWL partition may be split
 *            between the time we build this code and the time
 *            we look at the answer. When we find a potential
 *            hit, we have to go look up the partition bounds
 *            in the producerWL again, to ensure that the requisite
 *            PWL partition still exists. Also, even if the partitions
 *            remain unchanged in size, their order may be
 *            shuffled, so we have to search for the right one.
 *
 *        NB. The null intersect computation is required so that
 *            we can distinguish it from non-null intersects,
 *             for the cases where cube slicing will be required.
 *
 * @param  arg_node: the _sel_VxA_( idx, producerWL)
 * @param  arg_info.
 * @param  producerPart: An N_part of the producerWL.
 * @param  boundnum: 1 for bound1,     or 2 for bound2
 * @param  ivmin, ivmax AVIS_MIN/MAX for iv in  _sel_( iv, PWL)
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
    node *z = NULL;
    prf prfminmax;
    node *polyint = NULL;
    int shp;

    DBUG_ENTER ();

    DBUG_ASSERT (N_array == NODE_TYPE (ivmin), "Expected N_array ivmin");
    DBUG_ASSERT (N_array == NODE_TYPE (ivmax), "Expected N_array ivmax");

    pg = (boundnum == 1) ? GENERATOR_BOUND1 (PART_GENERATOR (producerPart))
                         : GENERATOR_BOUND2 (PART_GENERATOR (producerPart));

    pat = PMarray (1, PMAgetNode (&gen), 0);
    PMmatchFlatSkipExtrema (pat, pg);
    DBUG_ASSERT (N_array == NODE_TYPE (gen), "Expected N_array gen");
    pat = PMfree (pat);
    shp = SHgetUnrLen (ARRAY_FRAMESHAPE (gen));
    mmx = (1 == boundnum) ? ivmin : ivmax;

    if (NULL == polyint) { // Use old way if polyhedral analysis does not work

        mmx = AWLFItakeDropIv (shp, 0, mmx, &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info));

        gen = WLSflattenBound (DUPdoDupTree (gen), &INFO_VARDECS (arg_info),
                               &INFO_PREASSIGNS (arg_info));
        prfminmax = (1 == boundnum) ? F_max_VxV : F_min_VxV;
        fncall = TCmakePrf2 (prfminmax, TBmakeId (gen), TBmakeId (mmx));
        resavis = FLATGexpression2Avis (fncall, &INFO_VARDECS (arg_info),
                                        &INFO_PREASSIGNS (arg_info),
                                        TYmakeAKS (TYmakeSimpleType (T_int),
                                                   SHcreateShape (1, shp)));
        z = TUscalarizeVector (resavis, &INFO_PREASSIGNS (arg_info),
                               &INFO_VARDECS (arg_info));
    } else {
        z = polyint;
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IntersectNullComputationBuilder( node *idxavismin,
 *                                            node *idxavismax,
 *                                            node *bound1, node *bound2,
 *                                            info *arg_info)
 *
 * @brief:  Emit expression to determine if intersection
 *          of index vector set and partition bounds is null:
 *
 *            isnull = _or_VxV_( _le_VxV_( idxavismax, bound1),
 *                               _ge_VxV_( idxavismin, bound2));
 *
 * @param   idxmin: AVIS_MIN( consumerWL partition index vector)
 * @param   idxmax: AVIS_MAX( consumerWL partition index vector)
 * @param   bound1: N_avis of GENERATOR_BOUND1 of producerWL partition.
 * @param   bound2: N_avis of GENERATOR_BOUND2 of producerWL partition.
 * @param   arg_info: your basic arg_info node
 *
 * @result  N_avis node of generated computation's boolean result.
 *
 *****************************************************************************/

static node *
IntersectNullComputationBuilder (node *idxmin, node *idxmax, node *bound1, node *bound2,
                                 info *arg_info)
{
    node *resavis;
    node *idxavismin;
    node *idxavismax;
    int shp;
    node *fncall1;
    node *fncall2;
    node *fncall3;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (bound1), "Expected N_avis bound1");
    DBUG_ASSERT (N_avis == NODE_TYPE (bound2), "Expected N_avis bound2");
    shp = SHgetUnrLen (TYgetShape (AVIS_TYPE (bound1)));
    idxavismin = AWLFItakeDropIv (shp, 0, idxmin, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info));
    idxavismax = AWLFItakeDropIv (shp, 0, idxmax, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info));

    fncall1 = TCmakePrf2 (F_le_VxV, TBmakeId (idxavismax), TBmakeId (bound1));
    fncall1 = FLATGexpression2Avis (fncall1, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), NULL);
    fncall2 = TCmakePrf2 (F_ge_VxV, TBmakeId (idxavismin), TBmakeId (bound2));
    fncall2 = FLATGexpression2Avis (fncall2, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), NULL);
    fncall3 = TCmakePrf2 (F_or_VxV, TBmakeId (fncall1), TBmakeId (fncall2));
    resavis = FLATGexpression2Avis (fncall3, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), NULL);
    resavis = TUscalarizeVector (resavis, &INFO_PREASSIGNS (arg_info),
                                 &INFO_VARDECS (arg_info));

    DBUG_RETURN (resavis);
}

/** <!--********************************************************************-->
 *
 * @fn node *Intersect1PartBuilder( node *idxavismin,
 *                                  node *idxavismax,
 *                                  node *bound1, node *bound2,
 *                                  info *arg_info)
 *
 * @brief:  Emit symbiotic expression predicate to determine if intersection
 *          between index vector set and partition bounds is
 *          restricted to a single producerWL partition.
 *          If so, then we can blindly perform the AWLF for
 *          this partition.
 *
 *          We would like This predicate to be:
 *
 *            z = ( idxavismin >= bound1) && ( idxavismax <= bound2)
 *
 *          NB. Unfortunately, that does help in cases such as this:
 *              catenate( VecAKD1, VecAKD2);
 *              If one of the vectors can be empty, then we may end up
 *              with:  idxavismax == bound2, whereas we otherwise have
 *                     idxavismax >  bound2.
 *
 * @param   idxavismin: AVIS_MIN( consumerWL partn index vector)
 * @param   idxavismax: normalized AVIS_MAX( consumerWL partn index vector)
 * @param   bound1: N_avis of GENERATOR_BOUND1 of producerWL partn.
 * @param   bound2: N_avis of GENERATOR_BOUND2 of
 *                  producerWL partn.
 * @param   arg_info: your basic arg_info node
 *
 * @result  N_avis node of generated computation's boolean result.
 *
 *****************************************************************************/
static node *
Intersect1PartBuilder (node *idxmin, node *idxmax, node *bound1, node *bound2,
                       info *arg_info)
{
    node *resavis;
    node *idxavismin;
    node *idxavismax;
    int shp;
    node *fncall1;
    node *fncall2;
    node *fncall3;

    DBUG_ENTER ();

    DBUG_ASSERT (N_avis == NODE_TYPE (bound1), "Expected N_avis bound1");
    DBUG_ASSERT (N_avis == NODE_TYPE (bound2), "Expected N_avis bound2");
    shp = SHgetUnrLen (TYgetShape (AVIS_TYPE (bound1)));
    idxavismin = AWLFItakeDropIv (shp, 0, idxmin, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info));
    idxavismax = AWLFItakeDropIv (shp, 0, idxmax, &INFO_VARDECS (arg_info),
                                  &INFO_PREASSIGNS (arg_info));

    fncall1 = TCmakePrf2 (F_ge_VxV, TBmakeId (idxavismin), TBmakeId (bound1));
    fncall1 = FLATGexpression2Avis (fncall1, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), NULL);
    fncall2 = TCmakePrf2 (F_le_VxV, TBmakeId (idxavismax), TBmakeId (bound2));
    fncall2 = FLATGexpression2Avis (fncall2, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), NULL);
    fncall3 = TCmakePrf2 (F_and_VxV, TBmakeId (fncall1), TBmakeId (fncall2));
    resavis = FLATGexpression2Avis (fncall3, &INFO_VARDECS (arg_info),
                                    &INFO_PREASSIGNS (arg_info), NULL);
    resavis = TUscalarizeVector (resavis, &INFO_PREASSIGNS (arg_info),
                                 &INFO_VARDECS (arg_info));

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
 * @param  arg_node: The _sel_VxA( iv, producerWL).
 * @param  arg_info.
 * @param  boundnum: 1 for bound1,        2 for bound2
 * @param  ivavis: the N_avis of the index vector used by the sel() operation.
 *
 * @return An N_exprs node containing the ( 2 * # producerwlPart partitions)
 *         intersect expressions, in the form:
 *           p0bound1, p0bound2, p0intlo, p0inthi, p0nullint,
 *           p1bound1, p1bound2, p1intlo, p1inthi, p1nullint,
 *           ...
 *         If we are unable to produce an inverse mapping function
 *         for the CWL index vector, NULL.
 *
 * @note: For a naked consumer, we use iv as idxmin, and iv+1 as idxmax.
 *
 *****************************************************************************/

static node *
IntersectBoundsBuilder (node *arg_node, info *arg_info, node *ivavis)
{
    node *expn = NULL;
    node *pwlp;
    node *minarr;
    node *maxarr;
    node *curavis;
    node *avismin;
    node *avismax;
    node *pwlpb1;
    node *pwlpb2;
    node *gen1 = NULL;
    node *gen2 = NULL;
    node *minex;
    node *maxex;
    node *minel;
    node *maxel;
    node *hole;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *ivmin = NULL;
    node *ivmax = NULL;
    node *ivminmax = NULL;
#ifndef DBUG_OFF
    char *cwlnm;
#endif

    DBUG_ENTER ();

    pat1 = PMarray (1, PMAgetNode (&gen1), 0);
    pat2 = PMarray (1, PMAgetNode (&gen2), 0);
    pat3 = PMarray (1, PMAgetNode (&ivminmax), 0);

    ivmin = GenerateMinMaxForArray (ivavis, arg_info, FALSE);
    ivmax = GenerateMinMaxForArray (ivavis, arg_info, TRUE);

    pwlp = WITH_PART (INFO_PRODUCERWL (arg_info));

    if ((NULL != ivmin) && (NULL != ivmax)) {
        while (NULL != pwlp) {
            pwlpb1 = GENERATOR_BOUND1 (PART_GENERATOR (pwlp));
            if (PMmatchFlatSkipExtrema (pat1, pwlpb1)) {
                pwlpb1 = gen1;
            }
            pwlpb1 = WLSflattenBound (DUPdoDupTree (pwlpb1), &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info));

            pwlpb2 = GENERATOR_BOUND2 (PART_GENERATOR (pwlp));
            if (PMmatchFlatSkipExtrema (pat2, pwlpb2)) {
                pwlpb2 = gen2;
            }
            pwlpb2 = WLSflattenBound (DUPdoDupTree (pwlpb2), &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info));

            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (pwlpb1), NULL));
            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (pwlpb2), NULL));

            avismin
              = IntersectBoundsBuilderOne (arg_node, arg_info, pwlp, 1, ivmin, ivmax);
            DBUG_ASSERT (NULL != avismin, "Expected non-NULL avismin");

            avismax
              = IntersectBoundsBuilderOne (arg_node, arg_info, pwlp, 2, ivmin, ivmax);

            DBUG_ASSERT (NULL != avismax, "Expected non_NULL avismax");

            /* Swap (some) elements of avismin and avismax, due to
             * subtractions of the form (k - iv) in the index vector
             * We have to move down the scalarized version of avismin/avismax
             * as a following swap would otherwise result in value error.
             */
            minarr = DUPdoDupTree (LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avismin))));
            avismin = FLATGexpression2Avis (minarr, &INFO_VARDECS (arg_info),
                                            &INFO_PREASSIGNS (arg_info),
                                            TYcopyType (AVIS_TYPE (avismin)));
            maxarr = DUPdoDupTree (LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avismax))));
            avismax = FLATGexpression2Avis (maxarr, &INFO_VARDECS (arg_info),
                                            &INFO_PREASSIGNS (arg_info),
                                            TYcopyType (AVIS_TYPE (avismax)));

            minarr = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avismin)));
            maxarr = LET_EXPR (ASSIGN_STMT (AVIS_SSAASSIGN (avismax)));
            minex = ARRAY_AELEMS (minarr);
            maxex = ARRAY_AELEMS (maxarr);
            while (NULL != minex) {
                minel = EXPRS_EXPR (minex);
                maxel = EXPRS_EXPR (maxex);
                DBUG_ASSERT (AVIS_FINVERSESWAP (ID_AVIS (minel))
                               == AVIS_FINVERSESWAP (ID_AVIS (maxel)),
                             "Expected matching swap values");
                if (AVIS_FINVERSESWAP (ID_AVIS (minel))) {
                    DBUG_PRINT ("Swapping F-inverse %s and %s",
                                AVIS_NAME (ID_AVIS (minel)), AVIS_NAME (ID_AVIS (maxel)));
                    AVIS_FINVERSESWAP (ID_AVIS (minel)) = FALSE;
                    AVIS_FINVERSESWAP (ID_AVIS (maxel)) = FALSE;
                    EXPRS_EXPR (minex) = maxel;
                    EXPRS_EXPR (maxex) = minel;
                }

                minex = EXPRS_NEXT (minex);
                maxex = EXPRS_NEXT (maxex);
            }

            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (avismin), NULL));
            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (avismax), NULL));

            curavis
              = IntersectNullComputationBuilder (ivmin, ivmax, pwlpb1, pwlpb2, arg_info);
            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));

            curavis = Intersect1PartBuilder (ivmin, ivmax, pwlpb1, pwlpb2, arg_info);
            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (curavis), NULL));

            /* Reserve room for inverse projections */
            hole = IVEXImakeIntScalar (NOINVERSEPROJECTION, &INFO_VARDECS (arg_info),
                                       &INFO_PREASSIGNS (arg_info));
            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (hole), NULL));
            expn = TCappendExprs (expn, TBmakeExprs (TBmakeId (hole), NULL));

            pwlp = PART_NEXT (pwlp);
        }

#ifndef DBUG_OFF
        cwlnm = (NULL != INFO_CONSUMERWLIDS (arg_info))
                  ? AVIS_NAME (IDS_AVIS (INFO_CONSUMERWLIDS (arg_info)))
                  : "(naked consumer)";
#endif
        DBUG_PRINT ("Built bounds intersect computations for consumer-WL %s", cwlnm);
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (expn);
}

/** <!--********************************************************************-->
 *
 * @fn node *attachIntersectCalc( node *arg_node, info *arg_info, node *ivavis)
 *
 * @brief  We are looking at a sel() N_prf for:
 *
 *            iv = [ i, j, k];
 *            z = _sel_VxA_( iv, producerWL);
 *
 *         within the consumerWL (or not), and iv now has extrema attached to it,
 *         or iv is constant.
 *
 *         Alternately, we have
 *
 *            offset = idxs2offset( shape( producerWL), i, j, k...);
 *            z = _idx_sel_( offset, producerWL);
 *
 *         and the scalar indices i, j, k have extrema attached to them, or are
 *         constant.
 *
 *         If so, we are in a position to compute the intersection
 *         between iv's index set and that of the producerWL
 *         partitions.
 *
 *         We end up with:
 *
 *            iv = [ i, j, k];
 *            iv' = _noteintersect( iv, blah....);
 *            z = _sel_VxA_( iv', producerWL);
 *
 *         We create new iv'/offset' from idx, to hold the result
 *         of the intersect computations that we build here.
 *
 *      See IntersectBoundsBuilderOne for details.
 *
 *      ivavis is either an avis, from iv in a _sel_VxA_( iv, producerWL),
 *      or from a rebuilt iv from an _idx_sel( offset, producerWL).
 *
 * @return: The N_avis for the newly created iv'/offset' node.
 *         If we are unable to compute the inverse mapping function
 *         from the WL intersection to the CWL partition bounds,
 *         we return ivavis.
 *
 *****************************************************************************/

static node *
attachIntersectCalc (node *arg_node, info *arg_info, node *ivavis)
{
    node *ivpavis;
    node *ivassign;
    node *intersectcalc = NULL;
    node *args;
    ntype *ztype;
#ifndef DBUG_OFF
    const char *nm;
#endif
    node *noteint;

    DBUG_ENTER ();

#ifndef DBUG_OFF
    nm = (NULL != INFO_CONSUMERWLIDS (arg_info))
           ? AVIS_NAME (IDS_AVIS (INFO_CONSUMERWLIDS (arg_info)))
           : "(no consumer WL)";
#endif
    DBUG_PRINT ("Inserting attachextrema for producerWL %s into consumerWL %s",
                AVIS_NAME (ID_AVIS (INFO_PRODUCERWLLHS (arg_info))), nm);

    intersectcalc = IntersectBoundsBuilder (arg_node, arg_info, ivavis);

    if (NULL != intersectcalc) {
        /* WLARGNODE */
        args = TBmakeExprs (TBmakeId (ID_AVIS (PRF_ARG1 (arg_node))), NULL);
        /* WLPRODUCERWL */
        args = TCappendExprs (args, TBmakeExprs (TBmakeId (ID_AVIS (
                                                   INFO_PRODUCERWLLHS (arg_info))),
                                                 NULL));
        /* WLIVAVIS */
        args = TCappendExprs (args, TBmakeExprs (TBmakeId (ivavis), NULL));
        /* WLINTERSECT1/2 */
        args = TCappendExprs (args, intersectcalc);

        ztype = AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)));
        ivpavis
          = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ivavis)), TYeliminateAKV (ztype));

        INFO_VARDECS (arg_info) = TBmakeVardec (ivpavis, INFO_VARDECS (arg_info));
        noteint = TBmakePrf (F_noteintersect, args);
        PRF_NOTEINTERSECTINSERTIONCYCLE (noteint) = global.cycle_counter;
        ivassign = TBmakeAssign (TBmakeLet (TBmakeIds (ivpavis, NULL), noteint), NULL);

        INFO_PREASSIGNS (arg_info)
          = TCappendAssign (INFO_PREASSIGNS (arg_info), ivassign);
        AVIS_SSAASSIGN (ivpavis) = ivassign;

        if (NULL != INFO_CONSUMERWLPART (arg_info)) {
            PART_ISCONSUMERPART (INFO_CONSUMERWLPART (arg_info)) = TRUE;
        }
        INFO_FINVERSEINTRODUCED (arg_info) = TRUE;
    } else {
        ivpavis = ID_AVIS (PRF_ARG1 (arg_node));
        INFO_PRODUCERWLFOLDABLE (arg_info) = FALSE;
    }

    DBUG_RETURN (ivpavis);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIcheckProducerWLFoldable( node *arg_node)
 *
 * @brief We are looking at an N_id, arg_node, that may point to a WL, p.
 *        We want to determine if p is a WL that is a
 *        potential candidate for having some
 *        partition of itself folded into the consumerWL that
 *        contains the _sel_ expression that we were called from.
 *
 *        This function concerns itself only with the characteristics
 *        of the entire p: partition-dependent characteristics
 *        are determined by the AWLF phase later on.
 *
 *        The requirements for folding are:
 *
 *           - p is the result of a WL.
 *
 *           - p operator is a genarray or modarray.
 *
 *           - p is a single-operator WL.
 *
 *           - p has an SSAASSIGN (means that WITH_IDs are NOT legal),
 *
 *           - p is referenced only by the consumerWL.
 *             This is not strictly needed; it is only there
 *             to avoid potentially computing the same
 *             producerWL element more than once. FIXME: wl_needcount.c
 *             changes will relax this and restriction and base it
 *             on p element computation cost.
 *
 *           - p and the consumer(WL)  have a DEFDEPTH value,
 *             which the number of levels of WLs in which they are
 *             contained. AWLFI can proceed in these two cases:
 *
 *                AVIS_DEPTH( cwl) == ( 1 + AVIS_DEPTH( pwl))
 *                 This is the normal case of composition,
 *
 *                AVIS_DEPTH( cwl) == (     AVIS_DEPTH( pwl))
 *                 This is the "naked consumer" case.
 *
 *           - consumerWL and p generator bounds are
 *             the same shape.
 *
 *        There is an added requirement, that the index set of
 *        the consumerWL partition match, or be a subset, of
 *        p's partition index set.
 *
 *        The expressions hanging from the attachextrema inserted
 *        by attachExtremaCalc are intended to determine if this
 *        requirement is met. CF, CVP, and other optimizations
 *        should simplify those expressions and give AWLF
 *        the information it needs to determine if the index
 *        set requirements are met.
 *
 *
 * @param  arg_node: an N_id, hopefully pointing to a producer WL
 *
 * @result If any partition of p may be a legal
 *         candidate for folding into a consumerWL, return true.
 *         Else false.
 *
 *****************************************************************************/
bool
AWLFIcheckProducerWLFoldable (node *arg_node)
{
    bool z = FALSE;
    node *cellavis;
    node *pcode;
    ntype *typ;
    node *p;
#ifndef DBUG_OFF
    const char *nm;
#endif

    DBUG_ENTER ();

    p = AWLFIfindWL (arg_node);
    if ((NULL != p) && (WLUTisSingleOpWl (p)) && (noDefaultPartition (p))
        && (WITHOP_NEXT (WITH_WITHOP (p)) == NULL)
        && ((NODE_TYPE (WITH_WITHOP (p)) == N_genarray)
            || (NODE_TYPE (WITH_WITHOP (p)) == N_modarray))) {

        pcode = PART_CODE (WITH_PART (p));
        cellavis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (pcode)));
        typ = AVIS_TYPE (cellavis); /* Cell must be scalar */
        z = (!TYisAUD (typ)) && (0 == TYgetDim (typ));
    }

    if (z) {
        DBUG_PRINT ("PWL %s is OK for folding; WITH_REFERENCED_FOLD=%d",
                    AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (p));
    } else {
#ifndef DBUG_OFF
        nm = (NULL != p) ? AVIS_NAME (ID_AVIS (arg_node)) : "(not a WL) ";
#endif
        DBUG_PRINT ("PWL %s is not OK for folding", nm);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisCanAttachIntersectCalc( node *arg_node, node *ivavis
 *                                    info *arg_info)
 *
 * @brief  TRUE if iv/ivavis are in suitable shape that we can
 *         attach intersect calculations for the sel().
 *
 * @param arg_node:  _sel_VxA_( iv, producerWL)
 *        ivavis:    N_avis for iv's predecessor N_array
 *
 * @result boolean
 *
 *****************************************************************************/
bool
AWLFIisCanAttachIntersectCalc (node *arg_node, node *ivavis, info *arg_info)
{
    bool z = FALSE;
    node *narr;
    pattern *pat;
    node *ivid;
    node *aelems;
    node *elem;
    node *avis;

    DBUG_ENTER ();

    if (NULL != ivavis) {
        /*
         * Now we have to get fancy: we want to allow a mix of three
         * element types in the N_array: AKV, both-extrema-present,
         * and element defined-outside-the-WL.
         *
         * The latter appears in AWLF unit test realrelaxAKDLowerA.sac,
         * where we have:
         *
         *   A = genarray( [ m, n], 42);
         *   lower_A = drop( [m-1,0], A);
         *
         * In this code, m-1 is NOT constant, but neither does it have
         * extrema. Since it is WL-invariant, it is defined outside the
         * N_code block for the WL.
         *
         */

        z = TRUE;
        pat = PMarray (1, PMAgetNode (&narr), 1, PMskip (0));
        ivid = TBmakeId (ivavis);
        if (z && PMmatchFlat (pat, ivid)) {
            aelems = ARRAY_AELEMS (narr);
            while (NULL != aelems) {
                elem = EXPRS_EXPR (aelems);
                aelems = EXPRS_NEXT (aelems);
                if (N_id == NODE_TYPE (elem)) { // Ignore N_num
                    avis = ID_AVIS (elem);
                    DBUG_PRINT ("Looking at elem %s", AVIS_NAME (avis));
                    z = z
                        && ((TYisAKV (AVIS_TYPE (avis)))
                            || (IVEXPisAvisHasBothExtrema (avis))
                            || (!SWLDisDefinedInThisBlock (avis,
                                                           INFO_DEFDEPTH (arg_info))));
                }
            }
        }

        ivid = FREEdoFreeNode (ivid);
        pat = PMfree (pat);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisNakedWL( int cwllevel, int plev)
 *
 * @brief
 *
 * @param  cwllevel: INFO_DEFDEPTH of CWL (or naked sel())
 *         pwllevel: AVIS_DEFDEPTH of PWL
 *
 * @result True if we are allowing naked AWLF and
 *         the CWL and PWL (even though there is not a PWL...)
 *         are at the same level. I.e., we
 *         have this sort of code layout:
 *
 *           PWL = with (...);
 *           ...
 *           z = _sel_VxA_( iv, PWL);
 *
 *****************************************************************************/
bool
AWLFIisNakedWL (int cwllevel, int pwllevel)
{
    bool z;

    DBUG_ENTER ();
    z = (cwllevel == pwllevel) && global.optimize.doscwlf;

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIisUusualWL( int cwllevel, int plev)
 *
 * @brief
 *
 * @param  cwllevel: INFO_DEFDEPTH of CWL (or naked sel())
 *         pwllevel: AVIS_DEFDEPTH of PWL
 *
 * @result True if the CWL is one level deeper in WL nesting
 *        than the PWL. I.e., we
 *         have this sort of code layout:
 *
 *           PWL = with (...);
 *           ...
 *           z = with  (...
 *             el = _sel_VxA_( iv, PWL);
 *             );
 *
 *****************************************************************************/
bool
AWLFIisUsualWL (int cwllevel, int pwllevel)
{
    bool z;

    DBUG_ENTER ();
    z = cwllevel == (1 + pwllevel);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool AWLFIcheckBothFoldable( node *pwlid, node *cwlavis, int cwllevel)
 *
 * @brief Make any early checks we can that may show that
 *        the producerWL and consumerWL are not foldable.
 *
 *        We now allow a naked sel() [not in a WL] consumer to fold
 *        a producerWL into it. A typical case of this is:
 *
 *          PWL = iota( N);
 *          z = _sel_VxA_( [0], PWL);
 *
 *       If we do not fold this, then the entire PWL array is generated,
 *       which we deem A Bad Idea.
 *
 * @param  pwlid: N_id of PWL
 *         cwlids: N_ids of CWL or naked sel(), or NULL.
 *         cwllevel: INFO_DEFDEPTH of CWL (or naked sel())
 *
 * @result True if the consumerWL and PWL
 *         shapes are conformable for folding.
 *
 *****************************************************************************/
bool
AWLFIcheckBothFoldable (node *pwlid, node *cwlids, int cwllevel)
{
    int plev;
    bool z;
#ifndef DBUG_OFF
    const char *nmc;
    const char *nmp;
#endif

    DBUG_ENTER ();

    /* Composition-style AWLF: CWL sel() one level deeper than PWL */
    /* Naked consumer AWLF: PWL and CWL sel() at same nesting level */
    plev = AVIS_DEFDEPTH (ID_AVIS (pwlid));
    z = AWLFIisNakedWL (cwllevel, plev) || AWLFIisUsualWL (cwllevel, plev);

#ifndef DBUG_OFF
    nmp = (NULL != pwlid) ? AVIS_NAME (ID_AVIS (pwlid)) : "(not a WL";
    nmc = (NULL != cwlids) ? AVIS_NAME (IDS_AVIS (cwlids)) : "(not a WL";

    if (z) {
        DBUG_PRINT ("PWL %s foldable into CWL %s", nmp, nmc);
    } else {
        DBUG_PRINT ("PWL %s not foldable into CWL %s. DEFDEPTHs=%d, %d", nmp, nmc, plev,
                    cwllevel);
    }
#endif

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
    size_t optctr;

    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("Begin %s %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));

        optctr = global.optcounters.awlfi_expr;
        DBUG_PRINT ("At AWLFIfundef entry, global.optcounters.awlfi_expr is %zu", optctr);

        if (FUNDEF_BODY (arg_node) != NULL) {
            arg_node = SWLDdoSetWithloopDepth (arg_node);
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            /* If new vardecs were made, append them to the current set */
            if (INFO_VARDECS (arg_info) != NULL) {
                BLOCK_VARDECS (FUNDEF_BODY (arg_node))
                  = TCappendVardec (INFO_VARDECS (arg_info),
                                    BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
                INFO_VARDECS (arg_info) = NULL;
            }

            if (global.optcounters.awlfi_expr != optctr) {
                DBUG_PRINT ("optcounters was %zu; is now %zu", optctr,
                            global.optcounters.awlfi_expr);
                arg_node = SimplifySymbioticExpression (arg_node, arg_info);
            }

            FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
        }

        DBUG_PRINT ("End %s %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "(wrapper)" : "function"),
                    FUNDEF_NAME (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node AWLFIassign( node *arg_node, info *arg_info)
 *
 * @brief performs a bottom-up traversal.
 *        This is harder than top-down, but we may get more folding done; see below.
 *
 *        For a foldable WL, arg_node is x = _sel_VxA_(iv, producerWL).
 *
 *        Why bottom-up is better than top-down:
 *         Suppose we have these WLs:
 *          V1 = iota(N);
 *          V2 = 3 + V1;
 *          V3 = V1 * V2;
 *
 *         If we go top-down, we are unable to fold V1 into V2, because V1
 *         is also referenced by V2. Thus, on the first pass, we only fold
 *         V2 into V3. On the second cycle, we have:
 *          V1 = iota(N);
 *          V3 = V1 + ( 3 * V1);
 *         This will fold, because all references to V1 lie within V3.
 *
 *         If we do bottom-up, we immediately fold V2 into V3:
 *          V1 = iota(N);
 *          V3 = V1 + ( 3 * V1);
 *         Then, in the same cycle, we will fold V1 into V3, assuming
 *         we revisit V3 immediately.
 *
 *****************************************************************************/
node *
AWLFIassign (node *arg_node, info *arg_info)
{
    node *let;
    node *oldpreassigns;

    DBUG_ENTER ();

    oldpreassigns = INFO_PREASSIGNS (arg_info);
    INFO_PREASSIGNS (arg_info) = NULL;
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    INFO_PREASSIGNS (arg_info) = oldpreassigns;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_PREASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), arg_node);
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    let = ASSIGN_STMT (arg_node);
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

    DBUG_ENTER ();

    old_arg_info = arg_info;
    arg_info = MakeInfo (INFO_FUNDEF (arg_info));

    INFO_DEFDEPTH (arg_info) = INFO_DEFDEPTH (old_arg_info) + 1;
    INFO_VARDECS (arg_info) = INFO_VARDECS (old_arg_info);
    INFO_FINVERSEINTRODUCED (arg_info) = INFO_FINVERSEINTRODUCED (old_arg_info);

    INFO_CONSUMERWL (arg_info) = arg_node;
    INFO_CONSUMERWLIDS (arg_info) = LET_IDS (INFO_LET (old_arg_info));
    DBUG_PRINT ("Looking at %s with INFO_DEFDEPTH=%d",
                AVIS_NAME (IDS_AVIS (INFO_CONSUMERWLIDS (arg_info))),
                INFO_DEFDEPTH (arg_info));

    DBUG_PRINT ("Resetting WITH_REFERENCED_CONSUMERWL, etc.");
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
    DBUG_ENTER ();

    INFO_CONSUMERWLPART (arg_info) = arg_node;
    CODE_CBLOCK (PART_CODE (arg_node))
      = TRAVdo (CODE_CBLOCK (PART_CODE (arg_node)), arg_info);
    INFO_CONSUMERWLPART (arg_info) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
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

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at %s", AVIS_NAME (ID_AVIS (arg_node)));
    p = INFO_CONSUMERWL (arg_info);
    if ((NULL != p) && (NULL == WITH_REFERENCED_CONSUMERWL (p))) {
        /* First reference to this WL. */
        WITH_REFERENCED_CONSUMERWL (p) = INFO_CONSUMERWL (arg_info);
        WITH_REFERENCED_FOLD (p) = 0;
        DBUG_PRINT ("AWLFIid found first reference to %s",
                    AVIS_NAME (ID_AVIS (arg_node)));
    }

    /*
     * arg_node describes a WL, so
     * WITH_REFERENCED_FOLD( p) may have to be
     * incremented
     */
    if ((NULL != p) && (NULL != INFO_CONSUMERWL (arg_info))
        && (WITH_REFERENCED_CONSUMERWL (p) == INFO_CONSUMERWL (arg_info))) {
        (WITH_REFERENCED_FOLD (p))++;
        DBUG_PRINT ("Incrementing WITH_REFERENCED_FOLD(%s) = %d",
                    AVIS_NAME (ID_AVIS (arg_node)), WITH_REFERENCED_FOLD (p));
    } else {
        DBUG_PRINT ("%s is not defined by a WL", AVIS_NAME (ID_AVIS (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *AWLFIprf( node *arg_node, info *arg_info)
 *
 * @brief
 *   Examine a _sel_VxA_(idx, producerWL) primitive to see if
 *   we may be able to fold producerWL here, assuming that
 *   the _sel_ is within a potential consumerWL, OR that idx is constant.
 *
 *   We don't find out if all the conditions for folding can
 *   be met until this phase completes, so AWLF makes the
 *   final decision on folding.
 *
 *   When we do encounter an eligible sel() operation,
 *   with a constant PRF_ARG1, or
 *   with extrema available on PRF_ARG1, we construct
 *   an intersect computation between the now-available index
 *   set of idx, and each partition of the producerWL.
 *   These are attached to the sel() via an F_noteintersect
 *   guard.
 *
 *****************************************************************************/
node *
AWLFIprf (node *arg_node, info *arg_info)
{
    node *z;
    node *ivavis = NULL;
    node *pwlid;
#ifndef DBUG_OFF
    char *cwlnm;
#endif

    DBUG_ENTER ();

#ifndef DBUG_OFF
    cwlnm = (NULL != INFO_CONSUMERWLIDS (arg_info))
              ? AVIS_NAME (IDS_AVIS (INFO_CONSUMERWLIDS (arg_info)))
              : "(naked consumer)";
#endif

    switch (PRF_PRF (arg_node)) {
    default:
        break;

    case F_sel_VxA:
    case F_idx_sel:
        pwlid = AWLFIfindWlId (PRF_ARG2 (arg_node));
        INFO_PRODUCERWLLHS (arg_info) = pwlid;
        INFO_PRODUCERWL (arg_info) = AWLFIfindWL (pwlid);
        INFO_PRODUCERWLFOLDABLE (arg_info)
          = AWLFIcheckProducerWLFoldable (pwlid)
            && AWLFIcheckBothFoldable (pwlid, INFO_CONSUMERWLIDS (arg_info),
                                       INFO_DEFDEPTH (arg_info));

        PRF_ARGS (arg_node) = TRAVdo (PRF_ARGS (arg_node), arg_info);

        /* Maybe attach intersect calculations now. */
        if ((INFO_PRODUCERWLFOLDABLE (arg_info))
            && (!AWLFIisHasNoteintersect (arg_node))) {

            ivavis = IVUToffset2Vect (arg_node, &INFO_VARDECS (arg_info),
                                      &INFO_PREASSIGNS (arg_info),
                                      INFO_CONSUMERWLPART (arg_info), NULL);

            /* We need both extrema or constant index vector */
            /* Or, we need naked consumer */
            if ((AWLFIisCanAttachIntersectCalc (arg_node, ivavis, arg_info))) {
                // FIXME || isNaked( arg_node, ivavis)) {
                DBUG_PRINT ("Trying to attach F_noteintersect into cwl=%s", cwlnm);
                z = attachIntersectCalc (arg_node, arg_info, ivavis);
                if (z != ID_AVIS (PRF_ARG1 (arg_node))) {
                    FREEdoFreeNode (PRF_ARG1 (arg_node));
                    PRF_ARG1 (arg_node) = TBmakeId (z);
                    DBUG_PRINT ("Inserted F_noteintersect into cwl=%s", cwlnm);
                }
            }
        }
        break;

    case F_noteintersect:
        /* Maybe detach invalid intersect calculations now. */
        if (!AWLFIisValidNoteintersect (arg_node, INFO_PRODUCERWLLHS (arg_info))) {
            arg_node = AWLFIdetachNoteintersect (arg_node);
            DBUG_PRINT ("Detached invalid F_noteintersect from cwl=%s", cwlnm);
        }

        /* Maybe project partitionIntersectMin/Max back from PWL to CWL */
        if (AWLFIisValidNoteintersect (arg_node, INFO_PRODUCERWLLHS (arg_info))) {
            arg_node = BuildInverseProjections (arg_node, arg_info);
            DBUG_PRINT ("Building inverse projection for cwl=%s", cwlnm);
        }
        break;
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
    DBUG_ENTER ();

    COND_COND (arg_node) = TRAVdo (COND_COND (arg_node), arg_info);
    COND_THENASSIGNS (arg_node) = TRAVopt (COND_THENASSIGNS (arg_node), arg_info);
    COND_ELSEASSIGNS (arg_node) = TRAVopt (COND_ELSEASSIGNS (arg_node), arg_info);

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

    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (N_modarray == NODE_TYPE (arg_node)) {
        INFO_PRODUCERWL (arg_info) = AWLFIfindWlId (MODARRAY_ARRAY (arg_node));
        wl = INFO_PRODUCERWL (arg_info);
        (WITH_REFERENCED_FOLD (wl))++;
        DBUG_PRINT ("AWLFImodarray: WITH_REFERENCED_FOLD(%s) = %d",
                    AVIS_NAME (ID_AVIS (MODARRAY_ARRAY (arg_node))),
                    WITH_REFERENCED_FOLD (wl));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *AWLFIlet(node *arg_node, info *arg_info)
 *
 * description: Descend to get set AVIS_DEPTH and to handle the expr.
 *
 ******************************************************************************/
node *
AWLFIlet (node *arg_node, info *arg_info)
{
    node *oldlet;

    DBUG_ENTER ();

    oldlet = INFO_LET (arg_info);
    INFO_LET (arg_info) = arg_node;
    LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    INFO_LET (arg_info) = oldlet;

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
    DBUG_ENTER ();
    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
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
    DBUG_ENTER ();

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
    DBUG_ENTER ();

    WHILE_COND (arg_node) = TRAVopt (WHILE_COND (arg_node), arg_info);
    WHILE_BODY (arg_node) = TRAVopt (WHILE_BODY (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Algebraic with loop folding inference -->
 *****************************************************************************/

#undef DBUG_PREFIX
