/*
 *
 * $Log$
 * Revision 1.1  2004/11/23 22:21:37  sah
 * Initial revision
 *
 *
 *
 */

#include "traverse_helper.h"

/*
**
**  functionname  : TravSons
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : traverses all son nodes.
**  global vars   : ---
**  internal funs : Trav
**  external funs : ---
**
**  remarks       : TravSons can be used as dummy function for fun_tab entries
**                  where a specific function for a particular node type is
**                  not yet implemented or not necessary.
**
*/

node *
TRAVSons (node *arg_node, info *arg_info)
{
    int i;

    DBUG_ENTER ("TravSons");

    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
            arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    DBUG_RETURN (arg_node);
}

/*
**
**  functionname  : NoTrav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : does nothing on the given syntax tree,
**                  especially no further sons are traversed.
**                  The given son is returned unmodified.
**  global vars   : ---
**  internal funs : ---
**  external funs : ---
**
**  remarks       : NoTrav can be used as fun_tab entry where no further
**                  traversal of the syntax tree is needed in order
**                  to avoid unnecessary work.
**
*/

node *
TRAVNone (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TravNone");
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *TravError(node *arg_node, info *arg_info)
 *
 * description:
 *
 *  This function can be used whenever a certain node type is illegal in
 *  some compiler traversal phase. It helps to detect consistency failures
 *  in the abstract syntax tree.
 *
 ******************************************************************************/

node *
TRAVError (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TravError");

    DBUG_ASSERT ((FALSE), "Illegal node type found.");

    DBUG_RETURN (arg_node);
}
