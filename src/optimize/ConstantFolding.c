/*
 *
 * $Log$
 * Revision 1.2  1995/02/14 09:59:56  asi
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
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"

#include "optimize.h"
#include "ConstantFolding.h"

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
    arg_node = Trav (arg_node, info_node);
    free (info_node);
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
    arg_info->info.types = arg_node->info.ids->node->info.types;
    arg_node->node[0] = Trav (arg_node->node[0], arg_info); /* Trav expression */
    arg_info->info.types = NULL;
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CleanupCF
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
CleanupCF (int res_int, node **arg, node *arg_node, node *arg_info)
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
    free (arg[1]);
    free (arg_node);
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
SkalarPrf (int res_int, node **arg, node *arg_node, node *arg_info)
{
    node *returnnode = NULL;

#define SELARG(n) ((n->nodetype == N_num) ? n->info.cint : n->info.cfloat)
#define ARI(op, a1, a2)                                                                  \
    {                                                                                    \
        if (res_int)                                                                     \
            a1->info.cint = a1->info.cint op a2->info.cint;                              \
        else                                                                             \
            a1->info.cfloat = SELARG (a1) op SELARG (a2);                                \
        returnnode = CleanupCF (res_int, arg, arg_node, arg_info);                       \
    }

    DBUG_ENTER ("SkalarPrf");
    if ((arg[0]->nodetype == N_num || arg[0]->nodetype == N_float
         || arg[0]->nodetype == N_bool)
        && (arg[1]->nodetype == N_num || arg[1]->nodetype == N_float
            || arg[1]->nodetype == N_bool)) {
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
            ARI (/, arg[0], arg[1]);
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
        case F_not:
            arg[0]->info.cint = !arg[0]->info.cint;
            returnnode = CleanupCF (res_int, arg, arg_node, arg_info);
            break;
        default:
            break;
        }
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
ArrayPrf ()
{
    node *returnnode = NULL;

    DBUG_ENTER ("ArrayPrf");
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
    node *arg_t;
    short res_int;
    int i;

    DBUG_ENTER ("CFprf");
    i = 0;
    arg_t = arg_node->node[0];
    arg[i] = arg_t->node[0];
    while (arg_t->node[1] != NULL) {
        arg_t = arg_t->node[1];
        arg[i + 1] = arg_t->node[0];
        i++;
    }
    if (arg_info->info.types == NULL) {
        res_int = -1;
        returnnode = SkalarPrf (res_int, arg, arg_node, arg_info);
    } else {
        res_int = ((arg_info->info.types->simpletype == T_int)
                   || (arg_info->info.types->simpletype == T_bool));
        if (arg_info->info.types->dim == 0)
            returnnode = SkalarPrf (res_int, arg, arg_node, arg_info);
        else
            returnnode = ArrayPrf ();
    }
    if (returnnode == NULL)
        returnnode = arg_node;
    DBUG_RETURN (returnnode);
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
    DBUG_ENTER ("CFid");

    DBUG_RETURN (arg_node);
}
