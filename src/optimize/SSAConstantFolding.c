/*
 * $Log$
 * Revision 1.1  2001/03/20 16:16:54  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSAConstantFolding.c
 *
 * prefix: SSACF
 *
 * description:
 *   this module does constant folding on code in ssa form.
 *
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "SSAConstantFolding.h"

/* traversal like functions for ids structures */
static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSACFids (ids *arg_ids, node *arg_info);

/* functions for internal use only */

/* traversal functions */

/******************************************************************************
 *
 * function:
 *   node* SSACFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFfundef");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFblock(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFblock");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarg(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarg");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFvardec(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFvardec (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFvardec");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFassign");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFdo(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFdo");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFwhile(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFwhile");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFreturn(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFreturn");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFlet");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFap");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFnum(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFnum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFnum");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFfloat(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFfloat (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFfloat");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFdouble(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFdouble (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFdouble");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFchar(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFchar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFchar");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFbool(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFbool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFbool");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFstr(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFstr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFstr");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFarray(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFarray");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* SSACFprf(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSACFprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SSACFprf");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SSACFids(node *arg_ids, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/
static ids *
SSACFids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("SSACFids");

    /* do something useful */

    if (IDS_NEXT (arg_ids) != NULL) {
        IDS_NEXT (arg_ids) = TravIDS (IDS_NEXT (arg_ids), arg_info);
    }

    DBUG_RETURN (arg_ids);
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
    arg_ids = SSACFids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   node* SSAConstantFolding(node* syntax_tree)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
node *
SSAConstantFolding (node *syntax_tree)
{
    DBUG_ENTER ("SSAConstantFolding");

    DBUG_RETURN (syntax_tree);
}
