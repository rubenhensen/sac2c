/*
 *
 * $Log$
 * Revision 3.88  2004/11/29 19:10:02  sah
 * removed old phases
 *
 * Revision 3.87  2004/11/28 18:13:40  ktr
 * changed call to EMRdoRefCountPhase
 *
 * Revision 3.86  2004/11/28 12:56:44  ktr
 * Zombie phase PH_readsib added in order to have familiar phase numbering.
 *
 * Revision 3.85  2004/11/27 05:02:55  ktr
 * Some Bugfixes.
 *
 * Revision 3.84  2004/11/26 23:30:41  sbs
 * PrintAST call eliminated
 *
 * Revision 3.83  2004/11/26 23:21:59  mwe
 * OIdoObjinit
 * deactivated
 *
 * Revision 3.82  2004/11/25 17:53:48  cg
 * SacDevCamp 04
 *
 * Revision 3.81  2004/11/24 23:26:44  skt
 * big compiler brushing during SACDevCampDK 2k4
 *
 * Revision 3.80  2004/11/23 22:08:32  skt
 * changed BuildSpmdRegions into CONCdoConcurrent
 *
 * Revision 3.79  2004/11/23 19:41:21  cg
 * changed signature of GLOBinitializeGlobal().
 *
 * Revision 3.78  2004/11/23 16:22:15  cg
 * Added call to GLOBinitializeGlobals().
 *
 * Revision 3.77  2004/11/23 11:36:42  cg
 * Switched mac-file based declaration of global variables.
 *
 * Revision 3.76  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * Revision 3.75  2004/11/19 21:02:25  sah
 * added ObjectAnalysus
 *
 * Revision 3.74  2004/11/14 15:19:10  sah
 * changed the order of exports
 *
 * Revision 3.73  2004/11/07 18:03:18  sah
 * more parts of new module system activated
 *
 * Revision 3.72  2004/11/04 14:53:43  sah
 * implemented dependencies between modules
 *
 * Revision 3.71  2004/11/03 17:23:19  sah
 * the tree is freed again now.
 *
 * Revision 3.70  2004/10/28 17:16:17  sah
 * included some more parts of the new modulesystem
 *
 * Revision 3.69  2004/10/22 15:16:51  sah
 * added DoUseSymbol
 *
 * Revision 3.68  2004/10/22 13:22:59  sah
 * added DoAnnotateNamespace
 *
 * Revision 3.67  2004/10/21 17:53:30  sah
 * added ResolveAll.
 *
 * Revision 3.66  2004/10/21 17:20:01  ktr
 * Added a nasty hack to traverse MODUL_TYPES with the new typechecker even
 * when the old typechecker is used.
 *
 * Revision 3.65  2004/10/17 17:48:16  sah
 * reactivated CreateLibrary in new ast mode
 *
 * Revision 3.64  2004/10/17 14:52:06  sah
 * added export traversal
 *
 * Revision 3.63  2004/10/15 15:01:18  sah
 * Serialize is now called for modules only
 *
 * Revision 3.62  2004/10/11 16:48:10  sah
 * added serialize phase (aka writesib)
 *
 * Revision 3.61  2004/10/11 14:46:39  ktr
 * Replaced EMRefcount with rcphase.
 *
 * Revision 3.60  2004/10/05 14:16:01  sah
 * fixed a include problem :(
 *
 * Revision 3.59  2004/10/05 13:54:25  sah
 * more parts of compiler active in NEW_AST mode
 *
 * Revision 3.58  2004/09/28 14:07:30  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.57  2004/09/22 17:48:32  sbs
 * now, the setup phase is properly surrounded by PHASE_xxx macros....
 * this allows tracing to be done before scanning-parsing, e.g., for -libstat
 *
 * Revision 3.56  2004/09/22 15:22:03  sah
 * added support for new PrintStat
 *
 * Revision 3.55  2004/09/21 16:34:57  sah
 * added serialize traversal in NEW_AST mode
 *
 * Revision 3.54  2004/09/18 16:10:29  ktr
 * Old MT has been moved into phase 21 to be compatible with EMM
 *
 * Revision 3.53  2004/08/09 14:55:55  ktr
 * Replaced EMAllocateFill with ExplicitAllocation subphase
 *
 * Revision 3.52  2004/08/06 14:38:59  sah
 * ongoing work to use new AST in sac2c
 *
 * Revision 3.51  2004/07/30 17:33:00  sbs
 * new_typecheck.h included.
 *
 * Revision 3.50  2004/07/30 17:24:35  sbs
 * switch between old and new tc lifted into main
 *
 * Revision 3.49  2004/07/23 15:53:50  ktr
 * - removed OPT_BLIR
 * - removed -ktr
 * - added -emm -do/noeacc
 *
 * Revision 3.48  2004/07/21 17:26:35  ktr
 * removed blir.h
 *
 * Revision 3.47  2004/07/21 12:40:38  khf
 * phase WLPartitionGeneration exchanged by phase WLEnhancement
 *
 * Revision 3.46  2004/07/19 12:43:29  ktr
 * Added EM Reference counting.
 *
 * Revision 3.45  2004/07/15 13:40:49  ktr
 * removed show_refcount = FALSE in PH_refcnt
 *
 * Revision 3.44  2004/07/15 13:36:59  ktr
 * reorganized phases after wltransform.
 *
 * Revision 3.43  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.42  2004/07/14 15:29:54  ktr
 * Replaced call to SSARefCount by call to EMAllocateFill
 *
 * Revision 3.41  2004/06/07 14:00:30  skt
 * Position of SYSABORT for mtmode 3 moved BEHIND the break
 *
 * Revision 3.40  2004/05/12 13:03:05  ktr
 * If break_after == PH_multithread, now UndoSSA is done first.
 *
 * Revision 3.39  2004/05/12 08:17:40  skt
 * added conditional to check out ssa flag before using mtmode 3
 *
 * Revision 3.38  2004/05/11 12:49:47  ktr
 * Order of phases wltransform, refcount and mt streamlined
 *
 * Revision 3.37  2004/04/26 17:18:17  sah
 * added a hack so that new ssarefcnt phase
 * is passed always. solves the problem that
 * print does not behave as wanted during
 * PH_gencode.
 *
 * Revision 3.36  2004/04/21 16:38:56  ktr
 * Added SSA-based refcounting
 *
 * Revision 3.35  2004/04/08 11:24:34  skt
 * Position of SYSABORT for mtmode 3 moved
 *
 * Revision 3.34  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.33  2004/02/26 13:07:46  khf
 * PH_wlpartgen added
 *
 * Revision 3.32  2004/02/23 12:59:21  cg
 * Added include of ssa.h to avoid compiler warnings after usage
 * of DoSSA/UndoSSA functions.
 *
 * Revision 3.31  2004/02/09 19:27:54  skt
 * SSA-Transformation for mtmode 3 added
 *
 * Revision 3.30  2004/02/05 10:39:30  cg
 * Implementation for MT mode 1 (thread create/join) added.
 *
 * Revision 3.29  2003/12/10 16:07:14  skt
 * changed compiler flag from -mtn to -mtmode and expanded mt-versions by one
 *
 * Revision 3.28  2003/09/17 12:56:24  sbs
 * call to PrintTypeStatistics added.
 *
 * Revision 3.27  2003/06/24 21:23:40  dkr
 * revision 3.25 restored:
 * current implementation of RC uses GenerateMasks which is not implemented
 * for Nwith2 nodes. Therefore, RC should be performed before WLtrans!
 *
 * Revision 3.26  2003/06/23 15:15:02  dkr
 * refcounting phase is moved for oldMT only (instead of oldMT and noMT)
 * now
 *
 * Revision 3.25  2003/04/25 15:15:37  sbs
 * mkdtemp requires unistd.h to be included (at least on freeBSD.)
 *
 * Revision 3.24  2003/04/25 15:10:16  sbs
 * missing cast inserted 8-)
 * sah: GRGRGRRRRRRR
 *
 * Revision 3.23  2003/03/26 14:22:11  sah
 * added missing / in tmp_dirname.
 *
 * Revision 3.22  2003/03/26 13:16:02  sah
 * and another silly bug;)
 *
 * Revision 3.21  2003/03/26 13:02:10  sah
 * comments on mkdtemp and
 * silly bug fix;)
 *
 * Revision 3.20  2003/03/26 12:17:50  sah
 * replaced tmpnam by mkdtemp on
 * platforms supporting this
 *
 * Revision 3.19  2003/03/09 17:13:54  ktr
 * added basic support for BLIR.
 *
 * Revision 3.18  2002/04/30 09:02:06  dkr
 * no changes done
 *
 * Revision 3.17  2001/12/10 15:32:44  dkr
 * call of Compile_Tagged() added
 *
 * Revision 3.16  2001/11/19 20:21:16  dkr
 * global vars 'errors' and 'warnings' renamed into
 * 'errors_cnt' and 'warnings_cnt' respectively in order
 * to avoid linker warning
 *
 * Revision 3.15  2001/06/18 14:55:40  cg
 * SYSABORT which breaks compilation with -mtn (new multithreading)
 * moved directly before precompile.
 * This allows for testing mtn-code without running into strange
 * error messages due to missing implementation in the code
 * generation phase.
 *
 * Revision 3.14  2001/05/17 11:39:27  dkr
 * InitDupTree() added
 *
 * Revision 3.13  2001/05/17 08:27:41  sbs
 * PHASE_EPILOG splitted in PHASE_EPILOG and PHASE_DONE_EPILOG
 * the latter is invoked iff the phase was actually done!
 * MALLOC/FREE checked!
 *
 * Revision 3.12  2001/05/07 15:01:34  dkr
 * PrintAST is called even with -noPAB.
 * PAB_YES, PAB_NO replaced by TRUE, FALSE.
 *
 * Revision 3.11  2001/05/07 13:26:18  dkr
 * DBUG string 'AST' added: calls PrintAST(syntax_tree) if flag -b... is
 * given.
 *
 * Revision 3.9  2001/04/26 17:10:42  dkr
 * RemoveVoidFuns removed
 *
 * Revision 3.8  2001/04/24 17:12:44  dkr
 * output of 'current_allocated_mem' added
 *
 * Revision 3.7  2001/03/22 19:26:49  dkr
 * include of tree.h eliminated
 *
 * Revision 3.6  2001/03/09 11:15:55  sbs
 * call to ProfileFunctions added after type checking.
 *
 * Revision 3.5  2001/02/12 14:22:21  dkr
 * call of  PatchWith() moved
 *
 * Revision 3.4  2001/01/25 10:18:25  dkr
 * PH_spmdregions renamed into PH_multithread
 *
 * Revision 3.3  2000/12/12 15:30:47  dkr
 * internal flag 'dkr' added
 *
 * Revision 3.2  2000/12/06 19:19:55  cg
 * Deactivated new version of mt due to resource conflict with precompile.
 *
 * Revision 3.1  2000/11/20 17:59:33  sacbase
 * new release made
 *
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
#include "UndoSSATransform.h"

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
#if 0
  PHASE_PROLOG;
  NOTE_COMPILER_PHASE;
  syntax_tree = OIdoObjInit( syntax_tree);  /* objinit_tab */
  PHASE_DONE_EPILOG;
  PHASE_EPILOG;

  if (global.break_after == PH_objinit) goto BREAK;
#endif
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
        syntax_tree = USSATdoUndoSsaTransform (syntax_tree);
        break;
    case MT_createjoin:
        NOTE (("Using create-join version of multithreading (MT1)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = USSATdoUndoSsaTransform (syntax_tree);
        syntax_tree = CONCdoConcurrent (syntax_tree);
        break;
    case MT_startstop:
        NOTE (("Using start-stop version of multithreading (MT2)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = USSATdoUndoSsaTransform (syntax_tree);
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

        syntax_tree = USSATdoUndoSsaTransform (syntax_tree);
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
