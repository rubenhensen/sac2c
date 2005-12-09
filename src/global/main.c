/*
 *
 * $Id$
 *
 */

/*
 *  this file contains the main function of the SAC->C compiler!
 */

#include "phase.h"

#include "config.h"
#include "convert.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "usage.h"
#include "print.h"
#include "new_typecheck.h"
#include "type_statistics.h"
#include "optimize.h"
#include "filemgr.h"
#include "allocation.h"
#include "rcphase.h"
#include "scnprs.h"
#include "objinit.h"
#include "objects.h"
#include "uniquecheck.h"
#include "wltransform.h"
#include "concurrent.h"
#include "precompile.h"
#include "compile.h"
#include "annotate_fun_calls.h"
#include "libstat.h"
#include "resolveall.h"
#include "annotatenamespace.h"
#include "gatherdependencies.h"
#include "importsymbols.h"
#include "usesymbols.h"
#include "libstat.h"
#include "ccmanager.h"
#include "libbuilder.h"
#include "prepareinline.h"
#include "dependencies.h"
#include "resolvepragma.h"
#include "objanalysis.h"
#include "resource.h"
#include "options.h"
#include "multithread.h"
#include "WLEnhancement.h"
#include "export.h"
#include "traverse.h"
#include "ToOldTypes.h"
#include "ToNewTypes.h"
#include "setup.h"
#include "codesimplification.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * THE FOLLOWING MACROS ARE DEPRECATED!!  DO NOT USE!!!
 */

#define NOTE(s) CTInote s;

#define NOTE_COMPILER_PHASE                                                              \
    {                                                                                    \
        if (global.verbose_level > 1) {                                                  \
            fprintf (stderr, "\n** %d: %s: ...\n", global.compiler_phase,                \
                     PHphaseName (global.compiler_phase));                               \
        }                                                                                \
    }

#define ABORT_ON_ERROR CTIabortOnError ()

#define PHASE_PROLOG                                                                     \
    CHECK_DBUG_START;                                                                    \
    /* empty */

#ifdef SHOW_MALLOC
#define PHASE_DONE_EPILOG                                                                \
    CTIabortOnError ();                                                                  \
    DBUG_EXECUTE ("MEM_LEAK", ILIBdbugMemoryLeakCheck (););
#else
#define PHASE_DONE_EPILOG CTIabortOnError ();
#endif

#define PHASE_EPILOG CHECK_DBUG_STOP;

#ifndef DBUG_OFF
#define CHECK_DBUG_START                                                                 \
    {                                                                                    \
        if ((global.my_dbug) && (!global.my_dbug_active)                                 \
            && (global.compiler_phase >= global.my_dbug_from)                            \
            && (global.compiler_phase <= global.my_dbug_to)) {                           \
            DBUG_PUSH (global.my_dbug_str);                                              \
            global.my_dbug_active = 1;                                                   \
        }                                                                                \
    }
#define CHECK_DBUG_STOP                                                                  \
    {                                                                                    \
        if ((global.my_dbug) && (global.my_dbug_active)                                  \
            && (global.compiler_phase >= global.my_dbug_to)) {                           \
            DBUG_POP ();                                                                 \
            global.my_dbug_active = 0;                                                   \
        }                                                                                \
    }
#else /* DBUG_OFF */
#define CHECK_DBUG_START
#define CHECK_DBUG_STOP
#endif /* DBUG_OFF */

/*
 *  And now, the main function which triggers the whole compilation.
 */

int
main (int argc, char *argv[])
{
    node *syntax_tree = NULL;
    stringset_t *dependencies;

    /*
     * Unfortunately, a few initializations must be done before running the setup
     * compiler phase. Several options like -h or -V should be handled before
     * compile time output with respect to a regular compiler run is produced.
     * In particular for the -h option we need the global variables to
     * be initialized. As long as we work with pre-allocated buffers of fixed
     * length (which should be changed), this initialization requires allocation
     * to work properly.
     *
     * Furthermore, options like libstat need to be checked after the setup
     * phase but prior to the following compiler phases. This is achieved by
     * calling OPTcheckPostSetupOptions.
     */

    SETUPdoSetupCompiler (argc, argv);

    OPTcheckPreSetupOptions ();

    syntax_tree = PHrunCompilerPhase (PH_setup, syntax_tree);

    OPTcheckPostSetupOptions ();

    /*
     *  Finally the compilation process is started.
     */

    global.compiler_phase = PH_scanparse;

    /*
     * Scan/Parse
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = SPdoScanParse ();

    RSPdoResolvePragmas (syntax_tree);

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_scanparse)
        goto BREAK;
    global.compiler_phase++;

    PHASE_PROLOG;

    /*
     * Module system use
     */
    NOTE_COMPILER_PHASE;
    /*
     * print dependencies if requested
     */
    if (global.makedeps) {
        syntax_tree = PHrunCompilerSubPhase (SUBPH_gdp, syntax_tree);
        DEPdoPrintDependencies (syntax_tree);
    }

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rsa, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ans, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_gdp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_imp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_uss, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dep, syntax_tree);

    ABORT_ON_ERROR;

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_module)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Object init phase
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = OIdoObjectInit (syntax_tree); /* objinit_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_objinit)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Code simplification
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = CSdoCodeSimplification (syntax_tree);
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_simplify)
        goto BREAK;
    global.compiler_phase++;

    /*
     * creating SSA form
     */
    syntax_tree = PHrunCompilerPhase (PH_pretypecheck, syntax_tree);

    /*
     * typecheck
     */
    global.compiler_phase++;
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    syntax_tree = PHrunCompilerSubPhase (SUBPH_tc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_nt2ot, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_swr, syntax_tree);

    PHASE_DONE_EPILOG;
#if 1
    if (global.doprofile) {
        syntax_tree = PFdoProfileFunCalls (syntax_tree); /* profile_tab */
    }
#endif
    PHASE_EPILOG;

    if (global.break_after == PH_typecheck)
        goto BREAK;
    global.compiler_phase++;

    /*
     * export
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    syntax_tree = PHrunCompilerSubPhase (SUBPH_oan, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_exp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_asf, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ppi, syntax_tree);

    PHASE_DONE_EPILOG;

    PHASE_EPILOG;

    if (global.break_after == PH_export)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Object handling
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
#if 0
  syntax_tree = OBJdoHandleObjects( syntax_tree);  /* obj_tab */
#endif
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_objects)
        goto BREAK;
    global.compiler_phase++;

    /*
     * uniqueness checks
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = UNQdoUniquenessCheck (syntax_tree); /* unique_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_uniquecheck)
        goto BREAK;
    global.compiler_phase++;

    /*
     * user-type elimination and wrapper code creation
     */

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cwc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2funwc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssawc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_dfc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_eudt, syntax_tree);

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_elimudt)
        goto BREAK;

    /*
     * withloop enhancement
     */
    syntax_tree = PHrunCompilerPhase (PH_wlenhance, syntax_tree);

    /*
     * Optimizations
     */
    syntax_tree = PHrunCompilerPhase (PH_sacopt, syntax_tree);
    syntax_tree = TSdoPrintTypeStatistics (syntax_tree);

    /*
     * WLtransform
     */
    global.compiler_phase++;
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    syntax_tree = WLTRAdoWlTransform (syntax_tree); /* wltrans_tab */

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_wltrans)
        goto BREAK;

    /*
     * Explicit memory allocation
     */
    syntax_tree = PHrunCompilerPhase (PH_alloc, syntax_tree);

    global.compiler_phase++;
    PHASE_PROLOG;

    /*
     * MT I
     */
    switch (global.mtmode) {
    case MT_none:
        break;
    case MT_createjoin:
        break;
    case MT_startstop:
        break;
    case MT_mtstblock:
        NOTE_COMPILER_PHASE;
        NOTE (("Using mt/st-block version of multithreading (MT3)"));
        /* this version of multithreading is only for code in SSA-form */

        syntax_tree = MUTHdoMultiThread (syntax_tree);

        PHASE_DONE_EPILOG;
        break;
    }
    PHASE_EPILOG;

    if (global.break_after == PH_multithread)
        goto BREAK;

    /*
     * Reference counting
     */
    syntax_tree = PHrunCompilerPhase (PH_refcnt, syntax_tree);

    /*
     * MT II
     */
    global.compiler_phase++;
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    switch (global.mtmode) {
    case MT_none:
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl2, syntax_tree);
        break;
    case MT_createjoin:
        NOTE (("Using create-join version of multithreading (MT1)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl2, syntax_tree);
        syntax_tree = CONCdoConcurrent (syntax_tree);
        break;
    case MT_startstop:
        NOTE (("Using start-stop version of multithreading (MT2)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl2, syntax_tree);
        syntax_tree = CONCdoConcurrent (syntax_tree);
        break;
    case MT_mtstblock:
        CTIabort ("Mt/st-block version of multithreading de-activated !!");
        /* following comment concerning for mt/st-block version:
         * The core problem is that new-mt reuses the FUNDEF ATTRIB attribute
         * and thereby destroys its old contents. Unfortunately, it has turned
         * that this information is vital for precompile.
         */

        /* something's missing... */

        syntax_tree = PHrunCompilerSubPhase (SUBPH_ussa2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_fun2lac2, syntax_tree);
        syntax_tree = PHrunCompilerSubPhase (SUBPH_lacinl2, syntax_tree);
        break;
    }

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_multithread_finish)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Precompile
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = PRECdoPrecompile (syntax_tree); /* precomp_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_precompile)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Compile
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    /*
     * TODO: Compile should work with new types
     */
    syntax_tree = TOTdoToOldTypes (syntax_tree);
    syntax_tree = COMPdoCompile (syntax_tree); /* comp_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_compile)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Code generation
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    PRTdoPrint (syntax_tree);
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_genccode)
        goto BREAK;
    global.compiler_phase++;

    /*
     * prior to freeing the syntax tree, we have to save the dependencies
     * as these are needed for calling the CC
     */

    dependencies = MODULE_DEPENDENCIES (syntax_tree);
    MODULE_DEPENDENCIES (syntax_tree) = NULL;

    /*
     *  After the C file has been written, the syntax tree may be released.
     */

    syntax_tree = FREEdoFreeTree (syntax_tree);

    /*
     * Invoke CC
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    if (global.filetype != F_prog) {
        /*
         * finally generate the dependency table.
         * we do this here as new dependencies may be introduced
         * during the compilation steps up to here
         */
        DEPgenerateDependencyTable (dependencies);
    }

    if (global.gen_cccall) {
        /*
         * enable system call tracking
         */
        ILIBsystemCallStartTracking ();
    }

    CCMinvokeCC (dependencies);

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    global.compiler_phase++;

    /*
     * Create Library
     */
    PHASE_PROLOG;
    if (global.filetype != F_prog) {
        NOTE_COMPILER_PHASE;

        LIBBcreateLibrary (dependencies);
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    if (global.gen_cccall) {
        /*
         * stop tracking and close file
         */
        ILIBsystemCallStopTracking ();
    }

    /*
     * now we can free the set of dependencies now as well
     */
    dependencies = STRSfree (dependencies);

    global.compiler_phase = PH_final;

BREAK:

    CTIterminateCompilation (global.compiler_phase, global.break_specifier, syntax_tree);

    return (0);
}
