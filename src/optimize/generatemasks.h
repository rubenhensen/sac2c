/*
 *
 * $Log$
 * Revision 1.1  1999/01/07 17:37:34  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _generatemasks_h

#define _generatemasks_h

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

extern stack *mrdl_stack;

#define MAKE_MRDL_STACK                                                                  \
    mrdl_stack = Malloc (sizeof (stack));                                                \
    mrdl_stack->tos = -1;                                                                \
    mrdl_stack->st_len = MIN_STACK_SIZE;                                                 \
    mrdl_stack->stack = (stelm *)Malloc (sizeof (stelm) * MIN_STACK_SIZE);

#define FREE_MRDL_STACK                                                                  \
    FREE (mrdl_stack->stack);                                                            \
    FREE (mrdl_stack);

#define MRD_TOS mrdl_stack->stack[mrdl_stack->tos]
#define MRD_TAB                                                                          \
    (cf_tab == act_tab || unroll_tab == act_tab || unswitch_tab == act_tab               \
     || cse_tab == act_tab || ae_tab == act_tab || wli_tab == act_tab                    \
     || wlt_tab == act_tab)
#define MRD_LIST (long *)MRD_TOS.varlist
#define MRD(i) MRD_TOS.varlist[i]

extern node *MrdGet (int i, int varno, int outside_block);
extern int CheckScope (long *act_mrdl, node *assign_node, int varno, int checkdef);

#define MRD_GETSUBST(i, v) GetExpr (MrdGet (i, v, 0));       /* only in CF */
#define MRD_GETLAST(n, i, v) n = GetExpr (MrdGet (i, v, 1)); /* CF, Unroll, Unswitch */
#define MRD_GETDATA(n, i, v) n = GetExpr (MrdGet (i, v, 2)); /* only in CF, WLI */
#define MRD_GETCSE(n, i, v) n = GetExpr (MrdGet (i, v, 3));  /* only in CSE */

extern void PushMRDL (long NumVar);
extern void PushDupMRDL ();
extern void PopMRDL ();
extern void PopMRDL2 ();

#define INC_VAR(mask, var) mask[var] += 1
#define DEC_VAR(mask, var) mask[var] -= 1

#define VARNO arg_info->varno
#define DEF arg_node->mask[0]
#define USE arg_node->mask[1]

#define INFO_VARNO arg_info->varno /* number of variables in currenent function */
#define INFO_DEF arg_info->mask[0] /* added or removed variable declarations    */
#define INFO_USE arg_info->mask[1] /* added or removed variable useages         */

extern char *PrintMask (long *mask, int varno);
extern void PrintMasks (node *arg_node, node *arg_info);
extern void SetMask (long *mask, long value, int varno);
extern long *GenMask (int varno);
extern long *ReGenMask (long *mask, int varno);
extern void MinusMask (long *mask1, long *mask2, int varno);
extern long *DupMask (long *oldmask, int varno);
extern void OrMask (long *mask1, long *mask2, int varno);
extern void AndMask (long *mask1, long *mask2, int varno);
extern short CheckMask (long *mask1, long *mask2, int varno);
extern void PlusMask (long *mask1, long *mask2, int varno);
extern void PlusChainMasks (int pos, node *chain, node *arg_info);
extern void If3_2Mask (long *mask1, long *mask2, long *mask3, int varno);
extern short MaskIsNotZero (long *mask, int varno);
extern long ReadMask (long *mask, long number);

extern node *OptTrav (node *arg_node, node *arg_info, int node_no);
extern node *OPTTrav (node *trav_node, node *arg_info, node *arg_node);

extern node *GenerateMasks (node *arg_node, node *arg_info);
extern node *GNMfundef (node *arg_node, node *arg_info);
extern node *GNMarg (node *arg_node, node *arg_info);
extern node *GNMvardec (node *arg_node, node *arg_info);
extern node *GNMmodul (node *arg_node, node *arg_info);
extern node *GNMlet (node *arg_node, node *arg_info);
extern node *GNMid (node *arg_node, node *arg_info);
extern node *GNMassign (node *arg_node, node *arg_info);
extern node *GNMpp (node *arg_node, node *arg_info);
extern node *GNMblock (node *arg_node, node *arg_info);
extern node *GNMcond (node *arg_node, node *arg_info);
extern node *GNMloop (node *arg_node, node *arg_info);
extern node *GNMwith (node *arg_node, node *arg_info);
extern node *GNMNwith (node *arg_node, node *arg_info);
extern node *GNMicm (node *arg_node, node *arg_info);

#endif /* _generatemasks_h */
