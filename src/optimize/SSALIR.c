/*
 *
 * $Log$
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

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRarg(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRarg");

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
 *
 *
 ******************************************************************************/
node *
SSALIRblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRblock");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRassign");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSALIRlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSALIRlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSALIRlet");

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
    DBUG_ENTER ("SSALIRap");

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
    DBUG_ENTER ("SSALoopInvariantRemoval");

    DBUG_RETURN (fundef);
}
