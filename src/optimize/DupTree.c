/*
 *
 * $Log$
 * Revision 1.5  1995/06/15 15:32:37  asi
 * DupTree generates arg_info if not present
 *
 * Revision 1.4  1995/06/08  09:55:13  asi
 * if arg_info->flag is set to INLINE variables will be renamed
 *
 * Revision 1.3  1995/06/02  11:25:48  asi
 * Added functions for all nodes below fundef node
 *
 * Revision 1.2  1995/05/03  12:41:51  asi
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
#include "internal_lib.h"

#include "DupTree.h"
#include "optimize.h"
#include "Inline.h"

#define LEVEL arg_info->lineno

node *
DupTree (node *arg_node, node *arg_info)
{
    node *new_node;
    funptr *tmp_tab;

    DBUG_ENTER ("DupTree");

    tmp_tab = act_tab;
    act_tab = dup_tab;

    if (NULL == arg_info)
        arg_info = MakeNode (N_info);

    LEVEL = 0;
    new_node = Trav (arg_node, arg_info);

    act_tab = tmp_tab;

    DBUG_RETURN (new_node);
}

node *
DupInt (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupInt");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.cint = arg_node->info.cint;
    DBUG_RETURN (new_node);
}

node *
DupFloat (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupFloat");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.cfloat = arg_node->info.cfloat;
    DBUG_RETURN (new_node);
}

node *
DupStr (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupStr");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.id = StringCopy (arg_node->info.id);
    DBUG_RETURN (new_node);
}

ids *
DupIds (ids *old_ids, node *arg_info)
{
    ids *new_ids;

    DBUG_ENTER ("DupIds");
    switch (DUPTYPE) {
    case INLINE:
        new_ids = MakeIds (RenameInlinedVar (old_ids->id));
        new_ids->node = SearchDecl (new_ids->id, TYPES);
        DBUG_ASSERT ((NULL != new_ids->node),
                     ("No decleration found for %s", new_ids->id));
        break;
    default:
        new_ids = MakeIds (StringCopy (old_ids->id));
        new_ids->node = old_ids->node;
        break;
    }
    if (NULL != old_ids->next)
        old_ids->next = DupIds (old_ids->next, arg_info);
    DBUG_RETURN (new_ids);
}

node *
DupIIds (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupIIds");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.ids = DupIds (arg_node->info.ids, arg_info);
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

node *
DupChain (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupChain");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        LEVEL++;
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
        LEVEL--;
    }
    DBUG_RETURN (new_node);
}

node *
DupAssign (node *arg_node, node *arg_info)
{
    node *new_node = NULL;
    int i;

    DBUG_ENTER ("DupAssign");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    switch (DUPTYPE) {
    case INLINE:
        if ((0 == LEVEL) && (N_return == arg_node->node[0]->nodetype))
            break;
    default:
        new_node = MakeNode (arg_node->nodetype);
        new_node->nnode = arg_node->nnode;

        for (i = 0; i < arg_node->nnode; i++) {
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
            if (NULL == new_node->node[i])
                new_node->nnode = i;
        }
        break;
    }
    DBUG_RETURN (new_node);
}

node *
DupCast (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupCast");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
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
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.prf = arg_node->info.prf;
    new_node->nnode = arg_node->nnode;
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

node *
DupFun (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupFun");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.fun_name.id = StringCopy (arg_node->info.fun_name.id);
    new_node->info.fun_name.id_mod = StringCopy (arg_node->info.fun_name.id_mod);
    new_node->nnode = arg_node->nnode;
    new_node->node[1] = arg_node->node[1];
    new_node->node[2] = arg_node->node[2];
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}
