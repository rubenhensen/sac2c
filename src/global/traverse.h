/*
 *
 * $Log$
 * Revision 1.8  1995/02/07 10:59:23  asi
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

extern funptr *act_tab;

extern funptr imp_tab[];

extern funptr flat_tab[];

extern funptr print_tab[];

extern funptr type_tab[];

extern funptr opt_tab[];

extern funptr dead_tab[];

extern funptr lir_tab[];

extern funptr cf_tab[];

extern funptr free_tab[];

#endif /* _sac_traverse_h */
