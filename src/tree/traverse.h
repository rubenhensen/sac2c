/*
 *
 * $Log$
 * Revision 1.3  2000/01/26 23:25:06  dkr
 * traverse-mechanism enhanced:
 * to each traverse-function-table a preprocessing- and a postprecesing-function
 * is added. These functions are called in Trav() just before and after the
 * traversal of each node.
 *
 * Revision 1.2  2000/01/24 18:21:51  jhs
 * Added new traversal and functions for schedule_init.[ch].
 *
 * Revision 1.1  2000/01/21 15:38:53  dkr
 * Initial revision
 *
 * Revision 2.11  2000/01/21 14:27:29  jhs
 * Added muth_tab ...
 *
 * Revision 2.10  2000/01/21 12:42:32  dkr
 * new traverse table for lac2fun added
 *
 * Revision 2.9  1999/10/19 17:09:50  sbs
 * ntc_tab added
 *
 * Revision 2.8  1999/08/27 11:05:59  jhs
 * Added spmdco_tab.
 * Deleted hundreds of disgusting blank lines making the code totally
 * unreadable and ugly.
 *
 * Revision 2.7  1999/08/05 13:34:25  jhs
 * Added spmdlc, spmddn and spmdpm.
 *
 * Revision 2.6  1999/08/04 14:28:38  bs
 * traverse funtab entry for tsi added.
 *
 * Revision 2.5  1999/07/28 13:00:09  jhs
 * Added: funptr spmdcons_tab[].
 *
 * Revision 2.4  1999/06/25 14:58:34  jhs
 * Splitted spmdtrav_tab into spmdrmtrav_tab and spmdrotrav_tab.
 *
 * Revision 2.3  1999/06/16 10:18:22  jhs
 * Added spmdtrav_tab.
 *
 * Revision 2.2  1999/05/10 10:55:25  bs
 *  tsi_tab renamed to wlaa_tab
 *
 * Revision 2.1  1999/02/23 12:39:47  sacbase
 * new release made
 *
 * Revision 1.52  1999/01/15 17:00:19  sbs
 * freemask_tab added
 *
 * Revision 1.51  1999/01/15 15:14:32  cg
 * added tsi_tab for new compiler module tile size inference.
 *
 * Revision 1.50  1999/01/07 14:02:33  sbs
 * new tab opt_tab inserted and old "opt_tab" renamed to genmask_tab!
 *
 * Revision 1.49  1998/06/18 13:42:11  cg
 * added traversal function tables conc_tab and sched_tab
 *
 * Revision 1.48  1998/06/07 18:36:55  dkr
 * added new fun_tab (reuse_tab)
 *
 * Revision 1.47  1998/05/11 09:43:44  cg
 * added syntax tree traversal for generating startup code.
 *
 * Revision 1.46  1998/04/29 19:54:19  dkr
 * added funptr tabs
 *
 * Revision 1.45  1998/04/28 15:44:01  srs
 * added tcwl_tab
 *
 * Revision 1.44  1998/04/23 18:58:25  dkr
 * added tabs
 * changed usage of NIF
 *
 * Revision 1.43  1998/04/17 17:28:32  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.42  1998/04/03 11:56:56  dkr
 * renamed concregs_tab to concregions_tab
 *
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

typedef struct FUNREC {
    funptr travtab[N_ok + 1];
    funptr prefun;
    funptr postfun;
} funtab;

extern node *Trav (node *arg_node, node *arg_info);
extern node *TravSons (node *arg_node, node *arg_info);
extern node *NoTrav (node *arg_node, node *arg_info);

extern funtab *act_tab;

extern funtab *imp_tab;
extern funtab *flat_tab;
extern funtab *print_tab;
extern funtab *type_tab;
extern funtab *genmask_tab;
extern funtab *freemask_tab;
extern funtab *active_tab;
extern funtab *dcr_tab;
extern funtab *wlf_tab;
extern funtab *free_tab;
extern funtab *cf_tab;
extern funtab *refcnt_tab;
extern funtab *comp_tab;
extern funtab *lir_tab;
extern funtab *lir_mov_tab;
extern funtab *dup_tab;
extern funtab *inline_tab;
extern funtab *unroll_tab;
extern funtab *unswitch_tab;
extern funtab *idx_tab;
extern funtab *wli_tab;
extern funtab *ae_tab;
extern funtab *writesib_tab;
extern funtab *obj_tab;
extern funtab *impltype_tab;
extern funtab *objinit_tab;
extern funtab *analy_tab;
extern funtab *checkdec_tab;
extern funtab *writedec_tab;
extern funtab *unique_tab;
extern funtab *rmvoid_tab;
extern funtab *precomp_tab;
extern funtab *readsib_tab;
extern funtab *wlt_tab;
extern funtab *cse_tab;
extern funtab *dfr_tab;
extern funtab *tcwl_tab;
extern funtab *wltrans_tab;
extern funtab *spmdinit_tab;
extern funtab *spmdopt_tab;
extern funtab *spmdlift_tab;
extern funtab *syncinit_tab;
extern funtab *syncopt_tab;
extern funtab *gsc_tab;
extern funtab *reuse_tab;
extern funtab *o2nWith_tab;
extern funtab *sched_tab;
extern funtab *conc_tab;
extern funtab *opt_tab;
extern funtab *wlaa_tab;
extern funtab *spmdrmtrav_tab;
extern funtab *spmdrotrav_tab;
extern funtab *spmdcons_tab;
extern funtab *tsi_tab;
extern funtab *spmdlc_tab;
extern funtab *spmddn_tab;
extern funtab *spmdpm_tab;
extern funtab *spmdco_tab;
extern funtab *ntc_tab;
extern funtab *lac2fun_tab;
extern funtab *muth_tab;
extern funtab *schin_tab;

extern int nnode[];

#endif /* _sac_traverse_h */
