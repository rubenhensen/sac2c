/*
 *
 * $Log$
 * Revision 1.17  1998/02/11 11:02:53  dkr
 * *** empty log message ***
 *
 * Revision 1.16  1997/11/25 10:33:19  dkr
 * prototype CompNWith added
 *
 * Revision 1.15  1997/11/02 13:58:03  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.14  1997/10/31 11:04:11  dkr
 * with defined NEWTREE, node->nnode is not used anymore
 *
 * Revision 1.13  1996/01/26 15:34:11  cg
 * added macro TRI_ICM_REUSE
 *
 * Revision 1.12  1996/01/21  14:00:29  cg
 * added APPEND_ICM_ARG, new DBUG_PRINTs in CREATE_1_ARY_ICM
 *
 * Revision 1.11  1995/12/18  16:20:52  cg
 * added declaration of node *CompObjdef(node *arg_node, node *arg_info);
 *
 * Revision 1.10  1995/06/28  09:30:22  hw
 * moved some macros form compile.h to typecheck.h
 *
 * Revision 1.9  1995/06/26  14:06:24  hw
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
extern node *CompNWith (node *arg_node, node *arg_info);
extern node *CompArg (node *arg_node, node *arg_info);
extern node *CompFundef (node *arg_node, node *arg_info);
extern node *CompLoop (node *arg_node, node *arg_info);
extern node *CompCond (node *arg_node, node *arg_info);
extern node *CompBlock (node *arg_node, node *arg_info);
extern node *CompCast (node *arg_node, node *arg_info);
extern node *CompTypedef (node *arg_node, node *arg_info);
extern node *CompObjdef (node *arg_node, node *arg_info);

/* and now some macros for creation of N_icms */

#ifndef NEWTREE
#define MAKE_ICM(assign)                                                                 \
    assign = MakeNode (N_assign);                                                        \
    assign->node[0] = MakeNode (N_icm);                                                  \
    assign->nnode = 1
#else /* NEWTREE */
#define MAKE_ICM(assign)                                                                 \
    assign = MakeNode (N_assign);                                                        \
    assign->node[0] = MakeNode (N_icm)
#endif /* NEWTREE */

#define MAKE_ICM_NAME(var, name)                                                         \
    var->info.fun_name.id = name;                                                        \
    var->info.fun_name.id_mod = NULL

#ifndef NEWTREE
#define MAKE_ICM_ARG(var, new_node)                                                      \
    var = MakeNode (N_exprs);                                                            \
    var->node[0] = new_node;                                                             \
    var->nnode = 1
#else /* NEWTREE */
#define MAKE_ICM_ARG(var, new_node)                                                      \
    var = MakeNode (N_exprs);                                                            \
    var->node[0] = new_node;
#endif /* NEWTREE */

#ifndef NEWTREE
#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    {                                                                                    \
        node *tmp;                                                                       \
        MAKE_ICM_ARG (tmp, new_node);                                                    \
        prev->node[1] = tmp;                                                             \
        prev->nnode = 2;                                                                 \
        prev = tmp;                                                                      \
    }
#else /* NEWTREE */
#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    {                                                                                    \
        node *tmp;                                                                       \
        MAKE_ICM_ARG (tmp, new_node);                                                    \
        prev->node[1] = tmp;                                                             \
        prev = tmp;                                                                      \
    }
#endif /* NEWTREE */

#ifndef NEWTREE
#define APPEND_ICM_ARG(prev, new)                                                        \
    prev->node[1] = new;                                                                 \
    prev->nnode = 2;                                                                     \
    prev = new;
#else /* NEWTREE */
#define APPEND_ICM_ARG(prev, new)                                                        \
    prev->node[1] = new;                                                                 \
    prev = new;
#endif /* NEWTREE */

/*
 * The following macros use a variabel 'node *icm_arg', it has to be defined
 * before usage.
 */

#ifndef NEWTREE
#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    DBUG_PRINT ("COMP", ("Creating ICM \"%s\" ...", str));                               \
    MAKE_ICM (assign);                                                                   \
    MAKE_ICM_NAME (assign->node[0], str);                                                \
    MAKE_ICM_ARG (assign->node[0]->node[0], arg);                                        \
    icm_arg = assign->node[0]->node[0];                                                  \
    assign->node[0]->nnode = 1;                                                          \
    DBUG_PRINT ("COMP", ("ICM \"%s\" created", str));
#else /* NEWTREE */
#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    DBUG_PRINT ("COMP", ("Creating ICM \"%s\" ...", str));                               \
    MAKE_ICM (assign);                                                                   \
    MAKE_ICM_NAME (assign->node[0], str);                                                \
    MAKE_ICM_ARG (assign->node[0]->node[0], arg);                                        \
    icm_arg = assign->node[0]->node[0];                                                  \
    DBUG_PRINT ("COMP", ("ICM \"%s\" created", str));
#endif /* NEWTREE */

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

#ifndef NEWTREE
#define BIN_ICM_REUSE(reuse, str, arg1, arg2)                                            \
    reuse->nodetype = N_icm;                                                             \
    reuse->nnode = 1;                                                                    \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)
#else /* NEWTREE */
#define BIN_ICM_REUSE(reuse, str, arg1, arg2)                                            \
    reuse->nodetype = N_icm;                                                             \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)
#endif /* NEWTREE */

#ifndef NEWTREE
#define TRI_ICM_REUSE(reuse, str, arg1, arg2, arg3)                                      \
    reuse->nodetype = N_icm;                                                             \
    reuse->nnode = 1;                                                                    \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3);
#else /* NEWTREE */
#define TRI_ICM_REUSE(reuse, str, arg1, arg2, arg3)                                      \
    reuse->nodetype = N_icm;                                                             \
    MAKE_ICM_NAME (reuse, str);                                                          \
    MAKE_ICM_ARG (reuse->node[0], arg1);                                                 \
    icm_arg = reuse->node[0];                                                            \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3);
#endif /* NEWTREE */

#ifndef NEWTREE
#define APPEND_ASSIGNS(first, next)                                                      \
    first->node[1] = next;                                                               \
    first->nnode = 2;                                                                    \
    first = next
#else /* NEWTREE */
#define APPEND_ASSIGNS(first, next)                                                      \
    first->node[1] = next;                                                               \
    first = next
#endif /* NEWTREE */

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
