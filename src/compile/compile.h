/*
 * $Log$
 * Revision 2.7  2000/05/25 23:04:20  dkr
 * Prototype for GetAdjustedFoldCode() added
 * GetFoldCode() renamed into GetUnadjustedFoldCode()
 *
 * Revision 2.6  2000/04/20 11:36:13  jhs
 * Added COMPMT(signal|alloc|sync)
 *
 * Revision 2.5  2000/04/18 14:00:48  jhs
 * Added COMPSt and COMPMt.
 *
 * Revision 2.4  2000/03/21 15:46:43  dkr
 * ICM_INDENT explcitly set to 0 if nodes are reused as icm-nodes
 *
 * [ eliminated ]
 *
 * Revision 1.1  1995/03/29  12:38:10  hw
 * Initial revision
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
extern node *COMPSpmd (node *arg_node, node *arg_info);
extern node *COMPSync (node *arg_node, node *arg_info);
extern node *COMPNcode (node *arg_node, node *arg_info);
extern node *COMPNwith2 (node *arg_node, node *arg_info);
extern node *COMPWLseg (node *arg_node, node *arg_info);
extern node *COMPWLblock (node *arg_node, node *arg_info);
extern node *COMPWLublock (node *arg_node, node *arg_info);
extern node *COMPWLstride (node *arg_node, node *arg_info);
extern node *COMPWLgrid (node *arg_node, node *arg_info);
extern node *COMPWLsegVar (node *arg_node, node *arg_info);
extern node *COMPWLstriVar (node *arg_node, node *arg_info);
extern node *COMPWLgridVar (node *arg_node, node *arg_info);
extern node *COMPMt (node *arg_node, node *arg_info);
extern node *COMPSt (node *arg_node, node *arg_info);
extern node *COMPMTsignal (node *arg_node, node *arg_info);
extern node *COMPMTalloc (node *arg_node, node *arg_info);
extern node *COMPMTsync (node *arg_node, node *arg_info);

extern node *GetUnadjustedFoldCode (node *fundef);
extern node *GetAdjustedFoldCode (node *fundef, ids *acc, char *funname, node *cexpr);
extern node *GetFoldVardecs (node *fundef);

/*
 * some macros for creation of N_icms
 */

#define MAKE_NEXT_ICM_ARG(prev, new_node)                                                \
    {                                                                                    \
        node *tmp;                                                                       \
        tmp = MakeExprs (new_node, NULL);                                                \
        EXPRS_NEXT (prev) = tmp;                                                         \
        prev = tmp;                                                                      \
    }

#define APPEND_ICM_ARG(prev, new)                                                        \
    EXPRS_NEXT (prev) = new;                                                             \
    prev = new;

/*
 * The following macros use a variabel 'node *icm_arg', it has to be defined
 * before usage.
 *
 * PLEASE DO NOT USE THE FOLLOWING MACROS FOR NEW CODE!!!!!
 * Use the functions MakeIcm? and MakeAssignIcm? instead!!
 */

#define CREATE_1_ARY_ICM(assign, str, arg)                                               \
    assign = MakeAssign (MakeIcm (str, NULL, NULL), NULL);                               \
    ICM_ARGS (ASSIGN_INSTR (assign)) = MakeExprs (arg, NULL);                            \
    icm_arg = ICM_ARGS (ASSIGN_INSTR (assign));

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
    ICM_ARGS (reuse) = MakeExprs (arg1, NULL);                                           \
    ICM_INDENT (reuse) = 0;                                                              \
    icm_arg = ICM_ARGS (reuse);                                                          \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2)

#define TRI_ICM_REUSE(reuse, str, arg1, arg2, arg3)                                      \
    NODE_TYPE (reuse) = N_icm;                                                           \
    ICM_NAME (reuse) = str;                                                              \
    ICM_ARGS (reuse) = MakeExprs (arg1, NULL);                                           \
    ICM_INDENT (reuse) = 0;                                                              \
    icm_arg = ICM_ARGS (reuse);                                                          \
    MAKE_NEXT_ICM_ARG (icm_arg, arg2);                                                   \
    MAKE_NEXT_ICM_ARG (icm_arg, arg3)

#define APPEND_ASSIGNS(first, next)                                                      \
    ASSIGN_NEXT (first) = next;                                                          \
    first = next

/*
 * length of N_exprs-chain
 */
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
