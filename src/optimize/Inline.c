/*
 *
 * $Log$
 * Revision 1.21  1998/07/16 17:45:12  dkr
 * *** empty log message ***
 *
 * Revision 1.20  1998/07/16 17:20:58  sbs
 * InlineSingleApplication generated
 *
 * Revision 1.19  1998/05/08 15:46:03  srs
 * no semantic changes
 *
 * Revision 1.18  1998/04/16 16:07:34  srs
 * renamed macros which access arg_info
 *
 * Revision 1.17  1998/04/16 14:26:50  srs
 * removed NEWTREE
 *
 * Revision 1.16  1998/04/09 21:24:31  dkr
 * renamed macros:
 *   INLINE -> DUP_INLINE
 *   NORMAL -> DUP_NORMAL
 *   INVARIANT -> DUP_INVARIANT
 *
 * Revision 1.15  1998/02/06 11:21:11  srs
 * optimize.h was included twice
 *
 * Revision 1.14  1997/04/30 11:50:55  cg
 * links to global object definitions now stored in artificial variable
 * declarations when functions using global objects are inlined.
 *
 * Revision 1.13  1997/04/25  12:26:28  sbs
 * changed MAlloc to Malloc
 *
 * Revision 1.12  1996/01/17  14:40:57  asi
 * added globals.h
 *
 * Revision 1.11  1995/12/21  15:26:46  asi
 * INLblock removed
 *
 * Revision 1.10  1995/12/13  17:33:40  asi
 * added #ifndef NEWTREE for new virtuell syntaxtree
 *
 * Revision 1.9  1995/12/07  16:17:40  asi
 * changed to new access macros and new Make-functions
 * bug fixed: functions without arguments or without local variables
 *            will be inlined correctly now
 *
 * Revision 1.8  1995/10/06  16:35:40  cg
 * calls to MakeIds adjusted to new signature (3 parameters)
 *
 * Revision 1.7  1995/07/24  09:09:34  asi
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

#include "globals.h"
#include "tree.h"
#include "tree_basic.h"
#include "tree_compound.h"
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

#define INLINE_PREFIX "__inl"
#define INLINE_PREFIX_LENGTH 5

static int inline_nr = 0;

/*
 *
 *  functionname  : Inline
 *  arguments     : 1) ptr to root of the syntaxtree
 *                  R) ptr to root of the optimized syntaxtree
 *  description   : Starts Function Inlining
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav     (traverse.h)
 *                  MakeNode (tree.h)
 *  macros        : FREE
 *
 *  remarks       : ---
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
 *  arguments     : 1) N_modul - node
 *                  2) N_info - node
 *                  R) N_modul - node
 *  description   : stores pointer to first function in info-node
 *                  and traverses function-chain
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav     (traverse.h)
 *  macros        : MODUL_FUNS, FIRST_FUNC
 *
 *  remarks       : ---
 *
 */
node *
INLmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLmodul");
    if (MODUL_FUNS (arg_node)) {
        INFO_INL_FIRST_FUNC (arg_info) = MODUL_FUNS (arg_node);
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : InlineNo
 *  arguments     : 1) fist function of modul
 *  description   : temporary attribute FUNDEF_INLREC set to maximun
 *                  of allowed recursive substitutions
 *  global vars   : inlnum
 *  internal funs : ---
 *  external funs : ---
 *  macros        : FUNDEF_INLINE, FUNDEF_INLREC, FUNDEF_NEXT
 *
 *  remarks       : ---
 *
 */
void
InlineNo (node *first)
{
    node *fun_node;

    DBUG_ENTER ("InlineNo");

    fun_node = first;
    while (fun_node) {
        if (FUNDEF_INLINE (fun_node))
            FUNDEF_INLREC (fun_node) = inlnum;
        fun_node = FUNDEF_NEXT (fun_node);
    }
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : INLfundef
 *  arguments     : 1) N_fundef - node
 *                  2) N_info - node
 *                  R) N_fundef - node
 *  description   : Traverses instructons if function not inlined marked
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : Trav            (traverse.h)
 *                  AppendNodeChain (tree.h)
 *  macros        : FUNDEF_BODY, FUNDEF_INLINE, FUNDEF_NAME, FIRST_FUNC, INL_TYPES,
 *                  FUNDEF_INSTR, FUNDEF_VARDEC, FUNDEF_NEXT
 *
 *  remarks       : ---
 *
 */
node *
INLfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("INLfundef");

    if (FUNDEF_BODY (arg_node) && (0 == FUNDEF_INLINE (arg_node))) {
        DBUG_PRINT ("INL", ("*** Trav function %s", FUNDEF_NAME (arg_node)));
        InlineNo (INFO_INL_FIRST_FUNC (arg_info));

        INFO_INL_TYPES (arg_info) = NULL;

        FUNDEF_INSTR (arg_node) = Trav (FUNDEF_INSTR (arg_node), arg_info);

        FUNDEF_VARDEC (arg_node)
          = AppendNodeChain (0, INFO_INL_TYPES (arg_info), FUNDEF_VARDEC (arg_node));
    }

    if (FUNDEF_NEXT (arg_node))
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DoInline
 *  arguments     : 1) N_let - node
 *                  2) N_ap - node
 *                  3) N_info - node
 *                  R) N_assign - chain
 *  description   : Do funcion inlining
 *  global vars   : inline_nr
 *  internal funs : RenameInlinedVar, SearchDecl
 *  external funs : Trav            (traverse.h)
 *                  DupTree         (DupTree.h)
 *                  MakeAssignLet   (tree_compound.h)
 *		    AppendNodeChain (tree.h)
 *  macros        : AP_NAME, BLOCK_VARDEC, FUNDEF_BODY, AP_FUNDEF, FUNDEF_ARGS,
 *                  DUPTYPE, DUP_NORMAL, ARG_NAME, INL_TYPES, ASSIGN_INSTR, VARDEC_NEXT,
 *                  EXPRS_NEXT, NEWTREE, IDS_NEXT, DUP_INLINE,
 *
 *  remarks       : ---
 *
 */
node *
DoInline (node *let_node, node *ap_node, node *arg_info)
{
    node *inl_nodes, *header_nodes = NULL, *bottom_nodes = NULL, *var_node, *expr_node,
                     *new_expr, *vardec_node;
    ids *ids_node;
    char *new_name;

    DBUG_ENTER ("DoInline");
    DBUG_PRINT ("INL", ("Inlineing function %s", AP_NAME (ap_node)));

    DBUG_ASSERT ((arg_info != NULL), ("DoInline called with NULL arg_info!"));

    inl_fun++;
    /*
     * Generate new variables
     */
    if (BLOCK_VARDEC (FUNDEF_BODY (AP_FUNDEF (ap_node))))
        BLOCK_VARDEC (FUNDEF_BODY (AP_FUNDEF (ap_node)))
          = Trav (BLOCK_VARDEC (FUNDEF_BODY (AP_FUNDEF (ap_node))), arg_info);
    if (FUNDEF_ARGS (AP_FUNDEF (ap_node)))
        FUNDEF_ARGS (AP_FUNDEF (ap_node))
          = Trav (FUNDEF_ARGS (AP_FUNDEF (ap_node)), arg_info);

    /*
     * Make header for inlined function
     */
    var_node = FUNDEF_ARGS (AP_FUNDEF (ap_node));
    expr_node = AP_ARGS (ap_node);

    DUPTYPE = DUP_NORMAL;

    while (var_node && expr_node) {
        new_name = RenameInlinedVar (ARG_NAME (var_node));
        vardec_node = SearchDecl (new_name, INFO_INL_TYPES (arg_info));
        new_expr = DupTree (EXPRS_EXPR (expr_node), arg_info);
        inl_nodes = MakeAssignLet (new_name, vardec_node, new_expr);
        header_nodes = AppendNodeChain (1, inl_nodes, header_nodes);
        var_node = VARDEC_NEXT (var_node);
        expr_node = EXPRS_NEXT (expr_node);
    }

    /*
     * Make bottom for inlined function
     */
    ids_node = LET_IDS (let_node);
    expr_node = RETURN_EXPRS (FUNDEF_RETURN (AP_FUNDEF (ap_node)));

    DUPTYPE = DUP_INLINE;

    while (ids_node && expr_node) {
        new_name = StringCopy (IDS_NAME (ids_node));
        new_expr = DupTree (EXPRS_EXPR (expr_node), arg_info);
        inl_nodes = MakeAssignLet (new_name, IDS_VARDEC (ids_node), new_expr);
        bottom_nodes = AppendNodeChain (1, inl_nodes, bottom_nodes);
        ids_node = IDS_NEXT (ids_node);
        expr_node = EXPRS_NEXT (expr_node);
    }

    /*
     * Duplicate function (with variable renameing)
     */
    DUPTYPE = DUP_INLINE;
    inl_nodes = DupTree (BLOCK_INSTR (FUNDEF_BODY (AP_FUNDEF (ap_node))), arg_info);

    /*
     * Link it together
     */
    inl_nodes = AppendNodeChain (1, inl_nodes, bottom_nodes);
    inl_nodes = AppendNodeChain (1, header_nodes, inl_nodes);

    inline_nr++;
    DBUG_RETURN (inl_nodes);
}

/******************************************************************************
 *
 * function:
 *  node *InlineSingleApplication( node *let_node, node *fundef_node)
 *
 * description:
 *   this function allows a single function application to be inlined.
 *   It is ment for external calls only and preserves the actual fun_tab!
 *   <let_node> should point to the N_let node which RHS is to be unrolled,
 *   <fundef_node> should point to the N_fundef node of the function in which
 *      body the <let_node> is situated!! (needed for new vardecs only!)
 *   It returns an assignment-chain to be inserted for the N_assign node
 *   which has <let_node> as its body!
 *
 ******************************************************************************/

node *
InlineSingleApplication (node *let_node, node *fundef_node)
{
    node *arg_info, *assigns;
    funptr *mem_tab;

    DBUG_ENTER ("InlineSingleApplication");

    mem_tab = act_tab;
    act_tab = inline_tab;

    arg_info = MakeNode (N_info);
    INFO_INL_TYPES (arg_info) = FUNDEF_VARDEC (fundef_node);

    assigns = DoInline (let_node, LET_EXPR (let_node), arg_info);

    FUNDEF_VARDEC (fundef_node) = INFO_INL_TYPES (arg_info);
    FREE (arg_info);

    act_tab = mem_tab;

    DBUG_RETURN (assigns);
}

/*
 *
 *  functionname  : INLassign
 *  arguments     : 1) N_assign - chain
 *                  2) N_info - node
 *                  R) N_assign - chain
 *  description   : Initiate function inlining if substitution-counter not 0
 *  global vars   : ---
 *  internal funs : DoInline
 *  external funs : Trav            (traverse.h)
 *		    AppendNodeChain (tree.h)
 *  macros        : NODE_TYPE, AP_NAME, NODE_LINE, FUNDEF_INLINE, FUNDEF_INLREC,
 *                  AP_FUNDEF, ASSIGN_INSTR, ASSIGN_NEXT,
 *
 *  remarks       : ---
 *
 */
node *
INLassign (node *arg_node, node *arg_info)
{
    node *node_behind;
    node *inlined_nodes = NULL;

    DBUG_ENTER ("INLassign");
    if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        node_behind = NodeBehindCast (LET_EXPR ((ASSIGN_INSTR (arg_node))));
        if (N_ap == NODE_TYPE (node_behind)) {
            DBUG_PRINT ("INL",
                        ("Function call %s found in line %d with inline %d and to do %d",
                         AP_NAME (node_behind), NODE_LINE (arg_node),
                         FUNDEF_INLINE (AP_FUNDEF (node_behind)),
                         FUNDEF_INLREC (AP_FUNDEF (node_behind))));
            if (0 < FUNDEF_INLREC (AP_FUNDEF (node_behind)))
                inlined_nodes = DoInline (ASSIGN_INSTR (arg_node), node_behind, arg_info);
        }
    }

    if (!inlined_nodes) {
        /* Trav if-then-else and loops */
        if (ASSIGN_INSTR (arg_node))
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        /* Trav next assign */
        if (ASSIGN_NEXT (arg_node))
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        FUNDEF_INLREC (AP_FUNDEF (node_behind))--;
        inlined_nodes = Trav (inlined_nodes, arg_info);
        FUNDEF_INLREC (AP_FUNDEF (node_behind))++;

        if (ASSIGN_NEXT (arg_node))
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        arg_node = AppendNodeChain (1, inlined_nodes, FreeNode (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : RenameInlinedVar
 *  arguments     : 1) character string
 *                  R) character string
 *  description   : rename variable, for example a -> _inl100_a, if inline_nr == 100
 *                  and INLINE_PREFIX == _inl
 *  global vars   : inline_nr
 *  internal funs : ---
 *  external funs : sizeof, strlen, sprintf
 *                  Malloc                  (optimize.h)
 *  macros        : INLINE_PREFIX, INLINE_PREFIX_LENGTH
 *
 *  remarks       : ---
 *
 */
char *
RenameInlinedVar (char *old_name)
{
    char *new_name;

    DBUG_ENTER ("RenameInlinedVar");
    new_name = (char *)Malloc (sizeof (char)
                               * (strlen (old_name) + INLINE_PREFIX_LENGTH + 1 + /* _ */
                                  5 +  /* max length of inline_nr */
                                  1)); /* #0 at eol */
    sprintf (new_name, INLINE_PREFIX "%d_%s", inline_nr, old_name);
    DBUG_RETURN (new_name);
}

/*
 *
 *  functionname  : SearchDecl
 *  arguments     : 1) character string
 *                  2) N_vardec - or N_arg - chain
 *                  R) true or false
 *  description   : returns ptr to N_vardec - or N_arg - node  if variable or argument
 *                  with same name as in 1) has been found, else NULL
 *  global vars   : ---
 *  internal funs : ---
 *  external funs : strcmp
 *  macros        : NODE_TYPE, VARDEC_NAME, VARDEC_NEXT, ARG_NAME, ARG_NEXT
 *
 *  remarks       : ---
 *
 */
node *
SearchDecl (char *name, node *decl_node)
{
    node *found = NULL;

    DBUG_ENTER ("SearchDecl");
    while (NULL != decl_node) {
        if (N_vardec == NODE_TYPE (decl_node)) {
            if (!strcmp (name, VARDEC_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = VARDEC_NEXT (decl_node);
            }
        } else {
            if (!strcmp (name, ARG_NAME (decl_node))) {
                found = decl_node;
                decl_node = NULL;
            } else {
                decl_node = ARG_NEXT (decl_node);
            }
        }
    }
    DBUG_RETURN (found);
}

/*
 *
 *  functionname  : INLvar
 *  arguments     : 1) N_vardec - or N_arg - chain
 *                  2) N_info - node
 *                  R) N_vardec - or N_arg - chain
 *  description   : Duplicates and renames 1) for function inlining
 *  global vars   : ---
 *  internal funs : RenameInlinedVar, SearchDecl
 *  external funs : DuplicateTypes (typcheck.h)
 *                  MakeVardec     (tree_basic.h)
 *  macros        : VARDEC_NAME, INL_TYPES, VARDEC_TYPE, ARG_NAME, ARG_TYPE, ARG_NEXT
 *                  FREE, NEWTREE, VARDEC_NEXT, ARG_NEXT
 *
 *  remarks       : ---
 *
 */
node *
INLarg (node *arg_node, node *arg_info)
{
    char *new_name;
    types *new_type;

    DBUG_ENTER ("INLarg");
    DBUG_ASSERT (N_arg == NODE_TYPE (arg_node), ("wrong node type"));

    new_name = RenameInlinedVar (ARG_NAME (arg_node));
    if (!SearchDecl (new_name, INFO_INL_TYPES (arg_info))) {
        new_type = DuplicateTypes (ARG_TYPE (arg_node), 1);
        /* DuplicateTypes also copies attrib and status information. */

        INFO_INL_TYPES (arg_info)
          = MakeVardec (new_name, new_type, INFO_INL_TYPES (arg_info));

        VARDEC_OBJDEF (INFO_INL_TYPES (arg_info)) = ARG_OBJDEF (arg_node);
        /*
         * Here, a possible link to the original global object definition
         * stored within
         * an artificial function argument is copied to the respective
         * artificial variable declaration.
         */

    } else
        FREE (new_name);

    if (ARG_NEXT (arg_node))
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

node *
INLvardec (node *arg_node, node *arg_info)
{
    char *new_name;
    types *new_type;

    DBUG_ENTER ("INLvardec");
    DBUG_ASSERT (N_vardec == NODE_TYPE (arg_node), ("wrong node type"));

    new_name = RenameInlinedVar (VARDEC_NAME (arg_node));
    if (!SearchDecl (new_name, INFO_INL_TYPES (arg_info))) {
        new_type = DuplicateTypes (VARDEC_TYPE (arg_node), 1);
        /*
         * DuplicateTypes also copies attrib and status information.
         */

        INFO_INL_TYPES (arg_info)
          = MakeVardec (new_name, new_type, INFO_INL_TYPES (arg_info));

        VARDEC_OBJDEF (INFO_INL_TYPES (arg_info)) = VARDEC_OBJDEF (arg_node);
        /*
         * Here, a possible link to the original global object definition
         * stored within
         * an artificial function argument is copied to the respective
         * artificial variable declaration.
         */

    } else
        FREE (new_name);

    if (VARDEC_NEXT (arg_node))
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
