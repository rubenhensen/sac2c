/*
 *
 * $Log$
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
 * Revision 1.150  1999/02/19 18:08:30  dkr
 * flag -defence added
 *
 * Revision 1.149  1999/02/15 13:34:09  sbs
 * added -noDLAW opt_dlaw;
 *
 * Revision 1.148  1999/01/26 14:26:44  cg
 * -noopt now includes -noUIP
 *
 * Revision 1.147  1999/01/25 16:26:26  sbs
 * interrupt handler setup inserted.
 *
 * Revision 1.146  1999/01/18 15:31:39  sbs
 * mt for LINUX enabled 8-))
 *
 * Revision 1.145  1999/01/15 15:14:32  cg
 * added option -noTILE, modified option -intrinsic,
 * added ABORT when compiling multi-threaded code on Linux systems.
 *
 * Revision 1.144  1999/01/07 14:01:01  sbs
 * more sophisticated breaking facilities inserted;
 * Now, a break in a specific cycle can be triggered!
 *
 * Revision 1.143  1998/12/03 10:24:25  cg
 * Now, the specification of several source files on the sac2c command
 * line results in an appropriate error message rather than confusion.
 *
 * Revision 1.142  1998/10/26 12:34:14  cg
 * new compiler option:
 * use intrinsic array operations instead of with-loop based implementations
 * in the stdlib. The corresponding information is stored by the new
 * global variable intrinsics.
 *
 * Revision 1.141  1998/10/23 14:29:46  cg
 * added the new command line option -inparsize <no> which allows to
 * specify a minimum generator size for with-loops to be executed in
 * parallel if such execution is enabled.
 * The information stored by the global variable min_parallel_size.
 *
 * Revision 1.140  1998/08/27 12:48:00  sbs
 * -L args added to SYSTEMLIB_PATH as well so that readsib will
 * accept linkwith-pragma args that are not in the standard path
 * from the config!
 *
 * Revision 1.139  1998/08/07 18:11:29  sbs
 * inserted gen_mt_code; it prevents spmd regions from being created per default
 * only if one of the following options is set:
 * -mtstatic <no> / -mtdynamic <no> / -mtall <no>
 * spmd regions will be introduced!
 *
 * Revision 1.138  1998/07/23 10:08:06  cg
 * sac2c option -mt-static -mt-dynamic -mt-all renamed to
 * -mtstatic, -mtdynamic, -mtall resepctively
 *
 * Revision 1.137  1998/07/10 15:20:04  cg
 * included option -i to display copyright/disclaimer
 *
 * Revision 1.136  1998/07/07 13:41:08  cg
 * implemented the command line option -mt-all
 *
 * Revision 1.135  1998/06/29 08:52:19  cg
 * streamlined tracing facilities
 * tracing on new with-loop and multi-threading operations implemented
 *
 * Revision 1.134  1998/06/23 15:05:58  cg
 * added command line options -dcccall and -dshow_syscall
 *
 * Revision 1.133  1998/06/23 13:13:15  sbs
 * de-bugged -b1
 *
 * Revision 1.132  1998/06/19 16:35:09  dkr
 * added -noUIP
 *
 * Revision 1.131  1998/06/19 12:51:31  srs
 * compute_malloc_align_step() => ComputeMallocAlignStep()
 *
 * Revision 1.130  1998/06/18 13:41:09  cg
 * function SpmdRegion renamed to BuildSpmdRegion and now included
 * from concurrent.h instead of spmdregion.h
 *
 * Revision 1.129  1998/06/09 09:46:14  cg
 * added command line options -mt-static, -mt-dynamic, and -maxsyncfold.
 *
 * Revision 1.128  1998/05/27 11:19:44  cg
 * global variable 'filename' which contains the current file name in order
 * to provide better error messages is now handled correctly.
 *
 * Revision 1.127  1998/05/13 14:06:14  srs
 * added -maxwlunroll
 *
 * Revision 1.126  1998/05/13 13:40:33  srs
 * renamed switch -noUNR to -noLUNR.
 * New switch -noWLUNR to deactivate WL unrolling.
 *
 * ... [eliminated]
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

/*
 *  this file contains the main function of the SAC->C compiler!
 */

#include "tree.h"
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
#include "psi-opt.h"
#include "writesib.h"
#include "readsib.h"
#include "implicittypes.h"
#include "objinit.h"
#include "analysis.h"
#include "checkdec.h"
#include "objects.h"
#include "uniquecheck.h"
#include "rmvoidfun.h"
#include "wltransform.h"
#include "concurrent.h"
#include "precompile.h"
#include "compile.h"
#include "cccall.h"
#include "Old2NewWith.h"
#include "internal_lib.h"
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
    node *syntax_tree;
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

    /*
     * Now, we read in the sac2c configuration files.
     */

    ABORT_ON_ERROR;

    if (!libstat) {
        NOTE_COMPILER_PHASE;
    }

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

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = ScanParse ();
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_scanparse)
        goto BREAK;
    compiler_phase++;

    if (MODUL_IMPORTS (syntax_tree) != NULL) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        syntax_tree = Import (syntax_tree); /* imp_tab */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_import)
        goto BREAK;

    if (makedeps) {
        /*
         * This is not a real compiler run,
         * only dependencies are to be detected.
         */

        compiler_phase = PH_writedeps;
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        PrintDependencies (dependencies, makedeps);
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;

        FreeTree (syntax_tree);
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

    compiler_phase++;

    if (MODUL_STORE_IMPORTS (syntax_tree) != NULL) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        syntax_tree = ReadSib (syntax_tree); /* readsib_tab */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_readsib)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = objinit (syntax_tree); /* objinit_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_objinit)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = Flatten (syntax_tree); /* flat_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_flatten)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = Typecheck (syntax_tree); /* type_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_typecheck)
        goto BREAK;
    compiler_phase++;

#if 0
  CHECK_DBUG_START;
  syntax_tree = LaC2Fun( syntax_tree);
  CHECK_DBUG_STOP;
  ABORT_ON_ERROR;
#endif

    if (MODUL_FILETYPE (syntax_tree) != F_prog) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        syntax_tree = CheckDec (syntax_tree); /* writedec_tab and checkdec_tab */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_checkdec)
        goto BREAK;
    compiler_phase++;

#if 0
  CHECK_DBUG_START;
  syntax_tree = Fun2Lac( syntax_tree);
  CHECK_DBUG_STOP;
  ABORT_ON_ERROR;
#endif

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = RetrieveImplicitTypeInfo (syntax_tree); /* impltype_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_impltype)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = Analysis (syntax_tree); /* analy_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_analysis)
        goto BREAK;
    compiler_phase++;

    if (MODUL_FILETYPE (syntax_tree) != F_prog) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        syntax_tree = WriteSib (syntax_tree); /* writesib_tab */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_writesib)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = HandleObjects (syntax_tree); /* obj_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_objects)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = UniquenessCheck (syntax_tree); /* unique_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_uniquecheck)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = RemoveVoidFunctions (syntax_tree); /* rmvoid_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_rmvoidfun)
        goto BREAK;
    compiler_phase++;

    if (optimize) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        syntax_tree = Optimize (syntax_tree); /* see optimize.c, Optimize() */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_sacopt)
        goto BREAK;
    compiler_phase++;

    if (optimize) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        syntax_tree = PsiOpt (syntax_tree); /* idx_tab */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_psiopt)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = Refcount (syntax_tree); /* refcnt_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_refcnt)
        goto BREAK;
    compiler_phase++;

    if (Make_Old2NewWith) {
        NOTE2 (("   \n"
                "** Convert old with-loops into new ones ...\n"
                "   Generate multiple parts in new with-loops ...\n"));
        syntax_tree = Old2NewWith (syntax_tree); /* o2nWith_tab */
    }

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = WlTransform (syntax_tree); /* wltrans_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_wltrans)
        goto BREAK;
    compiler_phase++;

    if (gen_mt_code == GEN_MT_OLD) {
        NOTE_COMPILER_PHASE;
        NOTE (("using old version of mt"));
        CHECK_DBUG_START;
        syntax_tree = BuildSpmdRegions (syntax_tree); /* spmd..._tab, sync..._tab */
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    } else if (gen_mt_code == GEN_MT_NEW) {
        NOTE_COMPILER_PHASE;
        NOTE (("using new version of mt"));
        CHECK_DBUG_START;
        syntax_tree = BuildMultiThread (syntax_tree);
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    if (break_after == PH_spmdregions)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = precompile (syntax_tree); /* precomp_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_precompile)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = Compile (syntax_tree); /* comp_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_compile)
        goto BREAK;
    compiler_phase++;

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    Print (syntax_tree);
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_genccode)
        goto BREAK;
    compiler_phase++;

    /*
     *  After the C file has been written, the syntax tree may be released.
     */

    FreeTree (syntax_tree);

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    InvokeCC ();
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;
    compiler_phase++;

    if (filetype != F_prog) {
        NOTE_COMPILER_PHASE;
        CHECK_DBUG_START;
        CreateLibrary ();
        CHECK_DBUG_STOP;
        ABORT_ON_ERROR;
    }

    /*
     *  Finally, we do some clean up.
     */

    CleanUp ();

    /*
     *  After all, a success message is displayed.
     */

    NEWLINE (2);
    NOTE2 (("*** Compilation successful ***"));

#ifdef SHOW_MALLOC
    NOTE2 (("*** maximum allocated memory (bytes): %u", max_allocated_mem));
#endif

    NOTE2 (("*** Exit code 0"));
    NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
    NEWLINE (2);

    /*  FreeTree (syntax_tree); */
    return (0);

BREAK:

    if (compiler_phase >= PH_scanparse) {
        if ((print_after_break == PAB_YES) && (compiler_phase < PH_genccode)) {
            CHECK_DBUG_START; /* needed for DBUG-infos during print, i.e. PRINT_xxx */
            Print (syntax_tree);
            CHECK_DBUG_STOP;
        }
        FreeTree (syntax_tree);

    } else {
        RSCShowResources ();
    }

    /*
     *  Finally, we do some clean up.....
     */

    CleanUp ();

    /*
     * ....and display a success message.
     */

    NEWLINE (2);
    NOTE2 (("*** Compilation successful ***"));
    NOTE2 (("*** BREAK after: %s", compiler_phase_name[compiler_phase]));
    if (break_specifier[0] != '\0')
        NOTE2 (("*** BREAK specifier: '%s`", break_specifier));

#ifdef SHOW_MALLOC
    NOTE2 (("*** maximum allocated memory (bytes): %u", max_allocated_mem));
#endif

    NOTE2 (("*** Exit code 0"));
    NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
    NEWLINE (2);

    return (0);
}
