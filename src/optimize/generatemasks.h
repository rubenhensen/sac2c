/*
 *
 * $Log$
 * Revision 2.6  1999/11/15 18:07:26  dkr
 * some macros changed, replaced or modified (VARNO, MRD, ...)
 *
 * Revision 2.5  1999/10/29 16:44:55  dkr
 * ExpandMRDL() added
 *
 * Revision 2.4  1999/10/28 17:08:05  dkr
 * signature of functions Print...Mask() changed
 *
 * Revision 2.3  1999/04/13 14:03:28  cg
 * Bug fixed in MrdGet(): function looks behind applications
 * of F_reshape only in modes 2 and 3.
 * MrdGet() now returns the left hand side expression when it used
 * to point to an N_assign node with an N_let node as instruction.
 * The functionality of former function GetExpr() is thus integrated
 * into MrdGet().
 *
 * Revision 2.2  1999/03/31 15:10:02  bs
 * I did some code cosmetics with the MRD_GET... macros.
 *
 * Revision 2.1  1999/02/23 12:41:51  sacbase
 * new release made
 *
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
#define MRD_VLEN MRD_TOS.vl_len
#define MRD(i) MRD_TOS.varlist[i]

extern node *MrdGet (int i, int varno, int outside_block);
extern int CheckScope (long *act_mrdl, node *assign_node, int varno, int checkdef);

#define MRD_GETSUBST(i, v) MrdGet (i, v, 0); /* only in CF */
#define MRD_GETLAST(i, v) MrdGet (i, v, 1);  /* CF, Unroll, Unswitch */
#define MRD_GETDATA(i, v) MrdGet (i, v, 2);  /* CF, WLI, ArrayElimination */
#define MRD_GETCSE(i, v) MrdGet (i, v, 3);   /* only in CSE */
#define MRD_GETCFID(i, v) MrdGet (i, v, 4);  /* only in CFid */

extern void PushMRDL (long NumVar);
extern void PushDupMRDL ();
extern void PopMRDL ();
extern void ExpandMRDL (int new_num);

#define INC_VAR(mask, var) mask[var] += 1
#define DEC_VAR(mask, var) mask[var] -= 1

#define DEF arg_node->mask[0]
#define USE arg_node->mask[1]

#define INFO_DEF arg_info->mask[0] /* added or removed variable declarations    */
#define INFO_USE arg_info->mask[1] /* added or removed variable useages         */

extern void PrintDefUseMask (FILE *handle, long *mask, int varno);
extern void PrintDefUseMasks (FILE *handle, long *defmask, long *usemask, int varno);
extern void PrintMrdMask (FILE *handle, long *mrdmask, int varno);

extern void SetMask (long *mask, long value, int varno);
extern long *GenMask (int varno);
extern long *ReGenMask (long *mask, int varno);
extern void MinusMask (long *mask1, long *mask2, int varno);
extern long *DupMask (long *oldmask, int varno);
extern long *CopyMask (long *mask1, int varno1, long *mask2, int varno2);
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
