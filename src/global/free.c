/*
 *
 * $Log$
 * Revision 1.10  1995/08/24 14:01:56  cg
 * minor changes concerning objects etc.
 *
 * Revision 1.9  1995/06/16  13:10:46  asi
 * added FreePrf2, which will free memory occupied by a N_prf-subtree,
 *                 but it will not free one given argument.
 *
 * Revision 1.8  1995/03/24  15:42:31  asi
 * Bug fixed in FreeMask
 *
 * Revision 1.7  1995/03/17  15:54:56  hw
 * changed function FreeInfoType
 *
 * Revision 1.6  1995/01/26  15:16:55  asi
 * *** empty log message ***
 *
 * Revision 1.5  1995/01/18  17:28:37  asi
 * Added FreeTree, FreeNoInfo, FreeInfoId, FreeInfoIds, FreeInfoType, FreeModul
 *
 * Revision 1.4  1994/12/30  16:59:33  sbs
 * added FreeIds
 *
 * Revision 1.3  1994/12/20  17:42:23  hw
 * added includes dbug.h & stdio.h
 *
 * Revision 1.2  1994/12/20  17:34:35  hw
 * bug fixed in FreeIdsOnly
 * exchanged stdio.h with stdlib.h
 *
 * Revision 1.1  1994/12/20  15:42:10  sbs
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include "dbug.h"
#include "tree.h"
#include "traverse.h"
#include "optimize.h"

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

void
FreeIds (ids *ids)

{
    DBUG_ENTER ("FreeIds");
    if (ids->next != NULL)
        FreeIds (ids->next);
    DBUG_ASSERT ((ids->id != NULL), "Ids without name of identificator");
    FREE (ids->id);
    FREE (ids);
    DBUG_VOID_RETURN;
}

void
FreeIdsOnly (ids *ids)

{
    DBUG_ENTER ("FreeIdsOnly");
    if (ids->next != NULL)
        FreeIdsOnly (ids->next);
    FREE (ids);
    DBUG_VOID_RETURN;
}

void
FreeImplist (node *implist)

{
    int i;

    DBUG_ENTER ("FreeImplist");
    if (implist->node[0] != NULL)
        FreeImplist (implist->node[0]);
    for (i = 1; i < MAX_SONS; i++)
        if (implist->node[i] != NULL)
            FreeIds ((ids *)implist->node[i]);
    FREE (implist);
    DBUG_VOID_RETURN;
}

void
FreeMask (node *arg_node)
{
    int i;
    DBUG_ENTER ("FreeMask");
    for (i = 0; i < MAX_MASK; i++) {
        if (arg_node->mask[i] != NULL)
            FREE (arg_node->mask[i]);
    }
    DBUG_VOID_RETURN;
}

void
FreeShpseg (shpseg *shpseg)
{
    DBUG_ENTER ("FreeTypes");
    if (shpseg->next != NULL)
        FreeShpseg (shpseg->next);
    FREE (shpseg);
    DBUG_VOID_RETURN;
}

void
FreeTypes (types *type)
{
    DBUG_ENTER ("FreeTypes");
    if (type->next != NULL)
        FreeTypes (type->next);
    if (type->shpseg != NULL)
        FreeShpseg (type->shpseg);
    if (type->name != NULL)
        FREE (type->name);
    if (type->id != NULL)
        FREE (type->id);
    if (type->name_mod != NULL)
        FREE (type->name_mod);
    if (type->id_mod != NULL)
        FREE (type->id_mod);
    DBUG_VOID_RETURN;
}

void
FreeVarArg (node *var)
{
    DBUG_ENTER ("FreeVarArg");
    if (var->node[0] != NULL)
        FreeVarArg (var->node[0]);
    if (var->info.types != NULL)
        FreeTypes (var->info.types);
    FREE (var);
    DBUG_VOID_RETURN;
}

node *
FreeModul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("FreeModul");
    if (arg_node->node[0] != NULL)
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    if (arg_node->node[1] != NULL)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    if (arg_node->node[2] != NULL)
        arg_node->node[2] = Trav (arg_node->node[2], arg_info);
    if (arg_node->node[3] != NULL)
        arg_node->node[3] = Trav (arg_node->node[3], arg_info);
    if (arg_node->node[4] != NULL)
        arg_node->node[4] = Trav (arg_node->node[4], arg_info);
    FREE (arg_node);
    DBUG_RETURN ((node *)NULL);
}

node *
FreeNoInfo (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("FreeNoInfo");
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    FreeMask (arg_node);
    FREE (arg_node);
    DBUG_RETURN ((node *)NULL);
}

node *
FreeInfoIds (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("FreeInfoIds");
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    FreeMask (arg_node);
    /*  FreeIds(arg_node->info.ids);*/
    FREE (arg_node);
    DBUG_RETURN ((node *)NULL);
}

node *
FreeInfoId (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("FreeInfoId");
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    FreeMask (arg_node);
    FREE (arg_node->info.id);
    FREE (arg_node);
    DBUG_RETURN ((node *)NULL);
}

node *
FreeInfoType (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("FreeInfoType");
    for (i = 0; i < arg_node->nnode; i++)
        if (NULL != arg_node->node[i]) {
            arg_node->node[i] = Trav (arg_node->node[i], arg_info);
        }
    FreeMask (arg_node);
    FreeTypes (arg_node->info.types);
    FREE (arg_node);
    DBUG_RETURN ((node *)NULL);
}

/*
 *
 *  functionname  : FreeTree
 *  arguments     : 1) ptr to root of the (sub)tree
 *  description   : changes to functionlist free_tab, which frees the (sub)tree
 *  global vars   : syntax_tree, act_tab, free_tab
 *  internal funs : ---
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
void
FreeTree (node *arg_node)
{
    funptr *tmp_tab;

    DBUG_ENTER ("FreeTree");
    tmp_tab = act_tab;
    act_tab = free_tab;
    arg_node = Trav (arg_node, NULL);
    act_tab = tmp_tab;
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : FreePrf2
 *  arguments     : 1) prf-node
 *                  2) argument not to be freed
 *  description   : frees whole primitive, without argument arg_no
 *  global vars   : FreeTree
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       :
 *
 */
void
FreePrf2 (node *arg_node, int arg_no)
{
    node *tmp1, *tmp2;
    int i;

    DBUG_ENTER ("FreePrf2");
    tmp1 = arg_node->node[0];
    i = 0;
    while (NULL != tmp1) {
        if (i != arg_no)
            FreeTree (tmp1->node[0]);
        tmp2 = tmp1;
        tmp1 = tmp1->node[1];
        FREE (tmp2);
        i++;
    }
    FREE (arg_node);
    DBUG_VOID_RETURN;
}
