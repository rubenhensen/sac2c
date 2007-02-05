/*****************************************************************************
 *
 * $Id$
 *
 * file:   SSAInferLI.c
 *
 * prefix: ILI
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
#include "str.h"
#include "memory.h"
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
    node *exprchain;
};

/*
 * INFO macros
 */
#define INFO_ILI_FUNDEF(n) (n->fundef)
#define INFO_ILI_EXPRCHAIN(n) (n->exprchain)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ILI_FUNDEF (result) = NULL;
    INFO_ILI_EXPRCHAIN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!-- ***************************************************************** -->
 * @fn node *ILIarg(node *arg_node, info *arg_info)
 *
 * @brief in do/while special functions: set the LIR attribute for the
 *        args by comparing the args with the corresponding identifier
 *        in the recursive call. if they are identical the args is a loop
 *        invariant arg and will be tagged.
 *        This function relieas on INFO_ILI_EXPRCHAIN to be set to the
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
    DBUG_ASSERT ((INFO_ILI_EXPRCHAIN (arg_info) != NULL),
                 "reached ILIarg without having a link to the args "
                 "of the recursive call!");
    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_ILI_EXPRCHAIN (arg_info))) == N_id),
                 "function args are no identifiers");

    /* compare arg and fun-ap argument */
    if (ARG_AVIS (arg_node) == ID_AVIS (EXPRS_EXPR (INFO_ILI_EXPRCHAIN (arg_info)))) {
        DBUG_PRINT ("ILI", ("mark %s as loop invariant", ARG_NAME (arg_node)));
        AVIS_SSALPINV (ARG_AVIS (arg_node)) = TRUE;
    } else {
        DBUG_PRINT ("ILI", ("mark %s as non loop invariant", ARG_NAME (arg_node)));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_ILI_EXPRCHAIN (arg_info) = EXPRS_NEXT (INFO_ILI_EXPRCHAIN (arg_info));

        /* traverse to next arg */
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**  <!-- ***************************************************************** -->
 * @fn node* ILIfundef(node *arg_node, info *arg_info)
 *
 * @brief traverses cond functions. The body is traversed in order
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
    info *info;

    DBUG_ENTER ("ILIfundef");

    info = MakeInfo ();
    INFO_ILI_FUNDEF (info) = arg_node;

    /* traverse function body */
    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), info);
    }

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @fn node *ILIap(node *arg_node, info *arg_info)
 *
 * @brief detects whether the current function call is the recursive call of
 *        the given loop function. If so, INFO_ILI_EXPRCHAIN is set to
 *        the args of the N_ap node and the traversal of the N_arg chain
 *        stored in INFO_ILI_ARGCHAIN is started.
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
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        if ((AP_FUNDEF (arg_node) == INFO_ILI_FUNDEF (arg_info))
            && (AP_ARGS (arg_node) != NULL)) {
            /* save args-exprs */
            INFO_ILI_EXPRCHAIN (arg_info) = AP_ARGS (arg_node);

            /* traverse arg chain of fundef */
            FUNDEF_ARGS (INFO_ILI_FUNDEF (arg_info))
              = TRAVdo (FUNDEF_ARGS (INFO_ILI_FUNDEF (arg_info)), arg_info);

            /* remove exprs chain */
            INFO_ILI_EXPRCHAIN (arg_info) = NULL;
        } else {
            DBUG_PRINT ("ILI", ("traverse in special fundef %s",
                                FUNDEF_NAME (AP_FUNDEF (arg_node))));

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            DBUG_PRINT ("ILI", ("traversal of special fundef %s finished\n",
                                FUNDEF_NAME (AP_FUNDEF (arg_node))));
        }
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
    DBUG_ENTER ("ILIdoInferLoopInvariants");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "ILIdoInferLoopInvariants() is used for fundef nodes only");

    if (!(FUNDEF_ISLACFUN (fundef))) {
        TRAVpush (TR_ili);
        fundef = TRAVdo (fundef, NULL);
        TRAVpop ();
    }

    DBUG_RETURN (fundef);
}
