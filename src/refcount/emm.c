/*
 * $Id$
 */
#include "emm.h"

#include "phase.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "globals.h"
#include "dbug.h"
#include "print.h"

/** <!--********************************************************************-->
 *
 * @fn EMAprintPreFun
 *
 *   @brief
 *
 *   @param  node *arg_node
 *   @param  info *arg_info
 *
 *   @return node *           :  the transformed syntax tree
 *
 *****************************************************************************/
node *
EMAprintPreFun (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EMAprintPreFun");

    switch (NODE_TYPE (arg_node)) {
    case N_arg:
        if (ARG_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* ALIAS */");
        }
        if (AVIS_ISALIAS (ARG_AVIS (arg_node))) {
            fprintf (global.outfile, " /* alias */");
        }
        break;
    case N_ret:
        if (RET_ISALIASING (arg_node)) {
            fprintf (global.outfile, " /* ALIAS */");
        }
        break;
    case N_vardec:
        if (AVIS_ISALIAS (VARDEC_AVIS (arg_node))) {
            INDENT;
            fprintf (global.outfile, " /* alias */\n");
        }
        break;
    default:
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn EMMdoMemoryManagement
 *
 *   @brief
 *
 *   @param  node *syntax_tree:  the whole syntax tree
 *   @return node *           :  the transformed syntax tree
 *
 *****************************************************************************/
node *
EMMdoMemoryManagement (node *syntax_tree)
{
    DBUG_ENTER ("EMMdoMemoryManagement");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) == N_module),
                 "EMMdoMemoryManagement not started with modul node");

    /*
     * SIMD inference
     */
    if (global.simd) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_simd, syntax_tree);
    }

    /*
     * AUD SCL distinction
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_asd, syntax_tree);

    /*
     * Explicit copy
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_copy, syntax_tree);

    /*
     * Explicit allocation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_alloc, syntax_tree);

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_emmdcr, syntax_tree);
    }

    /*
     * Reuse candidate inference
     */
    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rci, syntax_tree);
    }

    /*
     * Interface analysis
     */
    TRAVsetPreFun (TR_prt, EMAprintPreFun);
    if (global.optimize.dosrf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ia, syntax_tree);
    }

    /*
     * Loop reuse optimization
     */
    if ((global.optimize.dosrf) && (global.optimize.dolro)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lro, syntax_tree);
    }

    /*
     * Alias analysis
     */
    if (global.optimize.dosrf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_aa, syntax_tree);
    }

    /*
     * Filter reuse candidates
     */
    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_frc, syntax_tree);
    }

    /*
     * Static reuse
     */
    if ((global.optimize.douip) && (global.optimize.dosrf)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_sr, syntax_tree);
    }

    /*
     * Reuse case dependent branching
     */
    if ((global.optimize.doipc) || ((global.optimize.douip) && (global.optimize.dodr))) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rb, syntax_tree);
    }

    /*
     * In Place computation
     */
    if (global.optimize.doipc) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ipc, syntax_tree);
    }

    /*
     * Data reuse
     */
    if ((global.optimize.douip) && (global.optimize.dodr)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dr, syntax_tree);
    }

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_emmdcr2, syntax_tree);
    }

    TRAVsetPreFun (TR_prt, NULL);

    /*
     * Reference counting
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rc, syntax_tree);

    /*
     * Refcount minimization
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcm, syntax_tree);

    /*
     * Reference counting optimizations
     */
    if (global.optimize.dorco) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rco, syntax_tree);
    }

    /*
     * Lift memory management instructions for shared variables from SPMD
     * functions
     */
    if ((global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_mvsmi, syntax_tree);
    }

    /*
     * Reuse elimination
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_re, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
