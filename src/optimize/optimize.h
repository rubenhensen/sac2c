/*
 *
 * $Log$
 * Revision 1.34  1995/07/24 11:48:52  asi
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

#ifndef _optimize_h

#define _optimize_h

/*
 * Global variables defined in main.c
 */
extern int optimize;
extern int sac_optimize;
extern int opt_dcr;
extern int opt_cf;
extern int opt_wr;
extern int opt_lir;
extern int opt_inl;
extern int opt_unr;
extern int opt_uns;
extern int opt_ae;
extern int optvar;
extern int inlnum;
extern int unrnum;
extern int max_optcycles;
extern int minarray;
extern char filename[]; /* is set temporary; will be set later on in main.c */
/* main.c end */

extern int dead_expr;
extern int dead_var;
extern int cf_expr;
extern int wr_expr;
extern int lir_expr;
extern int inl_fun;
extern int unr_expr;
extern int uns_expr;
extern int optvar_counter;
extern int elim_arrays;

#define INC_VAR(mask, var) mask[var] += 1
#define DEC_VAR(mask, var) mask[var] -= 1
#define VARNO arg_info->varno

#define DEF arg_node->mask[0]
#define USE arg_node->mask[1]

#define VAR_LENGTH 10

#define WARNO(s)                                                                         \
    if (!silent) {                                                                       \
        DoPrint s;                                                                       \
        fprintf (stderr, "\n");                                                          \
    }

#ifdef MALLOC_OPT
extern int malloc_verify ();
extern int malloc_debug (int level);
#endif /*MALLOC_OPT */

#ifdef MALLOC_OPT
#define FREE(address)                                                                    \
    DBUG_PRINT ("MEM", ("Give memory free at adress: %08x", address));                   \
    free (address);                                                                      \
    DBUG_EXECUTE ("MTOOL", malloc_verify (););
#else
#define FREE(address)                                                                    \
    DBUG_PRINT ("MEM", ("Give memory free at adress: %08x", address));                   \
    free (address)
#endif /*MALLOC_OPT */

extern node *Optimize (node *arg_node);

extern void *MAlloc (int size);
extern char *PrintMask (long *mask, int varno);
extern void PrintMasks (node *arg_node, node *arg_info);
extern void SetMask (long *mask, long value, int varno);
extern long *GenMask (int varno);
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

#endif /* _optimize_h */
