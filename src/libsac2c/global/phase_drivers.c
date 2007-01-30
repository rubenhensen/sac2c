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

    syntax_tree = PHrunCompilerSubPhase (SUBPH_loc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_cpp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_prs, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdrivePreprocess (node *syntax_tree)
{
    DBUG_ENTER ("PHDdrivePreprocess");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_hzgwl, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hwlg, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hwlo, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_acn, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_pragma, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_objinit, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveModuleSystem (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveModuleSystem");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rsa, syntax_tree, !global.makedeps);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ans, syntax_tree, !global.makedeps);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_gdp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_pdp, syntax_tree, global.makedeps);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_imp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_uss, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_asf, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveCodeSimplification (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveCodeSimplification");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_w2d, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hce, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_hm, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_flt, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdrivePreTypechecking (node *syntax_tree)
{
    DBUG_ENTER ("PHDdrivePreTypechecking");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rst, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_insvd, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_instc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ses, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_crtwrp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_oan, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_goi, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rso, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rra, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ewt, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2fun, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_elf, syntax_tree, global.elf);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssa, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveTypechecking (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveTypechecking");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_esp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ti, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_eat, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ebt, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_swr, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveExport (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveExport");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_exp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ppi, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveUniquenessCheck (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveUniquenessCheck");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cua, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_cu, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveWrapperCreation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveWrapperCreation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cwc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2funwc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssawc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dfc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_eudt, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveWithLoopEnhancement (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveWithLoopEnhancement");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_accu, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wldp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rtup, syntax_tree, global.optimize.dortup);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlpg, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveOptimization (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveOptimization");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_inl, syntax_tree, global.optimize.doinl);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dfr, syntax_tree, global.optimize.dodfr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dcr, syntax_tree, global.optimize.dodcr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lir, syntax_tree, global.optimize.dolir);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_isaa, syntax_tree,
                               global.optimize.dosaacyc && global.optimize.dodcr);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cyc, syntax_tree, ALWAYS);

    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_scyc, syntax_tree, global.run_stabilization_cycle);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_lir2, syntax_tree, global.optimize.dolir);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_uesd, syntax_tree, global.optimize.doesd);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dfr2, syntax_tree, global.optimize.dodfr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dcr2, syntax_tree, global.optimize.dodcr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rtc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fineat, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_finebt, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlfs, syntax_tree, global.optimize.dowlfs);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_cse2, syntax_tree,
                                         global.optimize.dowlfs && global.optimize.docse);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dcr3, syntax_tree,
                                         global.optimize.dowlfs && global.optimize.dodcr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlpg2, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_isaa2, syntax_tree,
                                         global.optimize.dosaa && global.optimize.dodcr);

    for (int i = 0; i < 3; i++) {
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svprfunr, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.doprfunr);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svtup, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.dotup);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_sveat, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.dotup);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svebt, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.dotup);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svcf, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.docf);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svcse, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.docse);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svcvp, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.docvp);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svwlsimp, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.dowlsimp);
        syntax_tree
          = PHrunCompilerSubPhase (SUBPH_svdcr, syntax_tree,
                                   global.optimize.dosaa && global.optimize.dodcr
                                     && global.optimize.dodcr);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_wrci, syntax_tree, global.optimize.douip);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wlidx, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ivesplit, syntax_tree,
                                         global.optimize.doive && global.optimize.dosaa
                                           && global.optimize.dodcr);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_cvpive, syntax_tree,
                               global.optimize.doive && global.optimize.dosaa
                                 && global.optimize.dodcr && global.optimize.docvp);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_cseive, syntax_tree,
                               global.optimize.doive && global.optimize.dosaa
                                 && global.optimize.dodcr && global.optimize.docse);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_iveras, syntax_tree,
                                         global.optimize.doive && global.optimize.dosaa
                                           && global.optimize.dodcr);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_esv, syntax_tree,
                                         global.optimize.dosaa && global.optimize.dodcr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lirive, syntax_tree,
                                         ((global.optimize.dosaa && global.optimize.dodcr)
                                          || global.optimize.doive)
                                           && global.optimize.dolir);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dcrive, syntax_tree,
                                         ((global.optimize.dosaa && global.optimize.dodcr)
                                          || global.optimize.doive)
                                           && global.optimize.dodcr);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_fdi, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_optstat, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_typestat, syntax_tree, ALWAYS);

    syntax_tree = PHrunCompilerSubPhase (SUBPH_pfap, syntax_tree, global.doprofile);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveWithLoopTransformation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveWithLoopTransformation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltussa, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltfun2lac, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltlacinl, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltra, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltlac2fun, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_wltssa, syntax_tree, ALWAYS);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_wltcvp, syntax_tree, global.optimize.docvp);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_wltdcr, syntax_tree, global.optimize.dodcr);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveMultithreading (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveMultithreading");

#ifndef BEMT

    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdinit, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_createmtfuns, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdlift, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_sched, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rmspmd, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_conclac2fun, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_concssa, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));

#endif

#ifndef PRODUCTION
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_tem, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_crwiw, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_pem, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_cdfg, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_asmra, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_crece, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_cegro, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_repfun, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_concel, syntax_tree, global.mtmode == MT_mtstblock);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_abort, syntax_tree, global.mtmode == MT_mtstblock);
#endif

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveMemoryManagement (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveMemoryManagement");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_simd, syntax_tree, global.simd);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_asd, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_copy, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_alloc, syntax_tree, ALWAYS);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_emmdcr, syntax_tree, global.optimize.dodcr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rci, syntax_tree, global.optimize.douip);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_shal, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ia, syntax_tree, global.optimize.dosrf);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lro, syntax_tree,
                                         global.optimize.dosrf && global.optimize.dolro);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_aa, syntax_tree, global.optimize.dosrf);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_srce, syntax_tree, global.optimize.douip);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_frc, syntax_tree, global.optimize.douip);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_sr, syntax_tree,
                                         global.optimize.douip && global.optimize.dosrf);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_rb, syntax_tree,
                               global.optimize.doipc
                                 || (global.optimize.douip && global.optimize.dodr));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ipc, syntax_tree, global.optimize.doipc);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dr, syntax_tree,
                                         global.optimize.douip && global.optimize.dodr);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_emmdcr2, syntax_tree, global.optimize.dodcr);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_unshal, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcm, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rco, syntax_tree, global.optimize.dorco);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mvsmi, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_re, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdrivePrecompilation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdrivePrecompilation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rec, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdfunfix, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rera, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_reso, syntax_tree, ALWAYS);

#ifdef BEMT
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdinit, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_createmtfuns, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
    syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdlift, syntax_tree,
                                         (global.mtmode == MT_createjoin)
                                           || (global.mtmode == MT_startstop));
#endif

    syntax_tree = PHrunCompilerSubPhase (SUBPH_sls, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mmv, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_moi, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rcs, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_fpc, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_tcp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_mng, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rid, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveCodeGeneration (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveCodeGeneration");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_tot, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_comp, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_prt, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_frtr, syntax_tree, ALWAYS);

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveBinaryCodeCreation (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveBinaryCodeCreation");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_hdep, syntax_tree, ALWAYS);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ivcc, syntax_tree, ALWAYS);
    syntax_tree
      = PHrunCompilerSubPhase (SUBPH_crlib, syntax_tree, global.filetype != F_prog);

    DBUG_RETURN (syntax_tree);
}
