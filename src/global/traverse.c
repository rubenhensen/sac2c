/*
 *
 * $Log$
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
#include "scnprs.h" /* needed for linenum only!!! */

#include "traverse.h"

funptr *act_tab;

/*
 *
 * global definitions of all funtabs needed for Trav
 *
 *
 * 1) imp_tab
 */
#define NIFimp(it_imp) it_imp
funptr imp_tab[] = {
#include "node_info.mac"
};

/*
 * 2) flat_tab
 */
#define NIFflat(it_flat) it_flat
funptr flat_tab[] = {
#include "node_info.mac"
};

/*
 * 3) print_tab
 */

#define NIFprint(it_print) it_print
funptr print_tab[] = {
#include "node_info.mac"
};

/*
 * 4) type_tab
 */

#define NIFtype(it_type) it_type
funptr type_tab[] = {
#include "node_info.mac"
};

/*
 * 5) genmask_tab
 */

#define NIFgenmask(it_genmask) it_genmask
funptr genmask_tab[] = {
#include "node_info.mac"
};

/*
 * 6) dead_tab
 */

#define NIFdcr(it_dcr) it_dcr
funptr dcr_tab[] = {
#include "node_info.mac"
};

/*
 * 7) wlf_tab
 */

#define NIFwlf(it_wlf) it_wlf
funptr wlf_tab[] = {
#include "node_info.mac"
};

/*
 * 8) free_tab
 */

#define NIFfree(it_free) it_free
funptr free_tab[] = {
#include "node_info.mac"
};

/*
 * 9) cf_tab
 */

#define NIFcf(it_cf) it_cf
funptr cf_tab[] = {
#include "node_info.mac"
};

/*
 * 10) refcnt_tab
 */

#define NIFrefcnt(it_refcnt) it_refcnt
funptr refcnt_tab[] = {
#include "node_info.mac"
};

/*
 * 11) comp_tab
 */

#define NIFcomp(it_comp) it_comp
funptr comp_tab[] = {
#include "node_info.mac"
};

/*
 * 12) lir_tab
 */

#define NIFlir(it_lir) it_lir
funptr lir_tab[] = {
#include "node_info.mac"
};

/*
 * 13) dup_tab
 */

#define NIFdup(it_dup) it_dup
funptr dup_tab[] = {
#include "node_info.mac"
};

/*
 * 14) inline_tab
 */

#define NIFinline(it_inline) it_inline
funptr inline_tab[] = {
#include "node_info.mac"
};

/*
 * 15) unroll_tab
 */

#define NIFunroll(it_unroll) it_unroll
funptr unroll_tab[] = {
#include "node_info.mac"
};

/*
 * 16) lir_mov_tab
 */

#define NIFlir_mov(it_lir_mov) it_lir_mov
funptr lir_mov_tab[] = {
#include "node_info.mac"
};

/*
 * 17) idx_tab
 */

#define NIFidx(it_idx) it_idx
funptr idx_tab[] = {
#include "node_info.mac"
};

/*
 * 18) unswitch_tab
 */

#define NIFunswitch(it_unswitch) it_unswitch
funptr unswitch_tab[] = {
#include "node_info.mac"
};

/*
 * 19) wli_tab
 */

#define NIFwli(it_wli) it_wli
funptr wli_tab[] = {
#include "node_info.mac"
};

/*
 * 20) ae_tab
 */

#define NIFae(it_ae) it_ae
funptr ae_tab[] = {
#include "node_info.mac"
};

/*
 * 21) writesib_tab
 */

#define NIFwritesib(it_writesib) it_writesib
funptr writesib_tab[] = {
#include "node_info.mac"
};

/*
 * 22) obj_tab
 */

#define NIFobj(it_obj) it_obj
funptr obj_tab[] = {
#include "node_info.mac"
};

/*
 * 23) impltype_tab
 */

#define NIFimpltype(it_impltype) it_impltype
funptr impltype_tab[] = {
#include "node_info.mac"
};

/*
 * 24) objinit_tab
 */

#define NIFobjinit(it_objinit) it_objinit
funptr objinit_tab[] = {
#include "node_info.mac"
};

/*
 * 25) analy_tab
 */

#define NIFanaly(it_analy) it_analy
funptr analy_tab[] = {
#include "node_info.mac"
};
#undef NIF

/*
 * 26) checkdec_tab
 */

#define NIFcheckdec(it_checkdec) it_checkdec
funptr checkdec_tab[] = {
#include "node_info.mac"
};

/*
 * 27) writedec_tab
 */

#define NIFwritedec(it_writedec) it_writedec
funptr writedec_tab[] = {
#include "node_info.mac"
};

/*
 * 28) unique_tab
 */

#define NIFunique(it_unique) it_unique
funptr unique_tab[] = {
#include "node_info.mac"
};

/*
 * 29) rmvoid_tab
 */

#define NIFrmvoid(it_rmvoid) it_rmvoid
funptr rmvoid_tab[] = {
#include "node_info.mac"
};

/*
 * 30) precomp_tab
 */

#define NIFprecomp(it_precomp) it_precomp
funptr precomp_tab[] = {
#include "node_info.mac"
};

/*
 * 31) active_tab
 */

#define NIFactive(it_active) it_active
funptr active_tab[] = {
#include "node_info.mac"
};

/*
 * 32) readsib_tab
 */

#define NIFreadsib(it_readsib) it_readsib
funptr readsib_tab[] = {
#include "node_info.mac"
};

/*
 * 33) wlt_tab
 */

#define NIFwlt(it_wlt) it_wlt
funptr wlt_tab[] = {
#include "node_info.mac"
};

/*
 * 34) cse_tab
 */

#define NIFcse(it_cse) it_cse
funptr cse_tab[] = {
#include "node_info.mac"
};

/*
 * 35) dfr_tab
 */

#define NIFdfr(it_dfr) it_dfr
funptr dfr_tab[] = {
#include "node_info.mac"
};

/*
 * 36) tcwl_tab
 */

#define NIFtcwl(it_cwl) it_cwl
funptr tcwl_tab[] = {
#include "node_info.mac"
};

/*
 * 37) spmdinit_tab
 */

#define NIFspmdinit(it_spmdinit) it_spmdinit
funptr spmdinit_tab[] = {
#include "node_info.mac"
};

/*
 * 38) spmdopt_tab
 */

#define NIFspmdopt(it_spmdopt) it_spmdopt
funptr spmdopt_tab[] = {
#include "node_info.mac"
};

/*
 * 39) spmdlift_tab
 */

#define NIFspmdlift(it_spmdlift) it_spmdlift
funptr spmdlift_tab[] = {
#include "node_info.mac"
};

/*
 * 40) syncinit_tab
 */

#define NIFsyncinit(it_syncinit) it_syncinit
funptr syncinit_tab[] = {
#include "node_info.mac"
};

/*
 * 41) syncopt_tab
 */

#define NIFsyncopt(it_syncopt) it_syncopt
funptr syncopt_tab[] = {
#include "node_info.mac"
};

/*
 * 42) wltrans_tab
 */

#define NIFwltrans(it_wltrans) it_wltrans
funptr wltrans_tab[] = {
#include "node_info.mac"
};

/*
 * 43) gsc_tab
 */

#define NIFgsc(it_gsc) it_gsc
funptr gsc_tab[] = {
#include "node_info.mac"
};

/*
 * 44) reuse_tab
 */

#define NIFreuse(it_reuse) it_reuse
funptr reuse_tab[] = {
#include "node_info.mac"
};

/*
 * 45) o2nWith_tab
 */

#define NIFo2nWith(it_o2nWith) it_o2nWith
funptr o2nWith_tab[] = {
#include "node_info.mac"
};

/*
 * 46) sched_tab
 */

#define NIFsched(it_sched) it_sched
funptr sched_tab[] = {
#include "node_info.mac"
};

/*
 * 47) conc_tab
 */

#define NIFconc(it_conc) it_conc
funptr conc_tab[] = {
#include "node_info.mac"
};

/*
 * 48) opt_tab
 */

#define NIFopt(it_opt) it_opt
funptr opt_tab[] = {
#include "node_info.mac"
};

/*
 * 49) wlaa_tab
 */

#define NIFwlaa(it_wlaa) it_wlaa
funptr wlaa_tab[] = {
#include "node_info.mac"
};

/*
 *  50) freemask_tab
 */

#define NIFfreemask(it_freemask) it_freemask
funptr freemask_tab[] = {
#include "node_info.mac"
};

/*
 *  51) spmdrmtrav_tab
 */

#define NIFspmdrm(it_spmdrm) it_spmdrm
funptr spmdrmtrav_tab[] = {
#include "node_info.mac"
};

/*
 *  52) spmdrotrav_tab
 */

#define NIFspmdro(it_spmdro) it_spmdro
funptr spmdrotrav_tab[] = {
#include "node_info.mac"
};

/*
 *  53) spmdcons_tab
 */

#define NIFspmdcons(it_spmdcons) it_spmdcons
funptr spmdcons_tab[] = {
#include "node_info.mac"
};

/*
 *  54) tsi_tab
 */

#define NIFtsi(it_tsi) it_tsi
funptr tsi_tab[] = {
#include "node_info.mac"
};

/*
 *  55) spmdlc_tab
 */

#define NIFspmdlc(it_spmdlc) it_spmdlc
funptr spmdlc_tab[] = {
#include "node_info.mac"
};

/*
 *  56) spmddn_tab
 */

#define NIFspmddn(it_spmddn) it_spmddn
funptr spmddn_tab[] = {
#include "node_info.mac"
};

/*
 *  57) spmddn_tab
 */

#define NIFspmdpm(it_spmdpm) it_spmdpm
funptr spmdpm_tab[] = {
#include "node_info.mac"
};

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

    /*
     * Make sure the line-number will be set
     * correctly in case MakeXxx is called.
     */
    linenum = NODE_LINE (arg_node);
    filename = NODE_FILE (arg_node);

#ifndef DBUG_OFF
    /*
     * This ifndef construction is ugly(!) but it simplifies debugging
     * because it allows to set break points "within" the assertions...
     */

    if (arg_node == NULL) {
        DBUG_ASSERT (0, "Trav: tried to traverse into subtree NULL !");
    }

    if (arg_node->nodetype >= N_ok) {
        DBUG_ASSERT (0, "Trav: illegal node type !");
    }

    DBUG_PRINT ("TRAV", ("case %s: node adress: %06x", mdb_nodetype[arg_node->nodetype],
                         arg_node));
#endif /* not DBUG_OFF */

    result = (*act_tab[arg_node->nodetype]) (arg_node, arg_info);
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
