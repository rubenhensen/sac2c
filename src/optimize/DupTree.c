/*
 *
 * $Log$
 * Revision 1.1  1995/05/01 15:32:27  asi
 * Initial revision
 *
 */

#include "tree.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h"

#include "DupTree.h"
#include "optimize.h"

node *
DupTree (node *arg_node, node *arg_info)
{
    node *new_node;
    funptr *tmp_tab;

    DBUG_ENTER ("DupTree");

    tmp_tab = act_tab;
    act_tab = dup_tab;

    new_node = Trav (arg_node, arg_info);

    act_tab = tmp_tab;

    DBUG_RETURN (new_node);
}

node *
DupNum (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupNum");
    new_node = MakeNode (N_num);
    new_node->info.cint = arg_node->info.cint;
    DBUG_RETURN (new_node);
}

node *
DupBool (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupBool");
    new_node = MakeNode (N_bool);
    new_node->info.cint = arg_node->info.cint;
    DBUG_RETURN (new_node);
}

node *
DupFloat (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFloat");
    new_node = MakeNode (N_float);
    new_node->info.cfloat = arg_node->info.cfloat;
    DBUG_RETURN (new_node);
}

node *
DupId (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupId");
    new_node = MakeNode (N_id);
    new_node->info.ids = MakeIds (arg_node->info.ids->id);
    if (NULL != arg_info) {
        new_node->info.ids->node = arg_node->info.ids->node;
        INC_VAR (arg_info->mask[0], arg_node->info.ids->node->varno);
    }
    DBUG_RETURN (new_node);
}

node *
DupArray (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupArray");
    new_node = MakeNode (N_array);
    new_node->nnode = 1;
    new_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (new_node);
}

node *
DupExprs (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupExprs");
    new_node = MakeNode (N_exprs);
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}
