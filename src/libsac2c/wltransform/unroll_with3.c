/** <!--********************************************************************-->
 *
 * @defgroup Unroll with3s
 *
 * Unroll with3s
 *
 * As an artifact of the with2 there are lots of with3 loops that just
 * go over a single element i=0, i<1; i++.  This wastes resources on
 * MGSim.
 *
 * The with3 must not have any fold withops.
 *
 * if (upperbound - lowerbound) <= global.mutc_unroll
 *
 * set index = lowerbound
 * range body
 * set lhs0 = result0
 * ...
 * set lhsn = resultn
 *
 * @ingroup
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file unroll_with3.c
 *
 * Prefix: UW3
 *
 *****************************************************************************/
#include "unroll_with3.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "UW3"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "constants.h"
#include "tree_compound.h"
#include "globals.h"
#include "free.h"
#include "pattern_match.h"
#include "print.h"
#include "shape.h"
#include "type_utils.h"
#include "new_types.h"
#include "DupTree.h"
#include "LookUpTable.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *assigns;
    node *results;
    node *operators;
    int ranges;
    node *withops;
    node *vardecs;
    /* Find Accu anon trav */
    bool fa_prf_accu;
    node *fa_init;
    node *fa_lhs;

    node *si_ops_init;
};

/**
 * INFO_ASSIGNS         assigns that should be placed where the with3 loop was
 *                      in the ast.
 *
 * INFO_RESULTS         results with3 loop
 *                      WARNING this is a pointer into the AST
 *
 * INFO_RANGES          Number of ranges in this with3 loop
 *
 * INFO_WITHOPS         The withops of this with loop
 *
 * INFO_ACCU            The accu of the with loop N_let
 *
 * INFO_OPERATORS       With3 operators
 * INFO_VARDECS         Current scopes vardecs
 *
 * ** Find Accu anon trav **
 * INFO_FA_PRF_ACCU     Looking at an ACCU prf
 * INFO_FA_INIT         Initial value of folds
 * INFO_FA_LHS          LHS of accu
 *
 * INFO_SI_OPS_INIT     Initial value of withops
 *
 */
#define INFO_ASSIGNS(info) (info->assigns)
#define INFO_RESULTS(info) (info->results) /* pointer */
#define INFO_RANGES(info) (info->ranges)
#define INFO_WITHOPS(info) (info->withops)
#define INFO_OPERATORS(info) (info->operators)
#define INFO_VARDECS(info) (info->vardecs)

#define INFO_FA_PRF_ACCU(info) (info->fa_prf_accu)
#define INFO_FA_INIT(info) (info->fa_init)
#define INFO_FA_LHS(info) (info->fa_lhs)

#define INFO_SI_OPS_INIT(info) (info->si_ops_init)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));
    INFO_ASSIGNS (result) = NULL;
    INFO_RESULTS (result) = NULL;
    INFO_RANGES (result) = 0;
    INFO_WITHOPS (result) = NULL;

    INFO_FA_PRF_ACCU (result) = FALSE;
    INFO_FA_INIT (result) = NULL;
    INFO_FA_LHS (result) = NULL;
    INFO_SI_OPS_INIT (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_ASSIGNS (info) == NULL,
                 "Trying to free info which still contains assigns");

    DBUG_ASSERT (INFO_FA_INIT (info) == NULL,
                 "Trying to free info which still contains initals of folds");

    DBUG_ASSERT (INFO_FA_LHS (info) == NULL, "Trying to free info which still has lhss");

    info = MEMfree (info);

    DBUG_RETURN (info);
}
/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/* Forward def */
static node *RemoveArrayIndirection (node *);

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *UW3doUnrollWith3( node *syntax_tree)
 *
 *****************************************************************************/
node *
UW3doUnrollWith3 (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    DBUG_PRINT ("Starting Unroll With3 traversal.");

    TRAVpush (TR_uw3);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    syntax_tree = RemoveArrayIndirection (syntax_tree);

    DBUG_PRINT ("Ending Unroll With3 traversal complete.");

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
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

static node *
MakeIntegerConst (int rhs, node **assigns, node **vardecs)
{
    node *avis;

    DBUG_ENTER ();

    avis = TBmakeAvis (TRAVtmpVar (),
                       TYmakeAKV (TYmakeSimpleType (T_int), COmakeConstantFromInt (rhs)));

    *vardecs = TCappendVardec (*vardecs, TBmakeVardec (avis, NULL));

    *assigns
      = TCappendAssign (*assigns,
                        TBmakeAssign (TBmakeLet (TBmakeIds (avis, NULL), TBmakeNum (rhs)),
                                      NULL));

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *ATravRangeResult( node *syntax_tree)
 *
 * @brief See RemoveArrayIndirection
 *
 *****************************************************************************/
static node *
ATravRangeResult (node *exprs)
{
    pattern *pat;
    node *id;
    DBUG_ENTER ();

    if (EXPRS_NEXT (exprs) != NULL) {
        EXPRS_NEXT (exprs) = ATravRangeResult (EXPRS_NEXT (exprs));
    }

    pat = PMarray (0, 1, PMvar (1, PMAgetNode (&id), 0));

    if (PMmatchFlat (pat, EXPRS_EXPR (exprs))) {
        node *xnew = DUPdoDupNode (id);

        EXPRS_EXPR (exprs) = FREEdoFreeTree (EXPRS_EXPR (exprs));
        EXPRS_EXPR (exprs) = xnew;
    }
    pat = PMfree (pat);

    DBUG_RETURN (exprs);
}
/** <!--********************************************************************-->
 *
 * @fn node *ATravRange( node *syntax_tree)
 *
 * @brief Remove array indirections from results
 *
 *****************************************************************************/
static node *
ATravRange (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_ASSERT (RANGE_RESULTS (arg_node) != NULL, "Missing results");

    arg_node = TRAVcont (arg_node, arg_info);

    RANGE_RESULTS (arg_node) = ATravRangeResult (RANGE_RESULTS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RemoveArrayIndirection( node *syntax_tree)
 *
 * @brief convert
 *
 * {
 *   ...
 *   foo = [ bar ];
 * } : foo;
 *
 * to
 *
 * {
 *   ...
 * } : bar;
 *
 * and remove foo
 *
 *****************************************************************************/
static node *
RemoveArrayIndirection (node *syntax_tree)
{
    anontrav_t trav[2] = {{N_range, &ATravRange}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (trav, &TRAVsons);
    syntax_tree = TRAVopt (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *JoinIdsExprs( node *ids, node *exprs)
 *
 * @brief Join ids and exprs into (assign->let)s
 *        The output sub ast is in ssa form.
 *
 * Input:
 *
 *        ids           exprs
 *         +> ids        +> exprs
 *             +> ids        +> exprs
 *
 * Output: ([] = new node)
 *        [assign]
 *         | +> [let]
 *         |     | +> ids
 *         |     +> exprs
 *         +> [assign]
 *             | +> [let]
 *             |     | +> ids
 *             |     +> exprs
 *             +> [assign]
 *                   +> [let]
 *                       | +> ids
 *                       +> exprs
 *
 *****************************************************************************/
static node *
JoinIdsExprs (node *arg_ids, node *exprs)
{
    node *assign, *ids, *rhs;
    DBUG_ENTER ();

    DBUG_ASSERT (arg_ids != NULL, "ids missing");
    DBUG_ASSERT (exprs != NULL, "exprs missing");
    DBUG_ASSERT (NODE_TYPE (arg_ids) == N_ids, "JoinIdsExprs called on non ids");
    DBUG_ASSERT (NODE_TYPE (EXPRS_EXPR (exprs)) == N_id, "Non id expr in exprs chain");

    if (IDS_NEXT (arg_ids) == NULL) {
        assign = NULL;
    } else {
        assign = JoinIdsExprs (IDS_NEXT (arg_ids), EXPRS_NEXT (exprs));
    }
    ids = DUPdoDupNode (arg_ids);
    rhs = DUPdoDupNode (EXPRS_EXPR (exprs));

#if 0
  if ( ! TUeqShapes( AVIS_TYPE( IDS_AVIS( ids)),
                     AVIS_TYPE( ID_AVIS( rhs)))){
    shape *shape = TYgetShape( AVIS_TYPE( IDS_AVIS( ids)));
    node *shapeNode = SHshape2Array( shape);
    node *dimNode = TBmakeNum( SHgetDim( shape));
    rhs =
      TBmakePrf( F_reshape_VxA,
                 TBmakeExprs( TBmakeNum( 1),
                            TBmakeExprs( dimNode,
                                         TBmakeExprs( shapeNode,
                                                      TBmakeExprs( rhs,
                                                                   NULL)))));
  }
#else
    if (TYgetDim (IDS_NTYPE (ids)) == (TYgetDim (AVIS_TYPE (ID_AVIS (rhs))) + 1)) {
        DBUG_ASSERT (SHgetExtent (TYgetShape (IDS_NTYPE (ids)), 0) == 1,
                     "Unexpected shape");
        rhs = TBmakeArray (TYcopyType (IDS_NTYPE (ids)), SHmakeShape (1),
                           TBmakeExprs (rhs, NULL));
    } else {
        DBUG_ASSERT (TYgetDim (IDS_NTYPE (ids)) == TYgetDim (AVIS_TYPE (ID_AVIS (rhs))),
                     "Unexpected dim");
    }
#endif

    assign = TBmakeAssign (TBmakeLet (ids, rhs), assign);

    /* avis->ssaassign needed to keep the AST legal */
    AVIS_SSAASSIGN (IDS_AVIS (ids)) = assign;

    DBUG_RETURN (assign);
}

/** <!--********************************************************************-->
 *
 * @fn node *ResetInfo( node *ids, node *exprs)
 *
 * @brief Reset state information in info structure
 *        Does not remove saved sub asts in the info structure
 *****************************************************************************/
static info *
ResetInfo (info *arg_info)
{
    DBUG_ENTER ();

    /* Remove possibly old pointers from info */
    INFO_RESULTS (arg_info) = NULL;

    /* Reset info counters */
    INFO_RANGES (arg_info) = 0;

    DBUG_RETURN (arg_info);
}

/** <!--********************************************************************-->
 *
 * @fn node *FAlet( node *arg_node, info *arg_info)
 *
 * @brief Save let in ast if let of accu
 *
 *****************************************************************************/
static node *
FAlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_FA_PRF_ACCU (arg_info)) {
        INFO_FA_LHS (arg_info) = LET_IDS (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *FAprf( node *arg_node, info *arg_info)
 *
 * @brief Is this prf an accu?
 *        If so note in arg_info
 *
 *****************************************************************************/
static node *
FAprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_FA_PRF_ACCU (arg_info) == FALSE) {
        INFO_FA_PRF_ACCU (arg_info) = (PRF_PRF (arg_node) == F_accu);
    } else {
        DBUG_ASSERT (PRF_PRF (arg_node) != F_accu, "Found too many _accu_s");
    }

    DBUG_RETURN (arg_node);
}

static node *
UpgradeTypes (node *ids, node *exprs)
{
    DBUG_ENTER ();

    if (IDS_NEXT (ids) != NULL) {
        DBUG_ASSERT (EXPRS_NEXT (exprs) != NULL,
                     "Chains of ids and exprs must be same length");
        IDS_NEXT (ids) = UpgradeTypes (IDS_NEXT (ids), EXPRS_NEXT (exprs));
    }

    AVIS_TYPE (IDS_AVIS (ids)) = TYfreeType (AVIS_TYPE (IDS_AVIS (ids)));

    AVIS_TYPE (IDS_AVIS (ids)) = TYcopyType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (exprs))));

    DBUG_RETURN (ids);
}

static node *
FAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_FA_PRF_ACCU (arg_info)) {
        node *old = arg_node;
        arg_node = TCappendAssign (JoinIdsExprs (UpgradeTypes (INFO_FA_LHS (arg_info),
                                                               INFO_FA_INIT (arg_info)),
                                                 INFO_FA_INIT (arg_info)),
                                   ASSIGN_NEXT (arg_node));
        old = FREEdoFreeNode (old);
        INFO_FA_LHS (arg_info) = NULL;
        INFO_FA_INIT (arg_info) = NULL;
        INFO_FA_PRF_ACCU (arg_info) = FALSE;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

static node *
FAfold (node *arg_node, info *arg_info)
{
    node *init;
    DBUG_ENTER ();

    arg_node = TRAVcont (arg_node, arg_info);

    init = FOLD_INITIAL (arg_node);
    if (init == NULL) {
        init = FOLD_NEUTRAL (arg_node);
    }

    INFO_FA_INIT (arg_info) = TBmakeExprs (init, INFO_FA_INIT (arg_info));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *ReplaceAccu( node *tree, node *ops)
 *
 * @brief Replace the accu by a chain of assigns of the initial value of the
 *        fold.
 *
 *        Do not traverse into nested with loops or we would replace the wrong
 *        accus.
 *
 * @param tree ast to replace accus in
 * @param ops  chain of withops to get inital value from
 *****************************************************************************/
static node *
ReplaceAccu (node *tree, node *ops)
{
    info *local_info;
    anontrav_t trav[] = {{N_let, &FAlet},      {N_assign, &FAassign},
                         {N_prf, &FAprf},      {N_fold, &FAfold},

                         {N_with, &TRAVnone},  {N_with2, &TRAVnone},
                         {N_with3, &TRAVnone}, {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (trav, &TRAVsons);

    local_info = MakeInfo ();

    ops = TRAVopt (ops, local_info);
    tree = TRAVopt (tree, local_info);

    TRAVpop ();

    DBUG_RETURN (tree);
}

static node *
S2Iprf (node *arg_node, info *arg_info)
{
    node *id;
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_syncin:
    case F_syncout:
        id = DUPdoDupTree (PRF_ARG1 (arg_node));
        arg_node = FREEdoFreeTree (arg_node);
        arg_node = id;
        break;
    default:
        arg_node = TRAVcont (arg_node, arg_info);
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *Sync2Id( node *tree)
 *
 * @brief Replace
 *
 *        _sync{in,out}_(b)
 *
 *        with
 *
 *        b
 *
 *        do not do this for nested with* loops
 *
 * @param tree ast to remove syncs from
 *****************************************************************************/
static node *
Sync2Id (node *arg_node)
{
    anontrav_t trav[] = {{N_prf, &S2Iprf},
                         {N_with, &TRAVnone},
                         {N_with2, &TRAVnone},
                         {N_with3, &TRAVnone},
                         {(nodetype)0, NULL}};
    DBUG_ENTER ();

    TRAVpushAnonymous (trav, &TRAVsons);

    arg_node = TRAVopt (arg_node, NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *GetInitals( node *folds)
 *
 * @brief Get the initals from a chain of folds and return as a chain
 *        of exprs.
 *
 * @param folds A chain of folds
 *****************************************************************************/
static node *
GetInitals (node *folds)
{
    node *exprs = NULL;
    DBUG_ENTER ();

    DBUG_ASSERT (folds != NULL, "Expected a chain of folds");

    DBUG_ASSERT (NODE_TYPE (folds) == N_fold, "Can only get initals from fold withops");

    if (FOLD_NEXT (folds) != NULL) {
        exprs = GetInitals (FOLD_NEXT (folds));
    }

    exprs = TBmakeExprs (DUPdoDupTree (FOLD_INITIAL (folds)), exprs);

    DBUG_RETURN (exprs);
}

static node *
SInext (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_SI_OPS_INIT (arg_info) = EXPRS_NEXT (INFO_SI_OPS_INIT (arg_info));

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

static node *
SIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FOLD_INITIAL (arg_node) = FREEoptFreeTree(FOLD_INITIAL (arg_node));

    FOLD_INITIAL (arg_node) = DUPdoDupTree (EXPRS_EXPR (INFO_SI_OPS_INIT (arg_info)));

    arg_node = SInext (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SetInitials( node *ops, node *opsInitial)
 *
 * @brief Set the initials from a chain of exprs
 *
 * @param opsInitial if match a fold make initial of fold
 * @param ops        withloop operations
 *****************************************************************************/
static node *
SetInitials (node *ops, node *opsInitial)
{
    info *info;
    DBUG_ENTER ();

    anontrav_t trav[]
      = {{N_fold, &SIfold},      {N_genarray, &SInext}, {N_modarray, &SInext},
         {N_propagate, &SInext}, {N_spfold, &SInext},   {N_break, &SInext},
         {(nodetype)0, NULL}};

    TRAVpushAnonymous (trav, &TRAVsons);

    info = MakeInfo ();
    INFO_SI_OPS_INIT (info) = opsInitial;
    ops = TRAVopt (ops, info);
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (ops);
}
/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *UW3assign(node *arg_node, info *arg_info)
 *
 * @brief Remove old let and replace with a new set.
 *
 * Let must not have exprs on its right hand side.  It should have
 * expr instead.
 *
 * Remove the old assign->let and replace with:
 *
 * assign->let
 *      +>assign->let
 *             +>...
 *
 * This is needed as there can be is more than one id on the left hand side.
 *
 *****************************************************************************/
node *
UW3assign (node *arg_node, info *arg_info)
{
    node *assign_stack;
    DBUG_ENTER ();

    assign_stack = INFO_ASSIGNS (arg_info);
    INFO_ASSIGNS (arg_info) = NULL;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_RESULTS (arg_info) != NULL) {
        /* with3 loop has been removed */
        node *arg_node_original = arg_node;
        node *let = ASSIGN_STMT (arg_node);

        DBUG_PRINT ("With3 unrolled compleatly");

        arg_node = TCappendAssign (JoinIdsExprs (LET_IDS (let), INFO_RESULTS (arg_info)),
                                   ASSIGN_NEXT (arg_node));
        LET_IDS (let) = NULL;
        ASSIGN_NEXT (arg_node_original) = NULL;
        arg_node_original = FREEdoFreeTree (arg_node_original);
        INFO_RESULTS (arg_info) = FREEdoFreeTree (INFO_RESULTS (arg_info));
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    if (INFO_ASSIGNS (arg_info) != NULL) {
        /* put saved assigns before result lets */
        arg_node = TCappendAssign (INFO_ASSIGNS (arg_info), arg_node);
        INFO_ASSIGNS (arg_info) = NULL;
    }

    DBUG_ASSERT (INFO_ASSIGNS (arg_info) == NULL,
                 "Assigns in info not expected at this point");
    INFO_ASSIGNS (arg_info) = assign_stack;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UW3with3(node *arg_node, info *arg_info)
 *
 * @brief Unroll this with3 if we can.
 *
 * Support for ranges that have a step are not supported.
 *
 * In the case of fold it is posible to have a noop with3 this has no
 * ranges only fold withops and can be replaced by the initial value.
 *
 *****************************************************************************/
node *
UW3with3 (node *arg_node, info *arg_info)
{
    node *operators_stack;
    DBUG_ENTER ();

    DBUG_ASSERT (INFO_RANGES (arg_info) == 0, "Counted ranges that where not expected");

    operators_stack = INFO_OPERATORS (arg_info);
    INFO_OPERATORS (arg_info) = WITH3_OPERATIONS (arg_node)
      = TRAVopt (WITH3_OPERATIONS (arg_node), arg_info);

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);
    INFO_OPERATORS (arg_info) = operators_stack;

    if ((INFO_RANGES (arg_info) == 0)
        && (TCcountWithopsNeq (WITH3_OPERATIONS (arg_node), N_fold) == 0)) {
        /*
         * All withops are fold withops and there are no ranges this must
         * be a noop with3 loop so repace with initial
         */
        INFO_RESULTS (arg_info) = GetInitals (WITH3_OPERATIONS (arg_node));
        arg_node = FREEdoFreeTree (arg_node);
    }

    arg_info = ResetInfo (arg_info);

    DBUG_RETURN (arg_node);
}
/** <!--********************************************************************-->
 *
 * @fn node *UW3fundef(node *arg_node, info *arg_info)
 *
 * @brief Save vardec chain into info structure
 *
 *****************************************************************************/
node *
UW3fundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    if (FUNDEF_BODY (arg_node) != NULL) {
        INFO_VARDECS (arg_info) = FUNDEF_VARDECS (arg_node);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        INFO_VARDECS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *UW3range(node *arg_node, info *arg_info)
 *
 * @brief Is this range unrollable? Collection needed info to unroll
 *        A range is considered unrollable if:
 *        lowerbound - upperbound == 1
 *        Must be able to staticly calculate the above.
 *
 *****************************************************************************/
node *
UW3range (node *arg_node, info *arg_info)
{
    constant *clower, *cupper;
    info *nested_info;
    DBUG_ENTER ();

    nested_info = MakeInfo ();
    INFO_VARDECS (nested_info) = INFO_VARDECS (arg_info);
    RANGE_BODY (arg_node) = TRAVopt (RANGE_BODY (arg_node), nested_info);
    nested_info = FreeInfo (nested_info);

    /* arg_info to count ranges */
    RANGE_NEXT (arg_node) = TRAVopt (RANGE_NEXT (arg_node), arg_info);

    INFO_RANGES (arg_info) = INFO_RANGES (arg_info) + 1;

    clower = COaST2Constant (RANGE_LOWERBOUND (arg_node));
    cupper = COaST2Constant (RANGE_UPPERBOUND (arg_node));

    if ((clower != NULL) && (cupper != NULL)) {
        int lower, upper;
        lower = COconst2Int (clower);
        upper = COconst2Int (cupper);
        if ((upper - lower) <= global.mutc_unroll) {
            int max = upper - lower;
            int i;

            DBUG_PRINT ("Unrolling range %d times", max);

            for (i = 0; i < max; i++) {
                /* Save the body of the with3 loop */
                lut_t *lut = LUTgenerateLut ();
                node *newIndex = MakeIntegerConst (lower + i, &INFO_ASSIGNS (arg_info),
                                                   &INFO_VARDECS (arg_info));
                node *newcode;

                lut = LUTinsertIntoLutP (lut, ID_AVIS (RANGE_INDEX (arg_node)), newIndex);
                newcode = DUPdoDupTreeLut (BLOCK_ASSIGNS (RANGE_BODY (arg_node)), lut);
                INFO_ASSIGNS (arg_info)
                  = TCappendAssign (INFO_ASSIGNS (arg_info),
                                    Sync2Id (
                                      ReplaceAccu (newcode, INFO_OPERATORS (arg_info))));

                INFO_OPERATORS (arg_info)
                  = SetInitials (INFO_OPERATORS (arg_info), RANGE_RESULTS (arg_node));
                lut = LUTremoveLut (lut);
            }
            /* range redundent now so remove */
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    if (clower != NULL) {
        clower = COfreeConstant (clower);
    }
    if (cupper != NULL) {
        cupper = COfreeConstant (cupper);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/

#undef DBUG_PREFIX
