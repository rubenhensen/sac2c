/*
 *
 * $Log$
 * Revision 1.3  1995/04/03 13:27:29  hw
 * bug fixed in creation of N_icm ND_CREATE_CONST_ARRAY.
 * primitive functions F_...SxA, F_..AxS, F_take can have constant
 *    arrays as argument
 *
 * Revision 1.2  1995/03/31  15:49:16  hw
 * added macros to create and insert 'N_icm'
 * added CompArray, CompId, CompReturn
 * changed CompPrf (now more functions will be compiled)
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
 *
 *
 */

#include <stdlib.h>
#include "print.h"
#include "dbug.h"
#include "my_debug.h"
#include "internal_lib.h"
#include "access_macros.h"
#include "traverse.h"
#include "compile.h"
#include "convert.h"
#include "refcount.h"  /* to use IsArray */
#include "typecheck.h" /* to use LookupType */
#include "free.h"

extern int malloc_verify ();
extern int malloc_debug (int level);

#define MAKE_ICM_ARG(var, new_node)                                                      \
    var = MakeNode (N_exprs);                                                            \
    var->node[0] = new_node;                                                             \
    var->nnode = 1

#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
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

#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    MAKE_ICM (assign);                                                                   \
    MAKE_ICM_NAME (assign->node[0], str);                                                \
    MAKE_ICM_ARG (assign->node[0]->node[0], arg);                                        \
    icm_arg = assign->node[0]->node[0];                                                  \
    assign->node[0]->nnode = 1

#define CREATE_2_ARY_ICM(assign, str, arg1, arg2)                                        \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define CREATE_3_ARY_ICM(assign, str, arg1, arg2, arg3)                                  \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3)

#define CREATE_4_ARY_ICM(assign, str, arg1, arg2, arg3, arg4)                            \
    CREATE_1_ARY_ICM (assign, str, arg1);                                                \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg4)

#define BIN_ICM_REUSE(reuse, str, arg1, arg2)                                            \
    reuse->nodetype = N_icm;                                                             \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define SET_VARS_FOR_MORE_ICMS                                                           \
    first_assign = arg_info->node[0];                                                    \
    old_arg_node = arg_node;                                                             \
    last_assign = arg_info->node[0]->node[1];                                            \
    arg_node = arg_info->node[1]->node[0]

#define APPEND_ASSIGNS(first, next)                                                      \
    first->node[1] = next;                                                               \
    first->nnode = 2;                                                                    \
    first = next

#define INSERT_ASSIGN                                                                    \
    if (NULL != last_assign) {                                                           \
        APPEND_ASSIGNS (first_assign, last_assign);                                      \
    }

#define CHECK_REUSE__ALLOC_ARRAY_ND(test)                                                \
    if (1 >= test->refcnt) { /* create ND_CHECK_REUSE  */                                \
        BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE", test, res);                  \
        first_assign = arg_info->node[0];                                                \
        old_arg_node = arg_node;                                                         \
        arg_node = arg_info->node[1]->node[0];                                           \
        /* create ND_ALLOC_ARRAY */                                                      \
        CREATE_2_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res);             \
        APPEND_ASSIGNS (first_assign, next_assign);                                      \
    } else { /* create ND_ALLOC_ARRAY */                                                 \
        BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);          \
        first_assign = arg_info->node[0];                                                \
        old_arg_node = arg_node;                                                         \
        arg_node = arg_info->node[1]->node[0];                                           \
    }

#define DEC_RC_ND(array, num_node) /* create ND_DEC_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define DEC_RC_FREE_ND(array, num_node) /* create ND_DEC_RC */                           \
    CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC_FREE", array, num_node);                   \
    APPEND_ASSIGNS (first_assign, next_assign)

#define INC_RC_ND(array, num_node) /* create ND_INC_RC */                                \
    CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", array, num_node);                        \
    APPEND_ASSIGNS (first_assign, next_assign)

#define GET_DIM(result, vardec_node)                                                     \
    if (T_user == vardec_node->SIMPLETYPE) {                                             \
        result = LookupType (vardec_node->NAME, vardec_node->NAME_MOD, 042)->DIM;        \
        result += vardec_node->DIM;                                                      \
    } else                                                                               \
        result = vardec_node->DIM

#define CONST_ARRAY(array)                                                               \
    array_is_const = 1;                                                                  \
    old_arg_node = arg_node;                                                             \
    tmp = array->node[0];                                                                \
    while (NULL != tmp) {                                                                \
        n_elems += 1;                                                                    \
        tmp = tmp->node[1];                                                              \
    }                                                                                    \
    MAKENODE_NUM (n_node, n_elems);                                                      \
    MAKENODE_NUM (n_node1, 1);                                                           \
    MAKENODE_ID (tmp_array, "__TMP");                                                    \
    arg_info->node[1]->nodetype = N_block; /* reuse previous N_let*/                     \
    CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node, tmp_array,         \
                      n_node1, n_node);                                                  \
    arg_node = first_assign;                                                             \
    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY", type_id_node, tmp_array,     \
                      n_node);                                                           \
    icm_arg->node[1] = array->node[0];                                                   \
    icm_arg->nnode = 2;                                                                  \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    CREATE_2_ARY_ICM (next_assign, "ND_REUSE", res, tmp_array);                          \
    APPEND_ASSIGNS (first_assign, next_assign);                                          \
    array = tmp_array /* set array to __TMP */

#define GET_BASIC_TYPE(res, vardec_node)                                                 \
    if (T_user == vardec_node->SIMPLETYPE)                                               \
        res = LookupType (vardec_node->NAME, vardec_node->NAME_MOD, 042)->SIMPLETYPE;    \
    else                                                                                 \
        res = vardec_node->SIMPLETYPE

#define MAKENODE_NUM(no, nr)                                                             \
    no = MakeNode (N_num);                                                               \
    no->info.cint = nr

#define MAKENODE_ID(no, str)                                                             \
    no = MakeNode (N_id);                                                                \
    no->IDS = MakeIds (str)

#define MAKENODE_ID_REUSE_IDS(no, Ids)                                                   \
    no = MakeNode (N_id);                                                                \
    no->IDS = Ids

#define FREE(a)                                                                          \
    DBUG_PRINT ("FREE", (P_FORMAT, a));                                                  \
    free (a)

#define FREE_TYPE(a)                                                                     \
    if (NULL != a->shpseg) {                                                             \
        FREE (a->shpseg);                                                                \
    }                                                                                    \
    FREE (a)

#define FREE_VARDEC(a)                                                                   \
    FREE_TYPE (a->TYPES);                                                                \
    FREE (a)

#define FREE_TREE(a) FreeTree (a)

#define FREE_IDS(a)                                                                      \
    DBUG_PRINT ("FREE", (P_FORMAT, a));                                                  \
    FreeIds (a)

#if 0
#define FREE(a)
#define FREE_TYPE(a)
#define FREE_VARDEC(a)
#define FREE_TREE(a)
#define FREE_IDS(a)
#endif

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

#ifdef MALLOC_TOOL
    malloc_debug (2);
#endif /* MALLOC_TOOL */

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

#ifdef MALLOC_TOOL
    malloc_verify ();
    malloc_debug (0);
#endif /* MALLOC_TOOL */

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
            exprs->node[0]->IDS = MakeIds (type_string[type_node->SIMPLETYPE]);
            arg_node->node[1] = exprs;

            dim = type_node->DIM + arg_node->DIM;
        }

        /* store name of variable */
        MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_id));
        tmp->node[0]->info.ids = MakeIds (arg_node->ID);

        /* store dimension */
        MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_num));
        tmp->node[0]->info.cint = dim;

        /* store shape infomation */
        for (i = 0; i < arg_node->DIM; i++) {
            MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_num));
            tmp->node[0]->info.cint = arg_node->SHP[i];
        }
        if (T_user == arg_node->SIMPLETYPE)
            for (i = 0; i < type_node->DIM; i++) {
                MAKE_NEXT_ICM_ARG (exprs, MakeNode (N_num));
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
 *                  arg_info->node[1] contains pointer to last N_let
 */
node *
CompPrf (node *arg_node, node *arg_info)
{
    node *array, *scalar, *tmp, *res, *res_ref, *n_node, *icm_arg, *exprs, *prf_id_node,
      *type_id_node, *arg1, *arg2, *n_node1, *tmp_array, *first_assign, *next_assign,
      *last_assign, *old_arg_node;
    simpletype s_type;
    int is_SxA = 0, n_elems = 0, is_drop = 0, dim, array_is_const = 0;

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
            is_SxA = 1;
            /* here is NO break missing */
        }
        case F_add_AxS:
        case F_div_AxS:
        case F_sub_AxS:
        case F_mul_AxS: {
            /* store arguments and result (as N_id)  */
            if (0 == is_SxA) {
                array = arg_node->node[0]->node[0];
                scalar = arg_node->node[0]->node[1]->node[0];
            } else {
                array = arg_node->node[0]->node[1]->node[0];
                scalar = arg_node->node[0]->node[0];
            }
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* store prf as N_id */
            MAKENODE_ID (prf_id_node, prf_string[arg_node->info.prf]);

            /* compute basic_type of result */
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

            if (N_id == array->nodetype) {
                last_assign = arg_info->node[0]->node[1];
                if (1 >= array->refcnt) {
                    /* create ND_CHECK_REUSE  */
                    BIN_ICM_REUSE (arg_info->node[1], "ND_CHECK_REUSE", array, res);
                    first_assign = arg_info->node[0];
                    old_arg_node = arg_node;
                    arg_node = arg_info->node[1]->node[0];

                    /* create ND_ALLOC_ARRAY */
                    CREATE_2_ARY_ICM (next_assign, "ND_ALLOC_ARRAY", type_id_node, res);
                    APPEND_ASSIGNS (first_assign, next_assign);
                } else {
                    /* create ND_ALLOC_ARRAY */
                    BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node,
                                   res);
                    first_assign = arg_info->node[0];
                    old_arg_node = arg_node;
                    arg_node = arg_info->node[1]->node[0];
                }
            } else {
                /* array is constant, so make a block , declare a temporary
                 * variable __TMP and create a constant array
                 */

                array_is_const = 1;
                old_arg_node = arg_node;

                /* count number of elements */
                tmp = array->node[0];
                while (NULL != tmp) {
                    n_elems += 1;
                    tmp = tmp->node[1];
                }

                MAKENODE_NUM (n_node, n_elems);
                MAKENODE_NUM (n_node1, 1);
                MAKENODE_ID (tmp_array, "__TMP");

                arg_info->node[1]->nodetype = N_block; /* reuse previous N_let*/
                CREATE_4_ARY_ICM (first_assign, "ND_KS_DECL_ARRAY", type_id_node,
                                  tmp_array, n_node1, n_node);
                arg_node = first_assign;

                /* create const array */
                CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY", type_id_node,
                                  tmp_array, n_node);
                icm_arg->node[1] = array->node[0];
                icm_arg->nnode = 2;
                APPEND_ASSIGNS (first_assign, next_assign);

                /* reuse temporary array __TMP */
                CREATE_2_ARY_ICM (next_assign, "ND_REUSE", res, tmp_array);
                APPEND_ASSIGNS (first_assign, next_assign);

                array = tmp_array; /* set array to __TMP */
            }

            if (0 == is_SxA) {
                /* create ND_BINOP_AxS_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_AxS_A", prf_id_node, array,
                                  scalar, res);
            } else {
                /* create ND_BINOP_SxA_A */
                CREATE_4_ARY_ICM (next_assign, "ND_BINOP_SxA_A", prf_id_node, scalar,
                                  array, res);
            }
            APPEND_ASSIGNS (first_assign, next_assign);

            if (0 == array_is_const) {
                /* create ND_DEC_RC */
                MAKENODE_NUM (n_node, 1);
                CREATE_2_ARY_ICM (next_assign, "ND_DEC_RC", array, n_node);
                APPEND_ASSIGNS (first_assign, next_assign);
            }

            /* create ND_INC_RC */
            CREATE_2_ARY_ICM (next_assign, "ND_INC_RC", res, res_ref);
            APPEND_ASSIGNS (first_assign, next_assign);

            if ((NULL != last_assign) && (0 == array_is_const)) {
                /* pointer to last assign node */
                APPEND_ASSIGNS (first_assign, last_assign);
            }
            FREE (old_arg_node);
            break;
        }
        case F_drop:
            is_drop = 1;
            /* here is NO break missing */

        case F_take: {
            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free arg_info->IDS !!! )
             */
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            /* compute basic_type of result */
            GET_BASIC_TYPE (s_type, arg_info->IDS_NODE);
            MAKENODE_ID (type_id_node, type_string[s_type]);

            /* store refcount of res as N_num */
            MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

            if (N_id == arg2->nodetype) {
                last_assign = arg_info->node[0]->node[1];
                CHECK_REUSE__ALLOC_ARRAY_ND (arg2);

            } else {
                CONST_ARRAY (arg2);
            }

            exprs = arg1->node[0];
            n_elems = 0;
            do {
                n_elems += 1;
                exprs = exprs->node[1];
            } while (NULL != exprs);

            MAKENODE_NUM (n_node, n_elems);
            if (1 == is_drop) {
                CREATE_3_ARY_ICM (next_assign, "ND_KD_DROP_CxA_A", arg2, res, n_node);
            } else {
                CREATE_3_ARY_ICM (next_assign, "ND_KD_TAKE_CxA_A", arg2, res, n_node);
            }
            icm_arg->node[1] = arg1->node[0];
            icm_arg->nnode = 2;
            APPEND_ASSIGNS (first_assign, next_assign);

            MAKENODE_NUM (n_node, 1);
            if (0 == array_is_const) {
                DEC_RC_ND (arg2, n_node);
            }
            INC_RC_ND (res, res_ref);
            if ((NULL != last_assign) && (0 == array_is_const)) {
                /* pointer to last assign node */
                APPEND_ASSIGNS (first_assign, last_assign);
            }
            FREE (old_arg_node);
            break;
        }
        case F_reshape: {
            arg2 = arg_node->node[0]->node[1]->node[0];
            FREE_TREE (arg_node->node[0]->node[0]);
            FREE (arg_node->node[0]);
            if (N_array == arg2->nodetype) {
                arg_node = CompArray (arg2, arg_info);
            } else {
                DBUG_ASSERT ((N_id == arg2->nodetype), "wrong nodetype");
                FREE_IDS (arg2->IDS);
                FREE (arg2);
                FREE (arg_node->node[0]->node[1]);
                arg_node = NULL;
                arg_info->node[1]->nodetype = N_icm;
                arg_info->node[1]->nnode = 0;
                /* don't free  arg_info->node[1]->IDS, because ..->IDS_ID is shared
                   with vardec
                   FREE_IDS(arg_info->node[1]->IDS);
                 */
                MAKE_ICM_NAME (arg_info->node[1], "NOOP");
            }
            break;
        }
        case F_psi: {
            /* store arguments and result (res contains refcount and pointer to
             * vardec ( don't free arg_info->IDS !!! )
             */
            arg1 = arg_node->node[0]->node[0];
            arg2 = arg_node->node[0]->node[1]->node[0];
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            last_assign = arg_info->node[0]->node[1];

            DBUG_ASSERT (N_id == arg2->nodetype, " N_array not implemented yet ");

            /* compute dimension of arg1 */
            if (N_id == arg2->nodetype) {
                GET_DIM (dim, arg2->IDS_NODE);
                MAKENODE_NUM (n_node, dim);
            } else {
                MAKENODE_NUM (n_node, 1);
            }

            if (0 == IsArray (res->IDS_NODE)) {
                if (N_id == arg1->nodetype) {
                    BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_VxA_S", arg2, res);
                    MAKE_NEXT_ICM_ARG (icm_arg, n_node);
                    MAKE_NEXT_ICM_ARG (icm_arg, arg1);
                } else {
                    BIN_ICM_REUSE (arg_info->node[1], "ND_KD_PSI_CxA_S", arg2, res);
                    MAKE_NEXT_ICM_ARG (icm_arg, n_node);
                    icm_arg->node[1] = arg1->node[0];
                    icm_arg->nnode = 2;
                    FREE (arg1);
                }
                first_assign = arg_info->node[0];
                old_arg_node = arg_node;
                arg_node = arg_info->node[1]->node[0];
                MAKENODE_NUM (n_node, 1);
                DEC_RC_FREE_ND (arg2, n_node);
                if (NULL != last_assign) {
                    /* pointer to last assign node */
                    APPEND_ASSIGNS (first_assign, last_assign);
                }
                FREE (old_arg_node);
            } else {
                /* compute basic type */
                GET_BASIC_TYPE (s_type, arg_info->IDS_NODE);
                MAKENODE_ID (type_id_node, type_string[s_type]);

                /* store refcount of res as N_num */
                MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

                CHECK_REUSE__ALLOC_ARRAY_ND (arg2);

                if (N_id == arg1->nodetype) {
                    CREATE_4_ARY_ICM (next_assign, "ND_KD_PSI_VxA_A", arg2, res, n_node,
                                      arg1);
                } else {
                    CREATE_3_ARY_ICM (next_assign, "ND_KD_PSI_CxA_A", arg2, res, n_node);
                    icm_arg->node[1] = arg1->node[0];
                    icm_arg->nnode = 2;
                    FREE (arg1);
                }
                APPEND_ASSIGNS (first_assign, next_assign);

                MAKENODE_NUM (n_node, 1);
                DEC_RC_ND (arg2, n_node);
                INC_RC_ND (res, res_ref);
                if (NULL != last_assign) {
                    /* pointer to last assign node */
                    APPEND_ASSIGNS (first_assign, last_assign);
                }
                FREE (old_arg_node);
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
 *                  2) info node
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

/*
 *
 *  functionname  : CompArray
 *  arguments     : 1) N_arrray node
 *                  2) info node
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        : DBUG...,
 *  remarks       : arg_info->info.ids contains name of assigned variable
 *                  arg_info->node[0] contains pointer to last assign_node
 *                  arg_info->node[1] contains pointer to previous N_let
 *
 */
node *
CompArray (node *arg_node, node *arg_info)
{
    node *tmp, *first_assign, *next_assign, *exprs, *res, *type_id_node, *old_arg_node,
      *icm_arg, *n_node, *res_ref, *last_assign;
    simpletype s_type;
    int n_elems = 0;

    DBUG_ENTER ("CompArray");

    /* store next assign */
    last_assign = arg_info->node[0]->node[1];

    /* compute basic_type of result */
    GET_BASIC_TYPE (s_type, arg_info->IDS_NODE);
    MAKENODE_ID (type_id_node, type_string[s_type]);

    /* store result as N_id */
    MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

    /* store refcount of res as N_num */
    MAKENODE_NUM (res_ref, arg_info->IDS_REFCNT);

    /* create ND_ALLOC_ARRAY */
    BIN_ICM_REUSE (arg_info->node[1], "ND_ALLOC_ARRAY", type_id_node, res);
    first_assign = arg_info->node[0];
    old_arg_node = arg_node;
    arg_node = arg_info->node[1]->node[0];

    /* create ND_CREATE_CONST_ARRAY */
    exprs = old_arg_node->node[0];
    do {
        n_elems += 1;
        exprs = exprs->node[1];
    } while (NULL != exprs);
    MAKENODE_NUM (n_node, n_elems);
    CREATE_3_ARY_ICM (next_assign, "ND_CREATE_CONST_ARRAY", type_id_node, res, n_node);
    icm_arg->node[1] = old_arg_node->node[0];
    icm_arg->nnode = 2;
    APPEND_ASSIGNS (first_assign, next_assign);

    /* create ND_SET_RC */
    CREATE_2_ARY_ICM (next_assign, "ND_SET_RC", res, res_ref);
    APPEND_ASSIGNS (first_assign, next_assign);

    if (NULL != last_assign) {
        /* pointer to last assign node */
        APPEND_ASSIGNS (first_assign, last_assign);
    }
    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompId
 *  arguments     : 1) N_id node
 *                  2) info node
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
CompId (node *arg_node, node *arg_info)
{
    node *first_assign, *next_assign, *tmp, *last_assign, *old_arg_node, *icm_arg, *res,
      *n_node, *icm_node;

    DBUG_ENTER ("CompId");
    if (1 == IsArray (arg_node->IDS_NODE)) {
        if (NULL != arg_info) {
            MAKENODE_ID_REUSE_IDS (res, arg_info->IDS);

            BIN_ICM_REUSE (arg_info->node[1], "ND_KS_ASSIGN_ARRAY", arg_node, res);
            SET_VARS_FOR_MORE_ICMS;
            if (1 >= arg_info->IDS_REFCNT) {
                MAKENODE_NUM (n_node, 1);
                DEC_RC_FREE_ND (res, n_node);
            } else {
                MAKENODE_NUM (n_node, arg_info->IDS_REFCNT - 1);
                INC_RC_ND (res, n_node);
            }
            INSERT_ASSIGN;
        } else {
            icm_node = MakeNode (N_icm);
            MAKE_ICM_NAME (icm_node, "ND_KS_RET_ARRAY");
            MAKE_ICM_ARG (icm_node->node[0], arg_node);
            icm_node->nnode = 1;
            arg_node = icm_node;
        }
    }

    DBUG_RETURN (arg_node);
}
/*
 *
 *  functionname  : CompAp
 *  arguments     : 1) N_Ap node
 *                  2) info node
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
CompAp (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompAp");
    if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], NULL);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompReturn
 *  arguments     : 1) N_return node
 *                  2) info node
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
CompReturn (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompReturn");

    if (NULL != arg_node->node[0])
        arg_node->node[0] = Trav (arg_node->node[0], NULL);

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompWith
 *  arguments     : 1) N_with node
 *                  2) info node
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
CompWith (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CompWith");

    DBUG_RETURN (arg_node);
}

/*
 *
 *  functionname  : CompArg
 *  arguments     : 1) N_with node
 *                  2) info node
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
CompArg (node *arg_node, node *arg_info)
{
    node *icm_arg, *id_node, *tmp, *type_id_node;
    simpletype s_type;

    DBUG_ENTER ("CompArg");
    if (1 == IsArray (arg_node)) {
        GET_BASIC_TYPE (s_type, arg_node);

        MAKENODE_ID (id_node, arg_node->ID);
        FREE_TYPE (arg_node->TYPES);
        arg_node->nodetype = N_icm;
        MAKE_ICM_NAME (arg_node, "ND_KS_RET_ARRAY");
        if (1 == arg_node->nnode)
            arg_node->node[1] = arg_node->node[0];
        MAKENODE_ID (type_id_node, type_string[s_type]);
        MAKE_ICM_ARG (arg_node->node[0], type_id_node);
        arg_node->nnode += 1;
        icm_arg = arg_node->node[0];
        MAKE_NEXT_ICM_ARG (icm_arg, id_node);
    }

    DBUG_RETURN (arg_node);
}
