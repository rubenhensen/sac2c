/*
 *
 * $Log$
 * Revision 1.62  1998/05/12 15:11:41  srs
 * added ae_tab to MRD_TAB
 *
 * Revision 1.61  1998/04/29 17:15:58  dkr
 * removed OPTSpmd
 *
 * Revision 1.60  1998/04/26 21:53:14  dkr
 * removed OPTSync
 *
 * Revision 1.59  1998/04/23 17:33:50  dkr
 * added OptSync
 *
 * Revision 1.58  1998/04/17 17:26:43  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.57  1998/04/09 12:01:40  dkr
 * ReGenMask() is now external
 *
 * Revision 1.56  1998/04/03 21:07:14  dkr
 * added OPTconc
 *
 * Revision 1.55  1998/04/03 11:58:12  srs
 * exported old_wlf_expr and old_wlt_expr to be visible in WithloopFolding.c
 *
 * Revision 1.54  1998/04/02 13:47:23  srs
 * NOTE_OPTIMIZER_PHASE() deleted
 *
 * Revision 1.53  1998/03/22 18:11:56  srs
 * added wlt_expr and NOTE_OPTIMIZER_PHASE()
 *
 * Revision 1.52  1998/03/21 22:46:54  srs
 * added wlt_tab to MRD_TAB
 *
 * Revision 1.51  1998/03/04 14:39:16  srs
 * changed wlf_tab to wli_tab in MRD_TAB
 *
 * Revision 1.50  1998/02/25 09:16:03  cg
 * Definition of global variable mrdl_stack moved to optimize.c
 *
 * Revision 1.49  1998/02/16 16:53:30  srs
 * added wlf_tab to MRD_TAB
 *
 * Revision 1.48  1998/02/13 13:40:15  srs
 * removed macro TOS (same as MRD_TOS)
 *
 * Revision 1.47  1998/02/12 11:10:26  srs
 * added OPTNwith
 *
 * Revision 1.46  1998/02/09 16:10:12  srs
 * introduced WLF
 *
 * Revision 1.45  1997/10/28 13:14:11  srs
 * removed dead code
 *
 * Revision 1.44  1997/04/25 12:13:00  sbs
 * MAlloc replaced by Malloc from internal.lib
 *
 * Revision 1.43  1997/04/24  15:03:43  sbs
 * HAVE_MALLOC_O inserted
 *
 * Revision 1.42  1996/08/09  16:42:52  asi
 * dead function removal added
 *
 * Revision 1.41  1996/02/13  13:58:14  asi
 * MRD_GETCSE changed
 *
 * Revision 1.40  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.38  1996/01/17  14:48:39  asi
 * global variables moved to globals.h
 * added typedefs stelm and stack
 * added defines MIN_STACK_SIZE, MAKE_MRDL_STACK, FREE_MRDL_STACK, TOS, MRD_TOS, MRD_TAB
 * MRD_LIST, MRD, MRD_GETSUBST, MRD_GETCS, MRD_GETLAST, MRD_GETDATA
 * added external definitions for functions MrdGet, GetExpr, PushMRDL, PushDupMRDL
 * PopMRDL, PopMRDL2
 *
 * Revision 1.37  1995/12/21  13:19:27  asi
 * added macros INFO_VARNO, INFO_DEF, INFO_USE and
 * function OPTTrav for traversing new syntax tree
 *
 * Revision 1.36  1995/12/01  17:19:48  cg
 * macro FREE removed.
 *
 * Revision 1.35  1995/11/13  09:04:35  asi
 * added breakae
 *
 * Revision 1.34  1995/07/24  11:48:52  asi
 * added array elimination
 *
 * Revision 1.33  1995/07/19  18:48:37  asi
 * added external decleration of max_optcycles
 *
 * Revision 1.32  1995/07/07  14:58:38  asi
 * added loop unswitching
 *
 * Revision 1.31  1995/06/26  11:53:18  asi
 * parameter for GenerateMasks changed
 *
 * Revision 1.30  1995/06/20  15:55:08  asi
 * added WARN macro
 *
 * Revision 1.29  1995/06/14  13:44:41  asi
 * added unrnum and unr_expr
 *
 * Revision 1.28  1995/06/07  14:30:06  asi
 * added GenerateMasks
 *
 * Revision 1.27  1995/06/02  11:36:15  asi
 * Added function inlining
 *
 * Revision 1.26  1995/05/26  12:46:20  asi
 * new debug option MTOOL added
 *
 * Revision 1.25  1995/05/16  12:46:09  asi
 * MALLOC_TOOL renamed to MALLOC_OPT
 *
 * Revision 1.24  1995/05/15  09:14:51  asi
 * functions malloc_debug & malloc_verify will onlu be called
 * if macro MALLOC_TOOL is defined.
 *
 * Revision 1.23  1995/05/15  08:45:25  asi
 * added variables : optvar, optvar_counter.
 * added functions : SetMask, OrMask, AndMask.
 *
 * Revision 1.22  1995/04/05  15:52:38  asi
 * loop invariant removal added
 *
 * Revision 1.21  1995/03/24  15:51:55  asi
 * added FREE
 *
 * Revision 1.20  1995/03/17  17:43:39  asi
 * added work reduction
 *
 * Revision 1.19  1995/03/14  11:12:40  asi
 * added OptTrav
 *
 * Revision 1.18  1995/03/13  17:11:38  asi
 * added extern var optimize
 *
 * Revision 1.17  1995/03/07  10:04:45  asi
 * added OPTwith and PrintMasks
 *
 * Revision 1.16  1995/02/28  18:28:51  asi
 * masks now have dynamic length, stored in varno
 *
 * Revision 1.15  1995/02/22  18:18:44  asi
 * ClearMask, ReadMask added -- GenMask, OPTfundef modified
 *
 * Revision 1.14  1995/02/16  15:50:07  asi
 * MAlloc added
 *
 * Revision 1.13  1995/02/13  16:40:29  asi
 * functions for dead code removal and constant folding
 * moved in DeadCodeRemoval.c and ConstantFolding.c
 * global variables opt_cf and opt_dcr defined in main.c defined extern
 *
 * Revision 1.12  1995/02/07  11:06:57  asi
 * renamed the functions OPT1* -> OPT*, OPT2* -> DEAD*, OPT3* -> LIR*
 * added CFprf
 *
 * Revision 1.11  1995/01/18  17:43:14  asi
 * Dead-Code-Removal now frees memory
 *
 * Revision 1.10  1995/01/17  11:05:16  asi
 * Added OPT3assign for loop invariant removal
 * OPT2while now named OPT2loop and used for do and while loops
 *
 * Revision 1.9  1995/01/12  13:15:00  asi
 * inserted OPT2fundef OPT2block OPT2assign OPT2while OPT2cond
 * for dead code removal in loops and in conditionals
 *
 * Revision 1.8  1995/01/06  14:56:09  asi
 * Dead-Vardec-Removal implemented
 *
 * Revision 1.7  1995/01/05  15:30:02  asi
 * Dead-Code-Removal implemented
 *
 * Revision 1.6  1995/01/02  16:04:46  asi
 * Renamed opt_tab in opt1_tab and all OPT.. in OPT1..
 * Added OPT1while, OPT1do, OPT1cond, OPT1cc
 * Added opt2_tab
 *
 * Revision 1.5  1995/01/02  10:50:49  asi
 * added OPTassign
 *
 * Revision 1.4  1994/12/19  15:26:07  asi
 * Added - OPTid
 *
 * Revision 1.3  1994/12/19  14:43:17  asi
 * Added - OPTfundef OPTarg OPTvardec OPTmodul OPTlet OPTblock
 * preperation for loop invariant removal
 *
 * Revision 1.2  1994/12/12  11:04:18  asi
 * added OPTdo
 *
 * Revision 1.1  1994/12/09  10:48:23  sbs
 * Initial revision
 *
 *
 */

#ifndef _sac_optimize_h

#define _sac_optimize_h

#include "free.h"

extern int dead_expr;
extern int dead_var;
extern int dead_fun;
extern int cf_expr;
extern int lir_expr;
extern int inl_fun;
extern int unr_expr;
extern int uns_expr;
extern int optvar_counter;
extern int elim_arrays;
extern int cse_expr;
extern int wlf_expr;
extern int wlt_expr;
extern int old_wlf_expr, old_wlt_expr;

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
extern node *GetExpr (node *arg_node);
extern int CheckScope (long *act_mrdl, node *assign_node, int varno, int checkdef);

#define MRD_GETSUBST(n, i, v) n = GetExpr (MrdGet (i, v, 0)); /* only in CF */
#define MRD_GETLAST(n, i, v) n = GetExpr (MrdGet (i, v, 1));  /* CF, Unroll, Unswitch */
#define MRD_GETDATA(n, i, v) n = GetExpr (MrdGet (i, v, 2));  /* only in CF */
#define MRD_GETCSE(n, i, v) n = GetExpr (MrdGet (i, v, 3));   /* only in CSE */

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

#define VAR_LENGTH 10

extern node *Optimize (node *arg_node);

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
extern node *OPTfundef (node *arg_node, node *arg_info);
extern node *OPTarg (node *arg_node, node *arg_info);
extern node *OPTvardec (node *arg_node, node *arg_info);
extern node *OPTmodul (node *arg_node, node *arg_info);
extern node *OPTlet (node *arg_node, node *arg_info);
extern node *OPTid (node *arg_node, node *arg_info);
extern node *OPTassign (node *arg_node, node *arg_info);
extern node *OPTpp (node *arg_node, node *arg_info);
extern node *OPTblock (node *arg_node, node *arg_info);
extern node *OPTcond (node *arg_node, node *arg_info);
extern node *OPTloop (node *arg_node, node *arg_info);
extern node *OPTwith (node *arg_node, node *arg_info);
extern node *OPTNwith (node *arg_node, node *arg_info);

#endif /* _sac_optimize_h */
