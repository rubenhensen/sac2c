/*
 *
 * $Log$
 * Revision 3.90  2004/12/01 16:33:04  ktr
 * Call to SSAundoSsa corrected.
 *
 * Revision 3.89  2004/11/30 16:11:40  sah
 * enabled phase pro/epilogue for object phase
 *
 * Revision 3.88  2004/11/29 19:10:02  sah
 * removed old phases
 *
 * Revision 3.87  2004/11/28 18:13:40  ktr
 * changed call to EMRdoRefCountPhase
 *
 * ... [eliminated]
 *
 */

/*
 *  this file contains the main function of the SAC->C compiler!
 */

#include "config.h"
#include "convert.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "ssa.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "usage.h"
#include "lac2fun.h"
#include "fun2lac.h"
#include "flatten.h"
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
#include "usesymbols.h"
#include "libstat.h"
#include "ccmanager.h"
#include "libbuilder.h"
#include "prepareinline.h"
#include "dependencies.h"
#include "resolvepragma.h"
#include "objanalysis.h"
#include "resource.h"
#include "interrupt.h"
#include "options.h"
#include "multithread.h"
#include "WLEnhancement.h"
#include "export.h"
#include "traverse.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 *  And now, the main function which triggers the whole compilation.
 */

int
main (int argc, char *argv[])
{
    node *syntax_tree = NULL;
    stringset_t *dependencies;

    /*
     * Initializations
     */

    global.argc = argc;
    global.argv = argv;

#ifdef SHOW_MALLOC
    ILIBcomputeMallocAlignStep ();
#endif

    GLOBinitializeGlobal ();
    FMGRinitPaths ();
    IRQsetupInterruptHandlers ();
    DUPinitDupTree ();

    OPTanalyseCommandline (argc, argv);
    OPTcheckOptionConsistency ();

    if (global.sacfilename == NULL) {
        global.puresacfilename = "stdin";
    }

    ABORT_ON_ERROR;

    global.compiler_phase = PH_setup;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    /*
     * Now, we read in the sac2c configuration files.
     */

    RSCevaluateConfiguration (global.target_name);

    /*
     * Now, we set our search paths for the source program, module declarations,
     * and module implementations...
     *
     * The original search path is ".".
     * Then, additional paths specified by the respective compiler options are
     * appended after having been transformed into absolute paths.
     * If this has happened, the current directory is moved to the end of the
     * path list because those paths specified on the command line are intended
     * to have a higher priority.
     * At last, the paths specified by environment variables are appended.
     * These have a lower priority.
     * At very last, the required paths for using the SAC standard library
     * relative to the shell variable SAC_HOME are added. These have the
     * lowest priority.
     */

    FMGRrearrangePaths ();

    /*
     * Now, we create tmp directories for files generated during the
     * compilation process.
     *
     * Actually, only one temp directory is created whose name may be
     * accessed trough the global variable global.tmp_dirname
     * which is defined in globals.c.
     */

#ifdef HAVE_MKDTEMP
    /* mkdtemp is safer than tempnam and recommended */
    /* on linux/bsd platforms.                       */

    /* malloc is used here as tempnam uses it */
    /* internally as well.                    */

    global.tmp_dirname = (char *)malloc (strlen (global.config.mkdir) + 12);
    global.tmp_dirname = strcpy (global.tmp_dirname, global.config.tmpdir);
    global.tmp_dirname = strcat (global.tmp_dirname, "/SAC_XXXXXX");

    global.tmp_dirname = mkdtemp (global.tmp_dirname);

    if (global.tmp_dirname == NULL) {
        SYSABORT (("System failed to create temporary directory.\n"));
    }
#else
    /* the old way for platforms not */
    /* supporting mkdtemp            */

    global.tmp_dirname = tempnam (global.config.tmpdir, "SAC_");

    ILIBsystemCall ("%s %s", global.config.mkdir, global.tmp_dirname);
#endif

    ABORT_ON_ERROR;

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_setup)
        goto BREAK;
    global.compiler_phase++;

    /*
     * If sac2c was started with the option -libstat,
     * then the library status is printed to stdout and the
     * compilation process is terminated immediately.
     */

    if (global.libstat) {
        LIBSprintLibStat (global.sacfilename);

        ERRcleanUp ();

        exit (0);
    }

    /*
     *  Finally the compilation process is started.
     */

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
    NOTE (("Processing use and import statements..."));
    RSAdoResolveAll (syntax_tree);
    NOTE (("Resolving namespaces..."));
    ANSdoAnnotateNamespace (syntax_tree);
    NOTE (("Getting used symbols..."));
    USSdoUseSymbols (syntax_tree);
    NOTE (("Resolving dependencies..."));
    DEPdoResolveDependencies (syntax_tree);

    ABORT_ON_ERROR;

    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_use)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Object init phase
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
#if 0
  syntax_tree = OIdoObjInit( syntax_tree);  /* objinit_tab */
#endif
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_objinit)
        goto BREAK;
    global.compiler_phase++;

    /*
     * flatten
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = FLATdoFlatten (syntax_tree); /* flat_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_flatten)
        goto BREAK;
    global.compiler_phase++;

    /*
     * typecheck
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    syntax_tree = NTCdoNewTypeCheck (syntax_tree); /* ntc_tab */

    PHASE_DONE_EPILOG;
#ifndef NEW_AST
    if (profileflag != 0) {
        syntax_tree = PFdoProfileFunCalls (syntax_tree); /* profile_tab */
    }
#endif /* NEW_AST */
    PHASE_EPILOG;

    if (global.break_after == PH_typecheck)
        goto BREAK;
    global.compiler_phase++;

    /*
     * export
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    OANdoObjectAnalysis (syntax_tree);
    EXPdoExport (syntax_tree);
    PPIdoPrepareInline (syntax_tree);
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
    syntax_tree = OBJdoHandleObjects (syntax_tree); /* obj_tab */
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
     * withloop enhancement
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = WLEdoWlEnhancement (syntax_tree); /* see WLEnhancement.c */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_wlenhance)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Optimizations
     */
    PHASE_PROLOG;
    /*if (optimize) {*/
    /* TODO - the new optimize flags disables the old kind of checking for
     * optimizations */
    NOTE_COMPILER_PHASE;
    syntax_tree = OPTdoOptimize (syntax_tree); /* see optimize.c, Optimize() */
    PHASE_DONE_EPILOG;
    /*  }*/
    PHASE_EPILOG;

    syntax_tree = TSdoPrintTypeStatistics (syntax_tree);

    if (global.break_after == PH_sacopt)
        goto BREAK;
    global.compiler_phase++;

    /*
     * WLtransform
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = WLTRAdoWlTransform (syntax_tree); /* wltrans_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_wltrans)
        goto BREAK;
    global.compiler_phase++;

    /*
     * Refcount I
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = EMAdoAllocation (syntax_tree); /* emalloc_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_alloc)
        goto BREAK;
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
    global.compiler_phase++;

    /*
     * Refcount II
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = EMRdoRefCountPhase (syntax_tree);
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (global.break_after == PH_refcnt)
        goto BREAK;
    global.compiler_phase++;

    /*
     * MT II
     */
    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;

    switch (global.mtmode) {
    case MT_none:
        syntax_tree = SSAundoSsa (syntax_tree);
        break;
    case MT_createjoin:
        NOTE (("Using create-join version of multithreading (MT1)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = SSAundoSsa (syntax_tree);
        syntax_tree = CONCdoConcurrent (syntax_tree);
        break;
    case MT_startstop:
        NOTE (("Using start-stop version of multithreading (MT2)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = SSAundoSsa (syntax_tree);
        syntax_tree = CONCdoConcurrent (syntax_tree);
        break;
    case MT_mtstblock:
        SYSABORT (("Mt/st-block version of multithreading de-activated !!"));
        /* following comment concerning for mt/st-block version:
         * The core problem is that new-mt reuses the FUNDEF ATTRIB attribute
         * and thereby destroys its old contents. Unfortunately, it has turned
         * that this information is vital for precompile.
         */

        /* something's missing... */

        syntax_tree = SSAundoSsa (syntax_tree);
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

    /*
     * now we can free the set of dependencies now as well
     */
    dependencies = STRSfree (dependencies);

    global.compiler_phase = PH_final;

BREAK:

    if (global.compiler_phase < PH_final) {
        if (global.compiler_phase < PH_scanparse) {
            RSCshowResources ();
        } else {
            if (global.print_after_break && (global.compiler_phase <= PH_compile)) {
                PRTdoPrint (syntax_tree);
            }
            syntax_tree = FREEdoFreeTree (syntax_tree);
        }
    }

    /*
     *  Finally, we do some clean up ...
     */

    /*
     *  ... and display a success message.
     */

    NEWLINE (2);
    NOTE2 (("*** Compilation successful ***"));

    if (global.compiler_phase < PH_final) {
        NOTE2 (
          ("*** BREAK after: %s", global.compiler_phase_name[global.compiler_phase]));
        if (global.break_specifier[0] != '\0') {
            NOTE2 (("*** BREAK specifier: '%s`", global.break_specifier));
        }
    }

#ifdef SHOW_MALLOC
    NOTE2 (("*** Maximum allocated memory (bytes):   %s",
            CVintBytes2String (global.max_allocated_mem)));
    NOTE2 (("*** Currently allocated memory (bytes): %s",
            CVintBytes2String (global.current_allocated_mem)));
#endif

    NOTE2 (("*** Exit code 0"));
    NOTE2 (("*** 0 error(s), %d warning(s)", global.warnings_cnt));
    NEWLINE (2);

    return (0);
}
