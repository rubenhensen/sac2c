/*
 *
 * $Log$
 * Revision 1.1  1995/03/29 12:38:10  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include "print.h"
#include "dbug.h"
#include "internal_lib.h"
#include "access_macros.h"
#include "traverse.h"
#include "compile.h"
#include "convert.h"
#include "refcount.h"  /* to use IsArray */
#include "typecheck.h" /* to use LookupType */

extern int malloc_verify ();
extern int malloc_debug (int level);

#define MAKE_ICM_ARG(var, new_node)                                                      \
    var = MakeNode (N_exprs);                                                            \
    var->node[0] = new_node;                                                             \
    var->nnode = 1

#define MAKE_NEXT_ICM_ARG(prev, tmp, new_node)                                           \
    MAKE_ICM_ARG (tmp, new_node);                                                        \
    prev->node[1] = tmp;                                                                 \
    prev->nnode = 2;                                                                     \
    prev = tmp

#define MAKE_ICM_NAME(var, name)                                                         \
    var->info.fun_name.id = name;                                                        \
    var->info.fun_name.id_mod = NULL

#define MAKE_ICM(assign)                                                                 \
    assign = MakeNode (N_assign);                                                        \
    assign->node[0] = MakeNode (N_icm);                                                  \
    assign->nnode = 1

#define MAKE_PREV_ICM(assign, new_assign)                                                \
    new_assign = MakeNode (N_assign);                                                    \
    new_assign->node[0] = MakeNode (N_icm);                                              \
    new_assign->nnode = 1;                                                               \
    new_assign->node[1] = assign;                                                        \
    new_assign->nnode = 2;                                                               \
    assign = new_assign

#define FREE(a) free (a)
#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg)                                                               \
        FREE (a->shpseg);                                                                \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (a->TYPES);                                                                \
    FREE (a)

/*
 *
 *  functionname  : Compile
 *  arguments     : 1) syntax tree
 *  description   : starts compilation  and initializes act_tab
 *
 *  global vars   : act_tab
 *  internal funs :
 *  external funs : Trav
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */
node *
Compile (node *arg_node)
{
    node *info;
    DBUG_ENTER ("Compile");
    /*
       malloc_debug(2);
    */
    act_tab = comp_tab; /* set new function-table for traverse */
    info = MakeNode (N_info);
    if (N_modul == arg_node->nodetype) {
        if (NULL != arg_node->node[1])
            /* traverse typedefs */
            arg_node->node[1] = Trav (arg_node->node[1], NULL);
        if (NULL != arg_node->node[2])
            /* traverse functions */
            arg_node->node[2] = Trav (arg_node->node[2], NULL);
    } else {
        DBUG_ASSERT ((N_fundef == arg_node->nodetype), "wrong node");
        arg_node = Trav (arg_node, info);
    }
    /*
       malloc_verify();

       malloc_debug(0);
    */
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompVardec
 *  arguments     : 1) N_vardec node
 *                  2) NULL
 *  description   : transforms N_vardec to N_icm node if it is the declaration
 *                  of an array
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : ----
 *
 */
node *
CompVardec (node *arg_node, node *arg_info)
{
    node *exprs, *tmp, *type_node, *assign;
    int i, dim;

    DBUG_ENTER ("CompVardec");

    if (1 == IsArray (arg_node)) {
        MAKE_ICM (assign);
        MAKE_ICM_NAME (assign->node[0], "ND_KS_DECL_ARRAY");

        /* store basic_type */
        MAKE_ICM_ARG (exprs, MakeNode (N_id));
        if (NULL != arg_node->node[0]) {
            assign->node[1] = arg_node->node[0];
            assign->node[0]->node[0] = exprs;
            assign->nnode = 2;
        } else
            assign->node[0]->node[0] = exprs;

        if (T_user != arg_node->SIMPLETYPE) {
            exprs->node[0]->IDS = MakeIds (type_string[arg_node->SIMPLETYPE]);
            arg_node->node[1] = exprs;

            dim = arg_node->DIM;
        } else {
            type_node = LookupType (arg_node->NAME, arg_node->NAME_MOD, 042);
            /* 042 is only a dummy argument */

            /* store basic_type */
            exprs->node[0]->info.ids = MakeIds (type_string[type_node->SIMPLETYPE]);
            arg_node->node[1] = exprs;

            dim = type_node->DIM + arg_node->DIM;
        }

        /* store name of variable */
        MAKE_NEXT_ICM_ARG (exprs, tmp, MakeNode (N_id));
        tmp->node[0]->info.ids = MakeIds (arg_node->ID);

        /* store dimension */
        MAKE_NEXT_ICM_ARG (exprs, tmp, MakeNode (N_num));
        tmp->node[0]->info.cint = dim;

        /* store shape infomation */
        for (i = 0; i < arg_node->DIM; i++) {
            MAKE_NEXT_ICM_ARG (exprs, tmp, MakeNode (N_num));
            tmp->node[0]->info.cint = arg_node->SHP[i];
        }
        if (T_user == arg_node->SIMPLETYPE)
            for (i = 0; i < type_node->DIM; i++) {
                MAKE_NEXT_ICM_ARG (exprs, tmp, MakeNode (N_num));
                tmp->node[0]->info.cint = arg_node->SHP[i];
            }

        /* now transform current node to one of type N_icm */
        FREE_VARDEC (arg_node);
        arg_node = assign;
        if (NULL != arg_node->node[1])
            arg_node->node[1] = Trav (arg_node->node[1], NULL);

    } else
      /* traverse next N_vardec node if any */
      if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], NULL);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompPrf
 *  arguments     : 1) N_prf node
 *                  2) NULL
 *  description   : transforms N_prf node to N_icm nodes if prf works on
 *                  arrays
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to next assign_node
 *
 */
node *
CompPrf (node *arg_node, node *arg_info)
{
    node *icm_node = NULL, *new_assign = NULL, *arg1, *arg2, *tmp, *res, *assign,
         *icm_arg, *type_node;
    simpletype s_type;

    DBUG_ENTER ("CompPrf");

    /* NOTE :  F_neq should be the last "function enumerator" that hasn't
     *         arrays as arguments.
     */
    if (arg_node->info.prf > F_neq)
        switch (arg_node->info.prf) {
        case F_add_SxA:
        case F_div_SxA:
        case F_sub_SxA:
        case F_mul_SxA: {

            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            res = MakeNode (N_id);
            res->info.ids = MakeIds (arg_info->info.ids->id);
            if (N_id == arg2->nodetype) {
                /* create ND_INC_RC */
                MAKE_ICM (assign);
                icm_node = assign->node[0];
                MAKE_ICM_NAME (icm_node, "ND_INC_RC");
                MAKE_ICM_ARG (icm_node->node[0], res);
                icm_node->nnode = 1;
                if (NULL != arg_info->node[0]->node[1]) {
                    /* pointer to next assign node */
                    assign->node[1] = arg_info->node[0]->node[1];
                    assign->nnode = 2;
                }

                /* create ND_DEC_RC */
                MAKE_PREV_ICM (assign, new_assign);
                icm_node = new_assign->node[0];
                MAKE_ICM_NAME (icm_node, "ND_DEC_RC");
                MAKE_ICM_ARG (icm_node->node[0], arg2);
                icm_node->nnode = 1;

                /* create ND_BINOP_SxA_A */
                MAKE_PREV_ICM (assign, new_assign);
                icm_node = new_assign->node[0];
                MAKE_ICM_NAME (icm_node, "ND_BINOP_SxA_A");
                MAKE_ICM_ARG (icm_node->node[0], MakeNode (N_id));
                icm_arg = icm_node->node[0];
                icm_arg->node[0]->IDS = MakeIds (prf_string[arg_node->info.prf]);
                icm_node->nnode = 1;
                MAKE_NEXT_ICM_ARG (icm_arg, tmp, arg1);
                MAKE_NEXT_ICM_ARG (icm_arg, tmp, arg2);
                MAKE_NEXT_ICM_ARG (icm_arg, tmp, res);

                /* create ND_ALLOC_ARRAY */
                /* compute basic_type of result */
                if (T_user == arg_info->info.ids->node->SIMPLETYPE) {
                    type_node = LookupType (arg_info->NAME, arg_info->NAME_MOD, 042);
                    s_type = type_node->SIMPLETYPE;
                } else
                    s_type = arg_info->info.ids->node->SIMPLETYPE;

                if (1 >= arg2->refcnt) {
                    /* possible reuse of argument 2, so there will be another
                     * N_icm in front of this.
                     */
                    MAKE_PREV_ICM (assign, new_assign);
                    icm_node = new_assign->node[0];
                } else {
                    /* last N_icm to build */
                    icm_node = arg_info->node[1]; /* take N_let as N_icm */
                    icm_node->nodetype = N_icm;
                    FREE (arg_info->node[1]->IDS);       /* free ids of N_let */
                    FREE (arg_info->node[1]->node[1]);   /* free N_prf node */
                    arg_info->node[0]->node[1] = assign; /* assign will be the next
                                                          * N_assign of N_assign of
                                                          * previous N_let
                                                          */
                }
                MAKE_ICM_NAME (icm_node, "ND_ALLOC_ARRAY");
                MAKE_ICM_ARG (icm_node->node[0], MakeNode (N_id));
                icm_arg = icm_node->node[0];
                icm_arg->node[0]->IDS = MakeIds (type_string[s_type]);
                if (1 < arg2->refcnt)
                    arg_node = icm_arg; /* set new arg_node */
                icm_node->nnode = 1;
                MAKE_NEXT_ICM_ARG (icm_arg, tmp, res);

                if (1 >= arg2->refcnt) {
                    /* create ND_CHECK_REUSE */
                    /* possible reuse of argument 2
                     * last N_icm to build
                     */
                    icm_node = arg_info->node[1]; /* take previous N_let as N_icm */
                    icm_node->nodetype = N_icm;
                    FREE (arg_info->node[1]->IDS);     /* free ids of N_let */
                    FREE (arg_info->node[1]->node[1]); /* free N_prf node */
                    MAKE_ICM_NAME (icm_node, "ND_CHECK_REUSE");
                    MAKE_ICM_ARG (icm_node->node[0], arg2);
                    icm_arg = icm_node->node[0];
                    arg_node = icm_arg; /* set new arg_node */
                    MAKE_NEXT_ICM_ARG (icm_arg, tmp, res);
                    arg_info->node[0]->node[1] = assign; /* assign will be the next
                                                          * N_assign of N_assign of
                                                          * previous N_let
                                                          */
                }
            }
            break;
        }
        default:
            /*   DBUG_ASSERT(0,"wrong prf"); */
            break;
        }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompAssign
 *  arguments     : 1) N_assign node
 *                  2) NULL
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to next assign_node
 *
 */
node *
CompAssign (node *arg_node, node *arg_info)
{
    node *old_next_assign;

    DBUG_ENTER ("CompAssign");
    if (2 == arg_node->nnode) {
        arg_info->node[0] = arg_node;
        old_next_assign = arg_node->node[1];
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
        arg_info->node[0] = NULL;
        old_next_assign = Trav (old_next_assign, arg_info);
    } else
        arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompLet
 *  arguments     : 1) N_Let node
 *                  2) NULL
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to next assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompLet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompLet");

    arg_info->node[1] = arg_node;
    arg_info->IDS = arg_node->IDS;
    arg_node->node[0] = Trav (arg_node->node[0], arg_info);
    arg_info->node[1] = NULL;

    DBUG_RETURN (arg_node);
}
