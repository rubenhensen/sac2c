/*
 *
 * $Log$
 * Revision 1.1  2001/05/30 13:48:36  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSAInferLI.c
 *
 * prefix: SSAILI
 *
 * description:
 *
 *   This module infers the loop invariant args of do and while loops.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "SSAInferLI.h"

/******************************************************************************
 *
 * function:
 *  node *SSAILIarg(node *arg_node, node *arg_info)
 *
 * description:
 *   in do/while special functions: set the SSALIR attribute for the args by
 *   comparing the args with the corresponding identifier in the recursive
 *   call. if they are identical the args is a loop invariant arg and will be
 *   tagged.
 *
 ******************************************************************************/
node *
SSAILIarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAILIarg");

    /* infere loop invarinat args */
    if (INFO_SSAILI_ARGCHAIN (arg_info) != NULL) {
        DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_SSAILI_ARGCHAIN (arg_info)))),
                     "function args are no identifiers");

        /* compare arg and fun-ap argument */
        if (ARG_AVIS (arg_node)
            == ID_AVIS (EXPRS_EXPR (INFO_SSAILI_ARGCHAIN (arg_info)))) {
            DBUG_PRINT ("SSAILI", ("mark %s as loop invariant", ARG_NAME (arg_node)));
            if (AVIS_SSALPINV (ARG_AVIS (arg_node)) != TRUE) {
                AVIS_SSALPINV (ARG_AVIS (arg_node)) = TRUE;
            }
        } else {
            DBUG_PRINT ("SSAILI", ("mark %s as non loop invariant", ARG_NAME (arg_node)));
        }
    }

    if (ARG_NEXT (arg_node) != NULL) {
        /* when checking for LI-args traverse to next parameter of recursive call */
        if (INFO_SSAILI_ARGCHAIN (arg_info) != NULL) {
            INFO_SSAILI_ARGCHAIN (arg_info)
              = EXPRS_NEXT (INFO_SSAILI_ARGCHAIN (arg_info));
        }

        /* traverse to next arg */
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* SSAILIfundef(node *arg_node, node *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
SSAILIfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSAILIfundef");

    INFO_SSAILI_FUNDEF (arg_info) = arg_node;

    /* traverse args of special (loop) functions to infere loop invariant args */
    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_IS_LOOPFUN (arg_node))) {

        DBUG_ASSERT ((FUNDEF_INT_ASSIGN (arg_node) != NULL),
                     "missing assignment link to internal recursive call");
        DBUG_ASSERT ((ASSIGN_INSTR (FUNDEF_INT_ASSIGN (arg_node)) != NULL),
                     "missing internal assigment instruction");
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (arg_node))))
                      == N_ap),
                     "missing recursive call in do/while special function");

        /* save pointer to argchain of recursive function application */
        INFO_SSAILI_ARGCHAIN (arg_info) = AP_ARGS (
          LET_EXPR (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSAILI_FUNDEF (arg_info)))));

    } else {
        /* non loop function */
        INFO_SSAILI_ARGCHAIN (arg_info) = NULL;
    }

    /* traverse args */
    if (FUNDEF_ARGS (arg_node) != NULL) {
        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    /* traverse function body */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *SSAILIap(node *arg_node, node *arg_info)
 *
 * description:
 *  traverse into special fundef if non-recursive call
 *
 ******************************************************************************/
node *
SSAILIap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSAILIap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion (only in single fundef mode) */
    if ((FUNDEF_IS_LACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_SSAILI_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSAILI", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSAILI", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        new_arg_info = FreeTree (new_arg_info);
    } else {
        DBUG_PRINT ("SSAILI", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSAInferLoopInvariants(node *fundef)
 *
 * description:
 *  starts the loop invariant inference for the given normal fundef.
 *
 ******************************************************************************/
node *
SSAInferLoopInvariants (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSAInferLoopInvariants");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSAInferLoopInvariants() is used for fundef nodes only");

    if (!(FUNDEF_IS_LACFUN (fundef))) {

        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = ssaili_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        arg_info = FreeTree (arg_info);
    }

    DBUG_RETURN (fundef);
}
