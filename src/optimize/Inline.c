/*
 *
 * $Log$
 * Revision 1.7  1995/07/24 09:09:34  asi
 * macro TYPES renamed to INL_TYPES
 *
 * Revision 1.6  1995/06/26  09:26:22  asi
 * inlineing in functions with inline tag disabled
 *
 * Revision 1.5  1995/06/23  13:48:10  asi
 * parameter added to functioncall DuplicateTypes in INLvar
 *
 * Revision 1.4  1995/06/08  10:03:44  asi
 * added multi inlineing and some bugs fixed
 *
 * Revision 1.3  1995/06/02  15:55:56  asi
 * Bug fixed in INLfundef
 *
 * Revision 1.2  1995/06/02  11:28:18  asi
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
#include "internal_lib.h"

#include "optimize.h"
#include "LoopInvariantRemoval.h"
#include "DupTree.h"
#include "Inline.h"
#include "optimize.h"

#define FIRST_FUNC arg_info->node[0]

#define INLINE_PREFIX "__inl"
#define INLINE_PREFIX_LENGTH 5

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
 *  functionname  : INLmodul
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
INLmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLmodul");
    if (NULL != arg_node->node[2]) {
        FIRST_FUNC = arg_node->node[2];
        arg_node->node[2] = Trav (arg_node->node[2], arg_info);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : InlineNo
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
InlineNo (node *first)
{
    node *fun_node;

    DBUG_ENTER ("InlineNo");

    fun_node = first;
    while (NULL != fun_node) {
        if (fun_node->flag)
            fun_node->refcnt = inlnum;
        fun_node = fun_node->node[1];
    }
    DBUG_VOID_RETURN;
}

node *
FindReturn (node *arg_node)
{
    DBUG_ENTER ("FindReturn");
    while (NULL != arg_node->node[1])
        arg_node = arg_node->node[1];
    DBUG_RETURN (arg_node->node[0]);
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

#if 0 /* inline in functions with inline tag */

  if (NULL!=arg_node->node[0])
    {
    DBUG_PRINT("INL",("*** Trav function %s", arg_node->info.types->id));
    InlineNo(FIRST_FUNC);
    
    if (0!=arg_node->flag)
      {
      DUPTYPE = NORMAL;
      arg_node->node[0]->node[2] = DupTree(arg_node->node[0]->node[0], arg_info);
      DUPTYPE = INLINE;
      arg_node->node[0]->node[2] = Trav(arg_node->node[0]->node[2], arg_info);
      }
    else
      {
      arg_node->node[0]->node[0] = Trav(arg_node->node[0]->node[0], arg_info);
      }
    
    arg_node->node[0]->node[1] = AppendNodeChain(0, INL_TYPES, arg_node->node[0]->node[1]);
    INL_TYPES = NULL;
    
    if (NULL!=arg_node->node[1])
      arg_node->node[1] = Trav(arg_node->node[1], arg_info);
    
    arg_node->refcnt=0;
    
    if (0!=arg_node->flag)
      {
      FreeTree(arg_node->node[0]->node[0]);
      arg_node->node[0]->node[0] = arg_node->node[0]->node[2];
      arg_node->node[0]->node[2] = NULL;
      arg_node->node[3] = FindReturn(arg_node->node[0]->node[0]);
      }

#else /* do not inline in functions with inline tag */

    if ((NULL != arg_node->node[0]) && (0 == arg_node->flag)) {
        DBUG_PRINT ("INL", ("*** Trav function %s", arg_node->info.types->id));
        InlineNo (FIRST_FUNC);

        arg_node->node[0]->node[0] = Trav (arg_node->node[0]->node[0], arg_info);

        arg_node->node[0]->node[1]
          = AppendNodeChain (0, INL_TYPES, arg_node->node[0]->node[1]);
        INL_TYPES = NULL;

        if (NULL != arg_node->node[1])
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);

        arg_node->refcnt = 0;

#endif
}
else
{
    if (NULL != arg_node->node[1])
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
}
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
    if (NULL != arg_node->node[0]) {
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
    node *inl_nodes, *header_nodes = NULL, *bottom_nodes = NULL, *var_node, *expr_node,
                     *new_expr;
    ids *ids_node;
    char *new_name;

    DBUG_ENTER ("DoInline");
    DBUG_PRINT ("INL", ("Inlineing function %s", ap_node->node[1]->info.types->id));

    inl_fun++;
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
        new_name = RenameInlinedVar (var_node->info.types->id);
        new_expr = DupTree (expr_node->node[0], arg_info);
        inl_nodes = INLMakeLet (new_name, new_expr);
        inl_nodes->node[0]->info.ids->node
          = SearchDecl (inl_nodes->node[0]->info.ids->id, INL_TYPES);
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
        new_name = StringCopy (ids_node->id);
        new_expr = DupTree (expr_node->node[0], arg_info);
        inl_nodes = INLMakeLet (new_name, new_expr);
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
    node *inlined_nodes = NULL;

    DBUG_ENTER ("INLassign");
    if (N_let == arg_node->node[0]->nodetype) {
        node_behind = NodeBehindCast (arg_node->node[0]->node[0]);
        if (N_ap == node_behind->nodetype) {
            DBUG_PRINT ("INL",
                        ("Function call %s found in line %d with inline %d and to do %d",
                         node_behind->info.fun_name.id, arg_node->lineno,
                         node_behind->node[1]->flag, node_behind->node[1]->refcnt));
            if (0 < node_behind->node[1]->refcnt)
                inlined_nodes = DoInline (arg_node->node[0], node_behind, arg_info);
        }
    }

    if (NULL == inlined_nodes) {
        /* Trav if-then-else and loops */
        if (1 <= arg_node->nnode)
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        /* Trav next assign */
        if (2 <= arg_node->nnode)
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    } else {
        node_behind->node[1]->refcnt--;
        inlined_nodes = Trav (inlined_nodes, arg_info);
        node_behind->node[1]->refcnt++;

        if (2 <= arg_node->nnode)
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        inlined_nodes = AppendNodeChain (1, inlined_nodes, arg_node->node[1]);
        arg_node->nnode = 1;
        FreeTree (arg_node);
        arg_node = inlined_nodes;
    }

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
    new_name
      = (char *)MAlloc ((sizeof (char)) * (strlen (old_name) + INLINE_PREFIX_LENGTH + 5));
    sprintf (new_name, INLINE_PREFIX "%d_%s", inline_nr, old_name);
    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : SearchDecl
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
SearchDecl (char *name, node *decl_node)
{
    node *found = NULL;

    DBUG_ENTER ("SearchDecl");
    while (NULL != decl_node) {
        if (!strcmp (name, decl_node->info.types->id)) {
            found = decl_node;
            decl_node = NULL;
        } else {
            decl_node = decl_node->node[0];
        }
    }
    DBUG_RETURN (found);
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
    char *new_name;

    DBUG_ENTER ("INLvar");

    new_name = RenameInlinedVar (arg_node->info.types->id);
    if (NULL == SearchDecl (new_name, INL_TYPES)) {
        new_vardec = MakeNode (N_vardec);
        new_vardec->info.types = DuplicateTypes (arg_node->info.types, 1);
        FREE (new_vardec->info.types->id);
        new_vardec->info.types->id = new_name;
        INL_TYPES = AppendNodeChain (0, new_vardec, INL_TYPES);
    } else {
        FREE (new_name);
    }
    if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (arg_node);
}
