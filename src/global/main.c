/*
 *
 * $Log$
 * Revision 3.96  2005/03/10 09:41:09  cg
 * Reorganized compiler setup phase.
 * This is just the first step towards reorganizing the triggering
 * of the entire compilation process.
 *
 * Revision 3.95  2005/03/04 21:21:42  cg
 * Locale set to en_US to avoid strange effects on German or
 * other internationized installations.
 *
 * Revision 3.94  2004/12/19 19:55:42  sbs
 * TNT called prior refconting now
 *
 * Revision 3.93  2004/12/19 14:29:51  sbs
 * TOT header included
 *
 * Revision 3.92  2004/12/19 13:35:07  sbs
 * replaced NT2OT traversals by TOT traversals
 *
 * Revision 3.91  2004/12/08 20:01:08  ktr
 * added some NT2OT traversals (before wltransform, compile)
 *
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

#include "phase.h"

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
#include "ToOldTypes.h"
#include "ToNewTypes.h"

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

#define PHASE_DONE_EPILOG                                                                \
    CTIabortOnError ();                                                                  \
    DBUG_EXECUTE ("MEM_LEAK", ILIBdbugMemoryLeakCheck (););                              \
    if (global.treecheck) {                                                              \
        global.syntax_tree = CHKdoTreeCheck (global.syntax_tree);                        \
    }

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
     * before compile time output with respect to a regular compiler run is
     * produced. In particularfor the -h option we need the global variables to
     * be initialized. As long as we work with pre-allocated buffers of fixed
     * length (which should be changed), this initialization requires allocation
     * to work properly.
     */

    SETUPdoSetupCompiler (argc, argv);

    syntax_tree = PHrunCompilerPhase (PH_setup, syntax_tree);

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
    /*
     * TODO:
     * - WLtransform must work with new types
     * - WLtransform must work in SSA-form
     */
    syntax_tree = TOTdoToOldTypes (syntax_tree);
    syntax_tree = WLTRAdoWlTransform (syntax_tree); /* wltrans_tab */
    syntax_tree = TNTdoToNewTypes (syntax_tree);
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
        CTIabort ("Mt/st-block version of multithreading de-activated !!");
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

    CTIterminateCompilation (global.compiler_phase, global.break_specifier, syntax_tree);

    return (0);
}
