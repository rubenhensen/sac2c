/*
 * $Log$
 * Revision 1.2  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.1  2001/05/22 09:09:45  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   rmcasts.c
 *
 * prefix: RM
 *
 * description:
 *
 *   This module removed all cast nodes from AST.
 *
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free.h"
#include "rmcasts.h"

/******************************************************************************
 *
 * function:
 *   node* RCcast(node *arg_node, info *arg_info)
 *
 * description:
 *   removes this cast node and returns the cast expression.
 *
 ******************************************************************************/
node *
RCcast (node *arg_node, info *arg_info)
{
    node *expr;

    DBUG_ENTER ("RCcast");

    expr = Trav (CAST_EXPR (arg_node), arg_info);

    CAST_EXPR (arg_node) = NULL;

    arg_node = FreeTree (arg_node);

    DBUG_RETURN (expr);
}

/******************************************************************************
 *
 * function:
 *   node* RemoveCasts(node* syntax_tree)
 *
 * description:
 *   starts traversal of syntax tree to remove all casts.
 *
 ******************************************************************************/
node *
RemoveCasts (node *syntax_tree)
{
    funtab *old_tab;

    DBUG_ENTER ("RemoveCasts");

    DBUG_PRINT ("OPT", ("starting remove casts traversal"));

    old_tab = act_tab;
    act_tab = rmcasts_tab;

    syntax_tree = Trav (syntax_tree, NULL);

    act_tab = old_tab;

    DBUG_RETURN (syntax_tree);
}
