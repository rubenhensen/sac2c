/*
 *
 * $Id$
 *
 */

#include "dbug.h"
#include "types.h"
#include "phase.h"
#include "globals.h"
#include "ctinfo.h"
#include "dependencies.h"
#include "optimize.h"
#include "annotate_fun_calls.h"
#include "type_statistics.h"
#include "tree_basic.h"

#include "phase_drivers.h"

node *
PHDdriveScanParse (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveScanParse");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_loc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_cpp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_sp, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdrivePreprocess (node *syntax_tree)
{
    DBUG_ENTER ("PHDdrivePreprocess");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_hzgwl, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hwlg, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hwlo, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_acn, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_pragma, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_objinit, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveModuleSystem (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveModuleSystem");

    if (global.makedeps) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_gdp, syntax_tree);
        DEPdoPrintDependencies (syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rsa, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ans, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_gdp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_imp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_uss, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_asf, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveCodeSimplification (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveCodeSimplification");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_w2d, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hce, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hm, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_flat, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdrivePreTypechecking (node *syntax_tree)
{
    DBUG_ENTER ("PHDdrivePreTypechecking");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rst, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_insvd, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_instc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ses, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_crtwrp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_oan, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_goi, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rso, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rra, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ewt, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2fun, syntax_tree);

    if (global.elf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_elf, syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssa, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveTypechecking (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveTypechecking");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_tc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_eat, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ebt, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_swr, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveExport (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveExport");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_exp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ppi, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveUniquenessCheck (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveUniquenessCheck");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cua, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_cu, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveWrapperCreation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveWrapperCreation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cwc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2funwc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssawc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dfc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_eudt, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveWithLoopEnhancement (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveWithLoopEnhancement");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_accu, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wldp, syntax_tree);

    if (global.optimize.dortup) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rtup, syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlpg, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveOptimization (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveOptimization");

    /*
     * apply INL (inlining)
     */
    if (global.optimize.doinl) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_inl, syntax_tree);
    }

    /*
     * apply DFR (dead function removal)
     */
    if (global.optimize.dodfr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dfr, syntax_tree);
    }

    /*
     * Array elimination (FIX ME!)
     */

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dcr, syntax_tree);
    }

    /**
     * Loop invariant removal
     */
    if (global.optimize.dolir) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lir, syntax_tree);
    }

    /*
     * Insert symbolic array attributes
     */
    if (global.optimize.dosaacyc && global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_isaa, syntax_tree);
    }

    /*
     * Intra-functional optimizations
     * THE CYCLE
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_intraopt, syntax_tree);

    /*
     * Loop invariant removal
     */
    if (global.optimize.dolir) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lir2, syntax_tree);
    }

    /*
     * UESD
     */
    if (global.optimize.doesd) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_uesd, syntax_tree);
    }

    /*
     * Dead function removal
     */
    if (global.optimize.dodfr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dfr2, syntax_tree);
    }

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dcr2, syntax_tree);
    }

    /*
     * apply RTC (final type inference)
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rtc, syntax_tree);

    /*
     * apply FINEAT (final tye variable elimination)
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fineat, syntax_tree);

    /*
     * apply FINEBT (final bottom type elimination)
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_finebt, syntax_tree);

    /*
     * With-loop fusion
     *
     * Fusion must run after final type inference because the type system
     * cannot handle multi-operator with-loops.
     */
    if (global.optimize.dowlfs) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_wlfs, syntax_tree);

        /*
         * Common subexpression elimination
         */
        if (global.optimize.docse) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_cse2, syntax_tree);
        }

        /*
         * Dead code removal
         */
        if (global.optimize.dodcr) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_dcr3, syntax_tree);
        }
    }

    /*
     * !!! If they should ever work again, WLAA, TSI, and AP must run here
     */

    /*
     * Another MANDATORY run of WLPG. This is necessary to prevent AKSIV
     * with-loops to arrive at wltransform
     *
     * with-loop partition generation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlpg2, syntax_tree);

    /*
     * Insert shape variables
     */
    if (global.optimize.dosaa && global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_isaa2, syntax_tree);

        for (int i = 0; i < 3; i++) {
            if (global.optimize.doprfunr) {
                syntax_tree = PHrunCompilerSubPhase (SUBPH_svprfunr, syntax_tree);
            }

            if (global.optimize.dotup) {
                syntax_tree = PHrunCompilerSubPhase (SUBPH_svtup, syntax_tree);
                syntax_tree = PHrunCompilerSubPhase (SUBPH_sveat, syntax_tree);
                syntax_tree = PHrunCompilerSubPhase (SUBPH_svebt, syntax_tree);
            }

            if (global.optimize.docf) {
                syntax_tree = PHrunCompilerSubPhase (SUBPH_svcf, syntax_tree);
            }

            if (global.optimize.docse) {
                syntax_tree = PHrunCompilerSubPhase (SUBPH_svcse, syntax_tree);
            }

            if (global.optimize.docvp) {
                syntax_tree = PHrunCompilerSubPhase (SUBPH_svcvp, syntax_tree);
            }
        }

        if (global.optimize.dowlsimp) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_svwlsimp, syntax_tree);
        }

        if (global.optimize.dodcr) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_svdcr, syntax_tree);
        }
    }

    /*
     * Withloop reuse candidate inference
     */
    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_wrci, syntax_tree);
    }

    /*
     * annotate offset scalars on with-loops
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlidx, syntax_tree);

    /*
     * apply index vector elimination (dependent on saa, as of 2006-11-30)
     */
    if (global.optimize.doive && global.optimize.dosaa && global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ivesplit, syntax_tree);

        /*
         * Constant and variable propagation
         */
        if (global.optimize.docvp) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_cvpive, syntax_tree);
        }

        /*
         * Common subexpression elimination
         */
        if (global.optimize.docse) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_cseive, syntax_tree);
        }

        /*
         * IVE Reuse Withloop Offsets and Scalarize Index Vectors
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_iveras, syntax_tree);
    }

    /*
     * Eliminate shape variables
     */
    if (global.optimize.dosaa && global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_esv, syntax_tree);
    }

    if ((global.optimize.dosaa && global.optimize.dodcr) || global.optimize.doive) {
        /*
         * Loop Invariant Removal
         */
        if (global.optimize.dolir) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_lirive, syntax_tree);
        }

        /*
         * Dead code removal after ive
         */
        if (global.optimize.dodcr) {
            syntax_tree = PHrunCompilerSubPhase (SUBPH_dcrive, syntax_tree);
        }
    }

    /*
     * apply FDI (free dispatch information)
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fdi, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_optstat, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_typestat, syntax_tree);

    if (global.doprofile) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_pfap, syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveWithLoopTransformation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveWithLoopTransformation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltussa, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltfun2lac, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltlacinl, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltra, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltlac2fun, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltssa, syntax_tree);

    if (global.optimize.docvp) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_wltcvp, syntax_tree);
    }

    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_wltdcr, syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveMultithreading (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveMultithreading");

#ifndef BEMT

    if ((global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop)) {

        syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdinit, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_createmtfuns, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdlift, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_sched, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rmspmd, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_conclac2fun, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_concssa, syntax_tree);
    }

#endif

#ifndef PRODUCTION
    if (global.mtmode == MT_mtstblock) {

        syntax_tree = PHrunCompilerSubPhase (SUBPH_tem, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_crwiw, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_pem, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_cdfg, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_asmra, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_crece, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_cegro, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_repfun, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_concel, syntax_tree);

        syntax_tree = PHrunCompilerSubPhase (SUBPH_abort, syntax_tree);
    }

#endif

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveMemoryManagement (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveMemoryManagement");

    if (global.simd) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_simd, syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_asd, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_copy, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_alloc, syntax_tree);

    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_emmdcr, syntax_tree);
    }

    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rci, syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_shal, syntax_tree);

    if (global.optimize.dosrf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ia, syntax_tree);
    }

    if ((global.optimize.dosrf) && (global.optimize.dolro)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lro, syntax_tree);
    }

    if (global.optimize.dosrf) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_aa, syntax_tree);
    }

    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_srce, syntax_tree);
    }

    if (global.optimize.douip) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_frc, syntax_tree);
    }

    if ((global.optimize.douip) && (global.optimize.dosrf)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_sr, syntax_tree);
    }

    if ((global.optimize.doipc) || ((global.optimize.douip) && (global.optimize.dodr))) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rb, syntax_tree);
    }

    if (global.optimize.doipc) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ipc, syntax_tree);
    }

    if ((global.optimize.douip) && (global.optimize.dodr)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_dr, syntax_tree);
    }

    if (global.optimize.dodcr) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_emmdcr2, syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_unshal, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rc, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcm, syntax_tree);

    if (global.optimize.dorco) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rco, syntax_tree);
    }

    if ((global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop)) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_mvsmi, syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_re, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdrivePrecompilation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdrivePrecompilation");

    /*
     * restore non-ssa form
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl, syntax_tree);

    /*
     * Remove External Code
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rec, syntax_tree);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdfunfix, syntax_tree);

    /*
     * Restore Reference Args
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rera, syntax_tree);

    /*
     * Restore Global Objects
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_reso, syntax_tree);

#ifdef BEMT
    /*
     * Create Multithreaded Code
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdinit, syntax_tree);

    /*
     * Create MT-funs for exported and provided functions in modules
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_createmtfuns, syntax_tree);

    /*
     * Lift SPMD blocks to SPMD functions
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdlift, syntax_tree);
#endif

    /*
     * Set Linksign
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_sls, syntax_tree);

    /*
     * MarkMemVals
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mmv, syntax_tree);

    /*
     * Manage Object Initialisers
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_moi, syntax_tree);

    /*
     * Resolve Code Sharing in With-Loops
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcs, syntax_tree);

    /*
     * Function precompilation
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fpc, syntax_tree);

    /*
     * Type conversions
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_tcp, syntax_tree);

#if 0  
  /*
   * Adjusting fold functions ( MT only )
   */
  if ( global.mtmode != MT_none) {
    syntax_tree = PHrunCompilerSubPhase( SUBPH_aff, syntax_tree);
  }
#endif

    /*
     * Mark Noop Grids
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mng, syntax_tree);

    /*
     * Rename identifiers
     */
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rid, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveCodeGeneration (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveCodeGeneration");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_tot, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_comp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_prt, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_frtr, syntax_tree);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveBinaryCodeCreation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveBinaryCodeCreation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_hdep, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ivcc, syntax_tree);

    if (global.filetype != F_prog) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_crlib, syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}
