/*
 *
 * $Log$
 * Revision 1.2  1995/06/02 11:28:18  asi
 * Added Inline, INLfundef, INLblock, INLMakeLet, DoInline, INLassign,
 *       RenameInlinedVar, SetDeclPtr and INLvar.
 *
 * Revision 1.1  1995/05/26  14:22:18  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "free.h"
#include "string.h"
#include "typecheck.h"

#include "optimize.h"
#include "LoopInvariantRemoval.h"
#include "DupTree.h"
#include "Inline.h"
#include "optimize.h"

#define FUNBLOCK arg_info->node[0]
#define TYPES arg_info->node[1]
#define VARDEC FUNBLOCK->node[1]

#define INLINE_PREFIX "__in"
#define INLINE_PREFIX_LENGTH 4

static int inline_nr = 0;

/*
 *
 *  functionname  : Inline
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
node *
Inline (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("Inline");
    act_tab = inline_tab;
    arg_info = MakeNode (N_info);

    arg_node = Trav (arg_node, arg_info);

    FREE (arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : INLfundef
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
node *
INLfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLfundef");
    if (1 <= arg_node->nnode) {
        FUNBLOCK = arg_node->node[0];
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        arg_node->node[0]->node[1]
          = AppendNodeChain (0, TYPES, arg_node->node[0]->node[1]);
        TYPES = NULL;
    }
    if (2 <= arg_node->nnode)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : INLblock
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
node *
INLblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLblock");
    if (1 <= arg_node->nnode) {
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : INLMakeLet
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
node *
INLMakeLet (char *var_name, node *expr)
{
    node *new_node;

    DBUG_ENTER ("INLMakeLet");
    new_node = MakeNode (N_assign);
    new_node->nnode = 1;
    new_node->node[0] = MakeNode (N_let);
    new_node->node[0]->info.ids = MakeIds (var_name);
    new_node->node[0]->nnode = 1;
    new_node->node[0]->node[0] = expr;
    DBUG_RETURN (new_node);
}

/*
 *
 *  functionname  : DoInline
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
node *
DoInline (node *let_node, node *ap_node, node *arg_info)
{
    node *inl_nodes, *header_nodes = NULL, *bottom_nodes = NULL, *var_node, *expr_node;
    ids *ids_node;

    DBUG_ENTER ("DoInline");
    DBUG_PRINT ("INL", ("Inlineing function %s\n", ap_node->node[1]->info.types->id));

    inl_fun++;
    ap_node->node[1]->flag--;

    /*
     * Generate new variables
     */
    ap_node->node[1]->node[0]->node[1]
      = Trav (ap_node->node[1]->node[0]->node[1], arg_info);
    ap_node->node[1]->node[2] = Trav (ap_node->node[1]->node[2], arg_info);

    /*
     * Make header for inlined function
     */
    var_node = ap_node->node[1]->node[2];
    expr_node = ap_node->node[0];

    DUPTYPE = NORMAL;

    while ((NULL != var_node) && (NULL != expr_node)) {
        inl_nodes = INLMakeLet (RenameInlinedVar (var_node->info.types->id),
                                DupTree (expr_node->node[0], arg_info));
        inl_nodes->node[0]->info.ids
          = SetDeclPtr (inl_nodes->node[0]->info.ids, arg_info);
        header_nodes = AppendNodeChain (1, inl_nodes, header_nodes);
        var_node = var_node->node[0];
        expr_node = expr_node->node[1];
    }

    /*
     * Make buttom for inlined function
     */
    ids_node = let_node->info.ids;
    expr_node = ap_node->node[1]->node[3]->node[0];

    DUPTYPE = INLINE;

    while ((NULL != ids_node) && (NULL != expr_node)) {
        inl_nodes
          = INLMakeLet (strdup (ids_node->id), DupTree (expr_node->node[0], arg_info));
        inl_nodes->node[0]->info.ids->node = ids_node->node;
        bottom_nodes = AppendNodeChain (1, inl_nodes, bottom_nodes);
        ids_node = ids_node->next;
        expr_node = expr_node->node[1];
    }

    /*
     * Duplicate function (with variable renameing)
     */
    inl_nodes = DupTree (ap_node->node[1]->node[0]->node[0], arg_info);

    /*
     * Link it together
     */
    inl_nodes = AppendNodeChain (1, inl_nodes, bottom_nodes);
    inl_nodes = AppendNodeChain (1, header_nodes, inl_nodes);

    inline_nr++;
    DBUG_RETURN (inl_nodes);
}

/*
 *
 *  functionname  : INLassign
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
node *
INLassign (node *arg_node, node *arg_info)
{
    node *node_behind;
    node *inlined_nodes;

    DBUG_ENTER ("INLassign");
    inlined_nodes = arg_node;
    if (N_let == arg_node->node[0]->nodetype) {
        node_behind = NodeBehindCast (arg_node->node[0]->node[0]);
        if (N_ap == node_behind->nodetype) {
            if (0 < node_behind->node[1]->flag) {
                inlined_nodes = DoInline (arg_node->node[0], node_behind, arg_info);
                AppendNodeChain (1, inlined_nodes, arg_node->node[1]);
                arg_node->nnode = 1;
                FreeTree (arg_node);
                arg_node = inlined_nodes;
            }
        }
    }

    if (2 <= arg_node->nnode)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RenameInlinedVar
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
char *
RenameInlinedVar (char *old_name)
{
    char *new_name;

    DBUG_ENTER ("RenameInlinedVar");
    if (!strncmp (old_name, "__", 2)) {
        new_name = strdup (old_name);
    } else {
        new_name = (char *)MAlloc ((sizeof (char))
                                   * (strlen (old_name) + INLINE_PREFIX_LENGTH + 5));
        sprintf (new_name, INLINE_PREFIX "%d_%s", inline_nr, old_name);
    }
    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : SetDeclPtr
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
ids *
SetDeclPtr (ids *ids_node, node *arg_info)
{
    node *decl_node;

    DBUG_ENTER ("SetDeclPtr");
    DBUG_PRINT ("INL", ("Searching decleration for %s", ids_node->id));
    ids_node->node = NULL;
    decl_node = TYPES;
    while (NULL != decl_node) {
        if (!strcmp (ids_node->id, decl_node->info.types->id)) {
            ids_node->node = decl_node;
            decl_node = NULL;
        } else {
            decl_node = decl_node->node[0];
        }
    }
    DBUG_ASSERT ((NULL != ids_node->node), ("No decleration found"));
    DBUG_RETURN (ids_node);
}

/*
 *
 *  functionname  : INLvar
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
node *
INLvar (node *arg_node, node *arg_info)
{
    node *new_vardec;
    char *old_name;

    DBUG_ENTER ("INLvar");
    new_vardec = MakeNode (N_vardec);
    new_vardec->info.types = DuplicateTypes (arg_node->info.types);
    old_name = new_vardec->info.types->id;
    new_vardec->info.types->id = RenameInlinedVar (old_name);
    FREE (old_name);
    TYPES = AppendNodeChain (0, new_vardec, TYPES);
    DBUG_RETURN (arg_node);
}
