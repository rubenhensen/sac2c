/*
 *
 * $Log$
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
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "print.h"
#include "convert.h"

#include "optimize.h"
#include "generatemasks.h"
#include "freemasks.h"
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "DeadFunctionRemoval.h"
#include "LoopInvariantRemoval.h"
#include "Inline.h"
#include "Unroll.h"
#include "Unswitch.h"
#include "ArrayElimination.h"
#include "CSE.h"
#include "WithloopFolding.h"
#include "WithloopScalarization.h"
#include "AssociativeLaw.h"
#include "DistributiveLaw.h"
#include "wl_access_analyze.h"
#include "tile_size_inference.h"
#include "index.h"
#include "pad.h"

#include "CheckAvis.h"
#include "SSATransform.h"
#include "UndoSSATransform.h"
#include "lac2fun.h"
#include "fun2lac.h"
#include "SSADeadCodeRemoval.h"
#include "SSACSE.h"
#include "SSAConstantFolding.h"
#include "SSALIR.h"
#include "while2do.h"
#include "SSALUR.h"
#include "SSAWithloopFolding.h"
#include "rmcasts.h"
#include "SSAInferLI.h"
#include "ElimSubDiv.h"
#include "UndoElimSubDiv.h"

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
int dl_expr;

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

void
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
    dl_expr = 0;

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
 *                          int off_ap_unsupported,
 *                          int off_wls_expr, int off_al_expr, int off_dl_expr,
 *                          int flag)
 *
 *   @brief prints all counters - specified offset provided that the respective
 *          optimization is turned on!
 *
 ******************************************************************************/

#define NON_ZERO_ONLY 0
#define ALL 1

void
PrintStatistics (int off_inl_fun, int off_dead_expr, int off_dead_var, int off_dead_fun,
                 int off_lir_expr, int off_wlir_expr, int off_cf_expr, int off_lunr_expr,
                 int off_wlunr_expr, int off_uns_expr, int off_elim_arrays,
                 int off_wlf_expr, int off_wlt_expr, int off_cse_expr, int off_ap_padded,
                 int off_ap_unsupported, int off_wls_expr, int off_al_expr,
                 int off_dl_expr, int flag)
{
    int diff;
    DBUG_ENTER ("PrintStatistics");

    diff = inl_fun - off_inl_fun;
    if ((optimize & OPT_INL) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d function(s) inlined", diff));

    diff = elim_arrays - off_elim_arrays;
    if ((optimize & OPT_AE) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d array(s) eliminated", diff));

    diff = cf_expr - off_cf_expr;
    if ((optimize & OPT_CF) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d primfun application(s) eliminated by constant folding", diff));

    diff = (dead_expr - off_dead_expr) + (dead_var - off_dead_var);
    if ((optimize & OPT_DCR) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d dead assignment(s) and %d unused variable declaration(s) removed",
               dead_expr - off_dead_expr, dead_var - off_dead_var));

    diff = dead_fun - off_dead_fun;
    if ((optimize & OPT_DFR) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d dead functions(s) removed", diff));

    diff = cse_expr - off_cse_expr;
    if ((optimize & OPT_CSE) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d common subexpression(s) eliminated", diff));

    diff = wlf_expr - off_wlf_expr;
    if ((optimize & OPT_WLF) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d with-loop(s) folded", diff));

    diff = wls_expr - off_wls_expr;
    if ((optimize & OPT_WLS) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d with-loop(s) scalarized", diff));

    diff = lir_expr - off_lir_expr;
    if ((optimize & OPT_LIR) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d loop invariant expression(s) moved", diff));

    diff = wlir_expr - off_wlir_expr;
    if ((optimize & OPT_LIR) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d with-loop invariant expression(s) moved", diff));

    diff = uns_expr - off_uns_expr;
    if ((optimize & OPT_LUS) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d loop(s) unswitched", diff));

    diff = lunr_expr - off_lunr_expr;
    if ((optimize & OPT_LUR) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d loop(s) unrolled", diff));

    diff = wlunr_expr - off_wlunr_expr;
    if ((optimize & OPT_WLUR) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d with-loop(s) unrolled", diff));

    diff = ap_padded - off_ap_padded;
    if ((optimize & OPT_AP) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d array type(s) padded", diff));

    diff = ap_unsupported - off_ap_unsupported;
    if ((optimize & OPT_AP) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d array type(s) unsupported for padding", diff));

    diff = al_expr - off_al_expr;
    if ((optimize & OPT_AL) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d associative law optimization(s)", diff));

    diff = dl_expr - off_dl_expr;
    if ((optimize & OPT_DL) && ((ALL == flag) || (diff > 0)))
        NOTE (("  %d distributive law optimization(s)", diff));

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
Optimize (node *arg_node)
{
    funtab *tmp_tab;
    node *arg_info;

    DBUG_ENTER ("Optimize");

    optvar_counter = optvar;

    ResetCounters ();

    tmp_tab = act_tab;
    act_tab = opt_tab;

    arg_info = MakeInfo ();

    Trav (arg_node, arg_info);

    arg_info = FreeTree (arg_info);

    act_tab = tmp_tab;

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
 * @fn node *OPTmodul( node *arg_node, node *arg_info)
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
 *               INL
 *               DFR
 *                |
 *   switch to ssa-form (optional)
 *                |
 *   optimize function-wise by calling Trav (and thus OPTfundef)
 *                |
 *   undo ssa-form (optional)
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
OPTmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("OPTmodul");

    INFO_OPT_MODUL (arg_info) = arg_node;

    /* remove all cast from AST */
    arg_node = RemoveCasts (arg_node); /* rmcasts_tab */

    if (optimize & OPT_INL) {
        arg_node = Inline (arg_node); /* inline_tab */
    }

    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "inl"))) {
        goto DONE;
    }

    if (optimize & OPT_DFR) {
        arg_node = DeadFunctionRemoval (arg_node, arg_info);
    }

    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "dfr"))) {
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

    if (use_ssaform) {
        NOTE (("using ssa-form based optimizations."));

        /* transform all while to do-loops (required for SSALIR) */
        arg_node = TransformWhile2Do (arg_node);
        if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
            && (0 == strcmp (break_specifier, "w2d"))) {
            goto DONE;
        }

        arg_node = Lac2Fun (arg_node);
        if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
            && (0 == strcmp (break_specifier, "l2f"))) {
            goto DONE;
        }

        arg_node = CheckAvis (arg_node);
        if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
            && (0 == strcmp (break_specifier, "cha"))) {
            goto DONE;
        }

        arg_node = SSATransform (arg_node);
        if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
            && (0 == strcmp (break_specifier, "ssa"))) {
            goto DONE;
        }
    }

    if (MODUL_FUNS (arg_node)) {
        /*
         * Now, we apply the intra-procedural optimizations function-wise!
         * The result of Trav is MUST NOT BE STORED in MODUL_FUNS, because
         * there might be some special fundefs duplicated and added in front
         * of the fundef chain.
         */
        Trav (MODUL_FUNS (arg_node), arg_info);

        /*
         * OPTfundef() sets the global variable 'do_break' to report a break!
         */
        if (do_break) {
            goto DONE;
        }
    }

    /* allows to stop after fundef optimizations */
    if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
        && (0 == strcmp (break_specifier, "funopt"))) {
        goto DONE;
    }

    if (use_ssaform) {
        NOTE (("undo ssa-form"));
        arg_node = UndoSSATransform (arg_node);
        if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
            && (0 == strcmp (break_specifier, "ussa"))) {
            goto DONE;
        }

        /* undo lac2fun transformation */
        arg_node = Fun2Lac (arg_node);

        if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
            && (0 == strcmp (break_specifier, "f2l"))) {
            goto DONE;
        }
    }

    /*
     * Now, it's indicated to analyze the array accesses within WLs.
     */
    if (optimize & (OPT_TSI | OPT_AP)) {
        arg_node = WLAccessAnalyze (arg_node);

        if ((break_after == PH_sacopt) && (0 == strcmp (break_specifier, "wlaa"))) {
            goto DONE;
        }
    }

    /*
     * Now, we apply array padding
     */
    if (optimize & OPT_AP) {
        arg_node = ArrayPadding (arg_node);

        if ((break_after == PH_sacopt) && (0 == strcmp (break_specifier, "ap"))) {
            goto DONE;
        }
    }

    /*
     * infere the tilesize
     */
    if (optimize & OPT_TSI) {
        arg_node = TileSizeInference (arg_node);

        if ((break_after == PH_sacopt) && (0 == strcmp (break_specifier, "tsi"))) {
            goto DONE;
        }
    }

    /*
     * we apply DFR once again!
     */
    if (optimize & OPT_DFR) {
        arg_node = DeadFunctionRemoval (arg_node, arg_info);
    }

    if ((break_after == PH_sacopt) && (0 == strcmp (break_specifier, "dfr2"))) {
        goto DONE;
    }

    NOTE ((""));
    NOTE (("overall optimization statistics:"));
    PrintStatistics (0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ALL);

    /*
     * index vector elimination
     */
    if (optimize & OPT_IVE) {
        arg_node = IndexVectorElimination (arg_node);

        if ((break_after == PH_sacopt) && (0 == strcmp (break_specifier, "ive"))) {
            goto DONE;
        }
    }

DONE:
    DBUG_RETURN (arg_node);
}

/** <!--*********************************************************************-->
 *
 * @fn node *OPTfundef( node *arg_node, node *arg_info)
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
 *        SSAILI     |   (only in ssa form)
 *        CF         |
 *        WLT        |
 *        WLF        |
 *        (CF)       |   (applied only if WLF succeeded!)
 *        DCR        |
 *        LUNR/WLUNR |
 *        (CF)       |   (applied only if Unrolling succeeded!)
 *        UNS        |   (not available in ssa form)
 *        LIR        |
 *        WLS        |
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
 *
 *   </pre>
 ******************************************************************************/

node *
OPTfundef (node *arg_node, node *arg_info)
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
    int mem_dl_expr = dl_expr;

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
    int old_dl_expr = dl_expr;

    int loop1 = 0;
    int loop2 = 0;

    node *arg;
    char *tmp_str;
    int tmp_str_size;

    static char argtype_buffer[80];
    static int buffer_space;

    DBUG_ENTER ("OPTfundef");

    strcpy (argtype_buffer, "");
    buffer_space = 77;

    /* no optimizations of prototypes and special fundefs */
    if ((FUNDEF_BODY (arg_node) != NULL) && (!(FUNDEF_IS_LACFUN (arg_node)))) {
        /*
         * The global variable 'do_break' is used to report a break to OPTmodul().
         * If no break occurs this variable is unset later on.
         */
        do_break = TRUE;

        /*
         * Any optimization technique may only be applied iff there's a function
         * body.
         */
        arg = FUNDEF_ARGS (arg_node);
        while ((arg != NULL) && (buffer_space > 5)) {
            tmp_str = Type2String (ARG_TYPE (arg), 0, TRUE);
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

            tmp_str = Free (tmp_str);
            arg = ARG_NEXT (arg);
        }

        NOTE (("optimizing function"));
        NOTE ((" %s( %s): ...", FUNDEF_NAME (arg_node), argtype_buffer));

        /*
         * !! Important !!
         * SSA-form based optimizations are now separated from Non-SSA-optimizations
         * by a single if-clause
         */

        if (use_ssaform) {

            /*
             * optimization for SSA-form
             */

            if (optimize & OPT_AE) {
                arg_node = ArrayElimination (arg_node, arg_node); /* ae_tab */
                arg_node = CheckAvisOneFunction (arg_node);
                arg_node = SSATransformOneFunction (arg_node);
            }

            if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
                && (0 == strcmp (break_specifier, "ae"))) {
                goto INFO;
            }

            if (optimize & OPT_DCR) {
                arg_node = SSADeadCodeRemoval (arg_node, INFO_OPT_MODUL (arg_info));
            }

            if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
                && (0 == strcmp (break_specifier, "dcr"))) {
                goto INFO;
            }

        } else {

            /*
             * optimization for Non-SSA-form
             */

            if (optimize & OPT_AE) {
                /*
                 * AE needs mask for MRD generation now.
                 */
                arg_node = GenerateMasks (arg_node, NULL);
                arg_node = ArrayElimination (arg_node, arg_node); /* ae_tab */
            }

            if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
                && (0 == strcmp (break_specifier, "ae"))) {
                goto INFO;
            }

            /*
             * necessary after AE (which does not care about masks while introducing
             * new variables:
             */
            arg_node = GenerateMasks (arg_node, NULL);

            if (optimize & OPT_DCR) {
                arg_node = DeadCodeRemoval (arg_node, arg_info);
            }

            if ((break_after == PH_sacopt) && (break_cycle_specifier == 0)
                && (0 == strcmp (break_specifier, "dcr"))) {
                goto INFO;
            }
        }

        /*
         * Now, we enter the first loop. It consists of:
         *   CSE, CF, WLT, WLF, (CF), DCR, LUNR/ WLUNR, UNS, LIR WLS, (ESD), AL and DL.
         */
        do {
            loop1++;
            DBUG_PRINT ("OPT",
                        ("---------------------------------------- loop ONE, pass %d",
                         loop1));

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
            old_dl_expr = dl_expr;

            /*
             * !! Important !!
             * SSA-form based optimizations are now separated from Non-SSA-optimizations
             * by a single if-clause
             */

            if (use_ssaform) {

                /*
                 * optimization for SSA-form
                 */

                if (optimize & OPT_CSE) {
                    arg_node = SSACSE (arg_node, INFO_OPT_MODUL (arg_info));
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cse"))) {
                    goto INFO;
                }

                /* infere loop invariant arguments */
                arg_node = SSAInferLoopInvariants (arg_node);

                if (optimize & OPT_CF) {
                    arg_node
                      = SSAConstantFolding (arg_node,
                                            INFO_OPT_MODUL (arg_info)); /* ssacf_tab */

                    /* srs: CF does not handle the USE mask correctly. For example
                       a = f(3);
                       b = a;
                       c = b;
                       will be transformed to
                       a = f(3);
                       b = a;
                       c = a;
                       after CF. But the USE mask of b is not reduced.
                       This leads to a DCR problem (b = a is removed but variable
                       declaration for b not. */
                    /* quick fix: always rebuild masks after CF */
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cf"))) {
                    goto INFO;
                }

                if (optimize & OPT_WLT) {
                    arg_node = SSAWithloopFoldingWLT (arg_node);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "wlt"))) {
                    goto INFO;
                }

                if (optimize & OPT_WLF) {
                    arg_node = SSAWithloopFolding (arg_node, loop1);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && ((0 == strcmp (break_specifier, "wli"))
                        || (0 == strcmp (break_specifier, "wlf")))) {
                    goto INFO;
                }

                if (wlf_expr != old_wlf_expr) {
                    /*
                     * this may speed up the optimization phase a lot if a lot of code
                     * has been inserted by WLF.
                     */
                    if (optimize & OPT_CF) {
                        arg_node
                          = SSAConstantFolding (arg_node, INFO_OPT_MODUL (arg_info));
                    }
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cf2"))) {
                    goto INFO;
                }

                if (optimize & OPT_DCR) {
                    arg_node = SSADeadCodeRemoval (arg_node, INFO_OPT_MODUL (arg_info));
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "dcr"))) {
                    goto INFO;
                }

                if (optimize & OPT_WLS) {
                    arg_node
                      = WithloopScalarization (arg_node, INFO_OPT_MODUL (arg_info));
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "wls"))) {
                    goto INFO;
                }

                if ((optimize & OPT_LUR) || (optimize & OPT_WLUR)) {
                    arg_node = SSALoopUnrolling (arg_node, INFO_OPT_MODUL (arg_info));
                    /*
                     * important:
                     *   SSALoopUnrolling uses internally WLUnroll to get the
                     *   WithLoopUnrolling.
                     */
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "lur"))) {
                    goto INFO;
                }

                if ((wlunr_expr != old_wlunr_expr) || (lunr_expr != old_lunr_expr)) {
                    /*
                     * this may speed up the optimization phase a lot if a lot of code
                     * has been inserted by Unrolling..
                     */
                    if (optimize & OPT_CF) {
                        arg_node
                          = SSAConstantFolding (arg_node, INFO_OPT_MODUL (arg_info));
                    }
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cf3"))) {
                    goto INFO;
                }

                /*
                 * !! LUS not implemented in SSA-form !!
                 */
                /*if ((optimize & OPT_LUS) && (!use_ssaform)) {
                 *arg_node=Unswitch(arg_node, arg_info);
                 *}
                 */
                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "lus"))) {
                    goto INFO;
                }

                if (optimize & OPT_LIR) {
                    arg_node
                      = SSALoopInvariantRemoval (arg_node, INFO_OPT_MODUL (arg_info));
                    /* ktr: This is a very dirty solution for bug #16.
                       The problem of AVIS_SSAASSIGN being wrongly assigned in SSALIR.c
                       is still unresolved. */
                    arg_node = SSATransformOneFunction (arg_node);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "lir"))) {
                    goto INFO;
                }

                if ((optimize & OPT_AL) || (optimize & OPT_DL)) {
                    arg_node = ElimSubDiv (arg_node, arg_info);
                }

                if (optimize & OPT_AL) {
                    arg_node = AssociativeLaw (arg_node, arg_info);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "al"))) {
                    goto INFO;
                }

                if (optimize & OPT_DL) {
                    arg_node = DistributiveLaw (arg_node, arg_info);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "dl"))) {
                    goto INFO;
                }

            } else {

                /*
                 * optimization for non-SSA-form
                 */

                if (optimize & OPT_CSE) {
                    arg_node = CSE (arg_node, arg_info);
                    arg_node = GenerateMasks (arg_node, NULL);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cse"))) {
                    goto INFO;
                }

                if (optimize & OPT_CF) {
                    arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
                    arg_node = GenerateMasks (arg_node, NULL);
                }
                /* srs: CF does not handle the USE mask correctly. For example
                   a = f(3);
                   b = a;
                   c = b;
                   will be transformed to
                   a = f(3);
                   b = a;
                   c = a;
                   after CF. But the USE mask of b is not reduced.
                   This leads to a DCR problem (b = a is removed but variable declaration
                   for b not. */
                /* quick fix: always rebuild masks after CF */

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cf"))) {
                    goto INFO;
                }

                if (optimize & OPT_WLT) {
                    arg_node = GenerateMasks (arg_node, NULL);
                    arg_node = WithloopFoldingWLT (arg_node); /* wlt */
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "wlt"))) {
                    goto INFO;
                }

                if (optimize & OPT_WLF) {
                    arg_node = GenerateMasks (arg_node, NULL);
                    arg_node = WithloopFolding (arg_node, loop1); /* wli, wlf */
                    /*
                     * rebuild mask which is necessary because of WL-body-substitutions
                     * and inserted new variables to prevent wrong variable bindings.
                     */
                    arg_node = GenerateMasks (arg_node, NULL);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && ((0 == strcmp (break_specifier, "wli"))
                        || (0 == strcmp (break_specifier, "wlf")))) {
                    goto INFO;
                }

                if (wlf_expr != old_wlf_expr) {
                    /*
                     * this may speed up the optimization phase a lot if a lot of code
                     * has been inserted by WLF.
                     */
                    if (optimize & OPT_CF) {
                        arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
                        arg_node = GenerateMasks (arg_node, NULL);
                    }
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cf2"))) {
                    goto INFO;
                }

                if (optimize & OPT_DCR) {
                    arg_node = DeadCodeRemoval (arg_node, arg_info);
                    arg_node = GenerateMasks (arg_node, NULL);
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "dcr"))) {
                    goto INFO;
                }

                if ((optimize & OPT_LUR) || (optimize & OPT_WLUR)) {
                    arg_node = Unroll (arg_node, arg_info); /* unroll_tab */
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "lur"))) {
                    goto INFO;
                }

                if ((wlunr_expr != old_wlunr_expr) || (lunr_expr != old_lunr_expr)) {
                    /*
                     * this may speed up the optimization phase a lot if a lot of code
                     * has been inserted by Unrolling..
                     */
                    if (optimize & OPT_CF) {
                        arg_node = GenerateMasks (arg_node, NULL);
                        arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
                        arg_node = GenerateMasks (arg_node, NULL);
                    }
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "cf3"))) {
                    goto INFO;
                }

                if (optimize & OPT_LUS) {
                    arg_node = Unswitch (arg_node, arg_info); /* unswitch_tab */
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "lus"))) {
                    goto INFO;
                }

                if (optimize & OPT_LIR) {
                    arg_node = LoopInvariantRemoval (arg_node, arg_info);
                    /* lir_tab and lir_mov_tab */
                }

                if ((break_after == PH_sacopt) && (break_cycle_specifier == loop1)
                    && (0 == strcmp (break_specifier, "lir"))) {
                    goto INFO;
                }
            }

        } while (((cse_expr != old_cse_expr) || (cf_expr != old_cf_expr)
                  || (wlt_expr != old_wlt_expr) || (wlf_expr != old_wlf_expr)
                  || (dead_fun + dead_var + dead_expr != old_dcr_expr)
                  || (lunr_expr != old_lunr_expr) || (wlunr_expr != old_wlunr_expr)
                  || (uns_expr != old_uns_expr) || (lir_expr != old_lir_expr)
                  || (wlir_expr != old_wlir_expr) || (wls_expr != old_wls_expr)
                  || (al_expr != old_al_expr) || (dl_expr != old_dl_expr))
                 && (loop1 < max_optcycles));
        /* dkr:
         * How about  cf_expr, wlt_expr, dcr_expr  ??
         * I think we should compare these counters here, too!
         */

        if (((optimize & OPT_AL) || (optimize & OPT_DL)) && (use_ssaform)) {
            arg_node = UndoElimSubDiv (arg_node, arg_info);
        }

        /*
         * Now, we enter the second loop consisting of
         *   CF and WLT only.
         */
        while ((wlt_expr != old_wlt_expr) && (optimize & OPT_CF)
               && ((loop1 + loop2) < max_optcycles)) {
            old_wlt_expr = wlt_expr;
            old_cf_expr = cf_expr;

            loop2++;

            DBUG_PRINT ("OPT",
                        ("---------------------------------------- loop TWO, pass %d",
                         loop2));

            /*
             * !! Important !!
             * SSA-form based optimizations are now separated from Non-SSA-optimizations
             * by a single if-clause
             */

            if (use_ssaform) {

                /*
                 * optimization for SSA-form
                 */

                if (optimize & OPT_CF) {
                    arg_node
                      = SSAConstantFolding (arg_node,
                                            INFO_OPT_MODUL (arg_info)); /* ssacf_tab */
                }

                if ((break_after == PH_sacopt)
                    && (break_cycle_specifier == (loop1 + loop2))
                    && (0 == strcmp (break_specifier, "cf"))) {
                    goto INFO;
                }

                /*
                 * This is needed to transform more index vectors in scalars or vice
                 * versa.
                 */
                if (optimize & OPT_WLT) {
                    arg_node = SSAWithloopFoldingWLT (arg_node);
                }

                if ((break_after == PH_sacopt)
                    && (break_cycle_specifier == (loop1 + loop2))
                    && (0 == strcmp (break_specifier, "wlt"))) {
                    goto INFO;
                }
            } else {

                /*
                 * optimization for Non-SSA-form
                 */

                if (optimize & OPT_CF) {
                    arg_node = ConstantFolding (arg_node, arg_info); /* cf_tab */
                    /* srs: CF does not handle the USE mask correctly. */
                    /* quick fix: always rebuild masks after CF */
                    arg_node = GenerateMasks (arg_node, NULL);
                }

                if ((break_after == PH_sacopt)
                    && (break_cycle_specifier == (loop1 + loop2))
                    && (0 == strcmp (break_specifier, "cf"))) {
                    goto INFO;
                }

                /*
                 * This is needed to transform more index vectors in scalars or vice
                 * versa.
                 */
                if (optimize & OPT_WLT) {
                    arg_node = WithloopFoldingWLT (arg_node); /* wlt */
                }

                if ((break_after == PH_sacopt)
                    && (break_cycle_specifier == (loop1 + loop2))
                    && (0 == strcmp (break_specifier, "wlt"))) {
                    goto INFO;
                }
            }
        }

        /*
         * Finally, we apply DCR once again:
         */
        if (use_ssaform) {
            if (optimize & OPT_DCR) {
                arg_node = SSADeadCodeRemoval (arg_node, INFO_OPT_MODUL (arg_info));
            }
        } else {
            if (optimize & OPT_DCR) {
                arg_node = DeadCodeRemoval (arg_node, arg_info);
            }
        }

        /*
         * no break yet!
         */
        do_break = FALSE;
    INFO:
        if (loop1 + loop2 == max_optcycles
            && ((cse_expr != old_cse_expr) || (cf_expr != old_cf_expr)
                || (wlt_expr != old_wlt_expr) || (wlf_expr != old_wlf_expr)
                || (dead_fun + dead_var + dead_expr != old_dcr_expr)
                || (lunr_expr != old_lunr_expr) || (wlunr_expr != old_wlunr_expr)
                || (uns_expr != old_uns_expr) || (lir_expr != old_lir_expr)
                || (wlir_expr != old_wlir_expr))) {
            SYSWARN (("maximal number of optimization cycles reached"));
        }
        PrintStatistics (mem_inl_fun, mem_dead_expr, mem_dead_var, mem_dead_fun,
                         mem_lir_expr, mem_wlir_expr, mem_cf_expr, mem_lunr_expr,
                         mem_wlunr_expr, mem_uns_expr, mem_elim_arrays, mem_wlf_expr,
                         mem_wlt_expr, mem_cse_expr, 0, 0, mem_wls_expr, mem_al_expr,
                         mem_dl_expr, NON_ZERO_ONLY);

        if (!(use_ssaform)) {
            DBUG_DO_NOT_EXECUTE ("PRINT_MASKS", arg_node = FreeMasks (arg_node););
        }
    }

    if (FUNDEF_NEXT (arg_node)) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* @} */
/* @} */ /* defgroup opt */
