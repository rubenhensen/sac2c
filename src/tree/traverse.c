/*
 *
 * $Log$
 * Revision 3.58  2004/07/16 14:41:34  sah
 * switch to new INFO structure
 * PHASE I
 *
 * Revision 3.57  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.56  2004/07/14 15:31:30  ktr
 * compiler phase emalloc added.
 *
 * Revision 3.55  2004/07/06 17:54:52  skt
 * support for propagate_executionmode (pem_tab) added
 *
 * Revision 3.54  2004/06/08 14:20:35  skt
 * support for tem_tab added
 *
 * Revision 3.53  2004/04/27 12:24:01  skt
 * some traversal functions for assignmentsrearrange added
 *
 * Revision 3.52  2004/04/22 14:13:12  skt
 * Added traversal asmra (assigmantrearrange)
 * for mt-mode 3
 *
 * Revision 3.51  2004/04/21 16:33:02  ktr
 * Added traversal ssarefcount
 *
 * Revision 3.50  2004/04/08 08:09:55  khf
 * wlfs_tab added
 *
 * Revision 3.49  2004/03/09 23:57:59  dkrHH
 * old backend removed
 *
 * Revision 3.48  2004/03/02 16:41:27  mwe
 * cvp_tab added
 *
 * Revision 3.47  2004/02/26 13:06:03  khf
 * wlpg_tab added
 *
 * Revision 3.46  2004/02/25 08:22:32  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 *
 * Revision 3.45  2003/09/17 12:55:27  sbs
 * typo eliminated
 *
 * Revision 3.44  2003/09/17 12:34:05  sbs
 * ts_tab added
 *
 * Revision 3.43  2003/08/16 08:45:54  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.42  2003/04/26 20:47:13  mwe
 * support for ElimSubDiv and UndoElimSubDiv added
 *
 * Revision 3.41  2003/03/09 17:15:00  ktr
 * added basic support for BLIR
 *
 * Revision 3.40  2003/02/08 16:50:57  mwe
 * remove copy and paste mistake
 *
 * Revision 3.39  2003/02/08 15:56:07  mwe
 * support for DistributiveLaw added
 *
 * Revision 3.38  2002/08/25 14:17:58  mwe
 * al_tab added.
 *
 * Revision 3.37  2002/08/13 10:23:32  sbs
 * hm_tab for handle_mops added.
 *
 * Revision 3.36  2002/08/09 13:16:22  dkr
 * cwc_tab added
 *
 * Revision 3.35  2002/08/05 17:03:45  sbs
 * several extensions required for the alpha version of the new type checker
 *
 * Revision 3.34  2002/07/09 12:53:19  sbs
 * hd_tab added
 *
 * Revision 3.33  2002/06/07 17:17:48  mwe
 * Support for AssociativeLaw added
 *
 * Revision 3.32  2002/04/15 13:53:03  dkr
 * precomp4_tab added
 *
 * Revision 3.31  2002/04/09 08:11:50  ktr
 * Support for WithloopScalarization added.
 *
 * Revision 3.30  2002/03/05 15:52:04  sbs
 * CRTWRP-tab added
 *
 * Revision 3.29  2002/02/22 12:30:00  sbs
 * insvd_tab added.
 *
 * Revision 3.28  2002/02/21 15:46:40  dkr
 * new traversal-tab for precompile added
 *
 * Revision 3.27  2001/12/10 15:36:11  dkr
 * comp2_tab added
 *
 * Revision 3.26  2001/07/13 13:23:41  cg
 * Some useless DBUG_PRINTs eliminated.
 *
 * Revision 3.25  2001/05/30 14:04:11  nmw
 * ssalil_tab traversal added (SSAInferLoopInvariants())
 *
 * Revision 3.24  2001/05/22 14:58:46  nmw
 * rmcasts traversal added
 *
 * Revision 3.23  2001/05/15 15:51:29  nmw
 * ssawli and ssawlf traversals added
 *
 * Revision 3.22  2001/05/14 15:59:55  nmw
 * ssawlt_tab added
 *
 * Revision 3.21  2001/04/26 17:08:46  dkr
 * rmvoid_tab removed
 *
 * Revision 3.20  2001/04/20 11:18:59  nmw
 * SSALUR traversal added
 *
 * Revision 3.19  2001/04/18 15:39:35  nmw
 * while2do added
 *
 * Revision 3.18  2001/04/09 15:55:08  nmw
 * partial traversal lirmov_tab added
 *
 * Revision 3.17  2001/04/04 15:12:04  nmw
 * dbug assert modified for better breakpoint
 *
 * Revision 3.16  2001/03/26 15:36:32  nmw
 * SSALIR traversal added
 *
 * Revision 3.15  2001/03/22 20:02:17  dkr
 * include of tree.h eliminated
 *
 * Revision 3.14  2001/03/20 16:15:18  nmw
 * SSACF Constant Folding on ssa from added
 *
 * Revision 3.13  2001/03/09 11:50:06  sbs
 * profile_tab added.
 *
 * Revision 3.12  2001/03/07 10:03:06  nmw
 * pretraversal function for cmptree_tab added
 *
 * Revision 3.11  2001/03/06 13:17:41  nmw
 * compare_tree added
 *
 * Revision 3.10  2001/03/05 16:18:45  nmw
 * SSACSE traversal added
 *
 * Revision 3.9  2001/02/23 13:39:50  nmw
 * SSADeadCodeRemoval added
 *
 * Revision 3.8  2001/02/22 12:42:12  nmw
 * UndoSSATransform traversal added
 *
 * Revision 3.7  2001/02/13 15:19:00  nmw
 * ssafrm_tab travseral added
 *
 * Revision 3.6  2001/02/12 17:04:36  nmw
 * chkavis_tab added
 *
 * Revision 3.5  2000/12/15 18:25:15  dkr
 * infer_dfms.h renamed into InferDFMs.h
 *
 * Revision 3.4  2000/12/06 20:12:18  dkr
 * l2f_infer... renamed into infdfms_...
 * l2f_lift... renamed into l2f...
 *
 * Revision 3.3  2000/12/06 19:22:02  dkr
 * PrintTravPre, PrintTravPost added
 * DupTreePre renamed into DupTreeTravPre
 * DupTreePost renamed into DupTreeTravPost
 *
 * Revision 3.2  2000/12/06 18:26:04  cg
 * Added new traversal tccp for typecheck constant propagation.
 *
 * Revision 3.1  2000/11/20 18:03:29  sacbase
 * new release made
 *
 * ... [eliminated] ...
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "flatten.h"
#include "insert_vardec.h"
#include "print.h"
#include "typecheck.h"
#include "typecheck_WL.h"
#include "optimize.h"
#include "free.h"
#include "generatemasks.h"
#include "DeadFunctionRemoval.h"
#include "import.h"
#include "DupTree.h"
#include "Inline.h"
#include "ArrayElimination.h"
#include "index.h"
#include "writesib.h"
#include "readsib.h"
#include "implicittypes.h"
#include "objinit.h"
#include "analysis.h"
#include "checkdec.h"
#include "objects.h"
#include "uniquecheck.h"
#include "refcount.h"
#include "alloc.h"
#include "wltransform.h"
#include "precompile.h"
#include "compile.h"
#include "ReuseWithArrays.h"
#include "cccall.h"
#include "PatchWith.h"
#include "WithloopScalarization.h"
#include "AssociativeLaw.h"
#include "DistributiveLaw.h"
#include "gen_startup_code.h"
#include "scheduling.h"
#include "concurrent.h"
#include "spmd_init.h"
#include "spmd_opt.h"
#include "spmd_lift.h"
#include "sync_init.h"
#include "sync_opt.h"
#include "spmd_trav.h"
#include "spmd_cons.h"
#include "schedule.h"
#include "wl_access_analyze.h"
#include "tile_size_inference.h"
#include "globals.h" /* needed for linenum only!!! */
#include "new_typecheck.h"
#include "multithread.h"
#include "InferDFMs.h"
#include "lac2fun.h"
#include "cleanup_decls.h"
#include "fun2lac.h"
#include "adjust_ids.h"
#include "schedule_init.h"
#include "blocks_init.h"
#include "blocks_expand.h"
#include "repfuns_init.h"
#include "mtfuns_init.h"
#include "blocks_cons.h"
#include "dataflow_analysis.h"
#include "blocks_propagate.h"
#include "barriers_init.h"
#include "tag_executionmode.h"
#include "propagate_executionmode.h"
#include "assignments_rearrange.h"
#include "blocks_lift.h"
#include "adjust_calls.h"
#include "pad_collect.h"
#include "pad_transform.h"
#include "print_interface_header.h"
#include "print_interface_wrapper.h"
#include "map_cwrapper.h"
#include "import_specialization.h"
#include "CheckAvis.h"
#include "SSATransform.h"
#include "UndoSSATransform.h"
#include "SSADeadCodeRemoval.h"
#include "SSACSE.h"
#include "compare_tree.h"
#include "annotate_fun_calls.h"
#include "SSAConstantFolding.h"
#include "SSALIR.h"
#include "SSALUR.h"
#include "SSAWLT.h"
#include "SSAWLI.h"
#include "SSAWLF.h"
#include "rmcasts.h"
#include "SSAInferLI.h"
#include "create_wrappers.h"
#include "handle_dots.h"
#include "new2old.h"
#include "create_wrapper_code.h"
#include "handle_mops.h"
#include "ElimSubDiv.h"
#include "UndoElimSubDiv.h"
#include "blir.h"
#include "SelectionPropagation.h"
#include "type_statistics.h"
#include "WLPartitionGeneration.h"
#include "ConstVarPropagation.h"
#include "WithloopFusion.h"

#include "traverse.h"

funtab *act_tab = NULL;

/*
 * global definitions of all funtabs needed for Trav
 */

/*
 *  (1) imp_tab
 */
static funtab imp_tab_rec = {{
#define NIFimp(it_imp) it_imp
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *imp_tab = &imp_tab_rec;

/*
 *  (2) flat_tab
 */
static funtab flat_tab_rec = {{
#define NIFflat(it_flat) it_flat
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *flat_tab = &flat_tab_rec;

/*
 *  (3) print_tab
 */
static funtab print_tab_rec = {{
#define NIFprint(it_print) it_print
#include "node_info.mac"
                               },
                               PrintTravPre,
                               PrintTravPost};
funtab *print_tab = &print_tab_rec;

/*
 *  (4) type_tab
 */
static funtab type_tab_rec = {{
#define NIFtype(it_type) it_type
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *type_tab = &type_tab_rec;

/*
 *  (5) genmask_tab
 */
static funtab genmask_tab_rec = {{
#define NIFgenmask(it_genmask) it_genmask
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *genmask_tab = &genmask_tab_rec;

/*
 *  (6) unused_tab3
 */
static funtab unused_tab3_rec = {{
#define NIFunused_3(it_unused_3) it_unused_3
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab3 = &unused_tab3_rec;

/*
 *  (7) unused_tab27
 */
static funtab unused_tab27_rec = {{
#define NIFunused_27(it_unused_27) it_unused_27
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab27 = &unused_tab27_rec;

/*
 *  (8) free_tab
 */
static funtab free_tab_rec = {{
#define NIFfree(it_free) it_free
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *free_tab = &free_tab_rec;

/*
 *  (9) unused_tab2
 */
static funtab unused_tab2_rec = {{
#define NIFunused_2(it_unused_2) it_unused_2
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab2 = &unused_tab2_rec;

/*
 *  (10) refcnt_tab
 */
static funtab refcnt_tab_rec = {{
#define NIFrefcnt(it_refcnt) it_refcnt
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *refcnt_tab = &refcnt_tab_rec;

/*
 *  (11) emrefcnt_tab
 */
static funtab emrefcnt_tab_rec = {{
#define NIFemrefcnt(it_emrefcnt) it_emrefcnt
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *emrefcnt_tab = &emrefcnt_tab_rec;

/*
 *  (12) unused_tab4
 */
static funtab unused_tab4_rec = {{
#define NIFunused_4(it_unused_4) it_unused_4
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab4 = &unused_tab4_rec;

/*
 *  (13) dup_tab
 */
static funtab dup_tab_rec = {{
#define NIFdup(it_dup) it_dup
#include "node_info.mac"
                             },
                             DupTreeTravPre,
                             DupTreeTravPost};
funtab *dup_tab = &dup_tab_rec;

/*
 *  (14) inline_tab
 */
static funtab inline_tab_rec = {{
#define NIFinline(it_inline) it_inline
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *inline_tab = &inline_tab_rec;

/*
 *  (15) unused_tab23
 */
static funtab unused_tab23_rec = {{
#define NIFunused_23(it_unused_23) it_unused_23
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab23 = &unused_tab23_rec;

/*
 *  (16) unused_tab21
 */
static funtab unused_tab21_rec = {{
#define NIFunused_21(it_unused_21) it_unused_21
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused21_tab = &unused_tab21_rec;

/*
 *  (17) idx_tab
 */
static funtab idx_tab_rec = {{
#define NIFidx(it_idx) it_idx
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *idx_tab = &idx_tab_rec;

/*
 *  (18) unused_tab24
 */
static funtab unused_tab24_rec = {{
#define NIFunused_24(it_unused_24) it_unused_24
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab24 = &unused_tab24_rec;

/*
 *  (19) unused_tab26
 */
static funtab unused_tab26_rec = {{
#define NIFunused_26(it_unused_26) it_unused_26
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab26 = &unused_tab26_rec;

/*
 *  (20) ae_tab
 */
static funtab ae_tab_rec = {{
#define NIFae(it_ae) it_ae
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *ae_tab = &ae_tab_rec;

/*
 *  (21) writesib_tab
 */
static funtab writesib_tab_rec = {{
#define NIFwritesib(it_writesib) it_writesib
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *writesib_tab = &writesib_tab_rec;

/*
 *  (22) obj_tab
 */
static funtab obj_tab_rec = {{
#define NIFobj(it_obj) it_obj
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *obj_tab = &obj_tab_rec;

/*
 *  (23) impltype_tab
 */
static funtab impltype_tab_rec = {{
#define NIFimpltype(it_impltype) it_impltype
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *impltype_tab = &impltype_tab_rec;

/*
 *  (24) objinit_tab
 */
static funtab objinit_tab_rec = {{
#define NIFobjinit(it_objinit) it_objinit
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *objinit_tab = &objinit_tab_rec;

/*
 *  (25) analy_tab
 */
static funtab analy_tab_rec = {{
#define NIFanaly(it_analy) it_analy
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *analy_tab = &analy_tab_rec;

/*
 *  (26) checkdec_tab
 */
static funtab checkdec_tab_rec = {{
#define NIFcheckdec(it_checkdec) it_checkdec
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *checkdec_tab = &checkdec_tab_rec;

/*
 *  (27) writedec_tab
 */
static funtab writedec_tab_rec = {{
#define NIFwritedec(it_writedec) it_writedec
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *writedec_tab = &writedec_tab_rec;

/*
 *  (28) unique_tab
 */
static funtab unique_tab_rec = {{
#define NIFunique(it_unique) it_unique
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *unique_tab = &unique_tab_rec;

/*
 *  (29) rmcasts_tab
 */
static funtab rmcasts_tab_rec = {{
#define NIFrmcasts(it_rmcasts) it_rmcasts
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *rmcasts_tab = &rmcasts_tab_rec;

/*
 *  (30) precomp2_tab
 */
static funtab precomp2_tab_rec = {{
#define NIFprecomp2(it_precomp2) it_precomp2
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *precomp2_tab = &precomp2_tab_rec;

/*
 *  (31) unused_TAB22
 */
static funtab unused_tab22_rec = {{
#define NIFunused_22(it_unused_22) it_unused_22
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab22 = &unused_tab22_rec;

/*
 *  (32) readsib_tab
 */
static funtab readsib_tab_rec = {{
#define NIFreadsib(it_readsib) it_readsib
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *readsib_tab = &readsib_tab_rec;

/*
 *  (33) unused_tab28
 */
static funtab unused_tab28_rec = {{
#define NIFunused_28(it_unused_28) it_unused_28
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab28 = &unused_tab28_rec;

/*
 *  (34) unused_tab1
 */
static funtab unused_tab1_rec = {{
#define NIFunused_1(it_unused_1) it_unused_1
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab1 = &unused_tab1_rec;

/*
 *  (35) dfr_tab
 */
static funtab dfr_tab_rec = {{
#define NIFdfr(it_dfr) it_dfr
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *dfr_tab = &dfr_tab_rec;

/*
 *  (36) tcwl_tab
 */
static funtab tcwl_tab_rec = {{
#define NIFtcwl(it_cwl) it_cwl
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *tcwl_tab = &tcwl_tab_rec;

/*
 *  (37) spmdinit_tab
 */
static funtab spmdinit_tab_rec = {{
#define NIFspmdinit(it_spmdinit) it_spmdinit
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *spmdinit_tab = &spmdinit_tab_rec;

/*
 *  (38) spmdopt_tab
 */
static funtab spmdopt_tab_rec = {{
#define NIFspmdopt(it_spmdopt) it_spmdopt
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *spmdopt_tab = &spmdopt_tab_rec;

/*
 *  (39) spmdlift_tab
 */
static funtab spmdlift_tab_rec = {{
#define NIFspmdlift(it_spmdlift) it_spmdlift
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *spmdlift_tab = &spmdlift_tab_rec;

/*
 *  (40) syncinit_tab
 */
static funtab syncinit_tab_rec = {{
#define NIFsyncinit(it_syncinit) it_syncinit
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *syncinit_tab = &syncinit_tab_rec;

/*
 *  (41) syncopt_tab
 */
static funtab syncopt_tab_rec = {{
#define NIFsyncopt(it_syncopt) it_syncopt
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *syncopt_tab = &syncopt_tab_rec;

/*
 *  (42) wltrans_tab
 */
static funtab wltrans_tab_rec = {{
#define NIFwltrans(it_wltrans) it_wltrans
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *wltrans_tab = &wltrans_tab_rec;

/*
 *  (43) gsc_tab
 */
static funtab gsc_tab_rec = {{
#define NIFgsc(it_gsc) it_gsc
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *gsc_tab = &gsc_tab_rec;

/*
 *  (44) reuse_tab
 */
static funtab reuse_tab_rec = {{
#define NIFreuse(it_reuse) it_reuse
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *reuse_tab = &reuse_tab_rec;

/*
 *  (45) patchwith_tab
 */
static funtab patchwith_tab_rec = {{
#define NIFpatchwith(it_patchwith) it_patchwith
#include "node_info.mac"
                                   },
                                   NULL,
                                   NULL};
funtab *patchwith_tab = &patchwith_tab_rec;

/*
 *  (46) sched_tab
 */
static funtab sched_tab_rec = {{
#define NIFsched(it_sched) it_sched
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *sched_tab = &sched_tab_rec;

/*
 *  (47) conc_tab
 */
static funtab conc_tab_rec = {{
#define NIFconc(it_conc) it_conc
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *conc_tab = &conc_tab_rec;

/*
 *  (48) opt_tab
 */
static funtab opt_tab_rec = {{
#define NIFopt(it_opt) it_opt
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *opt_tab = &opt_tab_rec;

/*
 *  (49) wlaa_tab
 */
static funtab wlaa_tab_rec = {{
#define NIFwlaa(it_wlaa) it_wlaa
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *wlaa_tab = &wlaa_tab_rec;

/*
 *  (50) unused_tab25
 */
static funtab unused_tab25_rec = {{
#define NIFunused_25(it_unused_25) it_unused_25
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused25_tab = &unused_tab25_rec;

/*
 *  (51) spmdrmtrav_tab
 */
static funtab spmdrmtrav_tab_rec = {{
#define NIFspmdrm(it_spmdrm) it_spmdrm
#include "node_info.mac"
                                    },
                                    NULL,
                                    NULL};
funtab *spmdrmtrav_tab = &spmdrmtrav_tab_rec;

/*
 *  (52) spmdrotrav_tab
 */
static funtab spmdrotrav_tab_rec = {{
#define NIFspmdro(it_spmdro) it_spmdro
#include "node_info.mac"
                                    },
                                    NULL,
                                    NULL};
funtab *spmdrotrav_tab = &spmdrotrav_tab_rec;

/*
 *  (53) spmdcons_tab
 */
static funtab spmdcons_tab_rec = {{
#define NIFspmdcons(it_spmdcons) it_spmdcons
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *spmdcons_tab = &spmdcons_tab_rec;

/*
 *  (54) tsi_tab
 */
static funtab tsi_tab_rec = {{
#define NIFtsi(it_tsi) it_tsi
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *tsi_tab = &tsi_tab_rec;

/*
 *  (55) spmdlc_tab
 */
static funtab spmdlc_tab_rec = {{
#define NIFspmdlc(it_spmdlc) it_spmdlc
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *spmdlc_tab = &spmdlc_tab_rec;

/*
 *  (56) spmddn_tab
 */
static funtab spmddn_tab_rec = {{
#define NIFspmddn(it_spmddn) it_spmddn
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *spmddn_tab = &spmddn_tab_rec;

/*
 *  (57) spmdpm_tab
 */
static funtab spmdpm_tab_rec = {{
#define NIFspmdpm(it_spmdpm) it_spmdpm
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *spmdpm_tab = &spmdpm_tab_rec;

/*
 *  (58) spmdco_tab
 */
static funtab spmdco_tab_rec = {{
#define NIFspmdco(it_spmdco) it_spmdco
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *spmdco_tab = &spmdco_tab_rec;

/*
 *  (59) ntc_tab
 */
static funtab ntc_tab_rec = {{
#define NIFntc(it_ntc) it_ntc
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *ntc_tab = &ntc_tab_rec;

/*
 *  (60) muth_tab
 */
static funtab muth_tab_rec = {{
#define NIFmuth(it_muth) it_muth
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *muth_tab = &muth_tab_rec;

/*
 *  (61) schin_tab
 */
static funtab schin_tab_rec = {{
#define NIFschin(it_schin) it_schin
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *schin_tab = &schin_tab_rec;

/*
 *  (62) blkin_tab
 */
static funtab blkin_tab_rec = {{
#define NIFblkin(it_blkin) it_blkin
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *blkin_tab = &blkin_tab_rec;

/*
 *  (63) rfin_tab
 */
static funtab rfin_tab_rec = {{
#define NIFrfin(it_rfin) it_rfin
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *rfin_tab = &rfin_tab_rec;

/*
 *  (64) fun2lac_tab
 */
static funtab fun2lac_tab_rec = {{
#define NIFfun2lac(it_fun2lac) it_fun2lac
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *fun2lac_tab = &fun2lac_tab_rec;

/*
 *  (65) ai_tab
 */
static funtab ai_tab_rec = {{
#define NIFai(it_ai) it_ai
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *ai_tab = &ai_tab_rec;

/*
 *  (66) blkex_tab
 */
static funtab blkex_tab_rec = {{
#define NIFblkex(it_blkex) it_blkex
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *blkex_tab = &blkex_tab_rec;

/*
 *  (67) mtfin_tab
 */
static funtab mtfin_tab_rec = {{
#define NIFmtfin(it_mtfin) it_mtfin
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *mtfin_tab = &mtfin_tab_rec;

/*
 *  (68) infdfms_tab
 */
static funtab infdfms_tab_rec = {{
#define NIFinfdfms(it_infdfms) it_infdfms
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *infdfms_tab = &infdfms_tab_rec;

/*
 *  (69) l2f_tab
 */
static funtab l2f_tab_rec = {{
#define NIFl2f(it_l2f) it_l2f
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *l2f_tab = &l2f_tab_rec;

/*
 *  (70) blkco_tab
 */
static funtab blkco_tab_rec = {{
#define NIFblkco(it_blkco) it_blkco
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *blkco_tab = &blkco_tab_rec;

/*
 *  (71) dfa_tab
 */
static funtab dfa_tab_rec = {{
#define NIFdfa(it_dfa) it_dfa
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *dfa_tab = &dfa_tab_rec;

/*
 *  (72) blkpp_tab
 */
static funtab blkpp_tab_rec = {{
#define NIFblkpp(it_blkpp) it_blkpp
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *blkpp_tab = &blkpp_tab_rec;

/*
 *  (73) cudecls_tab
 */
static funtab cudecls_tab_rec = {{
#define NIFcudecls(it_cudecls) it_cudecls
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *cudecls_tab = &cudecls_tab_rec;

/*
 *  (74) barin_tab
 */
static funtab barin_tab_rec = {{
#define NIFbarin(it_barin) it_barin
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *barin_tab = &barin_tab_rec;

/*
 *  (75) blkli_tab
 */
static funtab blkli_tab_rec = {{
#define NIFblkli(it_blkli) it_blkli
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *blkli_tab = &blkli_tab_rec;

/*
 *  (76) adjca1_tab
 */
static funtab adjca1_tab_rec = {{
#define NIFadjca1(it_adjca1) it_adjca1
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *adjca1_tab = &adjca1_tab_rec;

/*
 *  (77) adjca2_tab
 */
static funtab adjca2_tab_rec = {{
#define NIFadjca2(it_adjca2) it_adjca2
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *adjca2_tab = &adjca2_tab_rec;

/*
 *  (78) precomp1_tab
 */
static funtab precomp1_tab_rec = {{
#define NIFprecomp1(it_precomp1) it_precomp1
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *precomp1_tab = &precomp1_tab_rec;

/*
 *  (79) apc_tab
 */
static funtab apc_tab_rec = {{
#define NIFapc(it_apc) it_apc
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *apc_tab = &apc_tab_rec;

/*
 *  (80) apt_tab
 */
static funtab apt_tab_rec = {{
#define NIFapt(it_apt) it_apt
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *apt_tab = &apt_tab_rec;

/*
 *  (81) mapcw_tab
 */
static funtab mapcw_tab_rec = {{
#define NIFmapcw(it_macpw) it_macpw
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *mapcw_tab = &mapcw_tab_rec;

/*
 *  (82) pih_tab
 */
static funtab pih_tab_rec = {{
#define NIFpih(it_pih) it_pih
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *pih_tab = &pih_tab_rec;

/*
 *  (83) piw_tab
 */
static funtab piw_tab_rec = {{
#define NIFpiw(it_piw) it_piw
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *piw_tab = &piw_tab_rec;

/*
 *  (84) impspec_tab
 */
static funtab impspec_tab_rec = {{
#define NIFimpspec(it_impspec) it_impspec
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *impspec_tab = &impspec_tab_rec;

/*
 *  (85) tccp_tab
 */
static funtab tccp_tab_rec = {{
#define NIFtccp(it_tccp) it_tccp
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *tccp_tab = &tccp_tab_rec;

/*
 *  (86) ssafrm_tab
 */
static funtab ssafrm_tab_rec = {{
#define NIFssafrm(it_ssafrm) it_ssafrm
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssafrm_tab = &ssafrm_tab_rec;

/*
 *  (87) chkavis_tab
 */
static funtab chkavis_tab_rec = {{
#define NIFchkavis(it_chkavis) it_chkavis
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *chkavis_tab = &chkavis_tab_rec;

/*
 *  (88) undossa_tab
 */
static funtab undossa_tab_rec = {{
#define NIFundossa(it_undossa) it_undossa
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *undossa_tab = &undossa_tab_rec;

/*
 *  (89) ssadcr_tab
 */
static funtab ssadcr_tab_rec = {{
#define NIFssadcr(it_ssadcr) it_ssadcr
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssadcr_tab = &ssadcr_tab_rec;

/*
 *  (90) ssacse_tab
 */
static funtab ssacse_tab_rec = {{
#define NIFssacse(it_ssacse) it_ssacse
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssacse_tab = &ssacse_tab_rec;

/*
 *  (91) cmptree_tab
 */
static funtab cmptree_tab_rec = {{
#define NIFcmptree(it_cmptree) it_cmptree
#include "node_info.mac"
                                 },
                                 CMPTnodeType,
                                 NULL};
funtab *cmptree_tab = &cmptree_tab_rec;

/*
 *  (92) profile_tab
 */
static funtab profile_tab_rec = {{
#define NIFprofile(it_profile) it_profile
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *profile_tab = &profile_tab_rec;

/*
 *  (93) ssacf_tab
 */
static funtab ssacf_tab_rec = {{
#define NIFssacf(it_ssacf) it_ssacf
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *ssacf_tab = &ssacf_tab_rec;

/*
 *  (94) ssalir_tab
 */
static funtab ssalir_tab_rec = {{
#define NIFssalir(it_ssalir) it_ssalir
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssalir_tab = &ssalir_tab_rec;

/*
 *  (95) lirmov_tab
 */
static funtab lirmov_tab_rec = {{
#define NIFlirmov(it_lirmov) it_lirmov
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *lirmov_tab = &lirmov_tab_rec;

/*
 *  (96) wlpg_tab
 */
static funtab wlpg_tab_rec = {{
#define NIFwlpg(it_wlpg) it_wlpg
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *wlpg_tab = &wlpg_tab_rec;

/*
 *  (97) ssalur_tab
 */
static funtab ssalur_tab_rec = {{
#define NIFssalur(it_ssalur) it_ssalur
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssalur_tab = &ssalur_tab_rec;

/*
 *  (98) ssawlt_tab
 */
static funtab ssawlt_tab_rec = {{
#define NIFssawlt(it_ssawlt) it_ssawlt
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssawlt_tab = &ssawlt_tab_rec;

/*
 *  (99) ssawli_tab
 */
static funtab ssawli_tab_rec = {{
#define NIFssawli(it_ssawli) it_ssawli
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssawli_tab = &ssawli_tab_rec;

/*
 *  (100) ssawlf_tab
 */
static funtab ssawlf_tab_rec = {{
#define NIFssawlf(it_ssawlf) it_ssawlf
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssawlf_tab = &ssawlf_tab_rec;

/*
 *  (101) ssaili_tab
 */
static funtab ssaili_tab_rec = {{
#define NIFssaili(it_ssaili) it_ssaili
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *ssaili_tab = &ssaili_tab_rec;

/*
 *  (102) comp_tab
 */
static funtab comp_tab_rec = {{
#define NIFcomp(it_comp) it_comp
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *comp_tab = &comp_tab_rec;

/*
 *  (103) precomp3_tab
 */
static funtab precomp3_tab_rec = {{
#define NIFprecomp3(it_precomp3) it_precomp3
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *precomp3_tab = &precomp3_tab_rec;

/*
 *  (104) insvd_tab
 */
static funtab insvd_tab_rec = {{
#define NIFinsvd(it_insvd) it_insvd
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *insvd_tab = &insvd_tab_rec;

/*
 *  (105) crtwrp_tab
 */
static funtab crtwrp_tab_rec = {{
#define NIFcrtwrp(it_crtwrp) it_crtwrp
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *crtwrp_tab = &crtwrp_tab_rec;

/*
 *  (106) wls_tab
 */
static funtab wls_tab_rec = {{
#define NIFwls(it_wls) it_wls
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *wls_tab = &wls_tab_rec;

/*
 *  (107) precomp4_tab
 */
static funtab precomp4_tab_rec = {{
#define NIFprecomp4(it_precomp4) it_precomp4
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *precomp4_tab = &precomp4_tab_rec;

/*
 *  (108) al_tab
 */
static funtab al_tab_rec = {{
#define NIFal(it_al) it_al
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *al_tab = &al_tab_rec;

/*
 *  (109) hd_tab
 */
static funtab hd_tab_rec = {{
#define NIFhd(it_hd) it_hd
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *hd_tab = &hd_tab_rec;

/*
 *  (110) nt2ot_tab
 */
static funtab nt2ot_tab_rec = {{
#define NIFnt2ot(it_nt2ot) it_nt2ot
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *nt2ot_tab = &nt2ot_tab_rec;

/*
 *  (111) cwc_tab
 */
static funtab cwc_tab_rec = {{
#define NIFcwc(it_cwc) it_cwc
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *cwc_tab = &cwc_tab_rec;

/*
 *  (112) nt2ot_tab
 */
static funtab hm_tab_rec = {{
#define NIFhm(it_hm) it_hm
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *hm_tab = &hm_tab_rec;

/*
 *  (113) dl_tab
 */
static funtab dl_tab_rec = {{
#define NIFdl(it_dl) it_dl
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *dl_tab = &dl_tab_rec;

/*
 *  (114) blir_tab
 */
static funtab blir_tab_rec = {{
#define NIFblir(it_blir) it_blir
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *blir_tab = &blir_tab_rec;

/*
 *  (115) esd_tab
 */
static funtab esd_tab_rec = {{
#define NIFesd(it_esd) it_esd
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *esd_tab = &esd_tab_rec;

/*
 *  (116) uesd_tab
 */
static funtab uesd_tab_rec = {{
#define NIFuesd(it_uesd) it_uesd
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *uesd_tab = &uesd_tab_rec;

/*
 *  (117) sp_tab
 */
static funtab sp_tab_rec = {{
#define NIFsp(it_sp) it_sp
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *sp_tab = &sp_tab_rec;

/*
 *  (118) ts_tab
 */
static funtab ts_tab_rec = {{
#define NIFts(it_ts) it_ts
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *ts_tab = &ts_tab_rec;

/*
 *  (119) cvp_tab
 */
static funtab cvp_tab_rec = {{
#define NIFcvp(it_cvp) it_cvp
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *cvp_tab = &cvp_tab_rec;

/*
 *  (120) wlfs_tab
 */
static funtab wlfs_tab_rec = {{
#define NIFwlfs(it_wlfs) it_wlfs
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *wlfs_tab = &wlfs_tab_rec;

/*
 *  (121) asmra_tab
 */
static funtab asmra_tab_rec = {{
#define NIFasmra(it_asmra) it_asmra
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *asmra_tab = &asmra_tab_rec;

/*
 *  (122) tem_tab
 */
static funtab tem_tab_rec = {{
#define NIFtem(it_tem) it_tem
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *tem_tab = &tem_tab_rec;

/*
 *  (123) pem_tab
 */
static funtab pem_tab_rec = {{
#define NIFpem(it_pem) it_pem
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *pem_tab = &pem_tab_rec;

/*
 *  (124) emalloc_tab
 */
static funtab emalloc_tab_rec = {{
#define NIFemalloc(it_emalloc) it_emalloc
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *emalloc_tab = &emalloc_tab_rec;

/*
 *  (125) unused_tab5
 */
static funtab unused_tab5_rec = {{
#define NIFunused_5(it_unused_5) it_unused_5
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab5 = &unused_tab5_rec;

/*
 *  (126) unused_tab6
 */
static funtab unused_tab6_rec = {{
#define NIFunused_6(it_unused_6) it_unused_6
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab6 = &unused_tab6_rec;

/*
 *  (127) unused_tab7
 */
static funtab unused_tab7_rec = {{
#define NIFunused_7(it_unused_7) it_unused_7
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab7 = &unused_tab7_rec;

/*
 *  (128) unused_tab8
 */
static funtab unused_tab8_rec = {{
#define NIFunused_8(it_unused_8) it_unused_8
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab8 = &unused_tab8_rec;

/*
 *  (129) unused_tab9
 */
static funtab unused_tab9_rec = {{
#define NIFunused_9(it_unused_9) it_unused_9
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *unused_tab9 = &unused_tab9_rec;

/*
 *  (130) unused_tab10
 */
static funtab unused_tab10_rec = {{
#define NIFunused_10(it_unused_10) it_unused_10
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab10 = &unused_tab10_rec;

/*
 *  (131) unused_tab11
 */
static funtab unused_tab11_rec = {{
#define NIFunused_11(it_unused_11) it_unused_11
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab11 = &unused_tab11_rec;

/*
 *  (132) unused_tab12
 */
static funtab unused_tab12_rec = {{
#define NIFunused_12(it_unused_12) it_unused_12
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab12 = &unused_tab12_rec;

/*
 *  (133) unused_tab13
 */
static funtab unused_tab13_rec = {{
#define NIFunused_13(it_unused_13) it_unused_13
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab13 = &unused_tab13_rec;

/*
 *  (134) unused_tab14
 */
static funtab unused_tab14_rec = {{
#define NIFunused_14(it_unused_14) it_unused_14
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab14 = &unused_tab14_rec;

/*
 *  (135) unused_tab15
 */
static funtab unused_tab15_rec = {{
#define NIFunused_15(it_unused_15) it_unused_15
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab15 = &unused_tab15_rec;

/*
 *  (136) unused_tab16
 */
static funtab unused_tab16_rec = {{
#define NIFunused_16(it_unused_16) it_unused_16
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab16 = &unused_tab16_rec;

/*
 *  (137) unused_tab17
 */
static funtab unused_tab17_rec = {{
#define NIFunused_17(it_unused_17) it_unused_17
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab17 = &unused_tab17_rec;

/*
 *  (138) unused_tab18
 */
static funtab unused_tab18_rec = {{
#define NIFunused_18(it_unused_18) it_unused_18
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab18 = &unused_tab18_rec;

/*
 *  (139) unused_tab19
 */
static funtab unused_tab19_rec = {{
#define NIFunused_19(it_unused_19) it_unused_19
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab19 = &unused_tab19_rec;

/*
 *  (140) unused_tab20
 */
static funtab unused_tab20_rec = {{
#define NIFunused_20(it_unused_20) it_unused_20
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unused_tab20 = &unused_tab20_rec;

/*
 *  nnode
 */
#define NIFnnode(nnode) nnode
int nnode[] = {
#include "node_info.mac"
};

/*
**
**  functionname  : Trav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : higher order traverse function; selects particular
**                  function from the globaly available funtab
**                  pointed to by "act_tab" with respect to the
**                  actual node-type.
**  global vars   : act_tab
**  internal funs : ---
**  external funs : all funs from flatten.c
**
**  remarks       : could be done as macro as well!!
**
*/

node *
Trav (node *arg_node, info *arg_info)
{
    int old_linenum = linenum;
    char *old_filename = filename;
    node *result;

    DBUG_ENTER ("Trav");

#ifndef DBUG_OFF
    /*
     * This ifndef construction is ugly(!) but it simplifies debugging
     * because it allows to set break points "within" the assertions...
     */

    if (arg_node == NULL) {
        DBUG_ASSERT (0, "Trav: tried to traverse into subtree NULL !");
    }

    if (NODE_TYPE (arg_node) >= N_ok) {
        DBUG_ASSERT (0, "Trav: illegal node type !");
    }

#endif /* not DBUG_OFF */

    /*
     * Make sure the line-number will be set
     * correctly in case MakeXxx is called.
     */
    linenum = NODE_LINE (arg_node);
    filename = NODE_FILE (arg_node);

    if (*act_tab->prefun != NULL) {
        arg_node = (*act_tab->prefun) (arg_node, arg_info);
    }

    result = (*act_tab->travtab[NODE_TYPE (arg_node)]) (arg_node, arg_info);

    if (act_tab->postfun != NULL) {
        result = (*act_tab->postfun) (result, arg_info);
    }

    linenum = old_linenum;
    filename = old_filename;

    DBUG_RETURN (result);
}

/*
**
**  functionname  : TravSons
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : traverses all son nodes.
**  global vars   : ---
**  internal funs : Trav
**  external funs : ---
**
**  remarks       : TravSons can be used as dummy function for fun_tab entries
**                  where a specific function for a particular node type is
**                  not yet implemented or not necessary.
**
*/

node *
TravSons (node *arg_node, info *arg_info)
{
    int i;

    DBUG_ENTER ("TravSons");

    for (i = 0; i < nnode[NODE_TYPE (arg_node)]; i++)
        if (arg_node->node[i] != NULL)
            arg_node->node[i] = Trav (arg_node->node[i], arg_info);

    DBUG_RETURN (arg_node);
}

/*
**
**  functionname  : NoTrav
**  arguments     : 1) pointer to actual node
**                  2) pointer to further (top down) info's
**  description   : does nothing on the given syntax tree,
**                  especially no further sons are traversed.
**                  The given son is returned unmodified.
**  global vars   : ---
**  internal funs : ---
**  external funs : ---
**
**  remarks       : NoTrav can be used as fun_tab entry where no further
**                  traversal of the syntax tree is needed in order
**                  to avoid unnecessary work.
**
*/

node *
TravNone (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TravNone");
    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *  node *TravError(node *arg_node, info *arg_info)
 *
 * description:
 *
 *  This function can be used whenever a certain node type is illegal in
 *  some compiler traversal phase. It helps to detect consistency failures
 *  in the abstract syntax tree.
 *
 ******************************************************************************/

node *
TravError (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TravError");

    DBUG_ASSERT ((FALSE), "Illegal node type found.");

    DBUG_RETURN (arg_node);
}
