/*
 *
 * $Log$
 * Revision 1.9  1995/06/26 14:06:24  hw
 * added new macros (moved from compile.c )
 *
 * Revision 1.8  1995/06/08  17:48:57  hw
 * CompTypedef inserted
 *
 * Revision 1.7  1995/05/22  10:08:01  hw
 * added function "CompCast"
 *
 * Revision 1.6  1995/05/08  15:45:13  hw
 * added CompBlock
 *
 * Revision 1.5  1995/04/24  18:07:54  hw
 * - renamed CompWhile to CompLoop
 *
 * Revision 1.4  1995/04/24  14:18:20  hw
 * CompWhile & CompCond inserted
 *
 * Revision 1.3  1995/04/03  16:34:48  hw
 * CompFundef inserted
 *
 * Revision 1.2  1995/03/31  15:47:37  hw
 * CompId, CompArg, CompWith, CompArray, CompAp, CompReturn inserted
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
 *
 *
 */

#ifndef _compile_h

#define _compile_h

extern node *Compile (node *arg_node);
extern node *CompVardec (node *arg_node, node *arg_info);
extern node *CompPrf (node *arg_node, node *arg_info);
extern node *CompAssign (node *arg_node, node *arg_info);
extern node *CompLet (node *arg_node, node *arg_info);
extern node *CompArray (node *arg_node, node *arg_info);
extern node *CompId (node *arg_node, node *arg_info);
extern node *CompAp (node *arg_node, node *arg_info);
extern node *CompReturn (node *arg_node, node *arg_info);
extern node *CompWith (node *arg_node, node *arg_info);
extern node *CompArg (node *arg_node, node *arg_info);
extern node *CompFundef (node *arg_node, node *arg_info);
extern node *CompLoop (node *arg_node, node *arg_info);
extern node *CompCond (node *arg_node, node *arg_info);
extern node *CompBlock (node *arg_node, node *arg_info);
extern node *CompCast (node *arg_node, node *arg_info);
extern node *CompTypedef (node *arg_node, node *arg_info);

/* and now some macros for creation of N_icms */

#define MAKE_ICM(assign)                                                                 \
    assign = MakeNode (N_assign);                                                        \
    assign->node[0] = MakeNode (N_icm);                                                  \
    assign->nnode = 1

#define MAKE_ICM_NAME(var, name)                                                         \
    var->info.fun_name.id = name;                                                        \
    var->info.fun_name.id_mod = NULL

#define MAKE_ICM_ARG(var, new_node)                                                      \
    var = MakeNode (N_exprs);                                                            \
    var->node[0] = new_node;                                                             \
    var->nnode = 1

#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    {                                                                                    \
        node *tmp;                                                                       \
        MAKE_ICM_ARG (tmp, new_node);                                                    \
        prev->node[1] = tmp;                                                             \
        prev->nnode = 2;                                                                 \
        prev = tmp;                                                                      \
    }

/* following macros use a variabel 'node *icm_arg', it has to be defined
 * before
 */
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

#define CREATE_5_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5)                      \
    CREATE_4_ARY_ICM (assign, str, arg1, arg2, arg3, arg4);                              \
    MAKE_NEXT_ICM_ARG (icm_arg, arg5)

#define CREATE_6_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5, arg6)                \
    CREATE_5_ARY_ICM (assign, str, arg1, arg2, arg3, arg4, arg5);                        \
    MAKE_NEXT_ICM_ARG (icm_arg, arg6)

#define CREATE_7_ARY_ICM(assign, str, arg1, arg2, arg3, arg4, arg5, arg6, arg7)          \
    CREATE_6_ARY_ICM (assign, str, arg1, arg2, arg3, arg4, arg5, arg6);                  \
    MAKE_NEXT_ICM_ARG (icm_arg, arg7)

#define BIN_ICM_REUSE(reuse, str, arg1, arg2)                                            \
    reuse->nodetype = N_icm;                                                             \
    reuse->nnode = 1;                                                                    \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define APPEND_ASSIGNS(first, next)                                                      \
    first->node[1] = next;                                                               \
    first->nnode = 2;                                                                    \
    first = next

/* and now some usefull macros to get some information */

#define GET_DIM(result, type)                                                            \
    if (T_user == type->simpletype) {                                                    \
        result = LookupType (type->name, type->name_mod, 042)->DIM;                      \
        result += type->dim;                                                             \
    } else                                                                               \
        result = type->dim

#define GET_BASIC_TYPE(res, type)                                                        \
    if (T_user == type->simpletype)                                                      \
        res = LookupType (type->name, type->name_mod, 042)->SIMPLETYPE;                  \
    else                                                                                 \
        res = type->simpletype

/* number of total elements of an array */
#define GET_LENGTH(length, vardec_node)                                                  \
    {                                                                                    \
        types *type;                                                                     \
        int i;                                                                           \
        if (T_user == vardec_node->SIMPLETYPE)                                           \
            type = LookupType (vardec_node->NAME, vardec_node->NAME_MOD, 042)->TYPES;    \
        else                                                                             \
            type = vardec_node->TYPES;                                                   \
        for (i = 0, length = 1; i < type->dim; i++)                                      \
            length *= type->shpseg->shp[i];                                              \
    }

/* length of N_exprs-chain */
#define COUNT_ELEMS(n, exprs)                                                            \
    {                                                                                    \
        node *tmp;                                                                       \
        n = 0;                                                                           \
        tmp = exprs;                                                                     \
        do {                                                                             \
            n += 1;                                                                      \
            tmp = tmp->node[1];                                                          \
        } while (NULL != tmp);                                                           \
    }

#endif /* _compile_h */
