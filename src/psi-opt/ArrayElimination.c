/*
 *
 * $Log$
 * Revision 1.6  1997/11/04 13:25:20  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.5  1997/04/25 12:13:00  sbs
 * MAlloc replaced by Malloc from internal.lib
 *
 * Revision 1.4  1996/07/16  15:26:40  asi
 * macros from access_macros.h no longer used
 *
 * Revision 1.3  1996/01/17  14:17:52  asi
 * added globals.h
 *
 * Revision 1.2  1995/10/06  17:07:47  cg
 * adjusted calls to function MakeIds (now 3 parameters)
 *
 * Revision 1.1  1995/07/24  10:00:19  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h"
#include "free.h"
#include "DupTree.h"
#include "tree.h"
#include "internal_lib.h"

#include "optimize.h"
#include "Inline.h"
#include "LoopInvariantRemoval.h"
#include "ArrayElimination.h"

#define AE_TYPES arg_info->node[1]
#define TRUE 1
#define FALSE 0

#define AE_PREFIX "__ae_"
#define AE_PREFIX_LENGTH 5

/*
 *
 *  functionname  : ArrayElimination
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr optimized 1)
 *  description   : initiates array elimination for the intermediate sac-code:
 *  global vars   : syntax_tree, act_tab, ae_tab
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 *
 */
node *
ArrayElimination (node *arg_node, node *info_node)
{
    DBUG_ENTER ("ArrayElimination");
    act_tab = ae_tab;
    info_node = MakeNode (N_info);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : AEfundef
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
AEfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("AEfundef");
    if ((NULL != arg_node->node[0]) && (0 == arg_node->flag)) {
        DBUG_PRINT ("AE", ("*** Trav function %s", arg_node->info.types->id));

        AE_TYPES = NULL;
        arg_node->node[0]->node[0] = Trav (arg_node->node[0]->node[0], arg_info);

        arg_node->node[0]->node[1]
          = AppendNodeChain (0, AE_TYPES, arg_node->node[0]->node[1]);
        AE_TYPES = NULL;

        if (NULL != arg_node->node[1])
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);

        arg_node->refcnt = 0;
    } else {
        if (NULL != arg_node->node[1])
            arg_node->node[1] = Trav (arg_node->node[1], arg_info);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CorrectArraySize
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
int
CorrectArraySize (ids *ids_node)
{
    int answer = FALSE;
    int length, dim;
    types *type;

    DBUG_ENTER ("CorrectArraySize");

    type = ids_node->node->info.types;
    GET_LENGTH (length, type);
    GET_DIM (dim, type);

    if ((length <= minarray) && (0 != length) && (1 == dim)) {
        DBUG_PRINT ("AE", ("array %s with length %d to eliminated found",
                           ids_node->node->info.types->id, length));
        answer = TRUE;
    }
    DBUG_RETURN (answer);
}

/*
 *
 *  functionname  : GetNumber
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
char *
GetNumber (node *vector)
{
    char *number, *tmp;
    node *expr_node;

    DBUG_ENTER ("GetNumber");
    number = (char *)Malloc (sizeof (char) * ((minarray / 10) + 2));
    number[0] = atoi ("\0");
    expr_node = vector->node[0];
    do {
        tmp = itoa (expr_node->node[0]->info.cint);
        strcat (number, tmp);
        expr_node = expr_node->node[1];
    } while (NULL != expr_node);
    DBUG_RETURN (number);
}

/*
 *
 *  functionname  : GenIds
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
ids *
GenIds (node *arg[2])
{
    ids *ids_node;
    char *number, *new_name, *old_name;

    DBUG_ENTER ("GenIds");
    number = GetNumber (arg[0]);
    old_name = ID_NAME (arg[1]);
    new_name = (char *)Malloc (sizeof (char) * (strlen (old_name) + strlen (number))
                               + AE_PREFIX_LENGTH);
    sprintf (new_name, AE_PREFIX "%s%s", number, old_name);
    ids_node = MakeIds (new_name, NULL, ST_regular);
    DBUG_RETURN (ids_node);
}

/*
 *
 *  functionname  : GenPsi
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
GenPsi (ids *ids_node, node *arg_info)
{
    node *new_nodes = NULL, *expr, *new_let, *new_assign, *new_vardec;
    int length, i;
    types *type;
    node *arg[2];

    DBUG_ENTER ("GenPsi");
    type = ids_node->node->info.types;
    GET_LENGTH (length, type);
    for (i = 0; i < length; i++) {
        arg[0] = MakeNode (N_array);
        expr = MakeNode (N_exprs);
#ifndef NEWTREE
        arg[0]->nnode = 1;
#endif
        arg[0]->node[0] = expr;
#ifndef NEWTREE
        expr->nnode = 1;
#endif
        MAKENODE_NUM (expr->node[0], i);
        arg[1] = MakeNode (N_id);
        arg[1]->info.ids = DupIds (ids_node, arg_info);
        new_let = MakeNode (N_let);
        new_let->info.ids = GenIds (arg);
        DBUG_PRINT ("AE", ("Generating new value for %s", new_let->info.ids->id));
        new_let->info.ids->node = SearchDecl (new_let->info.ids->id, AE_TYPES);
        if (NULL == new_let->info.ids->node) {
            DBUG_PRINT ("AE", ("Generating new vardec for %s", new_let->info.ids->id));
            new_vardec = MakeNode (N_vardec);
            GET_BASIC_TYPE (new_vardec->info.types, type, 0);
            new_vardec->info.types = DuplicateTypes (new_vardec->info.types, 1);
            new_vardec->info.types->dim = 0;
            FREE (new_vardec->info.types->id);
            new_vardec->info.types->id = StringCopy (new_let->info.ids->id);
            AE_TYPES = AppendNodeChain (0, new_vardec, AE_TYPES);
            new_let->info.ids->node = new_vardec;
        }
#ifndef NEWTREE
        new_let->nnode = 1;
#endif
        new_let->node[0] = MakeNode (N_prf);
        new_let->node[0]->info.prf = F_psi;
#ifndef NEWTREE
        new_let->node[0]->nnode = 1;
#endif
        new_let->node[0]->node[0] = MakeNode (N_exprs);
        new_let->node[0]->node[0]->node[0] = arg[0];
#ifndef NEWTREE
        new_let->node[0]->node[0]->nnode = 2;
#endif
        new_let->node[0]->node[0]->node[1] = MakeNode (N_exprs);
        new_let->node[0]->node[0]->node[1]->node[0] = arg[1];
#ifndef NEWTREE
        new_let->node[0]->node[0]->node[1]->nnode = 1;
#endif
        new_assign = MakeNode (N_assign);
#ifndef NEWTREE
        new_assign->nnode = 1;
#endif
        new_assign->node[0] = new_let;
        new_nodes = AppendNodeChain (1, new_nodes, new_assign);
    }
    DBUG_RETURN (new_nodes);
}

/*
 *
 *  functionname  : AEassign
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
AEassign (node *arg_node, node *arg_info)
{
    node *new_nodes = NULL;
    ids *ids_node;

    DBUG_ENTER ("AEassign");
    if (N_let == NODE_TYPE (ASSIGN_INSTR (arg_node))) {
        ids_node = LET_IDS (ASSIGN_INSTR (arg_node));
        if (NULL == IDS_NEXT (ids_node)) {
            if (TRUE == CorrectArraySize (ids_node)) {
                VARDEC_FLAG (IDS_VARDEC (ids_node)) = TRUE;
                new_nodes = AppendNodeChain (1, new_nodes, GenPsi (ids_node, arg_info));
            } else {
                VARDEC_FLAG (IDS_VARDEC (ids_node)) = FALSE;
            }
        }
    }

    if (NULL == new_nodes) {
        /* Trav if-then-else and loops */
        if (NULL != ASSIGN_INSTR (arg_node))
            ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
        /* Trav next assign */
        if (NULL != ASSIGN_NEXT (arg_node))
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        if (NULL != ASSIGN_NEXT (arg_node))
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        new_nodes = AppendNodeChain (1, new_nodes, ASSIGN_NEXT (arg_node));
#ifndef NEWTREE
        arg_node->nnode = 2;
#endif
        ASSIGN_NEXT (arg_node) = new_nodes;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : AEprf
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
AEprf (node *arg_node, node *arg_info)
{
    node *arg[2], *new_node;

    DBUG_ENTER ("AEprf");
    if (F_psi == arg_node->info.prf) {
        arg[0] = NodeBehindCast (arg_node->node[0]->node[0]);
        arg[1] = NodeBehindCast (arg_node->node[0]->node[1]->node[0]);
        if ((N_id == arg[1]->nodetype) && (N_array == arg[0]->nodetype)) {
            if (TRUE == CorrectArraySize (arg[1]->info.ids)) {
                DBUG_PRINT ("AE", ("psi function with array %s to eliminated found",
                                   arg[1]->info.ids->id));
                new_node = MakeNode (N_id);
                new_node->info.ids = GenIds (arg);
                new_node->info.ids->node = SearchDecl (new_node->info.ids->id, AE_TYPES);
                if (NULL != new_node->info.ids->node) {
                    FreeTree (arg_node);
                    arg_node = new_node;
                } else {
                    FreeTree (new_node);
                }
            }
        }
    }
    DBUG_RETURN (arg_node);
}
