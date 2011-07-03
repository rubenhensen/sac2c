/*
 * $Log$
 * Revision 1.5  2005/06/28 20:55:43  cg
 * Separate traversal for transforming while-loops into do-loops reactivated.
 *
 * Revision 1.4  2003/09/11 08:21:21  sbs
 * DBUG_PRINTs OPT inserted.
 *
 * Revision 1.3  2001/05/17 11:44:02  dkr
 * FREE eliminated
 *
 * Revision 1.2  2001/04/19 11:48:13  nmw
 * missing recursive traversal in converted while loops added
 *
 * Revision 1.1  2001/04/18 15:38:34  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   while2do.c
 *
 * prefix: W2D
 *
 * description:
 *
 *   This module replaces all N_while nodes with N_do nodes contained in an
 *   conditional.
 *
 *   while(cond) {               if(cond) {
 *     body;                -->    do {
 *   }                               body;
 *                                 } while(cond);
 *                               }
 *
 *   this module can only be used before lac2fun is done!
 *
 *****************************************************************************/

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "while2do.h"

/******************************************************************************
 *
 * function:
 *   node* W2Dwhile(node *arg_node, node *arg_info)
 *
 * description:
 *   replaces the N_while node with an N_do node in a conditional.
 *
 *   while(cond){...}  -->  if(cond){do{ ...} while(cond)} else {}
 *
 ******************************************************************************/

node *
W2Dwhile (node *arg_node, info *arg_info)
{
    node *new_cond;
    node *new_do;

    DBUG_ENTER ();

    /* first traverse condition and body */
    if (WHILE_COND (arg_node) != NULL) {
        WHILE_COND (arg_node) = TRAVdo (WHILE_COND (arg_node), arg_info);
    }

    if (WHILE_BODY (arg_node) != NULL) {
        WHILE_BODY (arg_node) = TRAVdo (WHILE_BODY (arg_node), arg_info);
    }

    /* create new do-node */
    new_do = TBmakeDo (WHILE_COND (arg_node), WHILE_BODY (arg_node));

    /* create cond-node with do-loop in then-part */
    new_cond = TBmakeCond (DUPdoDupTree (DO_COND (new_do)),
                           TBmakeBlock (TBmakeAssign (new_do, NULL), NULL),
                           TBmakeBlock (TBmakeEmpty (), NULL));

    /* delete links in old while-node */
    WHILE_COND (arg_node) = NULL;
    WHILE_BODY (arg_node) = NULL;

    /* free old while-node */
    arg_node = FREEdoFreeTree (arg_node);

    DBUG_RETURN (new_cond);
}

/******************************************************************************
 *
 * function:
 *   node* W2DdoTransformWhile2Do(node* syntax_tree)
 *
 * description:
 *   starts traversal of syntax tree to transform all while- in to do-loops.
 *
 ******************************************************************************/

node *
W2DdoTransformWhile2Do (node *syntax_tree)
{
    DBUG_ENTER ();

    TRAVpush (TR_w2d);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
