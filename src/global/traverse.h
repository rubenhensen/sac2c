/*
 *
 * $Log$
 * Revision 1.41  1998/04/02 16:12:02  dkr
 * added new traverse tabular concregs_tab
 *
 * Revision 1.40  1998/03/22 18:07:09  srs
 * added wlt_tab
 *
 * Revision 1.39  1998/03/06 13:21:08  srs
 * added wli_tab
 *
 * Revision 1.38  1998/02/05 17:08:52  srs
 * removed wr_tab and changed fusion_tab into wlf_tab
 *
 * Revision 1.37  1997/11/19 19:40:15  dkr
 * added o2nWith_tab
 *
 * Revision 1.36  1997/11/05 16:30:24  dkr
 * moved nnode[] from tree_compound.[ch] to traverse.[ch]
 *
 * Revision 1.35  1997/03/19 13:37:55  cg
 * The entire link_tab is removed
 *
 * Revision 1.34  1996/08/09  16:44:12  asi
 * dead function removal added
 *
 * Revision 1.33  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.32  1996/01/02  15:49:35  cg
 * added link_tab, macro NIF extended.
 *
 * Revision 1.31  1995/12/29  10:27:33  cg
 * added readsib_tab
 *
 * Revision 1.30  1995/12/21  13:23:18  asi
 * changed dead_tab to dcr_tab and added active_tab
 *
 * Revision 1.29  1995/12/07  14:15:09  cg
 * removed DummyFun2
 * renamed DummyFun to TravSons
 *
 * Revision 1.28  1995/12/01  17:08:14  cg
 * new fun table 'precomp_tab'
 *
 * Revision 1.27  1995/11/16  19:38:34  cg
 * added new tab: rmvoid_tab.
 * NIF macro extended by 4 new parameters.
 *
 * Revision 1.26  1995/11/06  09:21:59  cg
 * added unique_tab
 *
 * Revision 1.25  1995/10/22  17:26:20  cg
 * added checkdec_tab and writedec_tab
 *  renamed sib_tab to writesib_tab
 *
 * Revision 1.24  1995/10/20  09:26:21  cg
 * added 'analy_tab`.
 *
 * Revision 1.23  1995/10/16  12:03:24  cg
 * added new function table objinit_tab.
 *
 * Revision 1.22  1995/10/05  16:03:28  cg
 * new traversal tab: impltype_tab
 *
 * Revision 1.21  1995/08/03  14:55:20  cg
 * sib_tab and obj_tab added.
 *
 * Revision 1.20  1995/07/24  11:47:35  asi
 * added ae_tab for array elimination
 *
 * Revision 1.19  1995/07/07  14:58:38  asi
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
extern node *TravSons (node *arg_node, node *arg_info);
extern node *NoTrav (node *arg_node, node *arg_info);

extern funptr *act_tab;

extern funptr imp_tab[];
extern funptr flat_tab[];
extern funptr print_tab[];
extern funptr type_tab[];
extern funptr opt_tab[];
extern funptr active_tab[];
extern funptr dcr_tab[];
extern funptr wlf_tab[];
extern funptr free_tab[];
extern funptr cf_tab[];
extern funptr refcnt_tab[];
extern funptr comp_tab[];
extern funptr lir_tab[];
extern funptr lir_mov_tab[];
extern funptr dup_tab[];
extern funptr inline_tab[];
extern funptr unroll_tab[];
extern funptr unswitch_tab[];
extern funptr idx_tab[];
extern funptr wli_tab[];
extern funptr ae_tab[];
extern funptr writesib_tab[];
extern funptr obj_tab[];
extern funptr impltype_tab[];
extern funptr objinit_tab[];
extern funptr analy_tab[];
extern funptr checkdec_tab[];
extern funptr writedec_tab[];
extern funptr unique_tab[];
extern funptr rmvoid_tab[];
extern funptr precomp_tab[];
extern funptr readsib_tab[];
extern funptr wlt_tab[];
extern funptr cse_tab[];
extern funptr dfr_tab[];
extern funptr concregs_tab[];
extern funptr o2nWith_tab[];

extern int nnode[];

#endif /* _sac_traverse_h */
