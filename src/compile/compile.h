/*
 *
 * $Log$
 * Revision 1.31  1998/04/26 21:49:40  dkr
 * COMPSPMD renamed to COMPSpmd
 *
 * Revision 1.30  1998/04/24 17:14:40  dkr
 * renamed Comp...() to COMP...()
 *
 * Revision 1.29  1998/04/23 17:33:02  dkr
 * added CompSync
 *
 * Revision 1.28  1998/04/21 13:31:28  dkr
 * added funs CompWL...
 *
 * Revision 1.27  1998/04/19 17:48:00  dkr
 * uses now access macros
 *
 * Revision 1.26  1998/04/17 17:26:57  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.25  1998/04/14 21:44:56  dkr
 * added CompModul
 *
 * Revision 1.24  1998/04/07 14:50:15  dkr
 * added CompNcode
 *
 * Revision 1.23  1998/04/02 18:46:56  dkr
 * added CompConc
 *
 * Revision 1.22  1998/03/02 22:25:57  dkr
 * macros for new with-loop moved to tree_basic
 *
 * Revision 1.20  1998/02/16 01:08:01  dkr
 * bugs fixed
 *
 * Revision 1.19  1998/02/15 04:06:28  dkr
 * added some macros for N_index-nodes (experimental !!)
 *
 * Revision 1.18  1998/02/11 16:31:05  dkr
 * removed NEWTREE, access-macros used
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

#ifndef _sac_compile_h

#define _sac_compile_h

extern node *Compile (node *arg_node);

extern node *COMPModul (node *arg_node, node *arg_info);
extern node *COMPVardec (node *arg_node, node *arg_info);
extern node *COMPPrf (node *arg_node, node *arg_info);
extern node *COMPAssign (node *arg_node, node *arg_info);
extern node *COMPLet (node *arg_node, node *arg_info);
extern node *COMPArray (node *arg_node, node *arg_info);
extern node *COMPId (node *arg_node, node *arg_info);
extern node *COMPAp (node *arg_node, node *arg_info);
extern node *COMPReturn (node *arg_node, node *arg_info);
extern node *COMPArg (node *arg_node, node *arg_info);
extern node *COMPFundef (node *arg_node, node *arg_info);
extern node *COMPLoop (node *arg_node, node *arg_info);
extern node *COMPCond (node *arg_node, node *arg_info);
extern node *COMPBlock (node *arg_node, node *arg_info);
extern node *COMPCast (node *arg_node, node *arg_info);
extern node *COMPTypedef (node *arg_node, node *arg_info);
extern node *COMPObjdef (node *arg_node, node *arg_info);
extern node *COMPWith (node *arg_node, node *arg_info);
extern node *COMPSpmd (node *arg_node, node *arg_info);
extern node *COMPSync (node *arg_node, node *arg_info);
extern node *COMPNcode (node *arg_node, node *arg_info);
extern node *COMPNwith2 (node *arg_node, node *arg_info);
extern node *COMPWLseg (node *arg_node, node *arg_info);
extern node *COMPWLblock (node *arg_node, node *arg_info);
extern node *COMPWLublock (node *arg_node, node *arg_info);
extern node *COMPWLstride (node *arg_node, node *arg_info);
extern node *COMPWLgrid (node *arg_node, node *arg_info);
extern node *COMPWLstriVar (node *arg_node, node *arg_info);
extern node *COMPWLgridVar (node *arg_node, node *arg_info);

/* and now some macros for creation of N_icms */

#define MAKE_ICM_ARG(var, new_node) var = MakeExprs (new_node, NULL);

#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    {                                                                                    \
        node *tmp;                                                                       \
        MAKE_ICM_ARG (tmp, new_node);                                                    \
        EXPRS_NEXT (prev) = tmp;                                                         \
        prev = tmp;                                                                      \
    }

#define APPEND_ICM_ARG(prev, new)                                                        \
    EXPRS_NEXT (prev) = new;                                                             \
    prev = new;

/*
 * The following macros use a variabel 'node *icm_arg', it has to be defined
 * before usage.
 */

#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    DBUG_PRINT ("COMP", ("Creating ICM \"%s\" ...", str));                               \
    assign = MakeAssign (MakeIcm (str, NULL, NULL), NULL);                               \
    MAKE_ICM_ARG (ICM_ARGS (ASSIGN_INSTR (assign)), arg);                                \
    icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));                                          \
    DBUG_PRINT ("COMP", ("ICM \"%s\" created", str));

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
    NODE_TYPE (reuse) = N_icm;                                                           \
    ICM_NAME (reuse) = str;                                                              \
    MAKE_ICM_ARG (ICM_ARGS (reuse), arg1);                                               \
    icm_arg = ICM_ARGS (reuse);                                                          \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define TRI_ICM_REUSE(reuse, str, arg1, arg2, arg3)                                      \
    NODE_TYPE (reuse) = N_icm;                                                           \
    ICM_NAME (reuse) = str;                                                              \
    MAKE_ICM_ARG (ICM_ARGS (reuse), arg1);                                               \
    icm_arg = ICM_ARGS (reuse);                                                          \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3)

#define APPEND_ASSIGNS(first, next)                                                      \
    ASSIGN_NEXT (first) = next;                                                          \
    first = next

/* length of N_exprs-chain */
#define COUNT_ELEMS(n, exprs)                                                            \
    {                                                                                    \
        node *tmp;                                                                       \
        n = 0;                                                                           \
        tmp = exprs;                                                                     \
        do {                                                                             \
            n++;                                                                         \
            tmp = EXPRS_NEXT (tmp);                                                      \
        } while (NULL != tmp);                                                           \
    }

#endif /* _sac_compile_h */
