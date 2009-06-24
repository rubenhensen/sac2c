/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup Remove with3s
 *
 * Remove unneeded with3s
 *
 *
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
/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *vardecs;
    node *assigns;
    node *results;
};

/**
 * INFO_VARDECS list of vardecs that must be placed into fundef.
 *
 * INFO_ASSIGNS assigns that should be placed where the with3 loop was
 *              in the ast.
 *
 * INFO_RESULTS results of removed with3 loop to be placed with ids in lets
 */
#define INFO_VARDECS(info) (info->vardecs)
#define INFO_ASSIGNS(info) (info->assigns)
#define INFO_RESULTS(info) (info->results)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));
    INFO_VARDECS (result) = NULL;
    INFO_ASSIGNS (result) = NULL;

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

    DBUG_PRINT ("RW3", ("Remove With3 traversal complete."));

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *JoinIdsExprs(node *ids, node *expr)
 *
 * @brief Join ids and exprs into (assign->let)s
 *
 *
 *****************************************************************************/
static node *
JoinIdsExprs (node *ids, node *exprs)
{
    node *let, *id, *assign, *expr;
    DBUG_ENTER ("JoinIdsExprs");

    DBUG_ASSERT ((ids != NULL), "ids missing");
    DBUG_ASSERT ((exprs != NULL), "exprs missing");

    id = ids;
    ids = IDS_NEXT (ids);
    IDS_NEXT (id) = NULL;

    let
      = TBmakeLet (id, TBmakeArray (TYcopyType (AVIS_TYPE (ID_AVIS (EXPRS_EXPR (exprs)))),
                                    SHcreateShape (1, 1),
                                    TBmakeExprs (EXPRS_EXPR (exprs), NULL)));

    expr = exprs;
    exprs = EXPRS_NEXT (exprs);
    EXPRS_NEXT (expr) = NULL;
    EXPRS_EXPR (expr) = NULL;
    expr = FREEdoFreeTree (expr); /* Remove remains of exprs node */

    DBUG_ASSERT (((ids == NULL && exprs == NULL) || (ids != NULL && exprs != NULL)),
                 "ids and exprs chains are of different length.");

    if (ids == NULL) {
        assign = TBmakeAssign (let, NULL);
    } else {
        assign = TBmakeAssign (let, JoinIdsExprs (ids, exprs));
    }

    /* avis->ssaassign needed to keep the AST legal */
    AVIS_SSAASSIGN (IDS_AVIS (LET_IDS (let))) = assign;

    DBUG_RETURN (assign);
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
    DBUG_ENTER ("RW3with3");

    WITH3_RANGES (arg_node) = TRAVopt (WITH3_RANGES (arg_node), arg_info);

    /* Only one range */
    if (RANGE_NEXT (WITH3_RANGES (arg_node)) == NULL) {
        constant *lower;
        constant *upper;
        lower = COaST2Constant (RANGE_LOWERBOUND (WITH3_RANGES (arg_node)));
        upper = COaST2Constant (RANGE_UPPERBOUND (WITH3_RANGES (arg_node)));

        if ((lower != NULL) && (upper != NULL)) { /* May not be AKV */
            constant *tmp;
            tmp = COsub (upper, lower);
            if (COconst2Int (tmp) <= 1) { /* < to support empty ranges */
                /* Can remove this range */
                INFO_VARDECS (arg_info)
                  = TCappendVardec (BLOCK_VARDEC (RANGE_BODY (WITH3_RANGES (arg_node))),
                                    INFO_VARDECS (arg_info));
                INFO_ASSIGNS (arg_info)
                  = TCappendAssign (BLOCK_INSTR (RANGE_BODY (WITH3_RANGES (arg_node))),
                                    INFO_ASSIGNS (arg_info));
                /* set index to lower bound */
                INFO_ASSIGNS (arg_info)
                  = TBmakeAssign (TBmakeLet (RANGE_INDEX (WITH3_RANGES (arg_node)),
                                             RANGE_LOWERBOUND (WITH3_RANGES (arg_node))),
                                  INFO_ASSIGNS (arg_info));

                /* Must set SSAASSIGN to make tree legal at this phase of the
                   compiler */
                AVIS_SSAASSIGN (
                  IDS_AVIS (LET_IDS (ASSIGN_INSTR (INFO_ASSIGNS (arg_info)))))
                  = INFO_ASSIGNS (arg_info);

                tmp = COfreeConstant (tmp);

                INFO_RESULTS (arg_info) = RANGE_RESULTS (WITH3_RANGES (arg_node));

                /* Remove pointers to nodes we wish to keep */
                BLOCK_VARDEC (RANGE_BODY (WITH3_RANGES (arg_node))) = NULL;
                BLOCK_INSTR (RANGE_BODY (WITH3_RANGES (arg_node))) = NULL;
                RANGE_INDEX (WITH3_RANGES (arg_node)) = NULL;
                RANGE_LOWERBOUND (WITH3_RANGES (arg_node)) = NULL;
                RANGE_RESULTS (WITH3_RANGES (arg_node)) = NULL;
                /* Free arg_node as we no longer need it and we have taken all
                   we need */
                arg_node = FREEdoFreeTree (arg_node);
            }
            lower = COfreeConstant (lower);
            upper = COfreeConstant (upper);
        }
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RW3fundef(node *arg_node, info *arg_info)
 *
 * @brief Insert any saved vardecs on end of fundef's vardec list.
 *
 *****************************************************************************/
node *
RW3fundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RW3fundef");

    DBUG_ASSERT ((INFO_VARDECS (arg_info) == NULL), "leftover vardecs found.");

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    if (INFO_VARDECS (arg_info) != NULL) {
        FUNDEF_VARDEC (arg_node)
          = TCappendVardec (FUNDEF_VARDEC (arg_node), INFO_VARDECS (arg_info));
        INFO_VARDECS (arg_info) = NULL;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RW3assign(node *arg_node, info *arg_info)
 *
 * @brief Remove old let and replace with a new one.
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

    if (INFO_RESULTS (arg_info) != NULL) {
        node *arg_node_original = arg_node;
        node *let = ASSIGN_INSTR (arg_node);
        arg_node = TCappendAssign (JoinIdsExprs (LET_IDS (let), INFO_RESULTS (arg_info)),
                                   ASSIGN_NEXT (arg_node));
        LET_IDS (let) = NULL;
        ASSIGN_NEXT (arg_node_original) = NULL;
        FREEdoFreeTree (arg_node_original);
        INFO_RESULTS (arg_info) = NULL;
    }

    if (INFO_ASSIGNS (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_ASSIGNS (arg_info), arg_node);
        INFO_ASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
