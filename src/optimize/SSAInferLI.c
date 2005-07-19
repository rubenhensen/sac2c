/*
 *
 * $Log$
 * Revision 1.4  2005/07/19 16:53:08  sah
 * removed INT/EXT_ASSIGN
 *
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
    node *exprchain;
};

/*
 * INFO macros
 */
#define INFO_SSAILI_FUNDEF(n) (n->fundef)
#define INFO_SSAILI_ARGCHAIN(n) (n->argchain)
#define INFO_SSAILI_EXPRCHAIN(n) (n->exprchain)

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

/** <!-- ***************************************************************** -->
 * @fn node *ILIarg(node *arg_node, info *arg_info)
 *
 * @brief in do/while special functions: set the SSALIR attribute for the
 *        args by comparing the args with the corresponding identifier
 *        in the recursive call. if they are identical the args is a loop
 *        invariant arg and will be tagged.
 *        This function relieas on INFO_SSAILI_EXPRCHAIN to be set to the
 *        N_exprs chain of the recusive calls' arguments. This is done in
 *        ILIap prior to traversing the args.
 *
 * @param arg_node N_arg node of a loop-function
 * @param arg_info the info structure
 *
 * @return
 */
node *
ILIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ILIarg");

    /* infere loop invarinat args */
    DBUG_ASSERT ((INFO_SSAILI_EXPRCHAIN (arg_info) != NULL),
                 "reached ILIarg without having a link to the args "
                 "of the recursive call!");
    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_SSAILI_EXPRCHAIN (arg_info)))),
                 "function args are no identifiers");

    /* compare arg and fun-ap argument */
    if (ARG_AVIS (arg_node) == ID_AVIS (EXPRS_EXPR (INFO_SSAILI_EXPRCHAIN (arg_info)))) {
        DBUG_PRINT ("SSAILI", ("mark %s as loop invariant", ARG_NAME (arg_node)));
        if (AVIS_SSALPINV (ARG_AVIS (arg_node)) != TRUE) {
            AVIS_SSALPINV (ARG_AVIS (arg_node)) = TRUE;
        }
    } else {
        DBUG_PRINT ("SSAILI", ("mark %s as non loop invariant", ARG_NAME (arg_node)));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        /* when checking for LI-args traverse to next parameter of recursive call */
        if (INFO_SSAILI_EXPRCHAIN (arg_info) != NULL) {
            INFO_SSAILI_EXPRCHAIN (arg_info)
              = EXPRS_NEXT (INFO_SSAILI_EXPRCHAIN (arg_info));
        }

        /* traverse to next arg */
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**  <!-- ***************************************************************** -->
 * @fn node* ILIfundef(node *arg_node, info *arg_info)
 *
 * @brief traverses cond functions. the argument N_arg chain is stored
 *        in INFO_SSAILI_ARGCHAIN and then the body is traversed in order
 *        to find the recursive call
 *
 * @param arg_node N_fundef node
 * @param arg_info info structure
 *
 * @return
 */
node *
ILIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ILIfundef");

    INFO_SSAILI_FUNDEF (arg_info) = arg_node;

    /* traverse args of special (loop) functions to infere loop invariant args */
    if ((FUNDEF_ARGS (arg_node) != NULL) && (FUNDEF_ISDOFUN (arg_node))) {

        /* save pointer to argchain of function definition */
        INFO_SSAILI_ARGCHAIN (arg_info) = FUNDEF_ARGS (arg_node);

        /* save pointer to current fundef */
        INFO_SSAILI_FUNDEF (arg_info) = arg_node;

        /* traverse function body */
        if (FUNDEF_BODY (arg_node) != NULL) {
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }

        /* remove pointers again */
        INFO_SSAILI_FUNDEF (arg_info) = NULL;
        INFO_SSAILI_ARGCHAIN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @fn node *ILIap(node *arg_node, info *arg_info)
 *
 * @brief detects whether the current function call is the recursive call of
 *        the given loop function. If so, INFO_SSAILI_EXPRCHAIN is set to
 *        the args of the N_ap node and the traversal of the N_arg chain
 *        stored in INFO_SSAILI_ARGCHAIN is started.
 *
 * @param arg_node N_ap node
 * @param arg_info info structure
 *
 * @return
 */
node *
ILIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ILIap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /*
     * if this is the recursive function call of this loop function
     * traverse into the saved fundef args and check them against
     * the exprs, so we save the exprs chain here...
     */
    if ((AP_FUNDEF (arg_node) == INFO_SSAILI_FUNDEF (arg_info))
        && (AP_ARGS (arg_node) != NULL)) {
        /* save args-exprs */
        INFO_SSAILI_EXPRCHAIN (arg_info) = AP_ARGS (arg_node);

        /* traverse arg chain of fundef */
        INFO_SSAILI_ARGCHAIN (arg_info)
          = TRAVdo (INFO_SSAILI_ARGCHAIN (arg_info), arg_info);

        /* remove exprs chain */
        INFO_SSAILI_EXPRCHAIN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @fn node *ILIdoInferLoopInvariants(node *fundef)
 *
 * @brief starts the loop invariant inference for the given normal fundef.
 *
 * @param fundef a N_fundef node
 *
 * @return tagged N_fundef node
 */
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
