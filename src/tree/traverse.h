/*
 *
 * $Log$
 * Revision 3.26  2002/06/07 17:18:24  mwe
 * Support for AssociativeLaw added
 *
 * Revision 3.25  2002/04/15 13:52:32  dkr
 * precomp4_tab added
 *
 * Revision 3.24  2002/04/09 08:11:50  ktr
 * Support for WithloopScalarization added.
 *
 * Revision 3.23  2002/03/05 15:52:04  sbs
 * CRTWRP-tab added
 *
 * Revision 3.22  2002/02/22 12:30:00  sbs
 * insvd_tab added.
 *
 * Revision 3.21  2002/02/21 15:47:04  dkr
 * new traversal-tab for precompile added
 *
 * Revision 3.20  2001/12/10 15:35:48  dkr
 * comp2_tab added
 *
 * Revision 3.19  2001/05/30 14:04:11  nmw
 * ssalil_tab traversal added (SSAInferLoopInvariants())
 *
 * Revision 3.18  2001/05/22 14:58:46  nmw
 * rmcasts traversal added
 *
 * Revision 3.17  2001/05/15 15:51:29  nmw
 * ssawli and ssawlf traversals added
 *
 * Revision 3.16  2001/05/14 15:59:55  nmw
 * ssawlt_tab added
 *
 * Revision 3.15  2001/04/26 17:08:53  dkr
 * rmvoid_tab removed
 *
 * Revision 3.14  2001/04/20 11:18:59  nmw
 * SSALUR traversal added
 *
 * Revision 3.13  2001/04/18 15:39:35  nmw
 * while2do added
 *
 * Revision 3.12  2001/04/09 15:55:08  nmw
 * partial traversal lirmov_tab added
 *
 * Revision 3.11  2001/03/26 15:36:32  nmw
 * SSALIR traversal added
 *
 * Revision 3.10  2001/03/20 16:15:18  nmw
 * SSACF Constant Folding on ssa from added
 *
 * Revision 3.9  2001/03/09 11:50:06  sbs
 * profile_tab added.
 *
 * Revision 3.8  2001/03/06 13:17:41  nmw
 * compare_tree added
 *
 * Revision 3.7  2001/03/05 16:18:45  nmw
 * SSACSE traversal added
 *
 * Revision 3.6  2001/02/23 13:39:50  nmw
 * SSADeadCodeRemoval added
 *
 * Revision 3.5  2001/02/22 12:42:12  nmw
 * UndoSSATransform traversal added
 *
 * Revision 3.4  2001/02/12 17:04:36  nmw
 * chkavis_tab added
 *
 * Revision 3.3  2000/12/06 20:12:25  dkr
 * l2f_infer... renamed into infdfms_...
 * l2f_lift... renamed into l2f...
 *
 * Revision 3.2  2000/12/06 18:26:04  cg
 * Added new traversal tccp for typecheck constant propagation.
 *
 * Revision 3.1  2000/11/20 18:03:30  sacbase
 * new release made
 *
 * Revision 1.22  2000/07/21 08:22:07  nmw
 * impspec_tab added
 *
 * Revision 1.21  2000/06/28 15:11:42  nmw
 * funtabs for PIH and PIW modified
 *
 * Revision 1.20  2000/06/23 16:40:17  nmw
 * pih and piw changed to mapcw and printi
 *
 * Revision 1.19  2000/06/13 13:40:43  dkr
 * O2NWith_tab renamed into patchwith_tab
 *
 * Revision 1.18  2000/06/08 15:29:45  nmw
 * pih_tab and piw_tab added
 *
 * Revision 1.17  2000/05/31 14:38:22  mab
 * renamed tables for array padding
 *
 * Revision 1.16  2000/05/31 11:25:22  mab
 * added traversal tables for array padding
 *
 * Revision 1.15  2000/05/29 14:29:50  dkr
 * a second traversal-table for precompile added
 *
 * Revision 1.14  2000/03/30 15:13:13  jhs
 * Added adjustcalls
 *
 * Revision 1.13  2000/03/29 16:10:31  jhs
 * blkli_tab added
 *
 * Revision 1.12  2000/03/22 17:36:19  jhs
 * Added N_MTsignal, N_MTalloc, N_MTsync and barin_tab.
 *
 * Revision 1.11  2000/03/17 15:57:47  dkr
 * cudecls_tab added
 *
 * Revision 1.10  2000/03/09 18:37:00  jhs
 * dfa, blkpp
 *
 * Revision 1.9  2000/03/02 13:05:38  jhs
 * Added blkco_tab, added functiobns for BLKCO and MTFIN.
 *
 * Revision 1.8  2000/02/23 23:03:44  dkr
 * second traversal table for LAC2FUN added
 *
 * Revision 1.7  2000/02/21 17:55:39  jhs
 * Added blkex_tab.
 * Added LOST OF UNUSED TRAVERSALS, THEY ARE ALL MINE >:[
 *
 * Revision 1.6  2000/02/17 16:21:13  cg
 * Added new tree traversal function tables fun2lac_tab and ai_tab.
 *
 * Revision 1.5  2000/02/04 14:46:53  jhs
 * Added rfin_tab and it's functions.
 * Added INFO_RFIN_xxx.
 *
 * Revision 1.4  2000/02/02 17:22:07  jhs
 * Added blkin_tab.
 *
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
 * [..., deleted 2000/01/04] jhs
 *
 * Revision 2.1  1999/02/23 12:39:47  sacbase
 * new release made
 *
 * Revision 1.52  1999/01/15 17:00:19  sbs
 * freemask_tab added
 *
 * [..., deleted 2000/01/04 jhs]
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
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
extern funtab *precomp1_tab;
extern funtab *precomp2_tab;
extern funtab *precomp3_tab;
extern funtab *precomp4_tab;
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
extern funtab *patchwith_tab;
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
extern funtab *muth_tab;
extern funtab *schin_tab;
extern funtab *blkin_tab;
extern funtab *rfin_tab;
extern funtab *fun2lac_tab;
extern funtab *ai_tab;
extern funtab *blkex_tab;
extern funtab *mtfin_tab;
extern funtab *infdfms_tab;
extern funtab *l2f_tab;
extern funtab *blkco_tab;
extern funtab *dfa_tab;
extern funtab *blkpp_tab;
extern funtab *cudecls_tab;
extern funtab *barin_tab;
extern funtab *blkli_tab;
extern funtab *adjca1_tab;
extern funtab *adjca2_tab;
extern funtab *apc_tab;
extern funtab *apt_tab;
extern funtab *mapcw_tab;
extern funtab *pih_tab;
extern funtab *piw_tab;
extern funtab *impspec_tab;
extern funtab *tccp_tab;
extern funtab *ssafrm_tab;
extern funtab *chkavis_tab;
extern funtab *undossa_tab;
extern funtab *ssadcr_tab;
extern funtab *ssacse_tab;
extern funtab *cmptree_tab;
extern funtab *profile_tab;
extern funtab *ssacf_tab;
extern funtab *ssalir_tab;
extern funtab *lirmov_tab;
extern funtab *w2d_tab;
extern funtab *ssalur_tab;
extern funtab *ssawlt_tab;
extern funtab *ssawli_tab;
extern funtab *ssawlf_tab;
extern funtab *rmcasts_tab;
extern funtab *ssaili_tab;
extern funtab *comp2_tab;
extern funtab *insvd_tab;
extern funtab *crtwrp_tab;
extern funtab *wls_tab;
extern funtab *al_tab;

extern int nnode[];

#endif /* _sac_traverse_h */
