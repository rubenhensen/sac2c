/*
 *
 * $Log$
 * Revision 1.2  1995/05/03 12:41:51  asi
 * added DupPrf, DupAp and DupIds
 *
 * Revision 1.1  1995/05/01  15:32:27  asi
 * Initial revision
 *
 */

#include <string.h>

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

ids *
DupIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupIds");
    new_ids = MakeIds (strdup (old_ids->id));
    new_ids->node = old_ids->node;
    if (NULL != arg_info) {
        INC_VAR (arg_info->mask[0], old_ids->node->varno);
    }
    if (NULL != old_ids->next) {
        old_ids->next = DupIds (old_ids->next, arg_info);
    }
    DBUG_RETURN (new_ids);
}

node *
DupId (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupId");
    new_node = MakeNode (N_id);
    new_node->info.ids = DupIds (arg_node->info.ids, arg_info);
    new_node->node[0] = arg_node->node[0];
    DBUG_RETURN (new_node);
}

node *
DupArray (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupArray");
    new_node = MakeNode (N_array);
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
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

node *
DupCast (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupCast");
    new_node = MakeNode (N_cast);
    new_node->info.types = DuplicateTypes (arg_node->info.types);
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

node *
DupPrf (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupPrf");
    new_node = MakeNode (N_prf);
    new_node->info.prf = arg_node->info.prf;
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

node *
DupAp (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupAp");
    new_node = MakeNode (N_ap);
    new_node->info.fun_name.id = strdup (arg_node->info.fun_name.id);
    new_node->info.fun_name.id_mod = strdup (arg_node->info.fun_name.id_mod);
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    new_node->node[2] = arg_node->node[2];
    DBUG_RETURN (new_node);
}
