/*
 * $Log$
 * Revision 1.1  2001/02/13 15:16:15  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   SSATRansform.c
 *
 * prefix: SSA
 *
 * description:
 *    this module traverses the AST and transformes the code in SSA form.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "free.h"
#include "SSATransform.h"

static ids *TravIDS (ids *arg_ids, node *arg_info);
static ids *SSAids (ids *arg_ids, node *arg_info);

node *
SSAfundef (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAblock (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAexprs (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAlet (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAarg (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAvardec (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAid (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSANwithid (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAcond (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAreturn (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAdo (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSAwhile (node *arg_node, node *arg_info)
{
    return (NULL);
}

node *
SSATransform (node *arg_node)
{
    return (NULL);
}
