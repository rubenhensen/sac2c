/*
 *
 * $Log$
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
#include "my_debug.h"
#include "DupTree.h"
#include "globals.h"
#include "Error.h"
#include "usage.h"
#include "lac2fun.h"
#include "fun2lac.h"
#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "type_statistics.h"
#include "optimize.h"
#include "filemgr.h"
#include "import.h"
#include "refcount.h"
#include "alloc.h"
#include "refcounting.h"
#include "scnprs.h"
#include "writesib.h"
#include "readsib.h"
#include "implicittypes.h"
#include "objinit.h"
#include "analysis.h"
#include "checkdec.h"
#include "objects.h"
#include "uniquecheck.h"
#include "wltransform.h"
#include "concurrent.h"
#include "precompile.h"
#include "compile.h"
#include "annotate_fun_calls.h"
#include "cccall.h"
#include "PatchWith.h"
#include "resource.h"
#include "interrupt.h"
#include "options.h"
#include "multithread.h"
#include "WLEnhancement.h"

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
    int i;

#ifdef SHOW_MALLOC
    ComputeMallocAlignStep ();
#endif

    /*
     * Initializations
     */

    InitPaths ();
    SetupInterruptHandlers ();
    InitDupTree ();

    compiler_phase = PH_setup;

    /*
     *  The command line is written to a single string.
     */
    strcpy (commandline, argv[0]);
    for (i = 1; i < argc; i++) {
        strcat (commandline, " ");
        strcat (commandline, argv[i]);
    }

    AnalyseCommandline (argc, argv);
    CheckOptionConsistency ();

    if (sacfilename[0] == '\0') {
        puresacfilename = "stdin";
    }

    ABORT_ON_ERROR;

    /*
     * Now, we read in the sac2c configuration files.
     */

    NOTE_COMPILER_PHASE;

    RSCEvaluateConfiguration (target_name);

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

    RearrangePaths ();

    /*
     * Now, we create tmp directories for files generated during the
     * compilation process.
     *
     * Actually, only one temp directory is created whose name may be
     * accessed trough the global variable tmp_dirname
     * which is defined in globals.c.
     */

#ifdef HAVE_MKDTEMP
    /* mkdtemp is safer than tempnam and recommended */
    /* on linux/bsd platforms.                       */

    /* malloc is used here as tempnam uses it */
    /* internally as well.                    */

    tmp_dirname = (char *)malloc (strlen (config.mkdir) + 12);
    tmp_dirname = strcpy (tmp_dirname, config.tmpdir);
    tmp_dirname = strcat (tmp_dirname, "/SAC_XXXXXX");

    tmp_dirname = mkdtemp (tmp_dirname);

    if (tmp_dirname == NULL) {
        SYSABORT (("System failed to create temporary directory.\n"));
    }
#else
    /* the old way for platforms not */
    /* supporting mkdtemp            */

    tmp_dirname = tempnam (config.tmpdir, "SAC_");

    SystemCall ("%s %s", config.mkdir, tmp_dirname);
#endif

    /*
     * If sac2c was started with the option -libstat,
     * then the library status is printed to stdout and the
     * compilation process is terminated immediately.
     */

    if (libstat) {
        PrintLibStat ();
        CleanUp ();

        exit (0);
    }

    ABORT_ON_ERROR;

    if (break_after == PH_setup)
        goto BREAK;
    compiler_phase++;

    /*
     *  Finally the compilation process is started.
     */

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = ScanParse ();
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_scanparse)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (MODUL_IMPORTS (syntax_tree) != NULL) {
        NOTE_COMPILER_PHASE;
        syntax_tree = Import (syntax_tree); /* imp_tab */
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    if (break_after == PH_import)
        goto BREAK;
    compiler_phase++;

    if (makedeps) {
        /*
         * This is not a real compiler run,
         * only dependencies are to be detected.
         */

        compiler_phase = PH_writedeps;
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        PrintDependencies (dependencies, makedeps);
        PHASE_DONE_EPILOG;
        PHASE_EPILOG;

        syntax_tree = FreeTree (syntax_tree);
        CleanUp ();

        /*
         *  After all, a success message is displayed.
         */

        NEWLINE (2);
        NOTE2 (("*** Dependency Detection successful ***"));
        NOTE2 (("*** Exit code 0"));
        NOTE2 (("*** 0 error(s), %d warning(s)", warnings_cnt));
        NEWLINE (2);

        return (0);
    }

    PHASE_PROLOG;
    if (MODUL_STORE_IMPORTS (syntax_tree) != NULL) {
        NOTE_COMPILER_PHASE;
        syntax_tree = ReadSib (syntax_tree); /* readsib_tab */
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    if (break_after == PH_readsib)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = objinit (syntax_tree); /* objinit_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_objinit)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Flatten (syntax_tree); /* flat_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_flatten)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Typecheck (syntax_tree); /* type_tab */
    PHASE_DONE_EPILOG;

    if (profileflag != 0) {
        syntax_tree = ProfileFunCalls (syntax_tree); /* profile_tab */
    }
    PHASE_EPILOG;

    if (break_after == PH_typecheck)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (MODUL_FILETYPE (syntax_tree) != F_prog) {
        NOTE_COMPILER_PHASE;
        syntax_tree = CheckDec (syntax_tree); /* writedec_tab and checkdec_tab */
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    if (break_after == PH_checkdec)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = RetrieveImplicitTypeInfo (syntax_tree); /* impltype_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_impltype)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Analysis (syntax_tree); /* analy_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_analysis)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (MODUL_FILETYPE (syntax_tree) != F_prog) {
        NOTE_COMPILER_PHASE;
        syntax_tree = WriteSib (syntax_tree); /* writesib_tab */
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    if (break_after == PH_writesib)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = HandleObjects (syntax_tree); /* obj_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_objects)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = UniquenessCheck (syntax_tree); /* unique_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_uniquecheck)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = WLEnhancement (syntax_tree); /* see WLEnhancement.c */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_wlenhance)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (optimize) {
        NOTE_COMPILER_PHASE;
        syntax_tree = Optimize (syntax_tree); /* see optimize.c, Optimize() */
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    syntax_tree = PrintTypeStatistics (syntax_tree);

    if (break_after == PH_sacopt)
        goto BREAK;
    compiler_phase++;

    if (!ktr) {
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        syntax_tree = Refcount (syntax_tree); /* refcnt_tab */
        PHASE_DONE_EPILOG;
        PHASE_EPILOG;
    }
    if (break_after == PH_oldrefcnt)
        goto BREAK;
    compiler_phase++;

#if 0
  if (patch_with) {
    NOTE2( ("   \n"
            "** Patching with-loops (generating multiple parts) ...\n"));
    syntax_tree = PatchWith( syntax_tree);  /* patchwith_tab */
  }
#endif

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = WlTransform (syntax_tree); /* wltrans_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_wltrans)
        goto BREAK;
    compiler_phase++;

    if (ktr) {
        syntax_tree = DoSSA (syntax_tree);
    }

    if (ktr) {
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        syntax_tree = EMAllocateFill (syntax_tree); /* emalloc_tab */
        PHASE_DONE_EPILOG;
        PHASE_EPILOG;
    }

    if (break_after == PH_alloc)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;

    switch (mtmode) {
    case MT_none:
        break;
    case MT_createjoin:
        NOTE_COMPILER_PHASE;
        NOTE (("Using create-join version of multithreading (MT1)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = BuildSpmdRegions (syntax_tree);
        PHASE_DONE_EPILOG;
        break;
    case MT_startstop:
        NOTE_COMPILER_PHASE;
        NOTE (("Using start-stop version of multithreading (MT2)"));
        /* spmd..._tab, sync..._tab */
        syntax_tree = BuildSpmdRegions (syntax_tree);
        PHASE_DONE_EPILOG;
        break;
    case MT_mtstblock:
        NOTE_COMPILER_PHASE;
        NOTE (("Using mt/st-block version of multithreading (MT3)"));
        /* this version of multithreading is only for code in SSA-form */

        /* After SSA-Refcounting, Code is already in SSA-Form */
        /* syntax_tree = DoSSA(syntax_tree); */
        syntax_tree = BuildMultiThread (syntax_tree);
        /* for compatibility reasons, the code is retransformed from SSA-form */
        /*syntax_tree = UndoSSA(syntax_tree);*/
        PHASE_DONE_EPILOG;
        break;
    }
    PHASE_EPILOG;

    if (break_after == PH_multithread)
        goto BREAK;
    compiler_phase++;

    if (mtmode == MT_mtstblock) {
        SYSABORT (("Mt/st-block version of multithreading de-activated !!"));
        /* following comment concerning for mt/st-block version:
         * The core problem is that new-mt reuses the FUNDEF ATTRIB attribute
         * and thereby destroys its old contents. Unfortunately, it has turned
         * that this information is vital for precompile.
         */
    }

    if (ktr) {
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        syntax_tree = EMRefCount (syntax_tree); /* emrefcnt_tab */
        PHASE_DONE_EPILOG;
        PHASE_EPILOG;
    }

    if (break_after == PH_refcnt)
        goto BREAK;
    compiler_phase++;

    if (mtmode == MT_mtstblock) {
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        /*syntax_tree = Precompile( syntax_tree);*/ /* ???_tab */
        PHASE_DONE_EPILOG;
        PHASE_EPILOG;
    }

    if (break_after == PH_multithread_finish)
        goto BREAK;
    compiler_phase++;

    if (ktr) {
        syntax_tree = UndoSSA (syntax_tree);
    }

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Precompile (syntax_tree); /* precomp_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_precompile)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Compile (syntax_tree); /* comp_tab */
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_compile)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    Print (syntax_tree);
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    if (break_after == PH_genccode)
        goto BREAK;
    compiler_phase++;

    /*
     *  After the C file has been written, the syntax tree may be released.
     */

    syntax_tree = FreeTree (syntax_tree);

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    InvokeCC ();
    PHASE_DONE_EPILOG;
    PHASE_EPILOG;

    compiler_phase++;

    PHASE_PROLOG;
    if (filetype != F_prog) {
        NOTE_COMPILER_PHASE;
        CreateLibrary ();
        PHASE_DONE_EPILOG;
    }
    PHASE_EPILOG;

    compiler_phase = PH_final;

BREAK:

    if (compiler_phase < PH_final) {
        if (compiler_phase < PH_scanparse) {
            RSCShowResources ();
        } else {
            DBUG_EXECUTE ("AST", PrintAST (syntax_tree););
            if (print_after_break && (compiler_phase <= PH_compile)) {
                Print (syntax_tree);
            }
            syntax_tree = FreeTree (syntax_tree);
        }
    }

    /*
     *  Finally, we do some clean up ...
     */

    CleanUp ();

    /*
     *  ... and display a success message.
     */

    NEWLINE (2);
    NOTE2 (("*** Compilation successful ***"));

    if (compiler_phase < PH_final) {
        NOTE2 (("*** BREAK after: %s", compiler_phase_name[compiler_phase]));
        if (break_specifier[0] != '\0') {
            NOTE2 (("*** BREAK specifier: '%s`", break_specifier));
        }
    }

#ifdef SHOW_MALLOC
    NOTE2 (("*** Maximum allocated memory (bytes):   %s",
            IntBytes2String (max_allocated_mem)));
    NOTE2 (("*** Currently allocated memory (bytes): %s",
            IntBytes2String (current_allocated_mem)));
#endif

    NOTE2 (("*** Exit code 0"));
    NOTE2 (("*** 0 error(s), %d warning(s)", warnings_cnt));
    NEWLINE (2);

    return (0);
}
