/*
 *
 * $Log$
 * Revision 1.26  1995/06/26 11:49:48  asi
 * logical prim-functions will generate boolean results now
 *
 * Revision 1.25  1995/06/21  13:23:58  asi
 * bug fix - same bug fixed for psi
 *
 * Revision 1.24  1995/06/21  13:10:02  asi
 * bug fix - ArrayPrf will check if it is a constant correctly
 *
 * Revision 1.23  1995/06/20  15:50:30  asi
 * Array fission modified
 *
 * Revision 1.22  1995/06/09  13:29:15  asi
 * now reallocating stack, if it sac-program has more than 50 levels
 *
 * Revision 1.21  1995/06/09  09:19:01  asi
 * *** empty log message ***
 *
 * Revision 1.20  1995/05/16  09:14:09  asi
 * usages of WARN1 adjust to changes in Error.h
 *
 * Revision 1.19  1995/05/05  09:06:17  asi
 * Constant arrays will no longer inserted in user defined or primitive functions,
 * if these functions could not be folded.
 *
 * Revision 1.18  1995/05/03  16:25:48  asi
 * CFap added
 *
 * Revision 1.17  1995/05/02  08:48:34  asi
 * DupConst and DupArray moved to DupTree.c
 *
 * Revision 1.16  1995/04/20  15:40:48  asi
 * mask handling in psi, dim and shape function added
 * bug fixed in psi: result isn't array, if vector and array dimension are equal
 *
 * Revision 1.15  1995/04/06  14:51:51  asi
 * constant folding in return-expessions disabled
 *
 * Revision 1.14  1995/04/06  11:35:20  asi
 * GetShapeVector now depends on SHP_SEG_SIZE
 *
 * Revision 1.13  1995/04/03  16:17:42  asi
 * Bug fixed in psi-calculation
 *
 * Revision 1.12  1995/03/24  15:54:25  asi
 * changed free() -> FREE()
 * changed Trav() -> OptTrav()
 * changed CFdo - replaced PopVL() with PopVL2()
 * changed CFwith - with-loops handeled like lokal funktions now
 *
 * Revision 1.11  1995/03/15  15:16:38  asi
 * modified mask handling in conditionals
 *
 * Revision 1.10  1995/03/13  17:58:12  asi
 * changover from 'info.id' to 'info.ids' of node N_id
 *
 * Revision 1.9  1995/03/13  16:59:05  asi
 * changed CFid
 *
 * Revision 1.8  1995/03/13  15:52:58  asi
 * changed DupConst
 *
 * Revision 1.7  1995/03/13  13:23:53  asi
 * bug fixed in CFwith : sets c.f. flag to false
 *
 * Revision 1.6  1995/03/08  17:28:09  asi
 * folding of primitive function psi() added
 * some changes for new prin. func. ..AxA, ..AxS and ..SxA
 *
 * Revision 1.5  1995/03/07  10:17:47  asi
 * added CFcast, constant folding within with loops
 * modified CFlet : determining basetype for user defined types
 *
 * Revision 1.4  1995/02/28  18:35:00  asi
 * added Cfwith
 * added counter for primfun application elimination
 * bug fixed in routines for shape, reshape and dim
 *
 * Revision 1.3  1995/02/22  18:16:34  asi
 * Fuctions CFfundef, CFassign, CFdo, CFwhile, CFcond, CFprf, CFid, CFlet
 * and PushVL, PushDupVL, PopVL for stack-management added
 *
 * Revision 1.2  1995/02/14  09:59:56  asi
 * added CFid
 *
 * Revision 1.1  1995/02/13  16:55:00  asi
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "free.h"
#include "print.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "typecheck.h"

#include "optimize.h"
#include "DupTree.h"
#include "Unroll.h"
#include "LoopInvariantRemoval.h"
#include "ConstantFolding.h"

extern char filename[]; /* is set temporary; will be set later on in main.c */

#define FALSE 0
#define TRUE 1
#define MAXARG 3

#define MIN_STACK_SIZE 50

stack *cf_stack;

#define TOS cf_stack->stack[cf_stack->tos]
#define VAR(i) TOS.varlist[i]

/*
 *  This macros secects the right element out of the union info
 */
#define SELARG(n) ((n->nodetype == N_float) ? n->info.cfloat : n->info.cint)

/*
 *
 *  functionname  : CheckStack
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
CheckStack (stack *my_stack)
{
    DBUG_ENTER ("CheckStack");
    if (cf_stack->tos == ((cf_stack->st_len) - 1)) {
        cf_stack->stack
          = realloc (cf_stack->stack, (sizeof (stelm) * (2 * cf_stack->st_len)));
        if (NULL == cf_stack->stack)
            Error ("out of memory", 1);
        cf_stack->st_len *= 2;
    }
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PushVL
 *  arguments     : 1) number of variables in new list
 *  description   : a new entry will be pushed on the cf_stack. The entry is a pointer
 *		    to an array with size NumVar. The elements of the array are pointers
 *		    to the last expression the variable is set to in control-flow
 *		    direction.
 *  global vars   : cf_stack
 *  internal funs : --
 *  external funs : Error, MAlloc
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
void
PushVL (long NumVar)
{
    int i;

    DBUG_ENTER ("PushVL");
    DBUG_PRINT ("STACK",
                ("Push Stack TOS = %d -> %d", cf_stack->tos, (cf_stack->tos) + 1));
    cf_stack->stack[++cf_stack->tos].varlist
      = (node **)MAlloc (sizeof (node *) * (NumVar + 1));
    cf_stack->stack[cf_stack->tos].vl_len = NumVar;
    for (i = 0; i < NumVar; i++)
        cf_stack->stack[cf_stack->tos].varlist[i] = NULL;
    CheckStack (cf_stack);
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PushDupVL
 *  arguments     : --
 *  description   : Duplicates the top entry of the cf_stack
 *  global vars   : cf_stack
 *  internal funs : --
 *  external funs : MAlloc
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
void
PushDupVL ()
{
    int NumVar, i;

    DBUG_ENTER ("PushDupVL");
    DBUG_PRINT ("STACK",
                ("Dup Stack TOS = %d -> %d", cf_stack->tos, (cf_stack->tos) + 1));
    NumVar = cf_stack->stack[cf_stack->tos].vl_len;
    cf_stack->stack[++cf_stack->tos].varlist
      = (node **)MAlloc (sizeof (node *) * (NumVar + 1));
    for (i = 0; i < NumVar; i++)
        cf_stack->stack[cf_stack->tos].varlist[i]
          = cf_stack->stack[cf_stack->tos - 1].varlist[i];
    cf_stack->stack[cf_stack->tos].vl_len = cf_stack->stack[cf_stack->tos - 1].vl_len;
    CheckStack (cf_stack);
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PopVL
 *  arguments     : --
 *  description   : The top entry of the cf_stack will be removed
 *  global vars   : cf_stack
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
void
PopVL ()
{
    DBUG_ENTER ("PopVL");
    DBUG_PRINT ("STACK", ("Pop TOS = %d -> %d", cf_stack->tos, (cf_stack->tos) - 1));
    FREE (cf_stack->stack[cf_stack->tos].varlist);
    cf_stack->tos--;
    DBUG_VOID_RETURN;
}

void
PopVL2 ()
{
    DBUG_ENTER ("PopVL2");
    DBUG_PRINT ("STACK",
                ("Pop second TOS = %d -> %d", cf_stack->tos, (cf_stack->tos) - 1));
    FREE (cf_stack->stack[(cf_stack->tos) - 1].varlist);
    cf_stack->stack[(cf_stack->tos) - 1].varlist = cf_stack->stack[cf_stack->tos].varlist;
    cf_stack->tos--;
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ConstantFolding
 *  arguments     : 1) ptr to root of the syntaxtree or a N_fundef - node.
 *		    2) NULL
 *                  R) ptr to root of the optimized syntax-tree
 *  description   : initiates constant folding for the intermediate sac-code:
 *		    - the constant-folding stack (cf_stack) will be initialize
 *		    - call Trav to start constant-folding
 *  global vars   : syntax_tree, cf_tab, act_tab, cf_stack
 *  internal funs : ---
 *  external funs : Trav, MakeNode, MAlloc
 *  macros        : DBUG..., MIN_STACK_SIZE
 *
 *  remarks       : --
 *
 *
 */
node *
ConstantFolding (node *arg_node, node *info_node)
{
    DBUG_ENTER ("ConstantFolding");
    act_tab = cf_tab;
    info_node = MakeNode (N_info);
    cf_stack = MAlloc (sizeof (stack));
    cf_stack->tos = -1;
    cf_stack->st_len = MIN_STACK_SIZE;
    cf_stack->stack = (stelm *)MAlloc (sizeof (stelm) * MIN_STACK_SIZE);

    arg_node = Trav (arg_node, info_node);

    FREE (info_node);
    FREE (cf_stack->stack);
    FREE (cf_stack);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFfundef
 *  arguments     : 1) fundef-node
 *		    2) NULL
 *		    R) fundef-node with constant folded body of function
 *  description   : - generates info_node
 *		    - varno of the info_node will be set to the number of local variables
 *			and arguments in this functions
 *		    - new entry will be pushed on the cf_stack
 *		    - generates two masks and links them to the info_node
 * 			[0] - variables not further defined in function and
 *			[1] - variables not further used in function after c.- f.
 *		    - calls Trav to fold constants in current function
 *		    - last entry will be poped from cf_stack
 *		    - updates masks in fundef node.
 *  global vars   : syntax_tree, cf_stack
 *  internal funs : PushVL, PopVL
 *  external funs : GenMask, MinusMask, OptTrav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
CFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFfundef");

    DBUG_PRINT ("OPT", ("Constant folding function: %s", arg_node->info.types->id));
    VARNO = arg_node->varno;
    PushVL (arg_info->varno);

    arg_node = OptTrav (arg_node, arg_info, 0); /* functionbody */

    PopVL (cf_stack);

    arg_node = OptTrav (arg_node, arg_info, 1); /* next function */
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : IsConst
 *  arguments     : 1) node to be examine
 *		    R) 0 if node isn't a constant
 *		       1 if node is a constant
 *		       2 if node is a array, but containing non constant elements
 *  description   : determines if expression is a constant
 *  global vars   : syntax_tree, N_num, N_float, N_array, N_str, N_bool, N_id
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
int
IsConst (node *arg_node)
{
    int isit;
    node *expr;

    DBUG_ENTER ("IsConst");

    if (NULL != arg_node) {
        switch (arg_node->nodetype) {
        case N_num:
        case N_float:
        case N_str:
        case N_bool:
            isit = 1;
            break;
        case N_array:
            isit = 1;
            expr = arg_node->node[0];
            while (NULL != expr) {
                if (N_id == expr->node[0]->nodetype)
                    isit = 2;
                expr = expr->node[1];
            }
            break;
        default:
            isit = 0;
            break;
        }
    } else {
        isit = 0;
    }
    DBUG_RETURN (isit);
}

/*
 *
 *  functionname  : CFid
 *  arguments     : 1) id-node
 *		    2) info-node
 *		    R) id-node or modified node representing constant value:
 *		       num-, float-, bool- or array-node.
 *  description   : the id-node will be replaced with a new node reppresenting the
 *                  constant value of the identifikator in this context
 *  global vars   : syntax_tree, info_node, cf_stack
 *  internal funs :
 *  external funs : DupTree
 *  macros        : DBUG..., TOS
 *
 *  remarks       : --
 *
 */
node *
CFid (node *arg_node, node *arg_info)
{
    node *value;

    DBUG_ENTER ("CFid");
    value = VAR (arg_node->info.ids->node->varno);
    if (NULL != value) {
        switch (value->nodetype) {
        case N_num:
        case N_float:
        case N_bool:
            if (NULL != arg_info)
                DEC_VAR (arg_info->mask[1], arg_node->info.ids->node->varno);
            FreeTree (arg_node);
            arg_node = DupTree (value, NULL);
            break;
        default:
            break;
        }
    }
    DBUG_RETURN (arg_node);
}

node *
CFap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFap");
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFcast
 *  arguments     : 1) cast-node
 *                  2) info-node
 *                  R) cast-node
 *  description   :
 *  global vars   : syntax_tree, info_node
 *  internal funs : --
 *  external funs : OptTrav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
CFcast (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFcast");

    arg_node = OptTrav (arg_node, arg_info, 0);

    switch (arg_node->node[0]->nodetype) {
    case N_num:
    case N_float:
    case N_bool:
    case N_array:
        arg_node = arg_node->node[0];
        break;
    case N_id:
    case N_with:
    case N_prf:
    case N_ap:
        break;
    default:
        DBUG_ASSERT ((FALSE), "Unknown nodetype for CFcast");
        break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFassign
 *  arguments     : 1) assign-node
 *                  2) info-node
 *                  R) assign-node
 *  description   : - constant folding of the instructions
 *		    - construction of a new assign-list if nessessary
 *  global vars   : syntax_tree, cf_stack, info_node
 *  internal funs : --
 *  external funs : GenMask, AppendNodeChain, PlusMask, OptTrav, MinusMask
 *  macros        : DBUG..., TOS
 *
 *  remarks       : --
 *
 */
node *
CFassign (node *arg_node, node *arg_info)
{
    node *returnnode;
    nodetype ntype;

    DBUG_ENTER ("CFassign");
    DBUG_PRINT ("CF", ("Begin folding of line %d", arg_node->lineno));
    returnnode = arg_node;
    if (N_return != arg_node->node[0]->nodetype) {
        arg_info->lineno = arg_node->lineno;
        ntype = arg_node->node[0]->nodetype;

        arg_node = OptTrav (arg_node, arg_info, 0); /* Trav instruction */

        arg_info->node[0] = NULL;
        switch (arg_node->node[0]->nodetype) {
        case N_empty:
            returnnode = arg_node->node[1];
            arg_node = OptTrav (arg_node, arg_info, 1); /* Trav next assign */
            arg_node->nnode = 1;
            FreeTree (arg_node);
            break;
        case N_assign:
            if (N_do == ntype) {
                arg_node->node[1] = Trav (arg_node->node[1], arg_info);
                returnnode = AppendNodeChain (1, arg_node->node[0], arg_node->node[1]);
            } else {
                returnnode = AppendNodeChain (1, arg_node->node[0], arg_node->node[1]);
                arg_node->node[0] = Trav (arg_node->node[0], arg_info);
            }
            break;
        default:
            arg_node = OptTrav (arg_node, arg_info, 1); /* Trav next assign */
            break;
        }
    }
    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : GetType
 *  arguments     : 1) type
 *                  R) real type
 *  description   : determines real type
 *  global vars   : syntax_tree
 *  internal funs : --
 *  external funs : LookupType
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
types *
GetType (types *type)
{
    node *tnode;

    DBUG_ENTER ("GetType");
    if (T_user == type->simpletype) {
        tnode = LookupType (type->name, type->name_mod, 0);
        type = tnode->info.types;
    }
    DBUG_RETURN (type);
}

/*
 *
 *  functionname  : CFlet
 *  arguments     : 1) let-node
 *		    2) info-node
 *                  R) let-node
 *  description   : - stores alink to the type of expression at the left hand side
 *		      of the let in the info_node
 *		    - initiates constant folding for the right hand side
 *                  - stack-entry modified for left hand expression
 *  global vars   : syntax_tree, cf_stack, info_node
 *  internal funs : --
 *  external funs : OptTrav
 *  macros        : DBUG..., VAR
 *
 *  remarks       :
 *
 */
node *
CFlet (node *arg_node, node *arg_info)
{
    node *arg2;

    DBUG_ENTER ("CFlet");

    arg_info->info.types = GetType (arg_node->info.ids->node->info.types);

    arg_node = OptTrav (arg_node, arg_info, 0); /* Trav expression */

    arg_info->info.types = NULL;

    if ((arg_node->node[0]->info.prf == F_reshape)
        && (arg_node->node[0]->nodetype == N_prf)) {
        arg2 = arg_node->node[0]->node[0]->node[1]->node[0];
        if (N_id == arg2->nodetype)
            VAR (arg_node->info.ids->node->varno) = VAR (arg2->info.ids->node->varno);
        else
            VAR (arg_node->info.ids->node->varno) = arg2;
    } else {
        VAR (arg_node->info.ids->node->varno) = arg_node->node[0];
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFwhile
 *  arguments     : 1) while-node
 *		    2) info_node
 *                  R) while-node or empty-node
 *  description   : returns empty-node if condition is false otherwise
 *                  constant folding inside while loop with new stack entry
 *  global vars   : syntax_tree, cf_stack, info_node
 *  internal funs : PushDupVL, PopVL
 *  external funs : MinusMask, PlusMask, FreeTree, ReadMask, OptTrav
 *  macros        : DBUG...,  WARNO, VAR
 *
 *  remarks       : --
 *
 */
node *
CFwhile (node *arg_node, node *arg_info)
{
    int i;
    node *node_behind, *cond_value;

    DBUG_ENTER ("CFwhile");

    node_behind = NodeBehindCast (arg_node->node[0]);

    switch (node_behind->nodetype) {
    case N_bool:
        if (!node_behind->info.cint) {
            MinusMask (arg_info->mask[0], arg_node->node[1]->mask[0], VARNO);
            MinusMask (arg_info->mask[1], arg_node->node[1]->mask[1], VARNO);
            FreeTree (arg_node);
            DBUG_PRINT ("CF", ("while-loop eliminated in line %d", arg_info->lineno));
            cf_expr++;
            arg_node = MakeNode (N_empty);
        }
        break;
    case N_id:
        cond_value = VAR (node_behind->info.ids->node->varno);
        if (1 == IsConst (cond_value)) {
            if (cond_value->info.cint) {
                DBUG_PRINT ("CF", ("while-loop changed to do-loop %d", arg_info->lineno));
                arg_node->nodetype = N_do;
            }
        }
    default: {
        long *used_while;

        used_while = arg_node->node[1]->mask[0];
        PushDupVL ();
        for (i = 0; i < TOS.vl_len; i++) {
            if (ReadMask (used_while, i) != 0) {
                VAR (i) = NULL;
            }
        }
        arg_node = OptTrav (arg_node, arg_info, 1); /* Trav while-body */
        if (opt_unr) {
            arg_node = Unroll (arg_node, arg_info);
        }
        PopVL ();
        for (i = 0; i < TOS.vl_len; i++) {
            if (ReadMask (used_while, i) != 0) {
                VAR (i) = NULL;
            }
        }
    } break;
    }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFdo
 *  arguments     : 1) do-node
 *		    2) info_node
 *		    R) do-node
 *  description   : initiates constant folding inside do-loop with new stack
 *  global vars   : syntax_tree, cf_stack, info_node
 *  internal funs : PushDupVL, PopVL
 *  external funs : ReadMask, OptTrav, FreeTree
 *  macros        : DBUG..., VAR
 *
 *  remarks       : --
 *
 */
node *
CFdo (node *arg_node, node *arg_info)
{
    int i;
    node *a_node, *tmp;

    DBUG_ENTER ("CFdo");
    PushDupVL ();
    a_node = arg_info->node[0];
    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_node->node[1]->mask[0], i) != 0) {
            VAR (i) = NULL;
        }
    }

    arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */

    arg_node = OptTrav (arg_node, arg_info, 0); /* Trav do-condition */

    if ((arg_node->node[0]->info.cint) && (arg_node->node[0]->nodetype == N_bool)) {
        /*
         WARNO(("WARNING in line %d: endless loop expected.",arg_info->lineno));
        */
    }
    if ((!arg_node->node[0]->info.cint) && (arg_node->node[0]->nodetype == N_bool)) {
        PopVL ();
        arg_node = OptTrav (arg_node, arg_info, 1); /* Trav do-body */
        DBUG_PRINT ("CF", ("do-loop eliminated in line %d", arg_info->lineno));
        cf_expr++;
        tmp = arg_node;
        arg_node = arg_node->node[1]->node[0];
        tmp->node[1]->nnode = 0;
        FreeTree (tmp);
    } else {
        if (opt_unr) {
            arg_node = Unroll (arg_node, arg_info);
        }
        PopVL2 ();
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFcond
 *  arguments     : 1) cond-node
 *		    2) info_node
 *		    R) cond-node , assign-node or empty-node
 *  description   : initiates constant folding for the conditional, if conditional
 *		    is true or false, return then or else part, otherwise constant
 *		    folding inside the conditional.
 *  global vars   : syntax_tree, info_node, cf_stack
 *  internal funs : PushDupVL, PopVL
 *  external funs : GenMask, OptTrav, MinusMask, PlusMask, ClearMask, FreeTree
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
CFcond (node *arg_node, node *arg_info)
{
    node *returnnode;
    int i;

    DBUG_ENTER ("CFcond");
    returnnode = arg_node;

    arg_node = OptTrav (arg_node, arg_info, 0);

    if (arg_node->node[0]->nodetype == N_bool) {
        if (arg_node->node[0]->info.cint != 0) {
            MinusMask (arg_info->mask[0], arg_node->node[2]->mask[0], VARNO);
            MinusMask (arg_info->mask[1], arg_node->node[2]->mask[1], VARNO);
            FreeTree (arg_node->node[2]);
            returnnode = arg_node->node[1]->node[0];
            FREE (arg_node->node[1]);
            DBUG_PRINT ("CF", ("else-part of conditional eliminated in line %d",
                               arg_info->lineno));
            cf_expr++;
        } else {
            MinusMask (arg_info->mask[0], arg_node->node[1]->mask[0], VARNO);
            MinusMask (arg_info->mask[1], arg_node->node[1]->mask[1], VARNO);
            FreeTree (arg_node->node[1]);
            returnnode = arg_node->node[2]->node[0];
            FREE (arg_node->node[2]);
            DBUG_PRINT ("CF", ("else-part of conditional eliminated in line %d",
                               arg_info->lineno));
            cf_expr++;
        }
    } else {
        PushDupVL ();

        arg_node = OptTrav (arg_node, arg_info, 1);

        PopVL ();
        PushDupVL ();

        arg_node = OptTrav (arg_node, arg_info, 2);

        PopVL ();
        for (i = 0; i < TOS.vl_len; i++) {
            if ((ReadMask (arg_node->node[1]->mask[0], i) != 0)
                || (ReadMask (arg_node->node[2]->mask[0], i) != 0)) {
                VAR (i) = NULL;
            }
        }
    }

    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : CFwith
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
CFwith (node *arg_node, node *arg_info)
{
    types *oldtype;

    DBUG_ENTER ("CFwith");
    oldtype = arg_info->info.types;
    arg_info->info.types = arg_node->node[0]->info.ids->node->info.types;

    arg_node = OptTrav (arg_node, arg_info, 0); /* Trav generator */

    arg_info->info.types = oldtype;

    PushDupVL ();

    arg_node = OptTrav (arg_node, arg_info, 2); /* Trav with-body */

    PopVL ();
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GotoExprNr
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
GotoExprNr (int n, node *arg_node)
{
    int i;

    DBUG_ENTER ("GotoExprNr");

    for (i = 0; i < n; i++)
        if (NULL != arg_node->node[1])
            arg_node = arg_node->node[1];
        else {
            arg_node = NULL;
            break;
        }

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GetShapeVector
 *  arguments     : 1) ptr to an array
 *		    2) ptr to an C-array, contains shape-vector after GetShapeVector
 *		    R) dimension of 1)
 *  description   : calulates dimension and shape-vector
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., SHP_SEG_SIZE
 *
 *  remarks       : --
 *
 */
int
GetShapeVector (node *array, int *vec_shape)
{
    int vec_dim = 0, i;
    node *expr;

    DBUG_ENTER ("GetShapeVector");

    expr = array->node[0];

    for (i = 0; i <= SHP_SEG_SIZE; i++) {

        if (NULL != expr) {
            vec_dim++;
            vec_shape[i] = expr->node[0]->info.cint;
            if (NULL != expr->node[1])
                expr = expr->node[1];
            else
                expr = NULL;
        } else
            vec_shape[i] = 0;
    }
    DBUG_RETURN (vec_dim);
}

/*
 *
 *  functionname  : ArraySize
 *  arguments     : 1) ptr to th array
 *		    R) size of the array
 *  description   : Counts the elements of the given array
 *  global vars   : --
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
int
ArraySize (node *array)
{
    int size;

    DBUG_ENTER ("ArraySize");
    size = 1;
    array = array->node[0];
    while (2 == array->nnode) {
        array = array->node[1];
        size++;
    }
    DBUG_RETURN (size);
}

/*
 *
 *  functionname  : FetchNum
 *  arguments     : 1) position in array
 *                  2) ptr to the array
 *                  R) ptr to fetchted element
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs : DupTree
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
FetchNum (int start, node *array)
{
    int i;
    node *returnexpr;

    DBUG_ENTER ("FetchNum");

    array = array->node[0];

    for (i = 0; i < start; i++)
        array = array->node[1];

    returnexpr = DupTree (array->node[0], NULL);

    DBUG_RETURN (returnexpr);
}

/*
 *
 *  functionname  : DupPartialArray
 *  arguments     : 1) start position in array
 *                  2) lenght of result array
 *                  3) ptr to the array which should be partial duplicated
 *                  4) arg_info countains used-mask, which will be updated
 *                  R) ptr to result array
 *  description   : duplicates an array between start and (start+length)
 *  global vars   : N_exprs, N_id
 *  internal funs : --
 *  external funs : DupTree, MakeNode
 *  macros        : DBUG..., DEC_VAR
 *
 *  remarks       : --
 *
 */
node *
DupPartialArray (int start, int length, node *array, node *arg_info)
{
    int i;
    node *expr;
    node *r_expr = NULL;

    DBUG_ENTER ("DupPartialArray");

    if (0 < length) {
        array = array->node[0];

        /*
         * Goto start position
         */
        for (i = 0; i < start; i++)
            array = array->node[1];

        /*
         * Duplicate first elenment
         */
        expr = MakeNode (N_exprs);
        expr->node[0] = DupTree (array->node[0], NULL);
        if (N_id == expr->node[0]->nodetype)
            DEC_VAR (arg_info->mask[1], expr->node[0]->info.ids->node->varno);
        expr->nnode = 1;
        array = array->node[1];
        r_expr = expr;

        /*
         * Duplicate rest of array till length reached
         */
        for (i = 1; i < length; i++) {
            expr->nnode++;
            expr->node[1] = MakeNode (N_exprs);
            expr = expr->node[1];
            expr->node[0] = DupTree (array->node[0], NULL);
            if (N_id == expr->node[0]->nodetype)
                DEC_VAR (arg_info->mask[1], expr->node[0]->info.ids->node->varno);
            expr->nnode = 1;
            array = array->node[1];
        }
    }

    DBUG_RETURN (r_expr);
}

/*
 *
 *  functionname  : FoundZero
 *  arguments     : 1) node to be examine
 *		    R) TRUE if node contains a 0, 0.0 , [..., 0, ...] or [..., 0.0, ...]
 *		       FALSE otherwise
 *  description   : determines if expression is a constant containing zero
 *  global vars   : syntax_tree, N_num, N_float, N_array
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., SELARG, NULL, FALSE, TRUE
 *
 *  remarks       : --
 *
 */
int
FoundZero (node *arg_node)
{
    int FoundZero = FALSE;
    node *expr;

    DBUG_ENTER ("IsZero");
    switch (arg_node->nodetype) {
    case N_num:
    case N_float:
        if (0 == SELARG (arg_node))
            FoundZero = TRUE;
        break;
    case N_array:
        expr = arg_node->node[0];
        while (NULL != expr) {
            if (0 == SELARG (expr->node[0]))
                FoundZero = TRUE;
            expr = expr->node[1];
        }
        break;
    default:
        FoundZero = FALSE;
        break;
    }
    DBUG_RETURN (FoundZero);
}

/*
 *
 *  functionname  : SkalarPrf
 *  arguments     : 1) arguments for prim-function
 *		    2) type of prim-function
 *		    3) result type
 *		    4) calculate (arg1 op arg2) if swap==FALSE or
 *				 (arg2 op arg1) if swap==TRUE
 *		    R) result-node
 *  description   : Calculates non array prim-functions
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : arguments have to be constants before calling this function
 *
 */
node *
SkalarPrf (node **arg, prf prf_type, types *res_type, int swap)
{

/*
 * This macro calculates all non prefix, non array primitive functions
 */
#define ARI(op, a1, a2)                                                                  \
    {                                                                                    \
        if (T_float == res_type->simpletype) {                                           \
            if (!swap) {                                                                 \
                a1->info.cfloat = SELARG (a1) op SELARG (a2);                            \
            } else {                                                                     \
                a1->info.cfloat = SELARG (a2) op SELARG (a1);                            \
            }                                                                            \
            a1->nodetype = N_float;                                                      \
        } else {                                                                         \
            if (!swap) {                                                                 \
                a1->info.cint = a1->info.cint op a2->info.cint;                          \
            } else {                                                                     \
                a1->info.cint = a2->info.cint op a1->info.cint;                          \
            }                                                                            \
        }                                                                                \
        cf_expr++;                                                                       \
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));       \
    }

    DBUG_ENTER ("SkalarPrf");

    /*
     * Calculate primitive Functions
     */
    switch (prf_type) {
    case F_not:
        arg[0]->info.cint = !arg[0]->info.cint;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[prf_type]));
        cf_expr++;
        break;
    case F_add:
    case F_add_AxA:
    case F_add_AxS:
    case F_add_SxA:
        ARI (+, arg[0], arg[1]);
        break;
    case F_sub:
    case F_sub_AxA:
    case F_sub_AxS:
    case F_sub_SxA:
        ARI (-, arg[0], arg[1]);
        break;
    case F_mul:
    case F_mul_AxA:
    case F_mul_AxS:
    case F_mul_SxA:
        ARI (*, arg[0], arg[1]);
        break;
    case F_div:
    case F_div_AxA:
    case F_div_AxS:
    case F_div_SxA:
        ARI (/, arg[0], arg[1]);
        break;
    case F_gt:
        ARI (>, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_lt:
        ARI (<, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_ge:
        ARI (>=, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_le:
        ARI (<=, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_eq:
        ARI (==, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_neq:
        ARI (!=, arg[0], arg[1]);
        arg[0]->nodetype = N_bool;
        break;
    case F_and:
        ARI (&&, arg[0], arg[1]);
        break;
    case F_or:
        ARI (||, arg[0], arg[1]);
        break;
    default:
        break;
    }
    DBUG_RETURN (arg[0]);
}

/*
 *
 *  functionname  : FoldExpr
 *  arguments     : 1) prf-node
 *		    2) argument number, which should be tested
 *		    3) argument number, which should be retured if test was successfully
 *		    4) test-pattern
 *		    5) arg_info-node needed for mask update
 *		    R) argument number res_arg or prf-node
 *  description   : folds the given prf to the argument number res_arg, if argument
 *		    number test_arg is an int, float or bool and this argument contains
 *		    the test_pattern
 *  global vars   : syntax_tree, N_num, N_bool, N_float, N_id
 *  internal funs : --
 *  external funs : FreePrf2
 *  macros        : DBUG..., DEC_VAR, SELARG, NULL
 *
 *  remarks       :
 *
 */
node *
FoldExpr (node *arg_node, int test_arg, int res_arg, int test_pattern, node *arg_info)
{
    node *tmp, *arg[2];
    nodetype node_t;

    DBUG_ENTER ("FoldExpr");

    arg[0] = arg_node->node[0]->node[0];
    arg[1] = arg_node->node[0]->node[1]->node[0];
    tmp = arg_node;
    node_t = arg[test_arg]->nodetype;
    if (((N_num == node_t) || (N_bool == node_t) || (N_float == node_t))
        && (test_pattern == SELARG (arg[test_arg]))) {
        arg_node = arg[res_arg];
        if ((N_id == arg[!res_arg]->nodetype) && (NULL != arg_info)) {
            DEC_VAR (arg_info->mask[1], arg[!res_arg]->info.ids->node->varno);
        }
        cf_expr++;
        DBUG_PRINT ("CF", ("primitive function %s folded", prf_string[tmp->info.prf]));
        FreePrf2 (tmp, res_arg);
        arg_node = arg[res_arg];
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : NoConstSkalarPrf
 *  arguments     : 1) prf-node
 *		    2) result type
 *		    3) arg_info includes used mask to be updated and varno
 *		    R) result-node
 *  description   : Calculates some  prim-functions with one non constant arguments
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : FreePrf2
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : if (NULL==arg_info) mask will not be updated
 *
 */
node *
NoConstSkalarPrf (node *arg_node, types *res_type, node *arg_info)
{
    DBUG_ENTER ("NoConstSkalarPrf");

    /*
     * Calculate prim-functions with non constant arguments
     */
    switch (arg_node->info.prf) {
    case F_and:
        arg_node = FoldExpr (arg_node, 0, 0, FALSE, arg_info); /* FALSE && x = FALSE */
        if (N_prf == arg_node->nodetype)
            arg_node
              = FoldExpr (arg_node, 1, 1, FALSE, arg_info); /* x && FALSE = FALSE */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 0, 1, TRUE, arg_info); /* x && TRUE = x */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 1, 0, TRUE, arg_info); /* TRUE && x = x */
        break;
    case F_or:
        arg_node = FoldExpr (arg_node, 0, 0, TRUE, arg_info); /* TRUE || x = TRUE */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 1, 1, TRUE, arg_info); /* x || TRUE = TRUE */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 0, 1, FALSE, arg_info); /* x || FALSE = x */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 1, 0, FALSE, arg_info); /* FALSE || x = x */
        break;
    case F_mul:
    case F_mul_AxS:
    case F_mul_SxA:
        arg_node = FoldExpr (arg_node, 0, 0, 0, arg_info); /* 0 * x = 0 */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 1, 1, 0, arg_info); /* x * 0 = 0 */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 0, 1, 1, arg_info); /* 1 * x = x */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 1, 0, 1, arg_info); /* x * 1 = x */
        break;
    case F_div:
    case F_div_SxA:
        if (TRUE == FoundZero (arg_node->node[0]->node[1]->node[0])) {
            WARNO (
              ("%s, %d:WARNING: Devision by zero expected.", filename, arg_node->lineno));
        }
        arg_node = FoldExpr (arg_node, 0, 0, 0, arg_info); /* 0 / x = 0 */
        break;
    case F_add:
    case F_add_AxS:
    case F_add_SxA:
    case F_sub:
    case F_sub_AxS:
    case F_sub_SxA:
        arg_node = FoldExpr (arg_node, 0, 1, 0, arg_info); /* 0 [+-] x = x */
        if (N_prf == arg_node->nodetype)
            arg_node = FoldExpr (arg_node, 1, 0, 0, arg_info); /* x [+-] 0 = x */
        break;
    default:
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : ArrayPrf
 *  arguments     : 1) prf-node
 *		    2) result type
 *		    3) arg_info->mask[1] need for eliminating identifier
 *		    R) result-node
 *  description   : Calculates array prim-functions
 *  global vars   : N_float, ... , N_not, ..., prf_string
 *  internal funs : --
 *  external funs : --
 *  macros        : DBUG..., NULL, FREE
 *
 *  remarks       : most arguments have to be constants before calling this function
 *
 */
node *
ArrayPrf (node *arg_node, types *res_type, node *arg_info)
{
    node *arg[MAXARG], *expr[MAXARG], *expr_arg[MAXARG], *tmp;
    shpseg *shape;
    int swap, i;
    long *used_sofar;

    DBUG_ENTER ("ArrayPrf");

    /*
     * Search Arguments for primitive Functions
     */
    tmp = arg_node->node[0];
    for (i = 0; i < MAXARG; i++) {
        if (tmp != NULL) {
            arg[i] = tmp->node[0];
            tmp = tmp->node[1];
        } else
            arg[i] = NULL;
    }

    /*
     * Calculate primitive Functions
     */
    switch (arg_node->info.prf) {
    case F_div_AxA:
    case F_div_AxS:
    case F_div_SxA:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_ge:
    case F_gt:
    case F_neq:
    case F_add_AxA:
    case F_add_AxS:
    case F_add_SxA:
    case F_sub_AxA:
    case F_sub_AxS:
    case F_sub_SxA:
    case F_mul_AxA:
    case F_mul_AxS:
    case F_mul_SxA: {
        node *old_arg[2], *value;

        DBUG_PRINT ("CF", ("Begin folding of %s", prf_string[arg_node->info.prf]));

        old_arg[0] = arg[0];
        old_arg[1] = arg[1];

        if (N_id == arg[0]->nodetype) {
            value = VAR (arg[0]->info.ids->node->varno);
            if (1 == IsConst (value)) {
                DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
                arg[0] = DupTree (value, NULL);
                used_sofar = arg_info->mask[1];
                arg_info->mask[1] = GenMask (VARNO);
                arg[0] = Trav (arg[0], arg_info);
                FREE (arg_info->mask[1]);
                arg_info->mask[1] = used_sofar;
            } else {
                break;
            }
        } else {
            if (1 == IsConst (arg[0]))
                arg[0] = Trav (arg[0], arg_info);
            else
                break;
        }

        if (N_id == arg[1]->nodetype) {
            value = VAR (arg[1]->info.ids->node->varno);
            if (1 == IsConst (value)) {
                DEC_VAR (arg_info->mask[1], arg[1]->info.ids->node->varno);
                arg[1] = DupTree (value, NULL);
                used_sofar = arg_info->mask[1];
                arg_info->mask[1] = GenMask (VARNO);
                arg[1] = Trav (arg[1], arg_info);
                FREE (arg_info->mask[1]);
                arg_info->mask[1] = used_sofar;
            } else {
                if (N_id == old_arg[0]->nodetype) {
                    INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }
        } else {
            if (1 == IsConst (arg[1]))
                arg[1] = Trav (arg[1], arg_info);
            else {
                if (N_id == old_arg[0]->nodetype) {
                    INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }
        }

        if (((F_div == arg_node->info.prf) || (F_div_SxA == arg_node->info.prf)
             || (F_div_AxS == arg_node->info.prf) || (F_div_AxA == arg_node->info.prf))
            && (TRUE == FoundZero (arg[1]))) {
            if (N_id == old_arg[0]->nodetype) {
                INC_VAR (arg_info->mask[1], old_arg[0]->info.ids->node->varno);
                FreeTree (arg[0]);
            }
            if (N_id == old_arg[1]->nodetype) {
                INC_VAR (arg_info->mask[1], old_arg[1]->info.ids->node->varno);
                FreeTree (arg[1]);
            }
            WARNO (
              ("%s, %d:WARNING: Devision by zero expected.", filename, arg_node->lineno));
            break;
        }

        /*
         * Swap arguments, to be sure that an array is first argument
         */
        if (N_array != arg[0]->nodetype) {
            tmp = arg[1];
            arg[1] = arg[0];
            arg[0] = tmp;
            swap = TRUE;
        } else {
            swap = FALSE;
        }

        if (N_array == arg[1]->nodetype) {
            /*
             * Calculate prim-function with two arrays
             */
            expr[0] = arg[0]->node[0];
            expr[1] = arg[1]->node[0];
            do {
                expr_arg[0] = expr[0]->node[0];
                expr_arg[1] = expr[1]->node[0];
                expr[0]->node[0]
                  = SkalarPrf (expr_arg, arg_node->info.prf, res_type, swap);
                expr[0] = expr[0]->node[1];
                expr[1] = expr[1]->node[1];
            } while ((NULL != expr[0]) && (NULL != expr[1]));
        } else {
            /*
             * Calculate prim-function with one array
             */
            expr[0] = arg[0]->node[0];
            expr_arg[1] = arg[1];
            do {
                expr_arg[0] = expr[0]->node[0];
                expr[0]->node[0]
                  = SkalarPrf (expr_arg, arg_node->info.prf, res_type, swap);
                expr[0] = expr[0]->node[1];
            } while ((NULL != expr[0]));
        }

        DBUG_PRINT ("CF", ("End folding of %s", prf_string[arg_node->info.prf]));

        /*
         * free useless nodes and store result
         */
        if (!swap) {
            if (old_arg[0] == arg[0])
                FreePrf2 (arg_node, 0);
            else
                FreeTree (arg_node);

            if (old_arg[1] != arg[1])
                FreeTree (arg[1]);

            arg_node = arg[0];
        } else {
            if (old_arg[1] == arg[0])
                FreePrf2 (arg_node, 1);
            else
                FreeTree (arg_node);

            if (old_arg[0] != arg[1])
                FreeTree (arg[1]);

            arg_node = arg[0];
            DBUG_PRINT ("MEM", ("Argument has address: %08x", arg_node));
        }
    } break;

    /***********************/
    /* Fold shape-function */
    /***********************/
    case F_shape:
        switch (arg[0]->nodetype) {
        case (N_array):
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * Count array length
             */
            i = 0;
            tmp = arg[0]->node[0];
            do {
                i++;
                tmp = tmp->node[1];
            } while (NULL != tmp);

            /*
             * Store result in this array
             */
            arg[0]->node[0]->node[0]->info.cint = i;
            arg[0]->node[0]->node[0]->nodetype = N_num;

            /*
             * Free rest of array and prf-node
             */
            if (NULL != arg[0]->node[0]->node[1])
                FreeTree (arg[0]->node[0]->node[1]);
            arg[0]->node[0]->nnode = 0;
            arg[0]->node[0]->node[1] = NULL;
            FREE (arg_node);

            /*
             * Store result
             */
            arg_node = arg[0];
            cf_expr++;
            break;
        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * free prf_node
             */
            FREE (arg_node);

            /*
             * Generate shape vector
             */
            arg_node = MakeNode (N_array);
            arg_node->nnode = 1;

            shape = arg[0]->info.ids->node->info.types->shpseg;

            arg_node->node[0] = MakeNode (N_exprs);
            arg_node->node[0]->node[0] = MakeNode (N_num);
            arg_node->node[0]->nnode = 1;
            arg_node->node[0]->node[0]->info.cint = shape->shp[0];

            tmp = arg_node->node[0];
            for (i = 1; i < res_type->shpseg->shp[0]; i++) {
                tmp->nnode++;
                tmp->node[1] = MakeNode (N_exprs);
                tmp = tmp->node[1];
                tmp->nnode = 1;
                tmp->node[0] = MakeNode (N_num);
                tmp->node[0]->info.cint = shape->shp[i];
            }

            DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
            FreeTree (arg[0]);
            cf_expr++;
            break;
        default:
            break;
        }
        break; /* shape */

    /***********************/
    /* Fold dim-function   */
    /***********************/
    case F_dim:
        switch (arg[0]->nodetype) {
        case N_array:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * result is always 1
             */
            arg_node->nodetype = N_num;
            arg_node->info.cint = 1;

            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
            arg_node->node[0] = NULL;
            arg_node->nnode = 0;

            cf_expr++;
            break;
        case N_id:
            DBUG_PRINT ("CF",
                        ("primitive function %s folded", prf_string[arg_node->info.prf]));
            /*
             * get result
             */
            arg_node->nodetype = N_num;
            arg_node->info.cint = arg[0]->info.ids->node->info.types->dim;

            /*
             * Free argument of prim function
             */
            FreeTree (arg_node->node[0]);
            arg_node->node[0] = NULL;
            arg_node->nnode = 0;

            DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
            cf_expr++;
            break;
        default:
            break;
        }
        break; /* dim */

    /***********************/
    /* Fold psi-function   */
    /***********************/
    case F_psi:

        switch (arg[1]->nodetype) {
        case N_array: {
            int start;
            node *tmp;

            if ((0 <= arg[0]->node[0]->node[0]->info.cint)
                && (arg[0]->node[0]->node[0]->info.cint < ArraySize (arg[1]))) {
                DBUG_PRINT ("CF", ("primitive function %s folded",
                                   prf_string[arg_node->info.prf]));
                cf_expr++;

                start = arg[0]->node[0]->node[0]->info.cint;
                tmp = DupTree (FetchNum (start, arg[1]), NULL);
                if (N_id == tmp->nodetype) {
                    DEC_VAR (arg_info->mask[1], tmp->info.ids->node->varno);
                }

                FreeTree (arg_node);
                arg_node = tmp;
            } else {
                WARNO (("%s, %d:WARNING: Illegal vector for primitive function psi.",
                        filename, arg_node->lineno));
            }
        } break;
        case N_id: {
            int length, start, mult, i, j, arg_length;
            int vec_dim;
            int vec_shape[SHP_SEG_SIZE];
            node *res_node, *old_arg_0, *value;

            old_arg_0 = arg[0];

            /*
             * Substitute shape-vector
             */
            if (N_id == arg[0]->nodetype) {
                value = VAR (arg[0]->info.ids->node->varno);
                if (1 == IsConst (value)) {
                    DEC_VAR (arg_info->mask[1], arg[0]->info.ids->node->varno);
                    arg[0] = DupTree (value, NULL);
                    used_sofar = arg_info->mask[1];
                    arg_info->mask[1] = GenMask (VARNO);
                    arg[0] = Trav (arg[0], arg_info);
                    FREE (arg_info->mask[1]);
                    arg_info->mask[1] = used_sofar;
                } else
                    break;
            } else {
                arg[0] = Trav (arg[0], arg_info);
            }

            /*
             * Substitution of shape-vector successful ?
             */
            if ((1 != IsConst (arg[0]))
                || (NULL == VAR (arg[1]->info.ids->node->varno))) {
                if (N_id == old_arg_0->nodetype) {
                    INC_VAR (arg_info->mask[1], old_arg_0->info.ids->node->varno);
                    FreeTree (arg[0]);
                }
                break;
            }

            /* Calculate dimension and shape vector of first argument */
            vec_dim = GetShapeVector (arg[0], vec_shape);

            /* Calculate length of result array */
            length = 1;
            for (i = 0; i < arg_info->info.types->dim; i++)
                length *= arg_info->info.types->shpseg->shp[i];

            /* Calculate startposition of result array in argument array */
            start = 0;
            for (i = 0; i < vec_dim; i++) {
                if (vec_shape[i] >= 0) {
                    mult = 1;
                    for (j = i + 1; j < arg[1]->info.ids->node->info.types->dim; j++)
                        mult *= arg[1]->info.ids->node->info.types->shpseg->shp[j];
                    start += vec_shape[i] * mult;
                } else {
                    start = -1;
                    i = vec_dim;
                }
            }

            arg_length = 1;
            for (i = 0; i < arg[1]->info.ids->node->info.types->dim; i++)
                arg_length *= arg[1]->info.ids->node->info.types->shpseg->shp[i];

            if ((start + length <= arg_length) && (start >= 0)) {
                DEC_VAR (arg_info->mask[1], arg[1]->info.ids->node->varno);
                if (vec_dim == arg[1]->info.ids->node->info.types->dim) {
                    res_node = FetchNum (start, VAR (arg[1]->info.ids->node->varno));
                    if (N_id == res_node->nodetype)
                        DEC_VAR (arg_info->mask[1], res_node->info.ids->node->varno);
                } else {
                    res_node = MakeNode (N_array);
                    res_node->nnode = 1;
                    res_node->node[0]
                      = DupPartialArray (start, length,
                                         VAR (arg[1]->info.ids->node->varno), arg_info);
                }
                FreeTree (arg_node);
                arg_node = res_node;
                DBUG_PRINT ("CF", ("primitive function %s folded in line %d",
                                   prf_string[arg_node->info.prf], arg_info->lineno));
                cf_expr++;
            } else {
                if (old_arg_0 != arg[0])
                    FreeTree (arg[0]);
                WARNO (("%s, %d:WARNING: Illegal vector for primitive function psi.",
                        filename, arg_node->lineno));
            }
        } break;
        default:
            break;
        }
        break;
    case F_take:
    case F_drop:
    case F_cat:
    case F_rotate:
    default:
        break;
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFprf
 *  arguments     : 1) prf-node
 *		    2) info_node
 *		    R) prf-node or calculated constant value
 *  description   : first replace constant values for variables in expression
 *		    then calculate primitive function.
 *  global vars   : syntax_tree, info_node, cf_stack
 *  internal funs : SkalarPrf, ArrayPrf
 *  external funs : Trav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
CFprf (node *arg_node, node *arg_info)
{
    node *arg[MAXARG];
    node *tmp;
    int i;

    DBUG_ENTER ("CFprf");

    /*
     * Do some foldings like 1 * x = x, etc.
     */
    arg_node = NoConstSkalarPrf (arg_node, arg_info->info.types, arg_info);

    if (N_prf == arg_node->nodetype) {
        if (arg_node->info.prf <= F_neq) {
            /*
             * substitute all arguments
             */
            arg_node->node[0] = Trav (arg_node->node[0], arg_info);

            /*
             * Search Arguments for primitive Functions
             */
            tmp = arg_node->node[0];
            for (i = 0; i < MAXARG; i++) {
                if (tmp != NULL) {
                    arg[i] = tmp->node[0];
                    tmp = tmp->node[1];
                } else
                    arg[i] = NULL;
            }

            /*
             * Calculate non array primitive functions
             */
            if ((1 == IsConst (arg[0]))
                && ((F_not == arg_node->info.prf) || (1 == IsConst (arg[1])))) {
                if (!((F_div == arg_node->info.prf) && (TRUE == FoundZero (arg[1])))) {
                    arg[0]
                      = SkalarPrf (arg, arg_node->info.prf, arg_info->info.types, FALSE);
                    FreePrf2 (arg_node, 0);
                    arg_node = arg[0];
                }
            }
        } else {
            /*
             * Calculate primitive functions with arrays
             */
            arg_node = ArrayPrf (arg_node, arg_info->info.types, arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}
