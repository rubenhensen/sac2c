/*
 *
 * $Log$
 * Revision 1.5  2000/02/03 17:47:38  dkr
 * Trav(): instruction DBUG_ASSERT(arg_node==NULL) now *before* the first
 * access to arg_node :)
 *
 * Revision 1.4  2000/02/02 17:22:07  jhs
 * Added blkin_tab.
 *
 * Revision 1.3  2000/01/26 23:22:59  dkr
 * traverse-mechanism enhanced:
 * to each traverse-function-table a preprocessing- and a postprecesing-function
 * is added. These functions are called in Trav() just before and after the
 * traversal of each node.
 *
 * Revision 1.2  2000/01/24 18:21:51  jhs
 * Added new traversal and functions for schedule_init.[ch].
 *
 * Revision 1.1  2000/01/21 15:38:52  dkr
 * Initial revision
 *
 * Revision 2.14  2000/01/21 14:27:29  jhs
 * Added muth_tab ...
 *
 * Revision 2.13  2000/01/21 12:42:14  dkr
 * new traverse table for lac2fun added
 *
 * Revision 2.12  1999/10/19 17:09:50  sbs
 * ntc_tab added
 *
 * Revision 2.11  1999/08/27 11:05:59  jhs
 * Added spmdco_tab.
 * Deleted hundreds of disgusting blank lines making the code totally
 * unreadable and ugly.
 *
 * Revision 2.10  1999/08/05 13:34:25  jhs
 * Added spmdlc, spmddn and spmdpm.
 *
 * Revision 2.9  1999/08/04 14:28:38  bs
 * traverse funtab entry for tsi added.
 *
 * Revision 2.8  1999/07/28 13:00:09  jhs
 * Added: funptr spmdcons_tab[].
 *
 * Revision 2.7  1999/07/23 17:17:46  jhs
 * Restructured node_info.mac and simplified it's usage.
 *
 * Revision 2.6  1999/06/25 14:58:34  jhs
 * Splitted spmdtrav_tab into spmdrmtrav_tab and spmdrotrav_tab.
 *
 * Revision 2.5  1999/06/16 10:18:22  jhs
 * Added spmdtrav_tab.
 *
 * Revision 2.4  1999/05/10 10:54:13  bs
 * tsi_tab renamed to wlaa_tab
 *
 * Revision 2.3  1999/05/06 15:35:42  sbs
 * filename- mechanism added for creating better error-messages
 * when multiple files are involved!
 *
 * Revision 2.2  1999/05/06 12:10:12  sbs
 * implicit linenum-setting mechanism added to Trav !!
 * => Makexxx-calls might yield better line-numbers now...
 *
 * Revision 2.1  1999/02/23 12:39:44  sacbase
 * new release made
 *
 * Revision 1.71  1999/01/15 17:00:19  sbs
 * freemask_tab added
 *
 * Revision 1.70  1999/01/15 15:14:32  cg
 * added tsi_tab for new compiler module tile size inference.
 *
 * Revision 1.69  1999/01/07 14:02:33  sbs
 * new tab opt_tab inserted and old "opt_tab" renamed to genmask_tab!
 *
 * Revision 1.68  1998/10/29 16:56:31  cg
 * Implementation of Trav() slightly modified in order to
 * simplify debugging.
 *
 * Revision 1.67  1998/06/18 13:42:11  cg
 * added traversal function tables conc_tab and sched_tab
 *
 * Revision 1.66  1998/06/07 18:36:36  dkr
 * added new fun_tab (reuse_tab)
 *
 * Revision 1.65  1998/05/11 09:43:44  cg
 * added syntax tree traversal for generating startup code.
 *
 * Revision 1.64  1998/04/29 19:53:22  dkr
 * added funptr tabs
 *
 * Revision 1.63  1998/04/28 15:43:52  srs
 * added tcwl_tab
 *
 * Revision 1.62  1998/04/23 18:58:33  dkr
 * added tabs
 * changed usage of NIF
 *
 * Revision 1.61  1998/04/17 17:28:29  dkr
 * 'concurrent regions' are now called 'SPMD regions'
 *
 * Revision 1.60  1998/04/03 11:27:55  dkr
 * concregs renamed to concregions
 *
 * ... [eliminated] ...
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"

#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "typecheck_WL.h"
#include "optimize.h"
#include "free.h"
#include "generatemasks.h"
#include "freemasks.h"
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "DeadFunctionRemoval.h"
#include "LoopInvariantRemoval.h"
#include "CSE.h"
#include "import.h"
#include "DupTree.h"
#include "Inline.h"
#include "Unroll.h"
#include "Unswitch.h"
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
#include "rmvoidfun.h"
#include "refcount.h"
#include "wltransform.h"
#include "precompile.h"
#include "compile.h"
#include "ReuseWithArrays.h"
#include "cccall.h"
#include "Old2NewWith.h"
#include "WithloopFolding.h"
#include "WLT.h"
#include "WLI.h"
#include "WLF.h"
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
#include "lac2fun.h"
#include "schedule_init.h"
#include "blocks_init.h"

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
                               NULL,
                               NULL};
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
 *  (6) dead_tab
 */
static funtab dcr_tab_rec = {{
#define NIFdcr(it_dcr) it_dcr
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *dcr_tab = &dcr_tab_rec;

/*
 *  (7) wlf_tab
 */
static funtab wlf_tab_rec = {{
#define NIFwlf(it_wlf) it_wlf
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *wlf_tab = &wlf_tab_rec;

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
 *  (9) cf_tab
 */
static funtab cf_tab_rec = {{
#define NIFcf(it_cf) it_cf
#include "node_info.mac"
                            },
                            NULL,
                            NULL};
funtab *cf_tab = &cf_tab_rec;

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
 *  (11) comp_tab
 */
static funtab comp_tab_rec = {{
#define NIFcomp(it_comp) it_comp
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *comp_tab = &comp_tab_rec;

/*
 *  (12) lir_tab
 */
static funtab lir_tab_rec = {{
#define NIFlir(it_lir) it_lir
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *lir_tab = &lir_tab_rec;

/*
 *  (13) dup_tab
 */
static funtab dup_tab_rec = {{
#define NIFdup(it_dup) it_dup
#include "node_info.mac"
                             },
                             DupTreePre,
                             DupTreePost};
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
 *  (15) unroll_tab
 */
static funtab unroll_tab_rec = {{
#define NIFunroll(it_unroll) it_unroll
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *unroll_tab = &unroll_tab_rec;

/*
 *  (16) lir_mov_tab
 */
static funtab lir_mov_tab_rec = {{
#define NIFlir_mov(it_lir_mov) it_lir_mov
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *lir_mov_tab = &lir_mov_tab_rec;

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
 *  (18) unswitch_tab
 */
static funtab unswitch_tab_rec = {{
#define NIFunswitch(it_unswitch) it_unswitch
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *unswitch_tab = &unswitch_tab_rec;

/*
 *  (19) wli_tab
 */
static funtab wli_tab_rec = {{
#define NIFwli(it_wli) it_wli
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *wli_tab = &wli_tab_rec;

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
 *  (29) rmvoid_tab
 */
static funtab rmvoid_tab_rec = {{
#define NIFrmvoid(it_rmvoid) it_rmvoid
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *rmvoid_tab = &rmvoid_tab_rec;

/*
 *  (30) precomp_tab
 */
static funtab precomp_tab_rec = {{
#define NIFprecomp(it_precomp) it_precomp
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *precomp_tab = &precomp_tab_rec;

/*
 *  (31) active_tab
 */
static funtab active_tab_rec = {{
#define NIFactive(it_active) it_active
#include "node_info.mac"
                                },
                                NULL,
                                NULL};
funtab *active_tab = &active_tab_rec;

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
 *  (33) wlt_tab
 */
static funtab wlt_tab_rec = {{
#define NIFwlt(it_wlt) it_wlt
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *wlt_tab = &wlt_tab_rec;

/*
 *  (34) cse_tab
 */
static funtab cse_tab_rec = {{
#define NIFcse(it_cse) it_cse
#include "node_info.mac"
                             },
                             NULL,
                             NULL};
funtab *cse_tab = &cse_tab_rec;

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
 *  (45) o2nWith_tab
 */
static funtab o2nWith_tab_rec = {{
#define NIFo2nWith(it_o2nWith) it_o2nWith
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *o2nWith_tab = &o2nWith_tab_rec;

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
 *  (50) freemask_tab
 */
static funtab freemask_tab_rec = {{
#define NIFfreemask(it_freemask) it_freemask
#include "node_info.mac"
                                  },
                                  NULL,
                                  NULL};
funtab *freemask_tab = &freemask_tab_rec;

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
 *  (60) lac2fun_tab
 */
static funtab lac2fun_tab_rec = {{
#define NIFlac2fun(it_lac2fun) it_lac2fun
#include "node_info.mac"
                                 },
                                 NULL,
                                 NULL};
funtab *lac2fun_tab = &lac2fun_tab_rec;

/*
 *  (61) muth_tab
 */
static funtab muth_tab_rec = {{
#define NIFmuth(it_muth) it_muth
#include "node_info.mac"
                              },
                              NULL,
                              NULL};
funtab *muth_tab = &muth_tab_rec;

/*
 *  (62) schin_tab
 */
static funtab schin_tab_rec = {{
#define NIFschin(it_schin) it_schin
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *schin_tab = &schin_tab_rec;

/*
 *  (63) blkin_tab
 */
static funtab blkin_tab_rec = {{
#define NIFblkin(it_blkin) it_blkin
#include "node_info.mac"
                               },
                               NULL,
                               NULL};
funtab *blkin_tab = &blkin_tab_rec;

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
Trav (node *arg_node, node *arg_info)
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

    DBUG_PRINT ("TRAV", ("case %s: node adress: %06x", mdb_nodetype[NODE_TYPE (arg_node)],
                         arg_node));
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
TravSons (node *arg_node, node *arg_info)
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
NoTrav (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("NoTrav");
    DBUG_RETURN (arg_node);
}
