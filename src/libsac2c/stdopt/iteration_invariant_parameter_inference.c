/*****************************************************************************
 *
 * $Id$
 *
 * file:   iteration_invariant_parameter_inference.c
 *
 * prefix: IIPI
 *
 * description:
 *
 *   This module identifies parameters of loop functions that are passed on
 *   directly to the recursive call. Typically, these are a result of a
 *   previously successfull application of LIR.
 *
 *   For example:
 *     loop( a, b, c, d) {
 *       ...
 *       if( loop-condition) {
 *         res = loop( c, b, a, x);
 *       } else {
 *         res = ...
 *       }
 *       ...
 *     }
 *
 *     would tag AVIS_SSALPINV( b) as TRUE and all other variables as FALSE.
 *     Parameters that are switched such as 'a' and 'c' in the example above
 *     must not be tagged as the values that are carried by these parameters
 *     in fact DO change from iteration to iteration.
 *
 *   on the implementation:
 *     The traversal always starts on non-LaC functions only. It follows
 *     applications of LaC functions until a recursive call of a loop function
 *     to itself is found. In that case, the actual argument expressions are
 *     put into the INFO structure and the formal parameters (arg-chain) of
 *     the loop function are traversed. While doing so, the iteration
 *     invariant parameters are tagged as such. A parameter is considered
 *     iteration invariant, iff its avis is identical to the corresponding
 *     actual argument's avis.
 *
 *     Whenever a new fundef is traversed we ensure that all avis nodes that
 *     belong to that very function are tagged as FALSE.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "str.h"
#include "memory.h"
#include "globals.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "ctinfo.h"
#include "iteration_invariant_parameter_inference.h"

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
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_EXPRCHAIN(n) (n->exprchain)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_EXPRCHAIN (result) = NULL;

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
 * @fn node *IIPIarg(node *arg_node, info *arg_info)
 *
 * @brief in do/while special functions: set the LIR attribute for the
 *        args by comparing the args with the corresponding identifier
 *        in the recursive call. if they are identical, the args is a
 *        loop-invariant arg, and will be tagged.
 *        This function relies on INFO_EXPRCHAIN to be set to the
 *        N_exprs chain of the recusive calls' arguments. This is done in
 *        IIPIap prior to traversing the args.
 *
 * @param arg_node N_arg node of a loop-function
 * @param arg_info the info structure
 *
 * @return
 */
node *
IIPIarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IIPIarg");

    /* Infer loop invariant args */
    DBUG_ASSERT ((INFO_EXPRCHAIN (arg_info) != NULL),
                 "reached IIPIarg without having a link to the args "
                 "of the recursive call!");
    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_EXPRCHAIN (arg_info))) == N_id),
                 "function args are no identifiers");

    /* compare arg and fun-ap argument */
    if (ARG_AVIS (arg_node) == ID_AVIS (EXPRS_EXPR (INFO_EXPRCHAIN (arg_info)))) {
        DBUG_PRINT ("IIPI", ("mark %s as loop-invariant", ARG_NAME (arg_node)));
        AVIS_SSALPINV (ARG_AVIS (arg_node)) = TRUE;
    } else {
        DBUG_PRINT ("IIPI", ("%s is non-loop-invariant", ARG_NAME (arg_node)));
    }

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_EXPRCHAIN (arg_info) = EXPRS_NEXT (INFO_EXPRCHAIN (arg_info));

        /* traverse to next arg */
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**  <!-- ***************************************************************** -->
 * @fn node* IIPIfundef(node *arg_node, info *arg_info)
 *
 * @brief traverses cond functions. The body is traversed in order
 *        to find the recursive call
 *
 * @param arg_node N_fundef node
 * @param arg_info info structure
 *
 * @return
 */

static node *
ATravAvis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ATravAvis");
    AVIS_SSALPINV (arg_node) = FALSE;
    DBUG_RETURN (arg_node);
}

node *
IIPIfundef (node *arg_node, info *arg_info)
{
    info *info;
    anontrav_t cleantrav[] = {{N_avis, &ATravAvis},
                              {N_block, &TRAVsons},
                              {N_arg, &TRAVsons},
                              {N_vardec, &TRAVsons},
                              {0, NULL}};

    DBUG_ENTER ("IIPIfundef");

    info = MakeInfo ();
    INFO_FUNDEF (info) = arg_node;

    /* set all avis tags AVIS_SSALPINV in this function to FALSE */
    TRAVpushAnonymous (cleantrav, &TRAVnone);
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), NULL);
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), NULL);
    TRAVpop ();

    /* traverse function body */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), info);

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @fn node *IIPIap(node *arg_node, info *arg_info)
 *
 * @brief detects whether the current function call is the recursive call of
 *        the given loop function. If so, INFO_EXPRCHAIN is set to
 *        the args of the N_ap node and the traversal of the N_arg chain
 *        stored in INFO_ARGCHAIN is started.
 *
 * @param arg_node N_ap node
 * @param arg_info info structure
 *
 * @return
 */
node *
IIPIap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IIPIap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    /*
     * if this is the recursive function call of this loop function
     * traverse into the saved fundef args and check them against
     * the exprs, so we save the exprs chain here...
     */
    if (FUNDEF_ISLACFUN (AP_FUNDEF (arg_node))) {
        if ((AP_FUNDEF (arg_node) == INFO_FUNDEF (arg_info))
            && (AP_ARGS (arg_node) != NULL)) {
            /* save args-exprs */
            INFO_EXPRCHAIN (arg_info) = AP_ARGS (arg_node);

            /* traverse arg chain of fundef */
            FUNDEF_ARGS (INFO_FUNDEF (arg_info))
              = TRAVdo (FUNDEF_ARGS (INFO_FUNDEF (arg_info)), arg_info);

            /* remove exprs chain */
            INFO_EXPRCHAIN (arg_info) = NULL;
        } else {
            DBUG_PRINT ("IIPI", ("traverse in special fundef %s",
                                 CTIitemName (AP_FUNDEF (arg_node))));

            AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

            DBUG_PRINT ("IIPI", ("traversal of special fundef %s finished\n",
                                 CTIitemName (AP_FUNDEF (arg_node))));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ***************************************************************** -->
 * @fn node *IIPIdoIterationInvariantParameterInference(node *fundef)
 *
 * @brief starts the inference at the given normal fundef.
 *
 * @param fundef a N_fundef node
 *
 * @return tagged N_fundef node
 */
node *
IIPIdoIterationInvariantParameterInference (node *fundef)
{
    DBUG_ENTER ("IIPIdoIterationInvariantParameterInference");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), "IIPIdoIterationInvariantParameterInfe"
                                                   "rence() is used for fundef nodes "
                                                   "only");

    if (!(FUNDEF_ISLACFUN (fundef))) {
        TRAVpush (TR_iipi);
        fundef = TRAVdo (fundef, NULL);
        TRAVpop ();
    }

    DBUG_RETURN (fundef);
}
