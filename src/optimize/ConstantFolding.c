/*
 *
 * $Log$
 * Revision 1.4  1995/02/28 18:35:00  asi
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
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "ConstantFolding.h"

typedef struct STELM {
    int vl_len;
    node **varlist;
} stelm;

typedef struct STACK {
    long tos;
    long st_len;
    stelm *stack;
} stack;

#define MIN_STACK_SIZE 50

stack *cf_stack;

#define TOS cf_stack->stack[cf_stack->tos]
#define VAR(i) TOS.varlist[i]

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
    DBUG_ENTER ("PushVL");
    cf_stack->stack[++cf_stack->tos].varlist = (node **)MAlloc (sizeof (node *) * NumVar);
    cf_stack->stack[cf_stack->tos].vl_len = NumVar;
    if (cf_stack->tos == cf_stack->st_len) {
        if (NULL != (cf_stack->stack = realloc (cf_stack->stack, 2 * cf_stack->st_len)))
            Error ("out of memory", 1);
        cf_stack->st_len *= 2;
    }
    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : PushDupVL
 *  arguments     : --
 *  description   : Duplicates a the top entry of the cf_stack
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
    NumVar = cf_stack->stack[cf_stack->tos].vl_len;
    cf_stack->stack[++cf_stack->tos].varlist = (node **)MAlloc (sizeof (node *) * NumVar);
    for (i = 0; i < NumVar; i++)
        cf_stack->stack[cf_stack->tos].varlist[i]
          = cf_stack->stack[cf_stack->tos - 1].varlist[i];
    cf_stack->stack[cf_stack->tos].vl_len = cf_stack->stack[cf_stack->tos - 1].vl_len;
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
    free (cf_stack->stack[cf_stack->tos--].varlist);
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

    free (info_node);
    free (cf_stack->stack);
    free (cf_stack);
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
 *  external funs : GenMask, MinusMask, Trav
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
CFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFfundef");
    if (arg_node->node[0] != NULL) {
        VARNO = arg_node->varno;
        PushVL (arg_info->varno);
        arg_info->mask[0] = GenMask (VARNO);
        arg_info->mask[1] = GenMask (VARNO);

        arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* functionbody */

        MinusMask (arg_node->mask[0], arg_info->mask[0], VARNO);
        MinusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
        free (arg_info->mask[0]);
        free (arg_info->mask[1]);
        PopVL (cf_stack);
    }

    if (arg_node->node[1] != NULL)
        arg_node->node[1] = Trav (arg_node->node[1], arg_info); /* next function */
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : DupConst
 *  arguments     : 1) num-, bool- or float-node
 *		    R) duplicted num-, bool- or float-node
 *  description   : duplicates a node representing a integer, boolean or float constant
 *  global vars   : syntax_tree
 *  internal funs :  --
 *  external funs : MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
DupConst (node *arg_node)
{
    node *new_node;

    DBUG_ENTER ("DupConst");
    new_node = MakeNode (N_num);
    new_node->nodetype = arg_node->nodetype;
    switch (arg_node->nodetype) {
    case N_num:
    case N_bool:
        new_node->info.cint = arg_node->info.cint;
        break;
    case N_float:
        new_node->info.cfloat = arg_node->info.cfloat;
        break;
    default:
        break;
    }
    DBUG_RETURN (new_node);
}

/*
 *
 *  functionname  : DupArray
 *  arguments     : 1) array-node
 *                  R) duplicated array-node
 *  description   : duplicates an array
 *  global vars   : syntax_tree
 *  internal funs : DupConst
 *  external funs : MakeNode
 *  macros        : DBUG...
 *
 *  remarks       : --
 *
 */
node *
DupArray (node *arg_node)
{
    node *new_node;
    node *expr;
    node *new_expr;

    DBUG_ENTER ("DupArray");
    new_node = MakeNode (N_array);
    new_node->nnode = 1;
    new_node->node[0] = MakeNode (N_exprs);
    expr = arg_node->node[0];
    new_expr = new_node->node[0];
    while (1) {
        new_expr->node[0] = DupConst (expr->node[0]);
        if (NULL != expr->node[1]) {
            new_expr->node[1] = MakeNode (N_exprs);
            new_expr->nnode = 2;
            new_expr = new_expr->node[1];
            expr = expr->node[1];
        } else {
            new_expr->nnode = 1;
            break;
        }
    }
    DBUG_RETURN (new_node);
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
 *  internal funs : DupConst, DupArray
 *  external funs : --
 *  macros        : DBUG..., TOS
 *
 *  remarks       : --
 *
 */
node *
CFid (node *arg_node, node *arg_info)
{
    node *return_node;
    node *value;

    DBUG_ENTER ("CFid");
    value = TOS.varlist[arg_node->node[0]->varno];
    if (value != NULL) {
        switch (value->nodetype) {
        case (N_num):
        case (N_float):
        case (N_bool):
            return_node = DupConst (value);
            INC_VAR (arg_info->mask[1], arg_node->node[0]->varno);
            break;
        case (N_array):
            return_node = DupArray (value);
            INC_VAR (arg_info->mask[1], arg_node->node[0]->varno);
            break;
        default:
            arg_info->nnode = 0;
            return_node = arg_node;
        }
    } else {
        arg_info->nnode = 0;
        return_node = arg_node;
    }
    DBUG_RETURN (return_node);
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
 *  external funs : GenMask, AppendNodeChain, PlusMask, Trav, MinusMask
 *  macros        : DBUG..., TOS
 *
 *  remarks       : --
 *
 */
node *
CFassign (node *arg_node, node *arg_info)
{
    long *oldmask[2];
    int i;
    node *tmp;

    DBUG_ENTER ("CFassign");
    oldmask[0] = arg_info->mask[0];
    oldmask[1] = arg_info->mask[1];
    arg_info->mask[0] = GenMask (VARNO);
    arg_info->mask[1] = GenMask (VARNO);
    arg_info->node[0] = arg_node;

    arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* Trav instruction */

    arg_info->node[0] = NULL;

    switch (arg_node->node[0]->nodetype) {
    case N_empty: /* a condition or a loop has been destroyed */
    case N_assign:
        tmp = arg_node;
        if (arg_node->node[0]->nodetype != N_empty)
            arg_node = AppendNodeChain (1, arg_node->node[0], arg_node->node[1]);
        else {
            free (arg_node->node[0]);
            arg_node = arg_node->node[1];
        }
        free (tmp);
        PlusMask (arg_info->mask[0], oldmask[0], VARNO);
        PlusMask (arg_info->mask[1], oldmask[1], VARNO);
        free (oldmask[0]);
        free (oldmask[1]);

        arg_node = Trav (arg_node, arg_info); /* Trav next assign */

        break;
    case N_cond:
    case N_do:
    case N_while:
        MinusMask (arg_node->mask[0], arg_info->mask[0], VARNO);
        MinusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
        for (i = 0; i < TOS.vl_len; i++) {
            if (ReadMask (arg_node->mask[0], i) != 0)
                VAR (i) = NULL;
        }
        PlusMask (arg_info->mask[0], oldmask[0], VARNO);
        PlusMask (arg_info->mask[1], oldmask[1], VARNO);
        free (oldmask[0]);
        free (oldmask[1]);
        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info); /* Trav next assign */
        }
        break;
    default:
        MinusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
        PlusMask (arg_info->mask[1], oldmask[1], VARNO);
        free (oldmask[0]);
        free (oldmask[1]);
        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info); /* Trav next assign */
        }
        break;
    }
    DBUG_RETURN (arg_node);
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
 *  external funs : Trav
 *  macros        : DBUG..., VAR
 *
 *  remarks       :
 *
 */
node *
CFlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFlet");
    arg_info->nnode = 1;
    arg_info->info.types = arg_node->info.ids->node->info.types;
    arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* Trav expression */
    arg_info->info.types = NULL;
    if (arg_info->nnode == 1)
        VAR (arg_node->info.ids->node->varno) = arg_node->node[0];
    else
        VAR (arg_node->info.ids->node->varno) = NULL;
    if ((arg_node->node[0]->info.prf == F_reshape)
        && (arg_node->node[0]->nodetype == N_prf))
        VAR (arg_node->info.ids->node->varno)
          = arg_node->node[0]->node[0]->node[1]->node[0];
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
 *  external funs : MinusMask, PlusMask, FreeTree, ReadMask, Trav
 *  macros        : DBUG..., WARN1, VAR
 *
 *  remarks       : --
 *
 */
node *
CFwhile (node *arg_node, node *arg_info)
{
    int i;
    node *a_node;

    DBUG_ENTER ("CFwhile");
    PushDupVL ();
    a_node = arg_info->node[0];
    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_info->node[0]->mask[0], i) != 0)
            VAR (i) = NULL;
    }

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    MinusMask (arg_node->mask[1], arg_info->mask[1], VARNO);

    switch (arg_node->node[0]->nodetype) {
    case N_bool:
        if (arg_node->node[0]->info.cint) {
            WARN1 (("WARNING in line %d: endless loop expected\n", a_node->varno));
        } else {
            PlusMask (arg_info->mask[0], a_node->mask[0], VARNO);
            PlusMask (arg_info->mask[1], a_node->mask[1], VARNO);
            FreeTree (arg_node);
            arg_node = MakeNode (N_empty);
            break;
        }
    default:
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        break;
    }
    PopVL ();
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
 *  external funs : ReadMask, Trav, FreeTree
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
        if (ReadMask (arg_info->node[0]->mask[0], i) != 0)
            VAR (i) = NULL;
    }

    arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    MinusMask (arg_node->mask[1], arg_info->mask[1], VARNO);

    if ((arg_node->node[0]->info.cint) && (arg_node->node[0]->nodetype == N_bool)) {
        WARN1 (("WARNING in line %d: endless loop expected\n", a_node->varno));
    } else {
        tmp = arg_node;
        arg_node = arg_node->node[1]->node[0];
        tmp->node[1]->nnode = 0;
        FreeTree (tmp);
    }
    PopVL ();
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
 *  external funs : GenMask, Trav, MinusMask, PlusMask, ClearMask, FreeTree
 *  macros        : DBUG...
 *
 *  remarks       :
 *
 */
node *
CFcond (node *arg_node, node *arg_info)
{
    long *oldmask[2];
    node *returnnode;

    DBUG_ENTER ("CFcond");
    returnnode = arg_node;
    PushDupVL ();
    oldmask[0] = arg_info->mask[0];
    oldmask[1] = arg_info->mask[1];
    arg_info->mask[0] = GenMask (VARNO);
    arg_info->mask[1] = GenMask (VARNO);

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    MinusMask (arg_node->mask[0], arg_info->mask[0], VARNO);
    MinusMask (arg_node->mask[1], arg_info->mask[1], VARNO);
    PlusMask (oldmask[0], arg_info->mask[0], VARNO);
    PlusMask (oldmask[1], arg_info->mask[1], VARNO);
    ClearMask (arg_info->mask[0], VARNO);
    ClearMask (arg_info->mask[1], VARNO);

    if (arg_node->node[0]->nodetype == N_bool) {
        if (arg_node->node[0]->info.cint != 0) {
            PlusMask (oldmask[0], arg_node->node[2]->mask[0], VARNO);
            PlusMask (oldmask[1], arg_node->node[2]->mask[1], VARNO);
            FreeTree (arg_node->node[2]);
            returnnode = arg_node->node[1]->node[0];
            free (arg_node->node[1]);
        } else {
            PlusMask (oldmask[0], arg_node->node[1]->mask[0], VARNO);
            PlusMask (oldmask[1], arg_node->node[1]->mask[1], VARNO);
            FreeTree (arg_node->node[1]);
            returnnode = arg_node->node[2]->node[0];
            free (arg_node->node[2]);
        }
    } else {
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);

        MinusMask (arg_node->node[1]->mask[0], arg_info->mask[0], VARNO);
        MinusMask (arg_node->node[1]->mask[1], arg_info->mask[1], VARNO);
        PlusMask (oldmask[0], arg_info->mask[0], VARNO);
        PlusMask (oldmask[1], arg_info->mask[1], VARNO);
        ClearMask (arg_info->mask[0], VARNO);
        ClearMask (arg_info->mask[1], VARNO);
        PopVL ();
        PushDupVL ();

        arg_node->node[2] = Trav (arg_node->node[2], arg_info);

        MinusMask (arg_node->node[2]->mask[0], arg_info->mask[0], VARNO);
        MinusMask (arg_node->node[2]->mask[1], arg_info->mask[1], VARNO);
        PlusMask (oldmask[0], arg_info->mask[0], VARNO);
        PlusMask (oldmask[1], arg_info->mask[1], VARNO);
    }
    free (arg_info->mask[0]);
    free (arg_info->mask[1]);
    arg_info->mask[0] = oldmask[0];
    arg_info->mask[1] = oldmask[1];
    PopVL ();
    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : CF_with
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
    DBUG_ENTER ("CFwith");
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : GetReturnType
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
GetReturnType (int res_int, node **arg, node *arg_info)
{
    DBUG_ENTER ("CleanupCF");
    if (res_int == -1)
        arg[0]->nodetype = N_bool;
    else
        switch (arg_info->info.types->simpletype) {
        case T_int:
            arg[0]->nodetype = N_num;
            break;
        case T_float:
            arg[0]->nodetype = N_float;
            break;
        case T_bool:
            arg[0]->nodetype = N_bool;
            break;
        default:
            break;
        }
    DBUG_RETURN (arg[0]);
}

/*
 *
 *  functionname  : SkalarPrf
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
SkalarPrf (int res_int, node **arg, int arg_no, int swap, node *arg_node, node *arg_info)
{
    node *returnnode = NULL;

#define SELARG(n) ((n->nodetype == N_num) ? n->info.cint : n->info.cfloat)
#define ARI(op, a1, a2)                                                                  \
    {                                                                                    \
        if (res_int)                                                                     \
            if (!swap)                                                                   \
                a1->info.cint = a1->info.cint op a2->info.cint;                          \
            else                                                                         \
                a1->info.cint = a2->info.cint op a1->info.cint;                          \
        else if (!swap)                                                                  \
            a1->info.cfloat = SELARG (a1) op SELARG (a2);                                \
        else                                                                             \
            a1->info.cfloat = SELARG (a2) op SELARG (a1);                                \
        returnnode = GetReturnType (res_int, arg, arg_info);                             \
    }

    DBUG_ENTER ("SkalarPrf");
    if (arg_info->nnode == 1) {
        cf_expr++;
        switch (arg_node->info.prf) {
        case F_not:
            arg[0]->info.cint = !arg[0]->info.cint;
            returnnode = arg[0];
            break;
        case F_add:
            ARI (+, arg[0], arg[1]);
            break;
        case F_sub:
            ARI (-, arg[0], arg[1]);
            break;
        case F_mul:
            ARI (*, arg[0], arg[1]);
            break;
        case F_div:
            if (SELARG (arg[1]) != 0) {
                ARI (/, arg[0], arg[1]);
            } else {
                WARN1 (("WARNING in line %d: division by zero error expected\n",
                        arg_info->node[0]->lineno));
                returnnode = arg_node;
            }
            break;
        case F_gt:
            ARI (>, arg[0], arg[1]);
            break;
        case F_lt:
            ARI (<, arg[0], arg[1]);
            break;
        case F_ge:
            ARI (>=, arg[0], arg[1]);
            break;
        case F_le:
            ARI (<=, arg[0], arg[1]);
            break;
        case F_eq:
            ARI (==, arg[0], arg[1]);
            break;
        case F_neq:
            ARI (!=, arg[0], arg[1]);
            break;
        case F_and:
            ARI (&&, arg[0], arg[1]);
            break;
        case F_or:
            ARI (||, arg[0], arg[1]);
            break;
        default:
            returnnode = arg_node;
        }
    } else
        returnnode = arg_node;
    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : ArrayPrf
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
ArrayPrf (int res_int, node **arg, node *arg_node, node *arg_info)
{
    node *returnnode;
    int swap, i, is_array;
    node *expr[3];
    node *expr_arg[3];
    shpseg *shape;

    DBUG_ENTER ("ArrayPrf");
    switch (arg_node->info.prf) {
    case F_add:
    case F_sub:
    case F_mul:
    case F_div:
    case F_and:
    case F_or:
    case F_le:
    case F_lt:
    case F_eq:
    case F_ge:
    case F_gt:
    case F_neq:
        if (!arg_info->nnode) {
            returnnode = arg_node;
            break;
        }
        if (arg[0]->nodetype != N_array) {
            swap = 1;
            returnnode = arg[1];
            arg[1] = arg[0];
            arg[0] = returnnode;
        } else {
            swap = 0;
            returnnode = arg[0];
        }
        expr[0] = arg[0]->node[0];
        expr_arg[0] = expr[0]->node[0];
        is_array = (arg[1]->nodetype == N_array);
        if (is_array) {
            expr[1] = arg[1]->node[0];
            expr_arg[1] = expr[1]->node[0];
        } else
            expr_arg[1] = arg[1];
        do {
            expr_arg[0] = SkalarPrf (res_int, expr_arg, 2, swap, arg_node, arg_info);
            i = (expr[0]->node[1] != NULL);
            if (i) {
                expr[0] = expr[0]->node[1];
                expr_arg[0] = expr[0]->node[0];
                if (is_array) {
                    expr[1] = expr[1]->node[1];
                    expr_arg[1] = expr[1]->node[0];
                }
            }
        } while (i);
        break;
    case F_shape:
        switch (arg[0]->nodetype) {
        case (N_array):
            i = 1;
            expr[0] = arg[0]->node[0];
            do {
                if (expr[0]->node[1] != NULL)
                    i++;
                expr[0] = expr[0]->node[1];
            } while (expr[0] != NULL);
            arg[0]->node[0]->node[0]->info.cint = i;
            arg[0]->node[0]->node[0]->nodetype = N_num;
            if (NULL != arg[0]->node[0]->node[1])
                FreeTree (arg[0]->node[0]->node[1]);
            arg[0]->node[0]->nnode = 0;
            arg[0]->node[0]->node[1] = 0;
            free (arg_node);
            returnnode = arg[0];
            break;
        case N_id:
            returnnode = MakeNode (N_array);
            returnnode->nnode = 1;
            returnnode->node[0] = MakeNode (N_exprs);
            expr[0] = returnnode->node[0];
            shape = arg[0]->node[0]->info.types->shpseg;
            expr[0]->node[0] = MakeNode (N_num);
            expr[0]->nnode = 1;
            expr[0]->node[0]->info.cint = shape->shp[0];
            for (i = 1; i < arg_info->info.types->shpseg->shp[0]; i++) {
                expr[0]->nnode++;
                expr[0]->node[1] = MakeNode (N_exprs);
                expr[0] = expr[0]->node[1];
                expr[0]->nnode = 1;
                expr[0]->node[0] = MakeNode (N_num);
                expr[0]->node[0]->info.cint = shape->shp[i];
            }
            break;
        default:
            returnnode = arg_node;
            break;
        }
        break; /* shape */
    case F_dim:
        switch (arg[0]->nodetype) {
        case N_array:
            arg_node->info.cint = 1;
            FreeTree (arg[0]);
            arg_node->nodetype = N_num;
            arg_node->nnode = 0;
            arg_node->node[0] = NULL;
            returnnode = arg_node;
            break;
        case N_id:
            arg_node->info.cint = arg[0]->node[0]->info.types->dim;
            FreeTree (arg[0]);
            arg_node->nodetype = N_num;
            arg_node->nnode = 0;
            arg_node->node[0] = NULL;
            returnnode = arg_node;
            break;
        case N_cast:
        default:
            returnnode = arg_node;
            break;
        }
        break; /* dim */
    default:
        returnnode = arg_node;
    }
    DBUG_RETURN (returnnode);
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
    node *returnnode = NULL;
    node *arg[3];
    short res_int;
    int i, arg_no;
    prf prf;
    int spezial_prf;

    DBUG_ENTER ("CFprf");
    prf = arg_node->info.prf;
    spezial_prf = ((prf == F_dim) || (prf == F_shape) || (prf == F_psi) || (prf == F_take)
                   || (prf == F_drop) || (prf == F_cat) || (prf == F_rotate));
    if (!spezial_prf)
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    for (i = 0; i < 3; i++)
        arg[i] = NULL;
    if (arg_node->node[0] != NULL) {
        arg[0] = arg_node->node[0]->node[0];
        if (arg_node->node[0]->node[1] != NULL) {
            arg[1] = arg_node->node[0]->node[1]->node[0];
            if (arg_node->node[0]->node[1]->node[1] != NULL)
                arg[2] = arg_node->node[0]->node[1]->node[1]->node[0];
        }
    }
    arg_no = 3;
    while ((arg[arg_no - 1] == NULL) && (arg_no != 1))
        arg_no--;

    if (arg_info->info.types == NULL) {
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        returnnode = SkalarPrf (-1, arg, arg_no, 0, arg_node, arg_info);
    } else {
        res_int = ((arg_info->info.types->simpletype == T_int)
                   || (arg_info->info.types->simpletype == T_bool));
        if ((arg_info->info.types->dim == 0) && (!spezial_prf)) {
            returnnode = SkalarPrf (res_int, arg, arg_no, 0, arg_node, arg_info);
        } else
            returnnode = ArrayPrf (res_int, arg, arg_node, arg_info);
    }
    DBUG_RETURN (returnnode);
}
