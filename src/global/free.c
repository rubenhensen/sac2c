/*
 *
 * $Log$
 * Revision 1.6  1995/01/26 15:16:55  asi
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
    free (ids->id);
    free (ids);
    DBUG_VOID_RETURN;
}

void
FreeIdsOnly (ids *ids)

{
    DBUG_ENTER ("FreeIdsOnly");
    if (ids->next != NULL)
        FreeIdsOnly (ids->next);
    free (ids);
    DBUG_VOID_RETURN;
}

void
FreeImplist (node *implist)

{
    int i;

    DBUG_ENTER ("FreeImplist");
    if (implist->node[0] != NULL)
        FreeImplist (implist->node[0]);
    for (i = 1; i < 4; i++)
        if (implist->node[i] != NULL)
            FreeIds ((ids *)implist->node[i]);
    free (implist);
    DBUG_VOID_RETURN;
}

void
FreeMask (node *arg_node)
{
    int i;
    DBUG_ENTER ("FreeMask");
    for (i = 0; i < MAX_MASK; i++)
        if (arg_node->mask[i] != NULL)
            free (arg_node->mask[i]);
    DBUG_VOID_RETURN;
}

void
FreeShpseg (shpseg *shpseg)
{
    DBUG_ENTER ("FreeTypes");
    if (shpseg->next != NULL)
        FreeShpseg (shpseg->next);
    free (shpseg);
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
        free (type->name);
    if (type->id != NULL)
        free (type->id);
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
    free (var);
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
    free (arg_node);
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
    free (arg_node);
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
    free (arg_node);
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
    free (arg_node->info.id);
    free (arg_node);
    DBUG_RETURN ((node *)NULL);
}

node *
FreeInfoType (node *arg_node, node *arg_info)
{
    int i;

    DBUG_ENTER ("FreeInfoType");
    for (i = 0; i < arg_node->nnode; i++)
        arg_node->node[i] = Trav (arg_node->node[i], arg_info);
    FreeMask (arg_node);
    FreeTypes (arg_node->info.types);
    free (arg_node);
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
