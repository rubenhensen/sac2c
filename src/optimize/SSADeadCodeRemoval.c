/*
 * $Log$
 * Revision 1.1  2001/02/23 13:37:50  nmw
 * Initial revision
 *

 */

/*****************************************************************************
 *
 * file:   SSADeadCodeRemoval.c
 *
 * prefix: SSADCR
 *
 * description:
 *    this module traverses ONE FUNCTION (and its liftet special funtions)
 *    and removes all dead code.
 *    this transformation is NOT conservative (it might remove
 *    endless loops)!
 *
 *****************************************************************************/

#include "dbug.h"
#include "globals.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "SSADeadCodeRemoval.h"

/* internal functions for traversing ids like nodes */
static ids *TravLeftIDS (ids *arg_ids, node *arg_info);
static ids *SSADCRleftids (ids *arg_ids, node *arg_info);
static ids *TravRightIDS (ids *arg_ids, node *arg_info);
static ids *SSADCRrightids (ids *arg_ids, node *arg_info);

/******************************************************************************
 *
 * function:
 *   node *SSADCRfundef(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRfundef");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRarg(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRarg");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRblock(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRblock");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRvardec(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRvardec");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRassign(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRassign");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRlet(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRlet");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRid(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRcond(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRdo(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRdo");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRwhile(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRwhile");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRreturn(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRreturn");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRap(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRap");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNwith(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNwith");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNpart(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNpart");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSADCRNcode(node *arg_node , node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADCRNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSADCRNcode");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSADCRleftids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
SSADCRleftids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSADCRleftids");

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *SSADCRrightids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
SSADCRrightids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSADCRrightids");

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravLeftIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
TravLeftIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravLeftIDS");

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   static ids *TravRightIDS(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
TravRightIDS (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("TravRightIDS");

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node *SSADeadCodeRemoval(node *fundef)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
SSADeadCodeRemoval (node *fundef)
{
    DBUG_ENTER ("SSADeadCodeRemoval");

    DBUG_RETURN (fundef);
}
