/*
 * $Log$
 * Revision 1.1  2001/03/06 13:16:50  nmw
 * Initial revision
 *
 *
 *
 */

/*****************************************************************************
 *
 * file:   compare_tree.c
 *
 * prefix: CMPT
 *
 * description:
 *   this module implements a literal tree compare for two given parts of
 *   the ast. it compares for equal structre, identifiers and values.
 *   this modules is used by SSACSE to find common subexpressions.
 *
 *****************************************************************************/
#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "compare_tree.h"

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *CMPTids (ids *arg_ids, node *arg_info);

/******************************************************************************
 *
 * function:
 *   node* CMPTnum(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTnum (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTnum");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTchar(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTchar (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTchar");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTbool(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTbool (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTbool");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTstr(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTstr (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTstr");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTfloat(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTfloat (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTfloat");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTdouble(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTdouble (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTdouble");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTblock(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTblock");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTassign(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTassign");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTreturn(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTreturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTreturn");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTdo(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTdo (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTdo");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTwhile(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTwhile");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTcond(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTcond");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTlet(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTlet");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTprf(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTprf");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTap(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTap");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTempty(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTempty (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTempty");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTexprs(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTexprs (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTexprs");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTarray(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTarray (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTarray");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNwith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNwith");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNpart(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNpart (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNpart");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNwithid(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNwithid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNwithid");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNgenerator(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNgenerator (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNgenerator");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNwithop(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNwithop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNwithop");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTNcode(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTNcode (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTNcode");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* CMPTunknown(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
node *
CMPTunknown (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CMPTunknown");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static ids *CMPTids(ids *arg_ids, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/
static ids *
CMPTids (ids *arg_ids, node *arg_info)
{
    DBUG_ENTER ("CMPTids");

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
    arg_ids = CMPTids (arg_ids, arg_info);

    DBUG_RETURN (arg_ids);
}

/******************************************************************************
 *
 * function:
 *   cmptree_t CompareTree(node* tree1, node *tree2)
 *
 * description:
 *
 *
 *
 ******************************************************************************/
cmptree_t
CompareTree (node *tree1, node *tree2)
{
    cmptree_t result;

    DBUG_ENTER ("CompareTree");

    result = CMPT_UKNWN;

    DBUG_RETURN (result);
}
