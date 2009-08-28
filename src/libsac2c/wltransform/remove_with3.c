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
 */
#define INFO_ASSIGNS(info) (info->assigns)
#define INFO_RESULTS(info) (info->results) /* pointer */
#define INFO_SAVED_RESULTS(info) (info->savedResults)
#define INFO_REMOVABLE_RANGE(info) (info->removableRange)
#define INFO_LOWERBOUND(info) (info->lowerBound)    /* pointer */
#define INFO_LOOP_ASSIGNS(info) (info->loopAssigns) /* pointer */
#define INFO_LOOP_INDEX(info) (info->loopIndex)     /* pointer */
#define INFO_RANGES(info) (info->ranges)

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
 *        Wrap all exprs into arrays
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
 *         |     +> [array]
 *         |          +> exprs
 *         +> [assign]
 *             | +> [let]
 *             |     | +> ids
 *             |     +> [array]
 *             |          +> exprs
 *             +> [assign]
 *                   +> [let]
 *                       | +> ids
 *                       +> [array]
 *                            +> exprs
 *
 *****************************************************************************/
static node *
JoinIdsExprs (node *arg_ids, node *exprs)
{
    node *let, *assign, *ids;
    DBUG_ENTER ("JoinIdsExprs");

    DBUG_ASSERT ((arg_ids != NULL), "ids missing");
    DBUG_ASSERT ((exprs != NULL), "exprs missing");

    if (IDS_NEXT (arg_ids) == NULL) {
        assign = NULL;
    } else {
        assign = JoinIdsExprs (IDS_NEXT (arg_ids), EXPRS_NEXT (exprs));
    }

    ids = DUPdoDupNode (arg_ids);

    let = TBmakeLet (ids, TBmakeArray (TYcopyType (IDS_NTYPE (ids)),
                                       SHcopyShape (TYgetShape (IDS_NTYPE (ids))),
                                       DUPdoDupNode (exprs)));

    assign = TBmakeAssign (let, assign);

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
 *****************************************************************************/
node *
RW3with3 (node *arg_node, info *arg_info)
{
    node *let, *index;
    DBUG_ENTER ("RW3with3");

    DBUG_ASSERT ((INFO_RANGES (arg_info) == 0), "Counted ranges that where not expected");

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    DBUG_ASSERT ((INFO_RANGES (arg_info) >= 1),
                 "At least one range expected in a with3 loop");

    if ((INFO_RANGES (arg_info) == 1) && (INFO_REMOVABLE_RANGE (arg_info) == TRUE)) {

        /* Save the body of the with3 loop */
        INFO_ASSIGNS (arg_info)
          = TCappendAssign (DUPdoDupTree (INFO_LOOP_ASSIGNS (arg_info)),
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
