/*
 * $Log$
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

#include "dbug.h"
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
W2Dwhile (node *arg_node, node *arg_info)
{
    node *new_cond;
    node *new_do;

    DBUG_ENTER ("W2Dwhile");

    /* first traverse condition and body */
    if (WHILE_COND (arg_node) != NULL) {
        WHILE_COND (arg_node) = Trav (WHILE_COND (arg_node), arg_info);
    }

    if (WHILE_BODY (arg_node) != NULL) {
        WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
    }

    /* create new do-node */
    new_do = MakeDo (WHILE_COND (arg_node), WHILE_BODY (arg_node));

    /* create cond-node with do-loop in then-part */
    new_cond
      = MakeCond (DupTree (DO_COND (new_do)), MakeBlock (MakeAssign (new_do, NULL), NULL),
                  MakeBlock (MakeEmpty (), NULL));

    /* delete links in old while-node */
    WHILE_COND (arg_node) = NULL;
    WHILE_BODY (arg_node) = NULL;

    /* free old while-node */
    arg_node = FreeTree (arg_node);

    DBUG_RETURN (new_cond);
}

/******************************************************************************
 *
 * function:
 *   node* TransformWhile2Do(node* syntax_tree)
 *
 * description:
 *   starts traversal of syntax tree to transform all while- in to do-loops.
 *
 ******************************************************************************/
node *
TransformWhile2Do (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("TransformWhile2Do");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = w2d_tab;

#ifndef DBUG_OFF
    if (compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("WHILE2DO"));
    }
#endif

    syntax_tree = Trav (syntax_tree, arg_info);

#ifndef DBUG_OFF
    if (compiler_phase == PH_sacopt) {
        DBUG_PRINT ("OPT", ("                        done"));
    }
#endif

    act_tab = old_tab;

    arg_info = FreeTree (arg_info);

    DBUG_RETURN (syntax_tree);
}
