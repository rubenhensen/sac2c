/*
 *
 * $Log$
 * Revision 1.2  2001/03/29 16:31:55  nmw
 * detection of loop invariant args implemented
 *
 * Revision 1.1  2001/03/26 15:37:45  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSALIR.c
 *
 * prefix: SSALIR
 *
 * description:
 *   this module implements loop invariant removal on code in ssa form.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "internal_lib.h"
#include "traverse.h"
#include "free.h"
#include "optimize.h"
#include "SSALIR.h"

/* functions for local usage only */
static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSALIRids (ids *arg_ids, node *arg_info);

/* traversal functions */
/******************************************************************************
 *
 * function:
 *   node* SSALIRfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRfundef");

    DBUG_PRINT ("SSALIR",
                ("loop invariant removal in fundef %s", FUNDEF_NAME (arg_node)));

    INFO_SSALIR_FUNDEF (arg_info) = arg_node;

    /* traverse args of do/while special functions to infere loop invariant args */
    if ((FUNDEF_ARGS (arg_node) != NULL)
        && ((FUNDEF_STATUS (arg_node) == ST_dofun)
            || (FUNDEF_STATUS (arg_node) == ST_whilefun))) {

        DBUG_ASSERT ((FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)) != NULL),
                     "missing assignment link to internal recursive call");
        DBUG_ASSERT ((ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))
                      != NULL),
                     "missing assigment instruction");
        DBUG_ASSERT ((NODE_TYPE (LET_EXPR (
                        ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))))
                      == N_ap),
                     "missing recursive call in do/while special function");

        INFO_SSALIR_ARGCHAIN (arg_info) = AP_ARGS (
          LET_EXPR (ASSIGN_INSTR (FUNDEF_INT_ASSIGN (INFO_SSALIR_FUNDEF (arg_info)))));

        FUNDEF_ARGS (arg_node) = Trav (FUNDEF_ARGS (arg_node), arg_info);
    }

    if (FUNDEF_BODY (arg_node) != NULL) {
        FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRarg(node *arg_node, node *arg_info)
 *
 * description:
 *   in do/while special functions: set the SSALIR attribute for the args by
 *   comparing the args with the corresponding identifier in the recursive
 *   call. if they are identical the args is a loop invariant arg and will be
 *   tagged.
 *
 ******************************************************************************/
node *
SSALIRarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRarg");

    DBUG_ASSERT ((INFO_SSALIR_ARGCHAIN (arg_info) != NULL),
                 "different chains: args/call args");

    DBUG_ASSERT ((NODE_TYPE (EXPRS_EXPR (INFO_SSALIR_ARGCHAIN (arg_info)))),
                 "function args are no identifiers");

    /* compare arg and fun-ap argument */
    if (ARG_AVIS (arg_node) == ID_AVIS (EXPRS_EXPR (INFO_SSALIR_ARGCHAIN (arg_info)))) {
        DBUG_PRINT ("SSALIR", ("mark %s as loop invariant", ARG_NAME (arg_node)));
        if (AVIS_SSALPINV (ARG_AVIS (arg_node)) != TRUE) {
            lir_expr++;
            AVIS_SSALPINV (ARG_AVIS (arg_node)) = TRUE;
        }
    } else {
        DBUG_PRINT ("SSALIR", ("mark %s as non loop invariant", ARG_NAME (arg_node)));
        AVIS_SSALPINV (ARG_AVIS (arg_node)) = FALSE;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        INFO_SSALIR_ARGCHAIN (arg_info) = EXPRS_NEXT (INFO_SSALIR_ARGCHAIN (arg_info));
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRvardec(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRvardec");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRblock(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse vardecs and block in this order
 *
 ******************************************************************************/
node *
SSALIRblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRblock");

    if (BLOCK_VARDEC (arg_node) != NULL) {
        BLOCK_VARDEC (arg_node) = Trav (BLOCK_VARDEC (arg_node), arg_info);
    }

    if (BLOCK_INSTR (arg_node) != NULL) {
        BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRassign(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse assign instructions in top-down order to infere LI-assignments.
 *
 ******************************************************************************/
node *
SSALIRassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRassign");

    DBUG_ASSERT ((ASSIGN_INSTR (arg_node)), "missing instruction in assignment");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    /* traverse next assignment */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRlet(node *arg_node, node *arg_info)
 *
 * description:
 *   traverse let expression and result identifiers
 *
 ******************************************************************************/
node *
SSALIRlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRlet");

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = Trav (LET_EXPR (arg_node), arg_info);
    }

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TravIDS (LET_IDS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRap (node *arg_node, node *arg_info)
{
    node *new_arg_info;

    DBUG_ENTER ("SSALIRap");

    DBUG_ASSERT ((AP_FUNDEF (arg_node) != NULL), "missing fundef in ap-node");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = Trav (AP_ARGS (arg_node), arg_info);
    }

    /* traverse special fundef without recursion */
    if (((FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_condfun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_dofun)
         || (FUNDEF_STATUS (AP_FUNDEF (arg_node)) == ST_whilefun))
        && (AP_FUNDEF (arg_node) != INFO_SSALIR_FUNDEF (arg_info))) {
        DBUG_PRINT ("SSALIR", ("traverse in special fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));

        /* stack arg_info frame for new fundef */
        new_arg_info = MakeInfo ();

        /* start traversal of special fundef */
        AP_FUNDEF (arg_node) = Trav (AP_FUNDEF (arg_node), new_arg_info);

        DBUG_PRINT ("SSALIR", ("traversal of special fundef %s finished\n",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
        FREE (new_arg_info);

    } else {
        /* no traversal into a normal fundef */
        DBUG_PRINT ("SSALIR", ("do not traverse in normal fundef %s",
                               FUNDEF_NAME (AP_FUNDEF (arg_node))));
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRreturn(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRreturn");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRNwith");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRNwithid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   ids *TravIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *   similar implementation of trav mechanism as used for nodes
 *   here used for ids.
 *
 ******************************************************************************/
static ids *
TravIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravIDS");

    DBUG_ASSERT (arg_ids != NULL, "traversal in NULL ids");
    arg_ids = SSALIRids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSALIRids (ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
SSALIRids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSALIRids");

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSALoopInvariantRemoval(node* fundef)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALoopInvariantRemoval (node *fundef)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("SSALoopInvariantRemoval");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "SSALoopInvariantRemoval called for non-fundef node");

    DBUG_PRINT ("SSALIR", ("starting loop independent removal in function %s",
                           FUNDEF_NAME (fundef)));

    /* do not start traversal in special functions */
    if ((FUNDEF_STATUS (fundef) != ST_condfun) && (FUNDEF_STATUS (fundef) != ST_dofun)
        && (FUNDEF_STATUS (fundef) != ST_whilefun)) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = ssalir_tab;

        fundef = Trav (fundef, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (fundef);
}
