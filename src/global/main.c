/*
 *
 * $Log$
 * Revision 3.11  2001/05/07 13:26:18  dkr
 * DBUG string 'AST' added: calls PrintAST(syntax_tree) if flag -b... is
 * given.
 *
 * Revision 3.10  2001/04/26 21:08:22  dkr
 * ups, include of rmcasts.h removed
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
 * Revision 2.26  2000/11/14 13:18:42  dkr
 * no '... might be used uninitialized' warnings anymore
 *
 * Revision 2.25  2000/07/27 15:22:09  nmw
 * undo to revision 2.23
 *
 * Revision 2.24  2000/07/25 10:05:40  nmw
 * no generation of dec files when compiling for c-libraries
 *
 * Revision 2.23  2000/07/12 10:09:18  dkr
 * phase numbers in comments corrected
 *
 * Revision 2.22  2000/07/11 15:49:34  dkr
 * IVE is part of Optimize() now
 * PsiOpt() removed
 *
 * Revision 2.21  2000/06/13 13:40:01  dkr
 * Old2NewWith() renamed into PatchWith()
 *
 * Revision 2.20  2000/06/08 12:13:04  jhs
 * Phase 17 (refcount) will de done after phase 19 (multithreading)
 * when -mtn is set.
 *
 * Revision 2.19  2000/05/29 14:31:22  dkr
 * precompile() renamed into Precompile()
 *
 * Revision 2.18  2000/03/22 20:10:17  dkr
 * some PHASE_PROLOG, PHASE_EPILOG macros have been at the wrong place
 * :-(
 *
 * Revision 2.17  2000/03/17 12:07:00  dkr
 * macros PHASE_PROLOG and PHASE_EPILOG are now always called even if a
 * phase is skipped
 *
 * Revision 2.16  2000/03/16 14:29:45  dkr
 * CHECK_DBUG_START replaced by PHASE_PROLOG
 * CHECK_DBUG_STOP replaced by PHASE_EPILOG
 * call of Lac2fun, Fun2lac embedded into PHASE_PROLOG, PHASE_EPILOG
 *
 * Revision 2.15  2000/03/02 18:50:04  cg
 * Added new option -lac2fun that activates lac2fun conversion and
 * vice versa between psi optimizations and precompiling.
 *
 * Revision 2.14  2000/02/17 16:28:18  cg
 * Added test facility for Fun2Lac().
 *
 * Revision 2.13  2000/02/03 17:03:10  dkr
 * CHECK_DBUG_START/STOP for call of LaC2Fun added
 *
 * Revision 2.12  2000/01/24 12:23:08  jhs
 * Added options to activate/dactivate printing after a break
 * (-noPAB, -doPAB).
 *
 * Revision 2.11  2000/01/21 18:04:44  dkr
 * include of lac2fun.h added
 *
 * Revision 2.10  2000/01/21 13:19:53  jhs
 * Added new mt ... infrastructure expanded ...
 *
 * Revision 2.9  1999/10/28 20:01:43  sbs
 * comment changed from ARRAY_FLAT to PRINT_xxx.
 *
 * Revision 2.8  1999/09/20 11:32:34  jhs
 * Added commented FreeTree after syntaxtree is not used anymore,
 * but it is not possible to free the tree ... :((
 *
 * Revision 2.7  1999/07/09 11:52:18  cg
 * Added consistency check for command line options.
 *
 * Revision 2.6  1999/05/31 18:33:59  sbs
 * Print in BREAK surrounded by CHECK_DBUGs
 * => enables DBUG-output during print,
 * e.g. ARRAY_FLAT, MASKS
 *
 * Revision 2.5  1999/05/18 12:29:10  cg
 * added new resource entry TMPDIR to specify where sac2c puts
 * its temporary files.
 *
 * Revision 2.4  1999/05/12 14:30:07  cg
 * Analysis of command line options moved to options.c
 *
 * Revision 2.3  1999/04/14 09:21:17  cg
 * Cache simulation may now be triggered by pragmas.
 *
 * Revision 2.2  1999/03/31 11:30:27  cg
 * added command line parameter -cachesim
 *
 * Revision 2.1  1999/02/23 12:39:28  sacbase
 * new release made
 *
 * ... [eliminated]
 *
 */

/*
 *  this file contains the main function of the SAC->C compiler!
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "free.h"
#include "my_debug.h"
#include "globals.h"
#include "Error.h"
#include "usage.h"
#include "lac2fun.h"
#include "fun2lac.h"
#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "optimize.h"
#include "filemgr.h"
#include "import.h"
#include "refcount.h"
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

#include <stdlib.h>
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

    tmp_dirname = tempnam (config.tmpdir, "SAC_");

    SystemCall ("%s %s", config.mkdir, tmp_dirname);

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
    PHASE_EPILOG;

    if (break_after == PH_scanparse)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (MODUL_IMPORTS (syntax_tree) != NULL) {
        NOTE_COMPILER_PHASE;
        syntax_tree = Import (syntax_tree); /* imp_tab */
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
        PHASE_EPILOG;

        syntax_tree = FreeTree (syntax_tree);
        CleanUp ();

        /*
         *  After all, a success message is displayed.
         */

        NEWLINE (2);
        NOTE2 (("*** Dependency Detection successful ***"));
        NOTE2 (("*** Exit code 0"));
        NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
        NEWLINE (2);

        return (0);
    }

    PHASE_PROLOG;
    if (MODUL_STORE_IMPORTS (syntax_tree) != NULL) {
        NOTE_COMPILER_PHASE;
        syntax_tree = ReadSib (syntax_tree); /* readsib_tab */
    }
    PHASE_EPILOG;

    if (break_after == PH_readsib)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = objinit (syntax_tree); /* objinit_tab */
    PHASE_EPILOG;

    if (break_after == PH_objinit)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Flatten (syntax_tree); /* flat_tab */
    PHASE_EPILOG;

    if (break_after == PH_flatten)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Typecheck (syntax_tree); /* type_tab */

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
    }
    PHASE_EPILOG;

    if (break_after == PH_checkdec)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = RetrieveImplicitTypeInfo (syntax_tree); /* impltype_tab */
    PHASE_EPILOG;

    if (break_after == PH_impltype)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Analysis (syntax_tree); /* analy_tab */
    PHASE_EPILOG;

    if (break_after == PH_analysis)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (MODUL_FILETYPE (syntax_tree) != F_prog) {
        NOTE_COMPILER_PHASE;
        syntax_tree = WriteSib (syntax_tree); /* writesib_tab */
    }
    PHASE_EPILOG;

    if (break_after == PH_writesib)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = HandleObjects (syntax_tree); /* obj_tab */
    PHASE_EPILOG;

    if (break_after == PH_objects)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = UniquenessCheck (syntax_tree); /* unique_tab */
    PHASE_EPILOG;

    if (break_after == PH_uniquecheck)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    if (optimize) {
        NOTE_COMPILER_PHASE;
        syntax_tree = Optimize (syntax_tree); /* see optimize.c, Optimize() */
    }
    PHASE_EPILOG;

    if (break_after == PH_sacopt)
        goto BREAK;
    compiler_phase++;

    if ((!dkr) && ((gen_mt_code != GEN_MT_NEW))) {
        compiler_phase += 2;
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        syntax_tree = Refcount (syntax_tree); /* refcnt_tab */
        PHASE_EPILOG;

        if (break_after == PH_refcnt)
            goto BREAK;
        compiler_phase -= 2;
    }

    if (make_patchwith) {
        NOTE2 (("   \n"
                "** Patching with-loops (generating multiple parts) ...\n"));
        syntax_tree = PatchWith (syntax_tree); /* patchwith_tab */
    }

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = WlTransform (syntax_tree); /* wltrans_tab */
    PHASE_EPILOG;

    if (break_after == PH_wltrans)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    /*
     * gen_mt_code can be GEN_MT_OLD, GEN_MT_NEW or GEN_MT_NONE
     */
    if (gen_mt_code == GEN_MT_OLD) {
        NOTE_COMPILER_PHASE;
        NOTE (("using old version of mt"));
        syntax_tree = BuildSpmdRegions (syntax_tree); /* spmd..._tab, sync..._tab */
    } else if (gen_mt_code == GEN_MT_NEW) {
        NOTE_COMPILER_PHASE;
        NOTE (("using new version of mt"));
        SYSABORT (("New version of multithreading de-activated !!"));
        /*
         * The core problem is that new-mt reuses the FUNDEF ATTRIB attribute
         * and thereby destroys its old contents. Unfortunately, it has turned
         * that this information is vital for precompile.
         */
        syntax_tree = BuildMultiThread (syntax_tree);
    }
    PHASE_EPILOG;

    if (break_after == PH_multithread)
        goto BREAK;
    compiler_phase++;

    if (dkr || (gen_mt_code == GEN_MT_NEW)) {
        PHASE_PROLOG;
        NOTE_COMPILER_PHASE;
        syntax_tree = Refcount (syntax_tree); /* refcnt_tab */
        PHASE_EPILOG;

        if (break_after == PH_refcnt)
            goto BREAK;
    }
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Precompile (syntax_tree); /* precomp_tab */
    PHASE_EPILOG;

    if (break_after == PH_precompile)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    syntax_tree = Compile (syntax_tree); /* comp_tab */
    PHASE_EPILOG;

    if (break_after == PH_compile)
        goto BREAK;
    compiler_phase++;

    PHASE_PROLOG;
    NOTE_COMPILER_PHASE;
    Print (syntax_tree);
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
    PHASE_EPILOG;

    compiler_phase++;

    PHASE_PROLOG;
    if (filetype != F_prog) {
        NOTE_COMPILER_PHASE;
        CreateLibrary ();
    }
    PHASE_EPILOG;

    compiler_phase = PH_final;

BREAK:

    if (compiler_phase < PH_final) {
        if (compiler_phase < PH_scanparse) {
            RSCShowResources ();
        } else {
            if ((print_after_break == PAB_YES) && (compiler_phase <= PH_compile)) {
                DBUG_EXECUTE ("AST", PrintAST (syntax_tree););
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
    NOTE2 (("*** Maximum allocated memory (bytes): %u", max_allocated_mem));
    NOTE2 (("*** Currently allocated memory (bytes): %u", current_allocated_mem));
#endif

    NOTE2 (("*** Exit code 0"));
    NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
    NEWLINE (2);

    return (0);
}
