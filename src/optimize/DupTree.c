/*
 *
 * $Log$
 * Revision 1.13  1995/07/28 12:58:22  asi
 * added function DupInfo
 *
 * Revision 1.12  1995/07/24  09:08:05  asi
 * macro DUP moved from DupTree.c to DupTree.h, macro TYPES renamed to INL_TYPES
 *
 * Revision 1.11  1995/07/04  11:39:58  hw
 * DupDouble inserted
 *
 * Revision 1.10  1995/06/27  16:03:05  asi
 * added DUP-macro : Duplicating simple structure elements
 * and DUP inserted in each DUP... function
 *
 * Revision 1.9  1995/06/27  09:40:09  hw
 * bug fixed in DubIds( an ids-chain will be duplicated correctly now)
 *
 * Revision 1.8  1995/06/26  10:03:52  sbs
 * ids->use copie in DupIds
 *
 * Revision 1.7  1995/06/26  08:10:21  asi
 * now linenumbers will be duplicated
 * unused vaiable i warning removed
 *
 * Revision 1.6  1995/06/23  13:09:20  hw
 * - functions "DupDec" & "DupFundef" inserted
 * -  added argument to call of 'DuplicateTypes'
 *
 * Revision 1.5  1995/06/15  15:32:37  asi
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
#include "free.h"

#include "DupTree.h"
#include "optimize.h"
#include "Inline.h"

#define LEVEL arg_info->lineno

node *
DupTree (node *arg_node, node *arg_info)
{
    node *new_node = NULL;
    funptr *tmp_tab;

    DBUG_ENTER ("DupTree");

    if (NULL != arg_node) {
        tmp_tab = act_tab;
        act_tab = dup_tab;

        if (NULL == arg_info)
            arg_info = MakeNode (N_info);

        LEVEL = 0;
        new_node = Trav (arg_node, arg_info);

        act_tab = tmp_tab;
    }

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
    DUP (arg_node, new_node);
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
    DUP (arg_node, new_node);
    DBUG_RETURN (new_node);
}

node *
DupDouble (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDouble");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.cdbl = arg_node->info.cdbl;
    DUP (arg_node, new_node);
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
    DUP (arg_node, new_node);
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
        new_ids->node = SearchDecl (new_ids->id, INL_TYPES);
        DBUG_ASSERT ((NULL != new_ids->node),
                     ("No decleration found for %s", new_ids->id));
        break;
    default:
        new_ids = MakeIds (StringCopy (old_ids->id));
        new_ids->node = old_ids->node;
        new_ids->use = old_ids->use;
        break;
    }
    if (NULL != old_ids->next)
        new_ids->next = DupIds (old_ids->next, arg_info);
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
    DUP (arg_node, new_node);
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
    DUP (arg_node, new_node);
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
        DUP (arg_node, new_node);
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
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    DUP (arg_node, new_node);
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
    DUP (arg_node, new_node);
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
    DUP (arg_node, new_node);
    new_node->node[1] = arg_node->node[1];
    new_node->node[2] = arg_node->node[2];
    for (i = 0; i < arg_node->nnode; i++) {
        new_node->node[i] = Trav (arg_node->node[i], arg_info);
    }
    DBUG_RETURN (new_node);
}

node *
DupFundef (node *arg_node, node *arg_info)
{
    node *new_node;
    int i;

    DBUG_ENTER ("DupFundef");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    DUP (arg_node, new_node);
    for (i = 0; i < MAX_SONS; i++)
        if (NULL != arg_node->node[i])
            new_node->node[i] = Trav (arg_node->node[i], arg_info);
    DBUG_RETURN (new_node);
}

/*
 * This function is used for N_vardec & N_arg nodes
 *
 */
node *
DupDec (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupDec");
    DBUG_ASSERT (((N_vardec == arg_node->nodetype) || (N_arg == arg_node->nodetype)),
                 "wrong nodetype");
    DBUG_PRINT ("DUP", ("Duplicating - %s", mdb_nodetype[arg_node->nodetype]));
    new_node = MakeNode (arg_node->nodetype);
    DUP (arg_node, new_node);
    new_node->info.types = DuplicateTypes (arg_node->info.types, 1);
    if (NULL != arg_node->node[0]) {
        new_node->node[0] = Trav (arg_node->node[0], arg_info);
        new_node->nnode = 1;
    }

    DBUG_RETURN (new_node);
}

node *
DupInfo (node *arg_node, node *arg_info)
{
    node *new_node;

    DBUG_ENTER ("DupInfo");
    if (UNS_NO == arg_node->flag) {
        new_node = DupTree (UNS_NODES, arg_info);
        FreeTree (arg_node);
    } else {
        new_node = MakeNode (N_info);
        new_node->flag = arg_node->flag;
    }
    DBUG_RETURN (new_node);
}
