/*
 *
 * $Log$
 * Revision 1.3  2004/11/26 17:20:43  mwe
 * SacDevCamp: Compiles!
 *
 * Revision 1.2  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
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
#include "node_basic.h"
#include "internal_lib.h"
#include "globals.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "SSAInferLI.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *argchain;
};

/*
 * INFO macros
 */
#define INFO_SSAILI_FUNDEF(n) (n->fundef)
#define INFO_SSAILI_ARGCHAIN(n) (n->argchain)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SSAILI_FUNDEF (result) = NULL;
    INFO_SSAILI_ARGCHAIN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * function:
 *  node *ILIarg(node *arg_node, info *arg_info)
 *
 * description:
 *   in do/while special functions: set the SSALIR attribute for the args by
 *   comparing the args with the corresponding identifier in the recursive
 *   call. if they are identical the args is a loop invariant arg and will be
 *   tagged.
 *
 ******************************************************************************/
node *
ILIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ILIarg");

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
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node* ILIfundef(node *arg_node, info *arg_info)
 *
 * description:
 *  traverses arg nodes and block in this order.
 *
 ******************************************************************************/
node *
ILIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ILIfundef");

    INFO_SSAILI_FUNDEF (arg_info) = arg_node;

    /* traverse args of special (loop) functions to infere loop invariant args */
    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_ISDOFUN (arg_node))) {

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
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
    }

    /* traverse function body */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *ILIap(node *arg_node, info *arg_info)
 *
 * description:
 *  traverse into special fundef if non-recursive call
 *
 ******************************************************************************/
node *
ILIap (node *arg_node, info *arg_info)
{
    info *new_arg_info;

    DBUG_ENTER ("ILIap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion (only in single fundef mode) */
    if ((FUNDEF_ISLACFUN (AP_FUNDEF (arg_node)))
        && (AP_FUNDEF (arg_node) != INFO_SSAILI_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSAILI", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSAILI", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        new_arg_info = FreeInfo (new_arg_info);
    } else {
        DBUG_PRINT ("SSAILI", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *ILIdoInferLoopInvariants(node *fundef)
 *
 * description:
 *  starts the loop invariant inference for the given normal fundef.
 *
 ******************************************************************************/
node *
ILIdoInferLoopInvariants (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("ILIdoInferLoopInvariants");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "ILIdoInferLoopInvariants() is used for fundef nodes only");

    if (!(FUNDEF_ISLACFUN (fundef))) {

        arg_info = MakeInfo ();

        TRAVpush (TR_ili);
        fundef = TRAVdo (fundef, arg_info);
        TRAVpop ();

        arg_info = FreeInfo (arg_info);
    }

    DBUG_RETURN (fundef);
}
