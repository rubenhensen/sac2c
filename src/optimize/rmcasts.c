/*
 * $Log$
 * Revision 1.3  2004/11/26 14:36:47  mwe
 * SacDevCamp: compiles!
 *
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

    expr = TRAVdo (CAST_EXPR (arg_node), arg_info);

    CAST_EXPR (arg_node) = NULL;

    arg_node = FREEdoFreeTree (arg_node);

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
RCdoRemoveCasts (node *syntax_tree)
{

    DBUG_ENTER ("RemoveCasts");

    DBUG_PRINT ("OPT", ("starting remove casts traversal"));

    TRAVpush (TR_rc);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}
