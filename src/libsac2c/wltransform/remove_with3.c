/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup Remove with3s
 *
 * Remove unneeded with3s
 *
 * As an artifact of the with2 there are lots of with3 loops that just
 * go over a single element i=0, i<1; i++.  This wastes resources on
 * MGSim.
 *
 * This traversal matains ssa form.
 *
 * The with3 must not have any fold withops.
 *
 * if (upperbound - lowerbound) == 1
 * and if count(N_range) == 1
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
 * @file remove_with3.c
 *
 * Prefix: RW3
 *
 *****************************************************************************/
#include "remove_with3.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "constants.h"
#include "tree_compound.h"
#include "free.h"
#include "print.h"
#include "shape.h"
#include "new_types.h"
#include "DupTree.h"
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *assigns;
    node *results;
    node *lowerBound;
    bool removableRange;
    node *loopAssigns;
    node *loopIndex;
    node *savedResults;
    int ranges;
    node *withops;
    /* Find Accu anon trav */
    bool fa_prf_accu;
    node *fa_init;
    node *fa_lhs;
};

/**
 * INFO_ASSIGNS         assigns that should be placed where the with3 loop was
 *                      in the ast.
 *
 * INFO_RESULTS         results with3 loop
 *                      WARNING this is a pointer into the AST
 *
 * INFO_RESULTS         results of removed with3 loop to be placed with ids in
 *                      lets
 *
 * INFO_REMOVABLE_RANGE Current with3 loop has a range that meets the
 *                      requirements for removal
 *
 * INFO_LOWERBOUND      The lower bound of the last range in the current with3
 *                      loop
 *
 * INFO_LOOP_ASSIGNS    The assigns from the last range body of the current
 *                      with loop
 *                      WARNING this is a pointer into the AST
 *
 * INFO_LOOP_INDEX      The N_ids of the index of the last range in the current
 *                      with3 loop
 *                      WARNING this is a pointer into the AST
 *
 * INFO_RANGES          Number of ranges in this with3 loop
 *
 * INFO_WITHOPS         The withops of this with loop
 *
 * INFO_ACCU            The accu of the with loop N_let
 *
 * ** Find Accu anon trav **
 * INFO_FA_PRF_ACCU     Looking at an ACCU prf
 * INFO_FA_INIT         Initial value of folds
 * INFO_FA_LHS          LHS of accu
 *
 */
#define INFO_ASSIGNS(info) (info->assigns)
#define INFO_RESULTS(info) (info->results) /* pointer */
#define INFO_SAVED_RESULTS(info) (info->savedResults)
#define INFO_REMOVABLE_RANGE(info) (info->removableRange)
#define INFO_LOWERBOUND(info) (info->lowerBound)    /* pointer */
#define INFO_LOOP_ASSIGNS(info) (info->loopAssigns) /* pointer */
#define INFO_LOOP_INDEX(info) (info->loopIndex)     /* pointer */
#define INFO_RANGES(info) (info->ranges)
#define INFO_WITHOPS(info) (info->withops)

#define INFO_FA_PRF_ACCU(info) (info->fa_prf_accu)
#define INFO_FA_INIT(info) (info->fa_init)
#define INFO_FA_LHS(info) (info->fa_lhs)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));
    INFO_ASSIGNS (result) = NULL;
    INFO_RESULTS (result) = NULL;
    INFO_LOWERBOUND (result) = NULL;
    INFO_REMOVABLE_RANGE (result) = FALSE;
    INFO_LOOP_ASSIGNS (result) = NULL;
    INFO_LOOP_INDEX (result) = NULL;
    INFO_SAVED_RESULTS (result) = NULL;
    INFO_RANGES (result) = 0;
    INFO_WITHOPS (result) = NULL;

    INFO_FA_PRF_ACCU (result) = FALSE;
    INFO_FA_INIT (result) = NULL;
    INFO_FA_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    DBUG_ASSERT ((INFO_ASSIGNS (info) == NULL),
                 "Trying to free info which still contains assigns");

    DBUG_ASSERT ((INFO_SAVED_RESULTS (info) == NULL),
                 "Trying to free info which still contains saved results");

    DBUG_ASSERT ((INFO_FA_INIT (info) == NULL),
                 "Trying to free info which still contains initals of folds");

    DBUG_ASSERT ((INFO_FA_LHS (info) == NULL),
                 "Trying to free info which still has lhss");

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
 * @fn node *RW3doRemoveWith3( node *syntax_tree)
 *
 *****************************************************************************/
node *
RW3doRemoveWith3 (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RW3doRemoveWith3");

    info = MakeInfo ();

    DBUG_PRINT ("RW3", ("Starting Remove With3 traversal."));

    TRAVpush (TR_rw3);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("RW3", ("Ending Remove With3 traversal complete."));

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
    DBUG_ENTER ("JoinIdsExprs");

    DBUG_ASSERT ((arg_ids != NULL), "ids missing");
    DBUG_ASSERT ((exprs != NULL), "exprs missing");

    if (IDS_NEXT (arg_ids) == NULL) {
        assign = NULL;
    } else {
        assign = JoinIdsExprs (IDS_NEXT (arg_ids), EXPRS_NEXT (exprs));
    }
    ids = DUPdoDupNode (arg_ids);
    rhs = DUPdoDupNode (EXPRS_EXPR (exprs));

    if (TYgetDim (IDS_NTYPE (ids)) > TYgetDim (AVIS_TYPE (ID_AVIS (rhs)))) {
        rhs = TBmakeArray (TYcopyType (IDS_NTYPE (ids)), SHmakeShape (1),
                           TBmakeExprs (rhs, NULL));
    }

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
    DBUG_ENTER ("ResetInfo");

    /* Remove possibly old pointers from info */
    INFO_RESULTS (arg_info) = NULL;
    INFO_LOOP_ASSIGNS (arg_info) = NULL;
    INFO_LOWERBOUND (arg_info) = NULL;
    INFO_LOOP_INDEX (arg_info) = NULL;

    /* Reset info counters */
    INFO_RANGES (arg_info) = 0;
    INFO_REMOVABLE_RANGE (arg_info) = FALSE;

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
    DBUG_ENTER ("FAlet");

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
    DBUG_ENTER ("FAprf");

    if (INFO_FA_PRF_ACCU (arg_info) == FALSE) {
        INFO_FA_PRF_ACCU (arg_info) = (PRF_PRF (arg_node) == F_accu);
    } else {
        DBUG_ASSERT ((PRF_PRF (arg_node) != F_accu), "Found too many _accu_s");
    }

    DBUG_RETURN (arg_node);
}

static node *
FAassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("FAassign");

    ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_FA_PRF_ACCU (arg_info)) {
        node *old = arg_node;
        arg_node = TCappendAssign (JoinIdsExprs (INFO_FA_LHS (arg_info),
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
    DBUG_ENTER ("FAfold");

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
 *        folds.
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
                         {N_with3, &TRAVnone}, {0, NULL}};
    DBUG_ENTER ("FindAccu");

    TRAVpushAnonymous (trav, &TRAVsons);

    local_info = MakeInfo ();

    ops = TRAVopt (ops, local_info);
    tree = TRAVopt (tree, local_info);

    TRAVpop ();

    DBUG_RETURN (tree);
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
    DBUG_ENTER ("GetInitals");

    DBUG_ASSERT ((folds != NULL), "Expected a chain of folds");

    DBUG_ASSERT ((NODE_TYPE (folds) == N_fold), "Can only get initals from fold withops");

    if (FOLD_NEXT (folds) != NULL) {
        exprs = GetInitals (FOLD_NEXT (folds));
    }

    exprs = TBmakeExprs (DUPdoDupTree (FOLD_INITIAL (folds)), exprs);

    DBUG_RETURN (exprs);
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
 * @fn node *RW3assign(node *arg_node, info *arg_info)
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
RW3assign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RW3assign");

    DBUG_ASSERT ((INFO_ASSIGNS (arg_info) == NULL), "leftover assigns found.");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_SAVED_RESULTS (arg_info) != NULL) {
        /* with3 loop has been removed */
        node *arg_node_original = arg_node;
        node *let = ASSIGN_INSTR (arg_node);
        arg_node
          = TCappendAssign (JoinIdsExprs (LET_IDS (let), INFO_SAVED_RESULTS (arg_info)),
                            ASSIGN_NEXT (arg_node));
        LET_IDS (let) = NULL;
        ASSIGN_NEXT (arg_node_original) = NULL;
        arg_node_original = FREEdoFreeTree (arg_node_original);
        INFO_SAVED_RESULTS (arg_info) = FREEdoFreeTree (INFO_SAVED_RESULTS (arg_info));
    }

    if (INFO_ASSIGNS (arg_info) != NULL) {
        /* put saved assigns before result lets */
        arg_node = TCappendAssign (INFO_ASSIGNS (arg_info), arg_node);
        INFO_ASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RW3with3(node *arg_node, info *arg_info)
 *
 * @brief Remove this with3 if we can.
 *
 * With3 must only have one range and that range must go over one element.
 * Support for ranges that have a step are not supported.
 *
 * In the case of fold it is posible to have a noop with3 this has no
 * ranges only fold withops and can be replaced by the initial value.
 *
 *****************************************************************************/
node *
RW3with3 (node *arg_node, info *arg_info)
{
    node *let, *index;
    DBUG_ENTER ("RW3with3");

    DBUG_ASSERT ((INFO_RANGES (arg_info) == 0), "Counted ranges that where not expected");

    WITH3_OPERATIONS (arg_node) = TRAVopt (WITH3_OPERATIONS (arg_node), arg_info);

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    DBUG_ASSERT (((INFO_RANGES (arg_info) >= 1)
                  || (TCcountWithopsEq (WITH3_OPERATIONS (arg_node), N_fold) != 0)),
                 "At least one range expected in a with3 loop unless folding");

    /*INFO_WITHOPS( arg_info) = DUPdoDupTree( WITH3_OPERATIONS( arg_node));*/

    if (WITH3_DENSE (arg_info) && (INFO_RANGES (arg_info) == 1)
        && (INFO_REMOVABLE_RANGE (arg_info) == TRUE)) {

        /* Save the body of the with3 loop */
        INFO_ASSIGNS (arg_info)
          = TCappendAssign (ReplaceAccu (DUPdoDupTree (INFO_LOOP_ASSIGNS (arg_info)),
                                         WITH3_OPERATIONS (arg_node)),
                            INFO_ASSIGNS (arg_info));

        /* set index to lower bound */
        index = DUPdoDupTree (INFO_LOOP_INDEX (arg_info));
        let = TBmakeLet (index, DUPdoDupTree (INFO_LOWERBOUND (arg_info)));
        INFO_ASSIGNS (arg_info) = TBmakeAssign (let, INFO_ASSIGNS (arg_info));

        /* Must set SSAASSIGN to make tree legal at this phase of the
           compiler */
        AVIS_SSAASSIGN (IDS_AVIS (index)) = INFO_ASSIGNS (arg_info);
        INFO_SAVED_RESULTS (arg_info) = DUPdoDupTree (INFO_RESULTS (arg_info));

        /* Free old ast */
        arg_node = FREEdoFreeTree (arg_node);
    } else if ((INFO_RANGES (arg_info) == 0)
               && (TCcountWithopsNeq (WITH3_OPERATIONS (arg_node), N_fold) == 0)) {
        /*
         * All withops are fold withops and there are no ranges this must
         * be a noop with3 loop so repace with initial
         */
        INFO_SAVED_RESULTS (arg_info) = GetInitals (WITH3_OPERATIONS (arg_node));
        arg_node = FREEdoFreeTree (arg_node);
    }

    arg_info = ResetInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RW3range(node *arg_node, info *arg_info)
 *
 * @brief Is this range removable? Collection needed info to remove
 *        A range is considered removable if:
 *        lowerbound - upperbound == 1
 *        Must be able to staticly calculate the above.
 *
 *****************************************************************************/
node *
RW3range (node *arg_node, info *arg_info)
{
    constant *clower, *cupper;
    info *nested_info;
    DBUG_ENTER ("RW3range");

    nested_info = MakeInfo ();
    arg_node = TRAVcont (arg_node, nested_info);
    nested_info = FreeInfo (nested_info);

    INFO_RANGES (arg_info) = INFO_RANGES (arg_info) + 1;
    INFO_LOWERBOUND (arg_info) = RANGE_LOWERBOUND (arg_node);
    INFO_RESULTS (arg_info) = RANGE_RESULTS (arg_node);
    INFO_LOOP_ASSIGNS (arg_info) = BLOCK_INSTR (RANGE_BODY (arg_node));
    INFO_LOOP_INDEX (arg_info) = RANGE_INDEX (arg_node);

    clower = COaST2Constant (RANGE_LOWERBOUND (arg_node));
    cupper = COaST2Constant (RANGE_UPPERBOUND (arg_node));

    if ((clower != NULL) && (cupper != NULL)) {
        int lower, upper;
        lower = COconst2Int (clower);
        upper = COconst2Int (cupper);
        if ((upper - lower) == 1) {
            INFO_REMOVABLE_RANGE (arg_info) = TRUE;
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
