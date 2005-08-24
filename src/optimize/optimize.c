/*
 *
 * $Log$
 * Revision 3.101  2005/08/24 10:26:01  ktr
 * added wlidxs traversal
 *
 * Revision 3.100  2005/08/20 23:42:47  ktr
 * added IVEI
 *
 * Revision 3.99  2005/08/20 12:06:50  ktr
 * added TypeConvElimination
 *
 * Revision 3.98  2005/07/19 17:08:26  ktr
 * replaced SSADeadCodeRemoval with deadcoderemoval
 *
 * Revision 3.97  2005/07/16 21:14:24  sbs
 * moved dispatch and rmcasts into WLEnhancement.c
 *
 * Revision 3.96  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.95  2005/07/15 15:23:08  ktr
 * removed type conversions before and after IVE
 *
 * Revision 3.94  2005/06/28 15:38:25  jhb
 * added phase.h by the includes
 *
 * Revision 3.93  2005/06/06 13:28:40  jhb
 * added PHrunCompilerSubPhase
 *
 * Revision 3.92  2005/06/02 13:42:48  mwe
 * rerun optimization cycle if sisi found optimization cases
 *
 * Revision 3.91  2005/05/31 13:41:38  mwe
 * run sisi only when dcr activated
 *
 * Revision 3.90  2005/05/25 09:52:33  khf
 * optimize normal functions instead of zombie functions
 *
 * Revision 3.89  2005/05/13 16:46:15  ktr
 * lacinlining is now performed in the cycle
 *
 * Revision 3.88  2005/04/20 19:14:31  ktr
 * removed SSArestoreSsaOneFunction after LIR
 *
 * Revision 3.87  2005/04/19 17:59:10  khf
 * removed transformation in ssa-form
 *
 * Revision 3.86  2005/04/19 17:26:09  ktr
 * "lacinl" break specifier introduced
 *
 * Revision 3.85  2005/03/17 19:05:17  sbs
 * IVE still runs on old types.....
 *
 * Revision 3.84  2005/03/17 14:03:11  sah
 * removed global check for function body
 *
 * Revision 3.83  2005/03/10 09:41:09  cg
 * Added #include "DupTree.h"
 *
 * Revision 3.82  2005/03/04 21:21:42  cg
 * Useless conditional eliminated.
 * Integration of silently duplicated LaC funs at the end of the
 * fundef chain added.
 *
 * Revision 3.81  2005/02/16 14:11:09  mwe
 * some renaming done, corrected break specifier
 *
 * Revision 3.80  2005/02/15 14:53:00  mwe
 * changes for esd and uesd
 *
 * Revision 3.79  2005/02/14 11:18:34  cg
 * Old inlining replaced by complete re-implementation.
 *
 * Revision 3.78  2005/02/11 12:13:11  mwe
 * position of sisi changed
 *
 * Revision 3.77  2005/02/03 18:28:22  mwe
 * new counter added
 * order of intrafunctional optimization changed
 * optimization output changed
 * some code beautifying
 *
 * Revision 3.76  2005/02/02 18:09:50  mwe
 * new counter added
 * signature simplification added
 *
 * Revision 3.75  2005/01/27 18:20:30  mwe
 * new counter for type_upgrade added
 *
 * Revision 3.74  2005/01/11 12:58:15  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.73  2004/12/09 13:08:54  mwe
 * type_upgrade now running before constant folding
 *
 * Revision 3.72  2004/12/09 10:59:30  mwe
 * support for type_upgrade added
 *
 * Revision 3.71  2004/11/27 01:48:24  jhb
 * comment out WLAdoAccessAnalysis and ApdoArrayPadding
 *
 * Revision 3.70  2004/11/26 18:14:35  mwe
 * change prefix of IVE
 *
 * Revision 3.69  2004/11/26 16:27:36  mwe
 * SacDevCamp
 *
 * Revision 3.68  2004/10/07 12:38:00  ktr
 * Replaced the old With-Loop Scalarization with a new implementation.
 *
 * Revision 3.67  2004/10/05 13:52:33  sah
 * disabled DistributiveLaw in NEW_AST mode
 *
 * Revision 3.66  2004/09/28 14:08:21  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.65  2004/07/22 15:09:09  ktr
 * updated call to WithloopScalarization
 *
 * Revision 3.64  2004/07/19 14:24:52  sah
 * removed useless second argument
 * to ArrayElimination
 *
 * Revision 3.63  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 3.62  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.61  2004/06/30 12:13:39  khf
 * wlpg_expr removed, comments modified
 *
 * Revision 3.60  2004/05/04 15:47:20  khf
 * appliance of SSACSE and SSADCR after WLFS added
 *
 * Revision 3.59  2004/04/08 08:09:55  khf
 * support for wlfs and wlpg added but are currently
 * deactivated in global.c
 *
 * Revision 3.58  2004/03/02 16:49:49  mwe
 * support for CVP added
 *
 * Revision 3.57  2004/02/25 15:53:06  cg
 * New functions RestoreSSAOneFunction and RestoreSSAOneFundef
 * now provide access to SSA transformations on a per function
 * basis.
 * Only functions from ssa.[ch] should be used to initiate the
 * transformation process in either direction!
 *
 * Revision 3.56  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.55  2004/02/02 16:00:57  skt
 * use DoSSA and UndoSSA instead of calling Lac2Fun, SSATransform etc.
 *
 * Revision 3.54  2003/08/16 08:44:25  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.53  2003/07/31 17:56:59  mwe
 * separate SSA-F
 * f
 *
 * separate SSA-form-based opimizations from Non-SSA-form-based optimizations
 *
 * Revision 3.52  2003/06/17 11:45:33  ktr
 * Quick fix of SSALoopInvariantRemoval by inserting SSATransformOneFunction afterwards.
 *
 * Revision 3.51  2003/05/20 16:29:56  ktr
 * Moved WLS before WLUR.
 *
 * Revision 3.50  2003/05/19 06:39:01  ktr
 * removed the condition mentioned before for the same results were produced
 * as with -wls_aggressive
 *
 * Revision 3.49  2003/05/18 21:58:23  ktr
 * Inserted a condition to ensure that (W)LUR is applied no earlier than the fourth
 * optimization cycle.
 *
 * Revision 3.48  2003/05/18 13:23:15  ktr
 * no changes done, experimented around with order of WLS and LUR.
 *
 * Revision 3.47  2003/04/26 20:45:38  mwe
 * ElimSubDiv and UndoElimSubDiv added
 *
 * Revision 3.46  2003/03/26 15:42:31  sbs
 * *** empty log message ***
 *
 * Revision 3.45  2003/03/26 15:22:36  sbs
 * doxygen again.
 *
 * Revision 3.44  2003/03/26 14:56:40  sbs
 * further doxygen
 *
 * Revision 3.43  2003/03/26 14:42:09  sbs
 * doxygenic
 *
 * Revision 3.42  2003/03/26 14:17:01  sbs
 * defgroup Optimizations and file included
 *
 * Revision 3.41  2003/03/12 16:29:47  dkr
 * comment modified
 *
 * Revision 3.40  2003/02/08 16:00:01  mwe
 * support for DistributiveLaw added
 *
 * Revision 3.39  2002/10/24 13:13:35  ktr
 * level of WLS aggressiveness now controlled by flag -wls_aggressive
 *
 * Revision 3.38  2002/10/18 14:15:53  ktr
 * use of WLS now depends on wls_aggressive > 0
 *
 * Revision 3.37  2002/10/14 12:08:19  mwe
 * counter for AL-optimize added
 *
 * Revision 3.36  2002/06/07 17:15:33  mwe
 * AssociativeLaw added
 *
 * Revision 3.35  2002/03/13 16:03:20  ktr
 * Withloop-Scalarization added to optimization-cycle
 *
 * Revision 3.34  2001/07/13 13:23:41  cg
 * Unused function GenOptVar eliminated.
 *
 * Revision 3.33  2001/05/31 10:51:26  sbs
 * Generate Mask required prior to CF3 in case of non-ssa!!!
 * otherwise wrong results in NASmg are produced ...
 *
 * Revision 3.32  2001/05/30 15:54:13  nmw
 * comment on cf3 added
 *
 * Revision 3.31  2001/05/30 14:02:14  nmw
 * SSAInfereLoopInvariant added, CF after LUR added
 *
 * Revision 3.30  2001/05/23 15:49:32  nmw
 * break specifier after fundef optimization cycle
 *
 * Revision 3.29  2001/05/22 14:54:17  nmw
 * call to RemoveCasts() in front of all optimizations added
 *
 * Revision 3.28  2001/05/17 12:46:31  nmw
 * MALLOC/FREE changed to Malloc/Free, result of Free() used
 *
 * Revision 3.27  2001/05/16 09:53:58  nmw
 * missing ) added :-)
 *
 * Revision 3.26  2001/05/16 09:23:46  nmw
 * removed unnecessary SSATransform and GenerateMasks to improve performance
 *
 * Revision 3.25  2001/05/15 08:02:29  nmw
 * call of SSAWithloopFoldingWLT added
 *
 * Revision 3.24  2001/05/09 12:29:40  nmw
 * removed all unused SSATransform operations between the several
 * optimizations
 *
 * Revision 3.23  2001/05/07 09:04:20  nmw
 * call to Unroll() in ssa optimizations removed. the withloop
 * unrolling is done be SSALUR now (it uses WLUnroll.c)
 *
 * Revision 3.22  2001/04/30 12:15:13  nmw
 * no separate optimizations of special fundefs anymore
 * Unroll() added behind SSALoopUnrolling to get WLUR
 *
 * Revision 3.21  2001/04/25 12:14:02  nmw
 * moved call of WLUR to SSALoopUnrolling
 *
 * Revision 3.20  2001/04/24 16:09:52  nmw
 * SingleFundef calls renamed to OneFunction calls
 *
 * Revision 3.19  2001/04/20 11:17:29  nmw
 * SSALoopUnrolling added to OPTfundef cycle
 *
 * Revision 3.18  2001/04/19 16:34:14  nmw
 * statistics for wlir added
 *
 * Revision 3.17  2001/04/19 11:45:36  nmw
 * calling while2do when using ssa form opts
 *
 * Revision 3.16  2001/04/18 12:56:13  nmw
 * call SSATransform/CheckAvis for single function instead of whole ast
 * to have a little speedup ;-)
 *
 * Revision 3.15  2001/04/04 23:21:52  dkr
 * warning message about max_optcyc modified
 *
 * Revision 3.14  2001/04/02 11:09:14  nmw
 * handling for multiple used special functions added
 * the MODUL_FUNS son is not written back after traversal
 *
 * Revision 3.13  2001/03/27 13:49:09  dkr
 * signature of Inline() modified
 *
 * Revision 3.12  2001/03/26 15:56:18  nmw
 * SSALoopInvarinatRemoval added
 *
 * Revision 3.11  2001/03/22 21:06:47  dkr
 * include of tree.h eliminated
 *
 * Revision 3.10  2001/03/22 14:30:47  nmw
 * SSAConstantFolding added
 *
 * Revision 3.9  2001/03/15 17:01:39  dkr
 * OPTfundef: FREE(tmp_str) added
 *
 * Revision 3.8  2001/03/15 14:21:53  nmw
 * break specifier for UndoSSA changed from "usd" to "ussa" according
 * to DEBUG specifier for this traversal
 *
 * Revision 3.7  2001/03/07 15:58:04  nmw
 * SSACSE for ssa optimization added
 *
 * Revision 3.6  2001/02/27 16:06:27  nmw
 * SSADeadCodeRemoval with -ssa switch added
 *
 * Revision 3.5  2001/02/22 12:50:43  nmw
 * UndoSSATransform after OPTfundef added
 *
 * Revision 3.4  2001/02/20 15:54:24  nmw
 * ssa-transformation for debugging included
 *
 * Revision 3.3  2001/02/15 17:09:14  nmw
 * ssa transformation switch and break specifier implemented
 *
 * Revision 3.2  2000/11/23 16:51:24  sbs
 * old_xxx_expr vars in OPT_fundef (superfluously) initialized to avoid
 * compiler warnings in product version.
 *
 * Revision 3.1  2000/11/20 18:00:44  sacbase
 * new release made
 *
 * Revision 2.25  2000/10/31 18:10:16  cg
 * Added additional break specifier -b15:dfr2.
 *
 * Revision 2.24  2000/10/31 18:08:09  cg
 *
 * Revision 2.23  2000/10/27 11:39:09  cg
 * Slightly modified layout of status messages during optimization.
 *
 * Revision 2.22  2000/10/17 17:08:14  dkr
 * -noWLT now skips the WLT in the second cycle, too
 *
 * Revision 2.21  2000/09/29 14:52:01  sbs
 * no changes
 *
 * Revision 2.20  2000/08/01 13:17:45  dkr
 * check of (break_cycle_specifier == 0) added for breaks before the
 * optimization cycle.
 *
 * Revision 2.19  2000/08/01 11:57:30  dkr
 * bug in break handling fixed
 *
 * Revision 2.18  2000/07/28 13:21:47  dkr
 * a break in OPTfundef is reported to OPTmoduls now
 *
 * Revision 2.17  2000/07/28 08:33:30  mab
 * fixed minor bug in ResetCounters
 *
 * Revision 2.16  2000/07/25 11:54:28  mab
 * added ap_padded and ap_unsupported
 *
 * Revision 2.15  2000/07/11 15:52:29  dkr
 * IVE added
 *
 * Revision 2.14  2000/05/29 14:28:49  dkr
 * include of pad.h added
 *
 * Revision 2.13  2000/05/26 14:22:52  sbs
 * WLAA and TSI lifted on module level
 * call to ArrayPadding added.
 *
 * Revision 2.12  2000/01/26 17:26:53  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.10  1999/10/28 19:39:16  dkr
 * DBUG-string MASK changed to PRINT_MASKS
 *
 * Revision 2.9  1999/08/30 14:09:36  bs
 * The tile size inference is activated now.
 *
 * Revision 2.8  1999/05/12 14:35:16  cg
 * Optimizations are now triggered by bit field optimize instead
 * of single individual int variables.
 *
 * Revision 2.7  1999/05/10 11:08:55  bs
 * New optimization phase 'WLAccessAnalize' added.
 *
 * Revision 2.6  1999/04/13 14:02:56  cg
 * function GetExpr() removed.
 *
 * Revision 2.5  1999/04/11 10:35:16  bs
 * TSI call moved to the 'right' place.
 *
 * Revision 2.4  1999/03/15 15:42:52  bs
 * temporary DBUG-Flag inserted
 *
 * Revision 2.3  1999/03/15 14:06:10  bs
 * Access macros renamed (take a look at tree_basic.h).
 *
 * Revision 2.2  1999/02/28 21:06:21  srs
 * removed DBUG output for WLF
 *
 * Revision 2.1  1999/02/23 12:41:53  sacbase
 * new release made
 *
 * ... [elminated] ...
 *
 * Revision 1.1  1994/12/09  10:47:40  sbs
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "ctinfo.h"
#include "dbug.h"
#include "node_basic.h"
#include "traverse.h"
#include "print.h"
#include "convert.h"
#include "new_types.h"
#include "DupTree.h"
#include "phase.h"

#include "optimize.h"
#include "DeadFunctionRemoval.h"
#include "inlining.h"
#include "lacinlining.h"
#include "ArrayElimination.h"
#include "wls.h"
#include "AssociativeLaw.h"
#include "DistributiveLaw.h"
#include "associativity.h"
#include "wl_access_analyze.h"
#include "tile_size_inference.h"
#include "index.h"
#include "index_infer.h"
#include "pad.h"
#include "ssa.h"
#include "deadcoderemoval.h"
#include "SSACSE.h"
#include "SSAConstantFolding.h"
#include "SSALIR.h"
#include "SSALUR.h"
#include "SSAWithloopFolding.h"
#include "rmcasts.h"
#include "SSAInferLI.h"
#include "ElimSubDiv.h"
#include "UndoElimSubDiv.h"
#include "ConstVarPropagation.h"
#include "WLPartitionGeneration.h"
#include "WithloopFusion.h"
#include "type_upgrade.h"
#include "signature_simplification.h"
#include "dispatchfuncalls.h"
#include "elimtypeconv.h"

#include "ToOldTypes.h"
#include "ToNewTypes.h"

/*
 * INFO structure
 */

typedef enum { OS_initial, OS_cycle1, OS_precycle2, OS_cycle2, OS_final } optstage_t;

struct INFO {
    node *module;
    optstage_t stage;
    bool cont;
    int ploop1;
    int ploop2;
};

/*
 * INFO macros
 */
#define INFO_OPT_MODULE(n) (n->module)
#define INFO_OPT_OPTSTAGE(n) (n->stage)
#define INFO_OPT_CONTINUE(n) (n->cont)
#define INFO_OPT_PASSESLOOP1(n) (n->ploop1)
#define INFO_OPT_PASSESLOOP2(n) (n->ploop2)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_OPT_MODULE (result) = NULL;
    INFO_OPT_OPTSTAGE (result) = OS_initial;
    INFO_OPT_PASSESLOOP1 (result) = 0;
    INFO_OPT_PASSESLOOP2 (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/**
 *
 * @defgroup opt Optimizations
 *
 * This group contains all those files/ modules that apply optimizations
 * on the level of SaC code.
 *
 * @{
 */

/**
 *
 * @file optimize.c
 *
 * This file contains the code for steering the sub phases of the high-level
 * optimizations.
 *
 */

int dead_expr;
int dead_var;
int dead_fun;
int cf_expr;
int lir_expr;
int wlir_expr;
int lunr_expr;
int wlunr_expr;
int uns_expr;
int elim_arrays;
int inl_fun;
int optvar_counter;
int cse_expr;
int wlf_expr;
int wlt_expr;
int wls_expr;
int old_wlf_expr, old_wlt_expr;
int ap_padded;
int ap_unsupported;
int al_expr;
int etc_expr;
int dl_expr;
int sp_expr;
int cvp_expr;
int wlfs_expr;
int tup_tu_expr;
int tup_rtu_expr;
int tup_wdp_expr;
int tup_fdp_expr;
int tup_fsp_expr;
int sisi_expr;

/**
 *
 * Global variable needed for the correct handling of break specifiers.
 * If a break is found during the call of OPTfundef() this fact must be
 * reported to OPTmodul() in order to skip the remaining optimization
 * steps of OPTmodul(), too.
 */
bool do_break = FALSE;

/**
 *
 * @name Functions for optimization statistics:
 *
 * <!--
 * void ResetCounters()       : resets all global optimization counters
 * void PrintStatistics(....) : prints all counters (given as args explicitly!)
 * -->
 *
 *@{
 */

/** <!--********************************************************************-->
 *
 * @fn void ResetCounters()
 *
 *   @brief sets all global optimization counters to zero.
 *
 ******************************************************************************/
static void
ResetCounters ()
{
    DBUG_ENTER ("ResetCounters");

    dead_expr = 0;
    dead_var = 0;
    dead_fun = 0;
    lir_expr = 0;
    wlir_expr = 0;
    cf_expr = 0;
    lunr_expr = 0;
    wlunr_expr = 0;
    uns_expr = 0;
    inl_fun = 0;
    elim_arrays = 0;
    wlf_expr = 0;
    wlt_expr = 0;
    wls_expr = 0;
    cse_expr = 0;
    ap_padded = 0;
    ap_unsupported = 0;
    al_expr = 0;
    etc_expr = 0;
    dl_expr = 0;
    sp_expr = 0;
    cvp_expr = 0;
    wlfs_expr = 0;
    tup_tu_expr = 0;
    tup_rtu_expr = 0;
    tup_wdp_expr = 0;
    tup_fdp_expr = 0;
    tup_fsp_expr = 0;
    sisi_expr = 0;

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void PrintStatistics(int off_inl_fun, int off_dead_expr, int off_dead_var,
 *                          int off_dead_fun, int off_lir_expr,
 *                          int off_wlir_expr, int off_cf_expr,
 *                          int off_lunr_expr, int off_wlunr_expr, int off_uns_expr,
 *                          int off_elim_arrays, int off_wlf_expr, int off_wlt_expr,
 *                          int off_cse_expr, int off_ap_padded,
 *                          int off_ap_unsupported, int off_wls_expr, int off_al_expr,
 *                          int off_etc_expr, int off_dl_expr,
 *                          int off_sp_expr, int off_cvp_expr, int off_wlfs_expr,
 *                          int off_tup_tu_expr, int off_tup_wdp_expr,
 *                          int off_tup_fdp_expr, int off_tup_rtu_expr,
 *                          int off_tup_fsp_expr, int off_sisi_expr,
 *                          int flag)
 *
 *   @brief prints all counters - specified offset provided that the respective
 *          optimization is turned on!
 *
 ******************************************************************************/

#define NON_ZERO_ONLY 0
#define ALL 1

static void
PrintStatistics (int off_inl_fun, int off_dead_expr, int off_dead_var, int off_dead_fun,
                 int off_lir_expr, int off_wlir_expr, int off_cf_expr, int off_lunr_expr,
                 int off_wlunr_expr, int off_uns_expr, int off_elim_arrays,
                 int off_wlf_expr, int off_wlt_expr, int off_cse_expr, int off_ap_padded,
                 int off_ap_unsupported, int off_wls_expr, int off_al_expr,
                 int off_etc_expr, int off_dl_expr, int off_sp_expr, int off_cvp_expr,
                 int off_wlfs_expr, int off_tup_tu_expr, int off_tup_wdp_expr,
                 int off_tup_fdp_expr, int off_tup_rtu_expr, int off_tup_fsp_expr,
                 int off_sisi_expr, int flag)
{
    int diff;
    DBUG_ENTER ("PrintStatistics");

    diff = inl_fun - off_inl_fun;
    if ((global.optimize.doinl) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d function(s) inlined", diff);
    }

    diff = elim_arrays - off_elim_arrays;
    if ((global.optimize.doae) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d array(s) eliminated", diff);
    }

    diff = cf_expr - off_cf_expr;
    if ((global.optimize.docf) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d primfun application(s) eliminated by constant folding", diff);
    }

    diff = cvp_expr - off_cvp_expr;
    if ((global.optimize.docvp) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d values propagated", diff);
    }

    diff = sp_expr - off_sp_expr;
    if ((global.optimize.dosp) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d primitive map operation(s) eliminated by selection propagation",
                 diff);
    }

    diff = (dead_expr - off_dead_expr) + (dead_var - off_dead_var);
    if ((global.optimize.dodcr) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d dead assignment(s) and %d unused variable declaration(s) removed",
                 dead_expr - off_dead_expr, dead_var - off_dead_var);
    }

    diff = dead_fun - off_dead_fun;
    if ((global.optimize.dodfr) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d dead functions(s) removed", diff);
    }

    diff = cse_expr - off_cse_expr;
    if ((global.optimize.docse) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d common subexpression(s) eliminated", diff);
    }

    diff = wlf_expr - off_wlf_expr;
    if ((global.optimize.dowlf) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d with-loop(s) folded", diff);
    }

    diff = wls_expr - off_wls_expr;
    if ((global.optimize.dowls) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d with-loop(s) scalarized", diff);
    }

    diff = lir_expr - off_lir_expr;
    if ((global.optimize.dolir) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d loop invariant expression(s) moved", diff);
    }

    diff = wlir_expr - off_wlir_expr;
    if ((global.optimize.dolir) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d with-loop invariant expression(s) moved", diff);
    }

    diff = uns_expr - off_uns_expr;
    if ((global.optimize.dolus) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d loop(s) unswitched", diff);
    }

    diff = lunr_expr - off_lunr_expr;
    if ((global.optimize.dolur) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d loop(s) unrolled", diff);
    }

    diff = wlunr_expr - off_wlunr_expr;
    if ((global.optimize.dowlur) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d with-loop(s) unrolled", diff);
    }

    diff = ap_padded - off_ap_padded;
    if ((global.optimize.doap) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d array type(s) padded", diff);
    }

    diff = ap_unsupported - off_ap_unsupported;
    if ((global.optimize.doap) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d array type(s) unsupported for padding", diff);
    }

    diff = al_expr - off_al_expr;
    if ((global.optimize.doal) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d associative law optimization(s)", diff);
    }

    diff = etc_expr - off_etc_expr;
    if ((global.optimize.doetc) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d typeconv(s) eliminated", diff);
    }

    diff = dl_expr - off_dl_expr;
    if ((global.optimize.dodl) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d distributive law optimization(s)", diff);
    }

    diff = wlfs_expr - off_wlfs_expr;
    if ((global.optimize.dowlfs) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d with-loop(s) fused", diff);
    }

    diff = tup_tu_expr - off_tup_tu_expr;
    if ((global.optimize.dotup) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d type(s) upgraded", diff);
    }

    diff = tup_rtu_expr - off_tup_rtu_expr;
    if ((global.optimize.dortup) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d type(s) reverse upgraded", diff);
    }

    diff = tup_wdp_expr - off_tup_wdp_expr;
    if ((global.optimize.dofsp) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d wrapper function(s) specialized", diff);
    }

    diff = tup_fdp_expr - off_tup_fdp_expr;
    if ((global.optimize.dosfd) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d user function(s) dispatched", diff);
    }

    diff = tup_fsp_expr - off_tup_fsp_expr;
    if ((global.optimize.dofsp) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d user function(s) specialized", diff);
    }

    diff = sisi_expr - off_sisi_expr;
    if ((global.optimize.dosisi) && ((ALL == flag) || (diff > 0))) {
        CTInote ("%d constant argument(s) from function signatures removed", diff);
    }

    DBUG_VOID_RETURN;
}

/*@}*/

/**
 *
 * @name Entry Function for Applying High-Level Optimizations:
 *
 * @{
 */

/** <!--*********************************************************************-->
 *
 * @fn node *Optimize( node *arg_node)
 *
 *   @brief governs the whole optimization phase and installs opt_tab.
 *
 ******************************************************************************/

node *
OPTdoOptimize (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("Optimize");

    optvar_counter = global.optvar;

    ResetCounters ();

    TRAVpush (TR_opt);
    arg_info = MakeInfo ();

    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/* @} */
/**
 *
 * @name Traversal Functions for the Optimizations:
 *
 * <!--
 * These are
 *   OPTmodul      which runs all inter-procedural optimizations, i.e.:
 *                 - Inline
 *   OPTfundef     which runs all intra-procedural optimizations.
 * -->
 *
 * @{
 */

/** <!--*********************************************************************-->
 *
 * @fn node *OPTmodule( node *arg_node, info *arg_info)
 *
 *   @brief this functions applies all those optimizations that are inter-procedural,
 *          i.e., Inlining (INL) and DeadFunctionRemoval (DFR).
 *
 *   Although INL could be done function-wise, i.e. in OPTfundef, it seems
 *   to be advantageous to do it for all functions BEFORE applying the other
 *   optimizations function-wise. The reason being, that it allows to prevent
 *   the optimization of functions that will be inlined and thus eliminated
 *   anyway in a later stage of optimization!
 *   Furthermore, DFR uses the inline-flag to indicate functions that are to
 *   be removed and thus requires all inlining to be finished before DFR is
 *   called.
 *   So the overall course of action during optimization is:
 *
 *   <pre>
 *
 *               DFC
 *               INL
 *               DFR
 *                |
 *   switch to ssa-form
 *                |
 *   optimize function-wise by calling Trav (and thus OPTfundef)
 *                |
 *   undo ssa-form
 *                |
 *               WLAA
 *               AP
 *               TSI
 *               DFR
 *               IVE
 *
 *   </pre>
 *
 ******************************************************************************/

node *
OPTmodule (node *arg_node, info *arg_info)
{
    int loop1 = 0;
    int loop2 = 0;

    DBUG_ENTER ("OPTmodul");

    INFO_OPT_MODULE (arg_info) = arg_node;
    CTIstate (" ");
    CTIstate ("  Starting initial interfunctional optimizations");

#if 0
  /*
   * TODO: the following two phases have been (temporarily) moved into
   *       WLEnhancement. For details see the TODO comment there....
   */
  /*
   * apply DFC (dispatch fun call where possible
   */
  arg_node = DFCdoDispatchFunCalls( arg_node);

  /* 
   * apply RC (remove all cast from AST)
   */
  arg_node = RCdoRemoveCasts (arg_node);

#endif

    /*
     * apply INL (inlining)
     */
    if (global.optimize.doinl) {
        arg_node = INLdoInlining (arg_node);
    }

    if ((global.break_after == PH_sacopt) && (global.break_cycle_specifier == 0)
        && (0 == strcmp (global.break_specifier, "inl"))) {
        goto DONE;
    }

    /*
     * apply DFR (dead function removal)
     */
    if (global.optimize.dodfr) {
        arg_node = DFRdoDeadFunctionRemoval (arg_node);
    }

    if ((global.break_after == PH_sacopt) && (global.break_cycle_specifier == 0)
        && (0 == strcmp (global.break_specifier, "dfr"))) {
        goto DONE;
    }

    /*
     * Starting intra-functional optimizations
     *
     * then:
     *   transform all while to do-loops
     *   remove loops and conditionals with lac2fun
     *   check for correct avis nodes and references
     *   bring AST in SSA form
     *   do function-optimizations
     *   convert back to standard form
     *   convert back with fun2lac
     */

    /*
     * apply intrafunctional optimizations
     */
    if (MODULE_FUNS (arg_node) != NULL) {
        /*
         * Now, we apply the intra-procedural optimizations function-wise!
         * The result of Trav is MUST NOT BE STORED in MODUL_FUNS, because
         * there might be some special fundefs duplicated and added in front
         * of the fundef chain.
         */

        INFO_OPT_OPTSTAGE (arg_info) = OS_initial;
        CTInote (" ");
        CTIstate ("  Starting initial intrafunctional optimizations");
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);

        /*
         * OPTfundef() sets the global variable 'do_break' to report a break!
         */
        if (do_break) {
            goto DONE;
        }

        /*---------------------------------------------------------------------*/

        INFO_OPT_OPTSTAGE (arg_info) = OS_cycle1;
        CTInote (" ");
        CTIstate ("  Starting first intrafunctional optimization cycle");
        do {
            loop1++;

            CTInote (" ");
            CTIstate ("  Cycle pass: %i", loop1);

            INFO_OPT_CONTINUE (arg_info) = FALSE;
            INFO_OPT_PASSESLOOP1 (arg_info) = loop1;
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);

            /*
             * OPTfundef() sets the global variable 'do_break' to report a break!
             */
            if (do_break) {
                goto DONE;
            }

            /*
             * apply DFR (dead function removal)
             */
            if (global.optimize.dodfr) {
                arg_node = DFRdoDeadFunctionRemoval (arg_node);
            }
            if ((global.break_after == PH_sacopt)
                && (global.break_cycle_specifier == loop1)
                && (0 == strcmp (global.break_specifier, "dfrcyc1"))) {
                goto DONE;
            }

            /*
             * apply SISI (signature simplification)
             */
            if ((global.optimize.dosisi) && (global.optimize.dodcr)) {
                int tmp = sisi_expr;
                arg_node = SISIdoSignatureSimplification (arg_node);

                if ((global.break_after == PH_sacopt)
                    && (0 == strcmp (global.break_specifier, "sisi"))) {
                    goto DONE;
                }

                if (sisi_expr > tmp) {
                    INFO_OPT_CONTINUE (arg_info) = TRUE;
                }
            }
        } while ((INFO_OPT_CONTINUE (arg_info)) && (loop1 < global.max_optcycles));

        /*-------------------------------------------------------------------*/

        INFO_OPT_OPTSTAGE (arg_info) = OS_precycle2;
        CTInote (" ");
        CTIstate ("  Starting intrafunctional optimization before second cycle");
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);

        /*
         * OPTfundef() sets the global variable 'do_break' to report a break!
         */
        if (do_break) {
            goto DONE;
        }

        /*-------------------------------------------------------------------*/

        CTInote (" ");
        CTIstate ("  Starting second intrafunctional optimization cycle");
        INFO_OPT_OPTSTAGE (arg_info) = OS_cycle2;
        /*
         * Now, we enter the second loop consisting of
         *   CF and WLT only.
         */
        while ((INFO_OPT_CONTINUE (arg_info)) && (global.optimize.docf)
               && ((loop1 + loop2) < global.max_optcycles)) {

            loop2++;

            INFO_OPT_CONTINUE (arg_info) = FALSE;
            INFO_OPT_PASSESLOOP2 (arg_info) = loop2;
            MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);

            /*
             * OPTfundef() sets the global variable 'do_break' to report a break!
             */
            if (do_break) {
                goto DONE;
            }
        }

        /*-------------------------------------------------------------------*/

        INFO_OPT_OPTSTAGE (arg_info) = OS_final;
        CTInote (" ");
        CTIstate ("  Starting final intrafunctional optimizations");
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);

        /*
         * OPTfundef() sets the global variable 'do_break' to report a break!
         */
        if (do_break) {
            goto DONE;
        }
    }
    /*----------------------------------------------------------------------*/

    /* allows to stop after fundef optimizations */
    if ((global.break_after == PH_sacopt) && (global.break_cycle_specifier == 0)
        && (0 == strcmp (global.break_specifier, "funopt"))) {
        goto DONE;
    }

    CTInote (" ");
    CTIstate ("  Starting final interfunctional optimizations");

    /*
     * !!! If they should ever work again, WLAA, TSI, and AP must run here
     */

    /*
     * apply index vector elimination
     */
    if (global.optimize.doive) {
        TRAVsetPreFun (TR_prt, IVEIprintPreFun);
        arg_node = PHrunCompilerSubPhase (SUBPH_ivei, arg_node);
        arg_node = PHrunCompilerSubPhase (SUBPH_ive, arg_node);
        TRAVsetPreFun (TR_prt, NULL);
    }

    /*
     * annotate offset scalars on with-loops
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_wlidx, arg_node);

    /*
     * apply USSA (undo ssa transformation)
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_ussa, arg_node);
    arg_node = PHrunCompilerSubPhase (SUBPH_fun2lac, arg_node);
    arg_node = PHrunCompilerSubPhase (SUBPH_lacinl, arg_node);

    CTInote (" ");
    CTInote ("Overall optimization statistics:");
    PrintStatistics (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     0, 0, 0, 0, 0, 0, ALL);

DONE:
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *OPTfundef( node *arg_node, info *arg_info)
 *
 *   @brief this function steers the intr-procedural optimization process.
 *
 *    It successively applies most of the optimizations, i.e. a single function
 *    is optimized "completely" before the next one is processed.
 *    The order in which the optimizations are applied is critical to the
 *    overall effect; so changes made here should be done very CAREFULLY!
 *    The actual course of action is:
 *
 *    the additional constant folding traversal after the unrolling seems to
 *    be an advantage, because we can fold constant and structual array
 *    expressions before they are moved out of the loop (LIR) and cannot be
 *    folded anymore (for now SSACF does not support intra functional
 *    structural folding operations).
 *
 *    <pre>
 *
 *        AE
 *        DCR
 *         |<-------\
 * loop1: CSE        |
 *        SSAILI     |
 *        CF         |
 *        CVP        |
 *        SP         |
 *        WLPG       |
 *        WLT        |
 *        WLF        |
 *        (CF)       |   (applied only if WLF succeeded!)
 *        DCR        |
 *        LUNR/WLUNR |
 *        (CF)       |   (applied only if Unrolling succeeded!)
 *        LIR        |
 *        WLS        |
 *        TUP        |
 *        ESD        |
 *        DL         |
 *        AL         |
 *         |--------/
 *        UESD
 *         |<-------\
 * loop2: CF         |
 *        WLT        |
 *         |--------/
 *        DCR
 *        WLFS
 *        (CSE)          (applied only if WLFS succeeded!)
 *        (DCR)          (applied only if WLFS succeeded!)
 *
 *   </pre>
 *****************************************************************************/

node *
OPTfundef (node *arg_node, info *arg_info)
{
    int mem_inl_fun = inl_fun;
    int mem_dead_expr = dead_expr;
    int mem_dead_var = dead_var;
    int mem_dead_fun = dead_fun;
    int mem_lir_expr = lir_expr;
    int mem_wlir_expr = wlir_expr;
    int mem_cf_expr = cf_expr;
    int mem_lunr_expr = lunr_expr;
    int mem_wlunr_expr = wlunr_expr;
    int mem_uns_expr = uns_expr;
    int mem_elim_arrays = elim_arrays;
    int mem_wlf_expr = wlf_expr;
    int mem_wlt_expr = wlt_expr;
    int mem_cse_expr = cse_expr;
    int mem_wls_expr = wls_expr;
    int mem_al_expr = al_expr;
    int mem_etc_expr = etc_expr;
    int mem_dl_expr = dl_expr;
    int mem_sp_expr = sp_expr;
    int mem_cvp_expr = cvp_expr;
    int mem_wlfs_expr = wlfs_expr;
    int mem_tup_tu_expr = tup_tu_expr;
    int mem_tup_rtu_expr = tup_rtu_expr;
    int mem_tup_wdp_expr = tup_wdp_expr;
    int mem_tup_fdp_expr = tup_fdp_expr;
    int mem_tup_fsp_expr = tup_fsp_expr;
    int mem_sisi_expr = sisi_expr;

    int old_inl_fun = inl_fun;
    int old_cse_expr = cse_expr;
    int old_cf_expr = cf_expr;
    int old_wlt_expr = wlt_expr;
    int old_wlf_expr = wlf_expr;
    int old_dcr_expr = dead_fun + dead_var + dead_expr;
    int old_lunr_expr = lunr_expr;
    int old_wlunr_expr = wlunr_expr;
    int old_uns_expr = uns_expr;
    int old_lir_expr = lir_expr;
    int old_wlir_expr = wlir_expr;
    int old_wls_expr = wls_expr;
    int old_al_expr = al_expr;
    int old_etc_expr = etc_expr;
    int old_dl_expr = dl_expr;
    int old_sp_expr = sp_expr;
    int old_cvp_expr = cvp_expr;
    int old_tup_tu_expr = tup_tu_expr;
    int old_tup_rtu_expr = tup_rtu_expr;
    int old_tup_wdp_expr = tup_wdp_expr;
    int old_tup_fdp_expr = tup_fdp_expr;
    int old_tup_fsp_expr = tup_fsp_expr;
    int old_sisi_expr = sisi_expr;

    int loop1 = INFO_OPT_PASSESLOOP1 (arg_info);
    int loop2 = INFO_OPT_PASSESLOOP2 (arg_info);

    node *arg;
    char *tmp_str;
    int tmp_str_size;

    static char argtype_buffer[80];
    static int buffer_space;

    DBUG_ENTER ("OPTfundef");

    if (!FUNDEF_ISZOMBIE (arg_node)) {
        strcpy (argtype_buffer, "");
        buffer_space = 77;

        /*
         * The global variable 'do_break' is used to report a break to OPTmodul().
         * If no break occurs this variable is unset later on.
         */
        do_break = TRUE;

        /*
         * print only the function name,
         * if the function will be traversed by some optimizations
         */
        if ((INFO_OPT_OPTSTAGE (arg_info) != OS_cycle1)
            || (FUNDEF_WASOPTIMIZED (arg_node))) {

            arg = FUNDEF_ARGS (arg_node);
            while ((arg != NULL) && (buffer_space > 5)) {

                tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), TRUE, 0);
                tmp_str_size = strlen (tmp_str);

                if ((tmp_str_size + 3) <= buffer_space) {
                    strcat (argtype_buffer, tmp_str);
                    buffer_space -= tmp_str_size;
                    if (ARG_NEXT (arg) != NULL) {
                        strcat (argtype_buffer, ", ");
                        buffer_space -= 2;
                    }
                } else {
                    strcat (argtype_buffer, "...");
                    buffer_space = 0;
                }

                tmp_str = ILIBfree (tmp_str);
                arg = ARG_NEXT (arg);
            }

            CTInote ("Optimizing function:\n  %s( %s): ...", FUNDEF_NAME (arg_node),
                     argtype_buffer);
        }

        /*------------------------------------------------------------------------*/

        switch (INFO_OPT_OPTSTAGE (arg_info)) {

        case OS_initial:
            /*
             *  BEGINNING OF OS_INITIAL
             */

            /*
             * apply AE (array elimination)
             */
            if (global.optimize.doae) {
                arg_node = AEdoArrayElimination (arg_node);
                arg_node = SSArestoreSsaOneFunction (arg_node);
            }

            if ((global.break_after == PH_sacopt) && (global.break_cycle_specifier == 0)
                && (0 == strcmp (global.break_specifier, "ae"))) {
                goto INFO;
            }

            /*
             * apply DCR (dead code removal)
             */
            if (global.optimize.dodcr) {
                arg_node = DCRdoDeadCodeRemoval (arg_node, INFO_OPT_MODULE (arg_info));
            }

            if ((global.break_after == PH_sacopt) && (global.break_cycle_specifier == 0)
                && (0 == strcmp (global.break_specifier, "dcr"))) {
                goto INFO;
            }

            /*
             * set FUNDEF_WASOPTIMIZED flag TRUE,
             * so all fundefs will be traversed in cycle1
             */
            FUNDEF_WASOPTIMIZED (arg_node) = TRUE;

            /*
             * END OF OS_INITIAL
             */
            break;

            /*----------------------------------------------------------------------*/

        case OS_cycle1:

            /*
             * BEGINNING OF OS_CYCLE1
             */

            /*
             * Now, we enter the first loop. It consists of:
             *   CSE, CF, WLT, WLF, (CF), DCR, LUNR/ WLUNR, LIR WLS, (ESD), AL,
             *   DLF and WLPG.
             */

            old_inl_fun = inl_fun;
            old_cse_expr = cse_expr;
            old_cf_expr = cf_expr;
            old_wlt_expr = wlt_expr;
            old_wlf_expr = wlf_expr;
            old_dcr_expr = dead_fun + dead_var + dead_expr;
            old_lunr_expr = lunr_expr;
            old_wlunr_expr = wlunr_expr;
            old_uns_expr = uns_expr;
            old_lir_expr = lir_expr;
            old_wlir_expr = wlir_expr;
            old_wls_expr = wls_expr;
            old_al_expr = al_expr;
            old_etc_expr = etc_expr;
            old_dl_expr = dl_expr;
            old_sp_expr = sp_expr;
            old_cvp_expr = cvp_expr;

            /*
             * try to optimize this function only if it was optimized in the last
             * cycle pass otherwise try only typeupgrade
             */
            if (FUNDEF_WASOPTIMIZED (arg_node)) {

                /*
                 * unset FUNDEF_WASOPTIMIZED
                 */
                FUNDEF_WASOPTIMIZED (arg_node) = FALSE;

                /*
                 * apply CSE (common subexpression elimination)
                 */
                if (global.optimize.docse) {
                    arg_node = CSEdoCse (arg_node, INFO_OPT_MODULE (arg_info));
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "cse"))) {
                    goto INFO;
                }

                /*
                 * apply ILI (infere loop invariant arguments)
                 */
                arg_node = ILIdoInferLoopInvariants (arg_node);

                /*
                 * apply TUP  (type upgrade),
                 *       RTUP (reverse type upgrade),
                 *       FSP  (function specialization),
                 *       FDP  (function dispatch)
                 */
                if ((global.optimize.dotup) || (global.optimize.dortup)
                    || (global.optimize.dofsp) || (global.optimize.dosfd)) {

                    arg_node = TUPdoTypeUpgrade (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "tup"))) {
                    goto INFO;
                }

                /*
                 * apply CF (constant folding)
                 */
                if (global.optimize.docf) {
                    arg_node = CFdoConstantFolding (arg_node, INFO_OPT_MODULE (arg_info));
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "cf"))) {
                    goto INFO;
                }

                /*
                 * apply CVP (constant and variable propagation)
                 */
                if (global.optimize.docvp) {
                    arg_node = CVPdoConstVarPropagation (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "cvp"))) {
                    goto INFO;
                }

                /*
                 * apply WLPG (with-loop partition generation)
                 */
                if (global.optimize.dowlpg) {
                    arg_node = WLPGdoWlPartitionGenerationOpt (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "wlpg"))) {
                    goto INFO;
                }

#ifdef _SSA_WLT_FIXED_

                /*
                 * apply WLT (with-loop ...)
                 */
                if (global.optimize.dowlt) {
                    arg_node = WLFdoWithloopFoldingWlt (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "wlt"))) {
                    goto INFO;
                }
#endif

                /*
                 * apply WLF (with-loop folding)
                 */
                if (global.optimize.dowlf) {
                    arg_node = WLFdoWithloopFolding (arg_node, loop1);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && ((0 == strcmp (global.break_specifier, "wli"))
                        || (0 == strcmp (global.break_specifier, "wlf")))) {
                    goto INFO;
                }

                /*
                 * [if necessary]
                 * apply CF (constant folding)
                 */
                if (wlf_expr != old_wlf_expr) {
                    /*
                     * this may speed up the optimization phase a lot if a lot of code
                     * has been inserted by WLF.
                     */
                    if (global.optimize.docf) {
                        arg_node
                          = CFdoConstantFolding (arg_node, INFO_OPT_MODULE (arg_info));
                    }
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "cf2"))) {
                    goto INFO;
                }

                /*
                 * apply DCR (dead code removal)
                 */
                if (global.optimize.dodcr) {
                    arg_node
                      = DCRdoDeadCodeRemoval (arg_node, INFO_OPT_MODULE (arg_info));
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "dcr"))) {
                    goto INFO;
                }

                /*
                 * apply WLS (with-loop scalarization)
                 */
                if (global.optimize.dowls) {
                    arg_node = WLSdoWithloopScalarization (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "wls"))) {
                    goto INFO;
                }

                /*
                 * apply LUR (loop unrolling/ with-loop unrolling)
                 */
                if ((global.optimize.dolur) || (global.optimize.dowlur)) {
                    arg_node = LURdoLoopUnrolling (arg_node, INFO_OPT_MODULE (arg_info));
                    /*
                     * important:
                     *   SSALoopUnrolling uses internally SSAWLUnroll to get the
                     *   WithLoopUnrolling.
                     */
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "lur"))) {
                    goto INFO;
                }

                /*
                 * [if necessary]
                 * apply CF (constant folding)
                 */
                if ((wlunr_expr != old_wlunr_expr) || (lunr_expr != old_lunr_expr)) {
                    /*
                     * this may speed up the optimization phase a lot if a lot of code
                     * has been inserted by Unrolling..
                     */
                    if (global.optimize.docf) {
                        arg_node
                          = CFdoConstantFolding (arg_node, INFO_OPT_MODULE (arg_info));
                    }
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "cf3"))) {
                    goto INFO;
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "lus"))) {
                    goto INFO;
                }

                /*
                 * apply LACINL (LAC function inlining)
                 */
                if (global.lacinline) {
                    arg_node = LINLdoLACInliningOneFundef (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "lacinl"))) {
                    goto INFO;
                }

                /*
                 * apply LIR (loop invariant removal)
                 */
                if (global.optimize.dolir) {
                    arg_node
                      = LIRdoLoopInvariantRemoval (arg_node, INFO_OPT_MODULE (arg_info));
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "lir"))) {
                    goto INFO;
                }

                /*
                 * apply ESD (eliminate subtraction and divison)
                 */
                if (global.optimize.doesd) {
                    arg_node = ESDdoElimSubDiv (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "esd"))) {
                    goto INFO;
                }

                /*
                 * apply ETC (typeconv elimination)
                 */
                if (global.optimize.doetc) {
                    arg_node = ETCdoEliminateTypeConversions (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "etc"))) {
                    goto INFO;
                }

                /*
                 * apply AL (associative law)
                 */
                if (global.optimize.doal) {
                    /* arg_node = ALdoAssociativeLaw (arg_node); */
                    arg_node = ASSOCdoAssociativityOptimization (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "al"))) {
                    goto INFO;
                }

                /*
                 * apply DL (distributive law)
                 */
                if (global.optimize.dodl) {
                    arg_node = DLdoDistributiveLaw (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "dl"))) {
                    goto INFO;
                }
            }

            else {

                /*
                 * The current function was not optimized in the whole last cycle pass.
                 * Because of the inter(!)functional optimization ability of typeupgrade
                 * it is possible, that new optimization cases are generated from outside.
                 * If that is the case, so at least type upgrade again will do some
                 * optimizations on this function now, and trigger so a new cycle
                 * traversal for this function for the next cycle pass.
                 */

                /*
                 * apply TUP  (type upgrade),
                 *       RTUP (reverse type upgrade),
                 *       FSP  (function specialization),
                 *       FDP  (function dispatch)
                 */
                if ((global.optimize.dotup) || (global.optimize.dortup)
                    || (global.optimize.dofsp) || (global.optimize.dosfd)) {
                    arg_node = TUPdoTypeUpgrade (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "tup"))) {
                    goto INFO;
                }

                /*
                 * apply LACINL (LAC function inlining)
                 */
                if (global.lacinline) {
                    arg_node = LINLdoLACInliningOneFundef (arg_node);
                }

                if ((global.break_after == PH_sacopt)
                    && (global.break_cycle_specifier == loop1)
                    && (0 == strcmp (global.break_specifier, "lacinl"))) {
                    goto INFO;
                }

                if ((tup_tu_expr != old_tup_tu_expr) || (tup_rtu_expr != old_tup_rtu_expr)
                    || (tup_wdp_expr != old_tup_wdp_expr)
                    || (tup_fdp_expr != old_tup_fdp_expr)
                    || (tup_fsp_expr != old_tup_fsp_expr) || (inl_fun != old_inl_fun)) {

                    /*
                     * at least one optimization was successful
                     * create some output
                     */

                    arg = FUNDEF_ARGS (arg_node);
                    while ((arg != NULL) && (buffer_space > 5)) {
                        tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), TRUE, 0);
                        tmp_str_size = strlen (tmp_str);

                        if ((tmp_str_size + 3) <= buffer_space) {
                            strcat (argtype_buffer, tmp_str);
                            buffer_space -= tmp_str_size;
                            if (ARG_NEXT (arg) != NULL) {
                                strcat (argtype_buffer, ", ");
                                buffer_space -= 2;
                            }
                        } else {
                            strcat (argtype_buffer, "...");
                            buffer_space = 0;
                        }

                        tmp_str = ILIBfree (tmp_str);
                        arg = ARG_NEXT (arg);
                    }

                    CTInote ("Optimizing function:\n  %s( %s): ...",
                             FUNDEF_NAME (arg_node), argtype_buffer);
                }
            }

            /*
             * if at least one optimization could be applied in this cycle pass:
             *   - set flag for one more cycle pass
             *   - run this function in the next cycle pass
             */
            if ((cse_expr != old_cse_expr) || (cf_expr != old_cf_expr)
                || (wlt_expr != old_wlt_expr) || (wlf_expr != old_wlf_expr)
                || (dead_fun + dead_var + dead_expr != old_dcr_expr)
                || (lunr_expr != old_lunr_expr) || (wlunr_expr != old_wlunr_expr)
                || (uns_expr != old_uns_expr) || (lir_expr != old_lir_expr)
                || (wlir_expr != old_wlir_expr) || (wls_expr != old_wls_expr)
                || (al_expr != old_al_expr) || (dl_expr != old_dl_expr)
                || (sp_expr != old_sp_expr) || (cvp_expr != old_cvp_expr)
                || (tup_tu_expr != old_tup_tu_expr) || (tup_rtu_expr != old_tup_rtu_expr)
                || (tup_wdp_expr != old_tup_wdp_expr)
                || (tup_fdp_expr != old_tup_fdp_expr)
                || (tup_fsp_expr != old_tup_fsp_expr) || (sisi_expr != old_sisi_expr)
                || (inl_fun != old_inl_fun) || (etc_expr != old_etc_expr)) {

                INFO_OPT_CONTINUE (arg_info) = TRUE;
                FUNDEF_WASOPTIMIZED (arg_node) = TRUE;
            }

            /*
             *  END OF OS_CYCLE11
             */
            break;

            /*----------------------------------------------------------------------*/

        case OS_precycle2:
            /*
             *  START OF OS_PRECYCLE2
             */

            /*
             * apply UESD (undo eliminate substraction and division)
             */
            if (global.optimize.doesd) {
                arg_node = UESDdoUndoElimSubDiv (arg_node);
            }
            if (global.optimize.dodcr) {
                arg_node = DCRdoDeadCodeRemoval (arg_node, INFO_OPT_MODULE (arg_info));
            }
            if ((global.break_after == PH_sacopt)
                && (0 == strcmp (global.break_specifier, "uesd"))) {
                goto INFO;
            }

            /*
             * END OF OS_PRECYCLE2
             */
            break;

            /*---------------------------------------------------------------------*/

        case OS_cycle2:
            /*
             *  START OF OS_CYCLE2
             */

            /*
             * Now, we enter the second loop consisting of
             *   CF and WLT only.
             */
            old_wlt_expr = wlt_expr;
            old_cf_expr = cf_expr;

            /*
             * apply CF (constant folding)
             */
            if (global.optimize.docf) {
                arg_node
                  = CFdoConstantFolding (arg_node,
                                         INFO_OPT_MODULE (arg_info)); /* ssacf_tab */
            }

            if ((global.break_after == PH_sacopt)
                && (global.break_cycle_specifier == (loop1 + loop2))
                && (0 == strcmp (global.break_specifier, "cf"))) {
                goto INFO;
            }

            /*
             * This is needed to transform more index vectors in scalars or vice versa.
             */
#ifdef _SSA_WLT_FIXED_

            /*
             * apply WLT (with-loop ...)
             */
            if (global.optimize.dowlt) {
                arg_node = WLFdoWithloopFoldingWlt (arg_node);
            }

            if ((global.break_after == PH_sacopt)
                && (global.break_cycle_specifier == (loop1 + loop2))
                && (0 == strcmp (global.break_specifier, "wlt"))) {
                goto INFO;
            }
#endif

            if (wlt_expr != old_wlt_expr) {
                INFO_OPT_CONTINUE (arg_info) = TRUE;
            }
            /*
             * END OF OS_CYCLE2
             */
            break;

            /*---------------------------------------------------------------------*/

        case OS_final:
            /*
             * BEGINNING OF OS_FINAL
             */

            /*
             * apply DCR (dead code removal)
             */
            if (global.optimize.dodcr) {
                arg_node = DCRdoDeadCodeRemoval (arg_node, INFO_OPT_MODULE (arg_info));
            }

            /*
             * apply WLFS (withloop fusion)
             */
            if (global.optimize.dowlfs) {
                arg_node = WLFSdoWithloopFusion (arg_node);
            }

            /*
             * [if necessary]
             * apply CSE (common subexpression elimination)
             * apply DCR (dead code removal)
             */
            if (wlfs_expr != mem_wlfs_expr) {

                if (global.optimize.docse) {
                    arg_node = CSEdoCse (arg_node, INFO_OPT_MODULE (arg_info));
                }

                if (global.optimize.dodcr) {
                    arg_node
                      = DCRdoDeadCodeRemoval (arg_node, INFO_OPT_MODULE (arg_info));
                }
            }

            if ((global.break_after == PH_sacopt) && (global.break_cycle_specifier == 0)
                && (0 == strcmp (global.break_specifier, "wlfs"))) {
                goto INFO;
            }
            /*
             *  END OF OS_FINAL
             */
            break;
        }

        /*
         * END OF SWITCH
         */

        /*-----------------------------------------------------------------------*/

        /*
         * no break yet!
         */
        do_break = FALSE;
    INFO:
        if (loop1 + loop2 == global.max_optcycles
            && ((cse_expr != old_cse_expr) || (cf_expr != old_cf_expr)
                || (wlt_expr != old_wlt_expr) || (wlf_expr != old_wlf_expr)
                || (dead_fun + dead_var + dead_expr != old_dcr_expr)
                || (lunr_expr != old_lunr_expr) || (wlunr_expr != old_wlunr_expr)
                || (uns_expr != old_uns_expr) || (lir_expr != old_lir_expr)
                || (wlir_expr != old_wlir_expr))) {
            CTIwarn ("Maximal number of optimization cycles reached");
        }

        /*
         * print statistics for current function
         */
        PrintStatistics (mem_inl_fun, mem_dead_expr, mem_dead_var, mem_dead_fun,
                         mem_lir_expr, mem_wlir_expr, mem_cf_expr, mem_lunr_expr,
                         mem_wlunr_expr, mem_uns_expr, mem_elim_arrays, mem_wlf_expr,
                         mem_wlt_expr, mem_cse_expr, 0, 0, mem_wls_expr, mem_al_expr,
                         mem_etc_expr, mem_dl_expr, mem_sp_expr, mem_cvp_expr,
                         mem_wlfs_expr, mem_tup_tu_expr, mem_tup_wdp_expr,
                         mem_tup_fdp_expr, mem_tup_rtu_expr, mem_tup_fsp_expr,
                         mem_sisi_expr, NON_ZERO_ONLY);
    }

    /*
     * traverse in next function
     */

    if (FUNDEF_NEXT (arg_node) == NULL) {
        FUNDEF_NEXT (arg_node) = DUPgetCopiedSpecialFundefs ();
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
/* @} */ /* defgroup opt */
