/*
 *
 * $Log$
 * Revision 1.3  1995/02/22 18:16:34  asi
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

stack *
PushVL (stack *VLStack, long NumVar)
{
    DBUG_ENTER ("PushVL");
    VLStack->stack[++VLStack->tos].varlist = (node **)MAlloc (sizeof (node *) * NumVar);
    VLStack->stack[VLStack->tos].vl_len = NumVar;
    if (VLStack->tos == VLStack->st_len) {
        if (NULL != (VLStack->stack = realloc (VLStack->stack, 2 * VLStack->st_len)))
            Error ("out of memory", 1);
        VLStack->st_len *= 2;
    }
    DBUG_RETURN (VLStack);
}

stack *
PushDupVL (stack *VLStack)
{
    int NumVar, i;

    DBUG_ENTER ("PushDupVL");
    NumVar = VLStack->stack[VLStack->tos].vl_len;
    VLStack->stack[++VLStack->tos].varlist = (node **)MAlloc (sizeof (node *) * NumVar);
    for (i = 0; i < NumVar; i++)
        VLStack->stack[VLStack->tos].varlist[i]
          = VLStack->stack[VLStack->tos - 1].varlist[i];
    VLStack->stack[VLStack->tos].vl_len = VLStack->stack[VLStack->tos - 1].vl_len;
    DBUG_RETURN (VLStack);
}

stack *
PopVL (stack *VLStack)
{
    DBUG_ENTER ("PopVL");
    free (VLStack->stack[VLStack->tos--].varlist);
    DBUG_RETURN (VLStack);
}

/*
 *
 *  functionname  : ConstantFolding
 *  arguments     : 1) ptr to root of the syntaxtree
 *                  R) ptr to root of the optimized syntaxtree
 *  description   : initiates constant folding for the intermediate sac-code
 *  global vars   : syntax_tree, cf_tab
 *  internal funs : ---
 *  external funs : Trav, MakeNode
 *  macros        : DBUG...
 *
 *  remarks       :
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
CFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFfundef");
    if (arg_node->node[0] != NULL) {
        arg_info->lineno = arg_node->lineno;
        cf_stack = PushVL (cf_stack, arg_info->lineno);
        arg_info->mask[0] = GenMask ();
        arg_info->mask[1] = GenMask ();

        arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* functionbody */

        MinusMask (arg_node->mask[0], arg_info->mask[0]);
        MinusMask (arg_node->mask[1], arg_info->mask[1]);
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
CFid (node *arg_node, node *arg_info)
{
    node *return_node;
    node *value;

    DBUG_ENTER ("CFid");
    value = cf_stack->stack[cf_stack->tos].varlist[arg_node->node[0]->lineno];
    if (value != NULL) {
        switch (value->nodetype) {
        case (N_num):
        case (N_float):
        case (N_bool):
            return_node = DupConst (value);
            INC_VAR (arg_info->mask[1], arg_node->node[0]->lineno);
            break;
        case (N_array):
            return_node = DupArray (value);
            INC_VAR (arg_info->mask[1], arg_node->node[0]->lineno);
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
CFassign (node *arg_node, node *arg_info)
{
    long *oldmask[2];
    int i;
    node *tmp;

    DBUG_ENTER ("CFassign");
    oldmask[0] = arg_info->mask[0];
    oldmask[1] = arg_info->mask[1];
    arg_info->mask[0] = GenMask ();
    arg_info->mask[1] = GenMask ();
    arg_info->node[0] = arg_node;

    arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* Trav instruction */

    arg_info->node[0] = NULL;

    switch (arg_node->node[0]->nodetype) {
    case N_empty:
    case N_assign:
        tmp = arg_node;
        if (arg_node->node[0]->nodetype != N_empty)
            arg_node = AppendNodeChain (1, arg_node->node[0], arg_node->node[1]);
        else {
            free (arg_node->node[0]);
            arg_node = arg_node->node[1];
        }
        free (tmp);
        PlusMask (arg_info->mask[0], oldmask[0]);
        PlusMask (arg_info->mask[1], oldmask[1]);
        free (oldmask[0]);
        free (oldmask[1]);

        arg_node = Trav (arg_node, arg_info); /* Trav next assign */

        break;
    case N_cond:
    case N_do:
    case N_while:
        MinusMask (arg_node->mask[0], arg_info->mask[0]);
        MinusMask (arg_node->mask[1], arg_info->mask[1]);
        for (i = 0; i < TOS.vl_len; i++) {
            if (ReadMask (arg_node->mask[0], i) != 0)
                VAR (i) = NULL;
        }
        PlusMask (arg_info->mask[0], oldmask[0]);
        PlusMask (arg_info->mask[1], oldmask[1]);
        free (oldmask[0]);
        free (oldmask[1]);
        if (NULL != arg_node->node[1]) {
            arg_node->node[1] = Trav (arg_node->node[1], arg_info); /* Trav next assign */
        }
        break;
    default:
        MinusMask (arg_node->mask[1], arg_info->mask[1]);
        PlusMask (arg_info->mask[1], oldmask[1]);
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
CFlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CFlet");
    arg_info->nnode = 1;
    arg_info->info.types = arg_node->info.ids->node->info.types;
    arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* Trav expression */
    arg_info->info.types = NULL;
    if (arg_info->nnode == 1)
        VAR (arg_node->info.ids->node->lineno) = arg_node->node[0];
    else
        VAR (arg_node->info.ids->node->lineno) = NULL;
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFwhile
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
CFwhile (node *arg_node, node *arg_info)
{
    int i;
    node *a_node;

    DBUG_ENTER ("CFwhile");
    cf_stack = PushDupVL (cf_stack);
    a_node = arg_info->node[0];
    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_info->node[0]->mask[0], i) != 0)
            VAR (i) = NULL;
    }

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    MinusMask (arg_node->mask[1], arg_info->mask[1]);

    switch (arg_node->node[0]->nodetype) {
    case N_bool:
        if (arg_node->node[0]->info.cint) {
            WARN1 (("WARNING in line %d: endless loop expected\n", a_node->lineno));
        } else {
            PlusMask (arg_info->mask[0], a_node->mask[0]);
            PlusMask (arg_info->mask[1], a_node->mask[1]);
            FreeTree (arg_node);
            arg_node = MakeNode (N_empty);
            break;
        }
    default:
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);
        break;
    }
    cf_stack = PopVL (cf_stack);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFdo
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
CFdo (node *arg_node, node *arg_info)
{
    int i;
    node *a_node, *tmp;

    DBUG_ENTER ("CFdo");
    cf_stack = PushDupVL (cf_stack);
    a_node = arg_info->node[0];
    for (i = 0; i < TOS.vl_len; i++) {
        if (ReadMask (arg_info->node[0]->mask[0], i) != 0)
            VAR (i) = NULL;
    }

    arg_node->node[1] = Trav (arg_node->node[1], arg_info);

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    MinusMask (arg_node->mask[1], arg_info->mask[1]);

    if ((arg_node->node[0]->info.cint) && (arg_node->node[0]->nodetype == N_bool)) {
        WARN1 (("WARNING in line %d: endless loop expected\n", a_node->lineno));
    } else {
        tmp = arg_node;
        arg_node = arg_node->node[1]->node[0];
        tmp->node[1]->nnode = 0;
        FreeTree (tmp);
    }
    cf_stack = PopVL (cf_stack);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CFcond
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
CFcond (node *arg_node, node *arg_info)
{
    long *oldmask[2];
    node *returnnode;

    DBUG_ENTER ("CFcond");
    returnnode = arg_node;
    cf_stack = PushDupVL (cf_stack);
    oldmask[0] = arg_info->mask[0];
    oldmask[1] = arg_info->mask[1];
    arg_info->mask[0] = GenMask ();
    arg_info->mask[1] = GenMask ();

    arg_node->node[0] = Trav (arg_node->node[0], arg_info);

    MinusMask (arg_node->mask[0], arg_info->mask[0]);
    MinusMask (arg_node->mask[1], arg_info->mask[1]);
    PlusMask (oldmask[0], arg_info->mask[0]);
    PlusMask (oldmask[1], arg_info->mask[1]);
    ClearMask (arg_info->mask[0]);
    ClearMask (arg_info->mask[1]);

    if (arg_node->node[0]->nodetype == N_bool) {
        if (arg_node->node[0]->info.cint != 0) {
            PlusMask (oldmask[0], arg_node->node[2]->mask[0]);
            PlusMask (oldmask[1], arg_node->node[2]->mask[1]);
            FreeTree (arg_node->node[2]);
            returnnode = arg_node->node[1]->node[0];
            free (arg_node->node[1]);
        } else {
            PlusMask (oldmask[0], arg_node->node[1]->mask[0]);
            PlusMask (oldmask[1], arg_node->node[1]->mask[1]);
            FreeTree (arg_node->node[1]);
            returnnode = arg_node->node[2]->node[0];
            free (arg_node->node[2]);
        }
    } else {
        arg_node->node[1] = Trav (arg_node->node[1], arg_info);

        MinusMask (arg_node->node[1]->mask[0], arg_info->mask[0]);
        MinusMask (arg_node->node[1]->mask[1], arg_info->mask[1]);
        PlusMask (oldmask[0], arg_info->mask[0]);
        PlusMask (oldmask[1], arg_info->mask[1]);
        ClearMask (arg_info->mask[0]);
        ClearMask (arg_info->mask[1]);
        cf_stack = PopVL (cf_stack);
        cf_stack = PushDupVL (cf_stack);

        arg_node->node[2] = Trav (arg_node->node[2], arg_info);

        MinusMask (arg_node->node[2]->mask[0], arg_info->mask[0]);
        MinusMask (arg_node->node[2]->mask[1], arg_info->mask[1]);
        PlusMask (oldmask[0], arg_info->mask[0]);
        PlusMask (oldmask[1], arg_info->mask[1]);
    }
    free (arg_info->mask[0]);
    free (arg_info->mask[1]);
    arg_info->mask[0] = oldmask[0];
    arg_info->mask[1] = oldmask[1];
    cf_stack = PopVL (cf_stack);
    DBUG_RETURN (returnnode);
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
    switch (arg_no) {
    case 1:
        switch (arg_node->info.prf) {
        case F_not:
            arg[0]->info.cint = !arg[0]->info.cint;
            returnnode = arg[0];
            break;
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
            break;
        default:
            returnnode = arg_node;
        }
        break;
    case 2:
        if (arg_info->nnode == 1) {
            switch (arg_node->info.prf) {
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
        break;
    default:
        returnnode = arg_node;
    }
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
        break;
    default:
        returnnode = arg_node;
    }
    DBUG_RETURN (returnnode);
}

/*
 *
 *  functionname  : CFprf
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
CFprf (node *arg_node, node *arg_info)
{
    node *returnnode = NULL;
    node *arg[3];
    short res_int;
    int i, arg_no;

    DBUG_ENTER ("CFprf");
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
        res_int = -1;
        returnnode = SkalarPrf (res_int, arg, arg_no, 0, arg_node, arg_info);
    } else {
        res_int = ((arg_info->info.types->simpletype == T_int)
                   || (arg_info->info.types->simpletype == T_bool));
        if (arg_info->info.types->dim == 0)
            returnnode = SkalarPrf (res_int, arg, arg_no, 0, arg_node, arg_info);
        else
            returnnode = ArrayPrf (res_int, arg, arg_node, arg_info);
    }
    DBUG_RETURN (returnnode);
}
