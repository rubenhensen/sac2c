/*
 *
 * $Log$
 * Revision 1.19  1995/07/07 14:58:38  asi
 * added loop unswitching - basic version
 *
 * Revision 1.18  1995/06/06  15:19:36  sbs
 * DummyFun2 inserted
 *
 * Revision 1.17  1995/06/02  12:14:19  sbs
 * idx_tab inserted
 *
 * Revision 1.16  1995/05/26  14:23:42  asi
 * function inlineing and loop unrolling added
 *
 * Revision 1.15  1995/05/01  15:34:57  asi
 * dup_tab inserted
 *
 * Revision 1.14  1995/04/11  15:57:47  asi
 * NIF macro enlarged
 *
 * Revision 1.13  1995/04/07  10:16:26  hw
 * added function NoTrav
 *
 * Revision 1.12  1995/04/05  15:52:38  asi
 * loop invariant removal added
 *
 * Revision 1.11  1995/03/29  12:00:31  hw
 * comp_tab inserted
 *
 * Revision 1.10  1995/03/17  17:41:48  asi
 * added work reduction
 *
 * Revision 1.9  1995/03/10  10:45:25  hw
 * refcnt_tab inserted
 *
 * Revision 1.8  1995/02/07  10:59:23  asi
 * renamed opt1_tab -> opt_tab, opt2_tab -> dead_tab, opt3_tab -> lir.tab and
 * added functionlist cf_tab for constant folding
 *
 * Revision 1.7  1995/01/16  10:54:50  asi
 * added opt3_tab for loop independent removal
 * and free_tree for deletion of a syntax(sub)-tree
 *
 * Revision 1.6  1995/01/02  16:04:46  asi
 * Renamed opt_tab in opt1_tab and all OPT.. in OPT1..
 * Added OPT1while, OPT1do, OPT1cond, OPT1cc
 * Added opt2_tab
 *
 * Revision 1.5  1994/12/16  14:22:10  sbs
 * imp_tab inserted and NIF macro enlarged
 *
 * Revision 1.4  1994/12/09  10:45:33  sbs
 * opt_tab inserted
 *
 * Revision 1.3  1994/12/01  17:41:09  hw
 * added extern funptr type_tab[];
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#ifndef _sac_traverse_h

#define _sac_traverse_h

typedef node *(*funptr) (node *, node *);

extern node *Trav (node *arg_node, node *arg_info);

extern node *DummyFun (node *arg_node, node *arg_info);

extern node *DummyFun2 (node *arg_node, node *arg_info);

extern node *NoTrav (node *arg_node, node *arg_info);

extern funptr *act_tab;

extern funptr imp_tab[];

extern funptr flat_tab[];

extern funptr print_tab[];

extern funptr type_tab[];

extern funptr opt_tab[];

extern funptr dead_tab[];

extern funptr cf_tab[];

extern funptr wr_tab[];

extern funptr free_tab[];

extern funptr refcnt_tab[];

extern funptr comp_tab[];

extern funptr lir_tab[];

extern funptr lir_mov_tab[];

extern funptr dup_tab[];

extern funptr inline_tab[];

extern funptr unroll_tab[];

extern funptr unswitch_tab[];

extern funptr idx_tab[];

#endif /* _sac_traverse_h */
