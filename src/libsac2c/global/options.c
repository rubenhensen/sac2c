/*
 * $Id$
 *
 */

/*****************************************************************************
 *
 * file:   options.c
 *
 * prefix: OPT
 *
 * description:
 *  This file provides means for the analysis of sac2c command line arguments.
 *  It uses the set macro definitions from getoptions.h
 *
 *****************************************************************************/

#include <stdlib.h>
#include <limits.h>
#include <stdio.h>

#include "config.h"
#include "dbug.h"

#include "getoptions.h"

#include "usage.h"
#include "filemgr.h"
#include "globals.h"
#include "str.h"
#include "memory.h"
#include "ctinfo.h"
#include "libstat.h"
#include "stringset.h"
#include "phase_options.h"
#include "phase_info.h"
#include "resource.h"

/******************************************************************************
 *
 * function:
 *   node *OPTcheckPreSetupOptions( int argc, char *argv[])
 *
 * description:
 *   Some options require processing as early as possible in the compilation
 *   process. For example, memory checks must be initiated BEFORE any
 *   allocations are done. This requires the corresponding option to be
 *   processed before global variables are initialised.
 *
 *   We deliberately switch off error detection because that would obviously
 *   complain about illegal options and because the compile time information
 *   system has not yet been set up properly.
 *
 ******************************************************************************/

#undef ARGS_ERROR
#define ARGS_ERROR(msg)

void
OPTcheckPreSetupOptions (int argc, char *argv[])
{
    DBUG_ENTER ("OPTcheckPreSetupOptions");

    ARGS_BEGIN (argc, argv);

    ARGS_OPTION_BEGIN ("d")
    {
        ARG_CHOICE_BEGIN ();
#ifdef SHOW_MALLOC
        ARG_CHOICE ("memcheck", global.memcheck = TRUE);
#endif
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("d");

    ARGS_END ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void OPTcheckOptionConsistency(void)
 *
 * description:
 *   This function is called right after command line arguments
 *   have been analysed. Errors and warnings are produced whenever the user
 *   has selected an incompatible combination of options.
 *
 ******************************************************************************/

void
OPTcheckOptionConsistency (void)
{
    DBUG_ENTER ("OPTcheckOptionConsistency");

    if (global.printConfig) {
        RSCprintConfigEntry (global.printConfig);
        exit (0);
    }

    if (STReq (global.config.backend, "MUTC")) {
        if (global.mtmode != MT_none) {
            CTIerror ("Traditional MT modes are not available for the MUTC "
                      "backend.");
        }

        CTInote ("Disabling reference counting optimisations not suitable "
                 "for mutc backend.");
        global.optimize.dosrf = FALSE;
        /* global.optimize.doipc = FALSE; */
        /* global.optimize.douip = FALSE; */
        /* global.optimize.dodr = FALSE; */
        global.optimize.dorco = FALSE;

        if (global.optimize.dophm) {
            CTInote ("Private heap management has been disabled due to use "
                     "of the mutc backend.");
            global.optimize.dophm = FALSE;
        }

        if ((global.mutc_disable_thread_mem == TRUE)
            && (global.mutc_thread_mem == TRUE)) {
            CTIerror ("-mutc_thread_mem can not be used with "
                      "-mutc_disable_thread_mem");
        }

        if ((global.mutc_distribution_mode == MUTC_DMODE_bounded)
            && (global.mutc_distribution_mode_arg == -1)) {
            CTIerror ("bounded distribution mode requires an argument");
        }

        if ((global.mutc_force_block_size != -1)
            && (global.mutc_static_resource_management)) {
            CTIerror ("Can only use one method of setting the block size at a time");
        }
    } else {
        if (global.mutc_fun_as_threads == TRUE) {
            CTIerror ("-mutc_fun_threads only works with mutc backend");
        }
        if (global.mutc_thread_mem == TRUE) {
            CTIerror ("-mutc_thread_mem only works with mutc backend");
        }
        if (global.mutc_disable_thread_mem == TRUE) {
            CTIerror ("-mutc_disable_thread_mem only works with mutc backend");
        }
        if (global.mutc_static_resource_management == TRUE) {
            CTIerror ("-mutc_static_resource_management needs mutc backend");
        }
        if (global.mutc_force_block_size != -1) {
            CTIerror ("-mutc_force_block_size only works with mutc backend");
        }
        if (global.mutc_benchmark == TRUE) {
            CTIerror ("-mutc_benchmark needs mutc backend");
        }
    }

    if (global.runtimecheck.boundary && global.optimize.doap) {
        global.optimize.doap = FALSE;
        CTIwarn ("Option -check b requires option -noAP\n"
                 "Array padding disabled");
    }

    if (global.runtimecheck.conformity && !global.insertconformitychecks) {
        global.insertconformitychecks = TRUE;
        CTIwarn ("Option -check c implies option -ecc.\n"
                 "Insertion of explicit conformity checks has been enabled.");
    }

#ifdef DISABLE_MT
    if (global.mtmode != MT_none) {
        global.mtmode = MT_none;
        global.num_threads = 1;
        CTIerror ("Code generation for multi-threaded program execution not"
                  " yet available for " ARCH " running " OS ".");
    }
#endif

#ifdef DISABLE_PHM
    if (global.optimize.dophm) {
        CTIerror ("Private heap management not yet available for " ARCH " running " OS
                  ".");
    }
#endif

    if (global.mtmode != MT_none) {
        if (global.docachesim) {
            CTIerror ("Cache simulation is not available for multi-threaded "
                      "program execution");
        }

        if (global.doprofile) {
            CTIerror ("Profiling is not available for multi-threaded "
                      "program execution");
        }
    }

    if (global.runtimecheck.heap && !global.optimize.dophm) {
        CTIerror ("Diagnostic heap management is only available in "
                  "conjunction with private heap management.\n"
                  "Diagnostic disabled");
    }

    /*
     * commandline switch for library generation not used,
     * set it to default and generate a standard SAC Library
     */
    if (!global.genlib.c && !global.genlib.sac) {
        global.genlib.sac = TRUE;
    }

    if (global.genlib.c && (global.mtmode != MT_none)) {
        CTIerror ("Multithreading is not yet available when compiling for "
                  "a C-library.");
    }

    if (global.optimize.dosaa && !global.optimize.dodcr) {
        CTIwarn ("Symbolic array attributes (SAA) require dead code"
                 "removal (DCR).\n"
                 "Symbolic array attributes disabled.");
        global.optimize.dosaa = FALSE;
    }

    if (global.optimize.doive && !global.optimize.dosaa) {
        CTIwarn ("Index vector elimination (IVE) requires symbolic array "
                 "attributes (SAA).\n"
                 "Index vector elimination disabled.");
        global.optimize.doive = FALSE;
    }

    if (!global.optimize.dowlur) {
        CTIwarn ("With-Loop unrolling (WLUR) was disabled using the command line. "
                 "However, unrolling of single-trip with-loops is required for "
                 "code generation. Therefore, WLUR will be re-enabled with the  "
                 "maximum number of unrolling steps set to 1.");
        global.wlunrnum = 1;
        global.optimize.dowlur = TRUE;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *OPTanalyseCommandline(  int argc, char *argv[])
 *
 * description:
 *   This function analyses the commandline options given to sac2c.
 *   Usually selections made are stored in global variables for later
 *   reference.
 *
 ******************************************************************************/

#undef ARGS_ERROR
#define ARGS_ERROR(msg)                                                                  \
    {                                                                                    \
        CTIerror ("%s: %s %s %s", msg, ARGS_argv[0], STRonNull ("", OPT),                \
                  STRonNull ("", ARG));                                                  \
    }

static void
AnalyseCommandlineSac2c (int argc, char *argv[])
{
    int store_num_threads = 0;
    mtmode_t store_mtmode = MT_none;

    DBUG_ENTER ("AnalyseCommandlineSac2c");

    ARGS_BEGIN (argc, argv);

    /*
     * Options starting with aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
     */

    ARGS_FLAG ("apdiag", global.apdiag = TRUE);

    ARGS_OPTION ("aplimit", { ARG_RANGE (global.padding_overhead_limit, 0, 100); });

    ARGS_OPTION ("apdiagsize", ARG_NUM (global.apdiag_limit));

    /*
     * Options starting with bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
     */

    ARGS_OPTION ("b", PHOinterpretBreakOption (ARG))

    /*
     * Options starting with ccccccccccccccccccccccccccccccccccccccccccc
     */

    ARGS_OPTION ("C", global.printConfig = ARG);

    ARGS_OPTION_BEGIN ("check")
    {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.runtimecheck = global.runtimecheck_all;
                      global.doruntimecheck = TRUE);
#define RTC(flag, char, default)                                                         \
    ARG_FLAGMASK (char, global.runtimecheck.flag = TRUE; global.doruntimecheck = TRUE);
#include "flags.mac"
        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("check");

    ARGS_OPTION ("chkfreq", ARG_RANGE (global.check_frequency, 0, 4));

    ARGS_FLAG ("copyright", USGprintCopyright (); exit (0));

    ARGS_OPTION_BEGIN ("cppI")
    {
        char *tmp;
        if (global.cpp_options == NULL) {
            tmp = STRcat (" -I", ARG);
        } else {
            tmp = STRcatn (3, global.cpp_options, " -I", ARG);
            MEMfree (global.cpp_options);
        }
        global.cpp_options = tmp;
    }
    ARGS_OPTION_END ("cppI")

    ARGS_OPTION_BEGIN ("csdefaults")
    {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('s', global.cachesim.simple = TRUE;
                      global.cachesim.advanced = FALSE);
        ARG_FLAGMASK ('a', global.cachesim.advanced = TRUE;
                      global.cachesim.simple = FALSE);
        ARG_FLAGMASK ('g', global.cachesim.global = TRUE; global.cachesim.block = FALSE);
        ARG_FLAGMASK ('b', global.cachesim.block = TRUE; global.cachesim.global = FALSE);
        ARG_FLAGMASK ('f', global.cachesim.file = TRUE; global.cachesim.pipe = FALSE;
                      global.cachesim.immediate = FALSE);
        ARG_FLAGMASK ('p', global.cachesim.pipe = TRUE; global.cachesim.file = FALSE;
                      global.cachesim.immediate = FALSE);
        ARG_FLAGMASK ('i', global.cachesim.immediate = TRUE; global.cachesim.pipe = FALSE;
                      global.cachesim.file = FALSE);
        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("csdefaults");

    ARGS_OPTION ("csdir", strncpy (global.cachesim_dir, ARG, MAX_FILE_NAME - 1));

    ARGS_OPTION ("csfile", strncpy (global.cachesim_file, ARG, MAX_FILE_NAME - 1));

    ARGS_OPTION ("cshost", strncpy (global.cachesim_host, ARG, MAX_FILE_NAME - 1));

    ARGS_FLAG ("cs", global.docachesim = FALSE);

    ARGS_FLAG ("c", global.break_after_phase = PH_cg);

    /*
     * Options starting with ddddddddddddddddddddddddddddddddddddddddddd
     */

    ARGS_FLAG ("ds", global.dynamic_shapes = TRUE);

    ARGS_FLAG ("dofoldfusion", global.no_fold_fusion = FALSE);

    ARGS_FLAG ("dofoldparallel", global.no_fold_parallel = FALSE);

    ARGS_OPTION_BEGIN ("do")
    {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("opt", global.optimize = global.optimize_all);

#define OPTIMIZE(str, abbr, devl, prod, name)                                            \
    ARG_CHOICE (str, global.optimize.do##abbr = TRUE);
#include "optimize.mac"

        ARG_CHOICE ("pab", global.print_after_break = TRUE);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("do");

#ifndef DBUG_OFF

    ARGS_OPTION_BEGIN ("d")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("treecheck", global.treecheck = TRUE);
        ARG_CHOICE ("lacfuncheck", global.lacfuncheck = TRUE);
        ARG_CHOICE ("sancheck", global.sancheck = TRUE);
#ifdef SHOW_MALLOC
        ARG_CHOICE ("memcheck", global.memcheck = TRUE);
#endif
        ARG_CHOICE ("efence", global.use_efence = TRUE);
        ARG_CHOICE ("nocleanup", global.cleanup = FALSE);
        ARG_CHOICE ("nolacinline", global.lacinline = FALSE);
        ARG_CHOICE ("syscall", global.show_syscall = TRUE);
        ARG_CHOICE ("cccall", global.gen_cccall = TRUE; global.cleanup = FALSE);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("d");

#endif /* DBUG_OFF */

    ARGS_OPTION_BEGIN ("D")
    {
        char *tmp;
        if (global.cpp_options == NULL) {
            tmp = STRcat (" -D", ARG);
        } else {
            tmp = STRcatn (3, global.cpp_options, " -D", ARG);
            MEMfree (global.cpp_options);
        }
        global.cpp_options = tmp;
    }
    ARGS_OPTION_END ("D")

    /*
     * Options starting with eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
     */

    ARGS_FLAG ("ecc", global.insertconformitychecks = TRUE);
    ARGS_FLAG ("elf", global.elf = TRUE);

    ARGS_OPTION_BEGIN ("E")
    {
        FMGRappendPath (PK_extlib_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("E");

    ARGS_FLAG ("enforceIEEE", global.enforce_ieee = TRUE);

    ARGS_FLAG ("enforceFloat", global.enforce_float = TRUE);

    /*
     * Options starting with fffffffffffffffffffffffffffffffffffffffffff
     */

    ARGS_FLAG ("force_naive", global.force_naive_with2 = TRUE);

    /*
     * Options starting with ggggggggggggggggggggggggggggggggggggggggggg
     */

    ARGS_FLAG ("g", global.cc_debug = TRUE);

    ARGS_OPTION_BEGIN ("genlib")
    {
        ARG_CHOICE_BEGIN ();

#define GENLIB(flag, str, default) ARG_CHOICE (str, global.genlib.flag = default);
#include "flags.mac"

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("genlib");

    /*
     * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
     */

    ARGS_FLAG ("h", USGprintUsage (); exit (0));
    ARGS_FLAG ("help", USGprintUsage (); exit (0));

    /*
     * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
     */

    ARGS_OPTION ("initmheap", ARG_NUM (global.initial_master_heapsize));
    ARGS_OPTION ("initwheap", ARG_NUM (global.initial_worker_heapsize));
    ARGS_OPTION ("inituheap", ARG_NUM (global.initial_unified_heapsize));
    ARGS_OPTION ("iveo", ARG_NUM (global.iveo));
    ARGS_OPTION ("ive", ARG_NUM (global.ive));
    ARGS_FLAG ("extrema", global.doivext = TRUE);

    ARGS_OPTION_BEGIN ("I")
    {
        FMGRappendPath (PK_imp_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("I");

    /*
     * Options starting with kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
     */

    /*
     * Options starting with lllllllllllllllllllllllllllllllllllllllllll
     */

    ARGS_OPTION_BEGIN ("L")
    {
        FMGRappendPath (PK_lib_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("L");

    ARGS_FLAG ("libstat", global.libstat = TRUE);

    ARGS_OPTION_BEGIN ("linksetsize")
    {
        ARG_RANGE (global.linksetsize, 0, INT_MAX);
        if (global.linksetsize == 0) {
            global.linksetsize = INT_MAX;
        }
    }
    ARGS_OPTION_END ("linksetsize");

    /*
     * Options starting with mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
     */

    ARGS_FLAG ("mt", {
        if (store_mtmode == MT_none) {
            global.mtmode = MT_startstop; /*default*/
        } else {
            global.mtmode = store_mtmode;
        }

        if (store_num_threads > 0) {
            global.num_threads = store_num_threads;
        } else {
            global.num_threads = 0;
        }
    });

    ARGS_OPTION_BEGIN ("mtmode")
    {
        ARG_RANGE (store_mtmode, MT_createjoin, MT_mtstblock);
        if (global.mtmode != MT_none)
            global.mtmode = store_mtmode;
    }
    ARGS_OPTION_END ("mtmode");

    ARGS_OPTION ("maxnewgens", ARG_NUM (global.max_newgens));

    ARGS_OPTION ("maxoptcyc", ARG_NUM (global.max_optcycles));

    ARGS_OPTION ("maxrecinl", CTIabort ("Option -maxrecinl de-activated temporarily");
                 ARG_NUM (global.max_recursive_inlining));

    ARGS_OPTION ("maxlur", ARG_NUM (global.unrnum));

    ARGS_OPTION ("maxwlur", ARG_NUM (global.wlunrnum));

    ARGS_OPTION ("maxprfur", ARG_NUM (global.prfunrnum));

    ARGS_OPTION ("maxspec", ARG_NUM (global.maxspec));

    ARGS_OPTION ("maxthreads", ARG_NUM (global.max_threads));

    ARGS_OPTION ("maxae", ARG_NUM (global.minarray));

    ARGS_OPTION ("minmtsize", ARG_NUM (global.min_parallel_size));

    ARGS_OPTION ("maxrepsize", ARG_NUM (global.max_replication_size));

    ARGS_OPTION ("maxwls", ARG_NUM (global.maxwls));

    ARGS_OPTION_BEGIN ("minarrayrep")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("s", global.min_array_rep = MAR_scl_aks);
        ARG_CHOICE ("d", global.min_array_rep = MAR_scl_akd);
        ARG_CHOICE ("+", global.min_array_rep = MAR_scl_aud);
        ARG_CHOICE ("*", global.min_array_rep = MAR_aud);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("minarrayrep");

    ARGS_FLAG ("M", global.makedeps = TRUE);
    ARGS_FLAG ("Mlib", global.makedeps = global.makelibdeps = TRUE);

    /* mutc options */
    ARGS_FLAG ("mutc_benchmark", global.mutc_benchmark = TRUE);
    ARGS_FLAG ("mutc_disable_thread_mem", global.mutc_disable_thread_mem = TRUE);

    ARGS_OPTION ("mutc_distribute_arg", ARG_NUM (global.mutc_distribution_mode_arg));

    ARGS_OPTION ("mutc_force_block_size", ARG_NUM (global.mutc_force_block_size));

    ARGS_OPTION ("mutc_unroll", ARG_NUM (global.mutc_unroll));

    ARGS_OPTION_BEGIN ("mutc_distribute");
    ARG_CHOICE_BEGIN ();
    ARG_CHOICE ("bounded", global.mutc_distribution_mode = MUTC_DMODE_bounded);
    ARG_CHOICE ("toplevel", global.mutc_distribution_mode = MUTC_DMODE_toplevel);
    ARG_CHOICE_END ();
    ARGS_OPTION_END ();

    ARGS_FLAG ("mutc_fun_threads", global.mutc_fun_as_threads = TRUE);
    ARGS_FLAG ("mutc_thread_mem", global.mutc_thread_mem = TRUE);
    ARGS_FLAG ("mutc_static_resource_management",
               global.mutc_static_resource_management = TRUE);

    /*
     * Options starting with nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
     */

    ARGS_FLAG ("nofoldfusion", global.no_fold_fusion = TRUE);

    ARGS_FLAG ("nofoldparallel", global.no_fold_parallel = TRUE);

    ARGS_FLAG ("noprelude", global.loadprelude = FALSE);

    ARGS_OPTION_BEGIN ("numthreads")
    {
        ARG_RANGE (store_num_threads, 1, global.max_threads);
        if (global.mtmode != MT_none) {
            global.num_threads = store_num_threads;
        }
    }
    ARGS_OPTION_END ("numthreads");

    ARGS_OPTION_BEGIN ("no")
    {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("opt", global.optimize = global.optimize_none);

#define OPTIMIZE(str, abbr, devl, prod, name)                                            \
    ARG_CHOICE (str, global.optimize.do##abbr = FALSE);
#include "optimize.mac"

        ARG_CHOICE ("pab", global.print_after_break = FALSE);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("no");

    /*
     * Options starting with ooooooooooooooooooooooooooooooooooooooooooo
     */

    ARGS_OPTION ("o", global.outfilename = STRcpy (ARG));
    /*
     * The option is only stored in outfilename,
     * the correct settings of the global variables
     * outfilename, cfilename, and targetdir will be done
     * in SetFileNames() in scnprs.c. This cannot be done here
     * because you have to know the kind of file (program
     * or module/class implementation).
     */

    ARGS_OPTION ("O", ARG_RANGE (global.cc_optimize, 0, 3));

    /*
     * Options starting with ppppppppppppppppppppppppppppppppppppppppppp
     */

    ARGS_FLAG ("prsc", global.print_resources = TRUE);

    ARGS_OPTION_BEGIN ("print")
    {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.print = global.print_all; global.doprint = TRUE);
#define PRINT(flag, char, default)                                                       \
    ARG_FLAGMASK (char, global.print.flag = TRUE; global.doprint = TRUE);
#include "flags.mac"
        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("print");

    ARGS_OPTION_BEGIN ("profile")
    {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.profile = global.profile_all; global.doprofile = TRUE);

#define PROFILE(flag, char, default)                                                     \
    ARG_FLAGMASK (char, global.profile.flag = TRUE; global.doprofile = TRUE);
#include "flags.mac"

        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("profile");

    /*
     * Options starting with rrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrrr
     */

    ARGS_FLAG ("ssaiv", global.ssaiv = TRUE);

    /*
     * Options starting with sssssssssssssssssssssssssssssssssssssssssss
     */

    ARGS_OPTION_BEGIN ("sigspec")
    {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("akv", global.sigspec_mode = SSP_akv);
        ARG_CHOICE ("aks", global.sigspec_mode = SSP_aks);
        ARG_CHOICE ("akd", global.sigspec_mode = SSP_akd);
        ARG_CHOICE ("aud", global.sigspec_mode = SSP_aud);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("sigspec");

    ARGS_FLAG ("simd", global.simd = TRUE);

    ARGS_OPTION_BEGIN ("specmode")
    {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("aks", global.spec_mode = SS_aks);
        ARG_CHOICE ("akd", global.spec_mode = SS_akd);
        ARG_CHOICE ("aud", global.spec_mode = SS_aud);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("specmode");

    /*
     * Options starting with ttttttttttttttttttttttttttttttttttttttttttt
     */

    ARGS_OPTION ("target", global.target_name = ARG);

    ARGS_FLAG ("tog", global.dotogstuff = TRUE);

    ARGS_OPTION_BEGIN ("trace")
    {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.trace = global.trace_all; global.dotrace = TRUE);

#define TRACE(flag, char, default)                                                       \
    ARG_FLAGMASK (char, global.trace.flag = TRUE; global.dotrace = TRUE);
#include "flags.mac"

        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("trace");

    ARGS_OPTION ("t", global.target_name = ARG);

    /*
     * Options starting with uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
     */

    ARGS_FLAG ("utrace", global.dousertrace = TRUE);

    /*
     * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
     */

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 5));

    ARGS_FLAG ("V", USGprintVersion (); exit (0));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); exit (0));

    /*
     * Options starting with wwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwwww
     */

    ARGS_FLAG ("wls_aggressive", global.wls_aggressive = TRUE);

    /*
     * Options starting with strange symbols
     */

#ifndef DBUG_OFF

    ARGS_OPTION_BEGIN ("#")
    {
        if (NULL == strchr (ARG, '/')) {
            global.my_dbug = TRUE;
            global.my_dbug_from = PHIfirstPhase ();
            global.my_dbug_to = PHIlastPhase ();
            global.my_dbug_str = STRcpy (ARG);
        } else {
            PHOinterpretDbugOption (ARG);
        }

        if (global.my_dbug_from == PHIfirstPhase ()) {
            DBUG_PUSH (global.my_dbug_str);
            global.my_dbug_active = TRUE;
        }
    }
    ARGS_OPTION_END ("#");

#endif /* DBUG_OFF */

    /*
     * Arguments
     */

    ARGS_ARGUMENT ({
        if (global.sacfilename == NULL) {
            global.sacfilename = STRcpy (ARG);

            global.puresacfilename = strrchr (global.sacfilename, '/');

            if (global.puresacfilename == NULL) {
                global.puresacfilename = global.sacfilename;
            } else {
                global.puresacfilename += 1;
            }
        } else {
            ARGS_ERROR ("Too many source files specified");
        }
    });

    ARGS_UNKNOWN (ARGS_ERROR ("Invalid command line entry"));

    ARGS_END ();

    DBUG_VOID_RETURN;
}

static void
AnalyseCommandlineSac4c (int argc, char *argv[])
{
    DBUG_ENTER ("AnalyseCommandlineSac4c");

    ARGS_BEGIN (argc, argv);

    /*
     * Options starting with aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
     */

    /*
     * Options starting with bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
     */

    ARGS_OPTION ("b", PHOinterpretBreakOption (ARG))

    /*
     * Options starting with ccccccccccccccccccccccccccccccccccccccccccc
     */

    ARGS_FLAG ("ccflags", global.printccflags = TRUE;);

    ARGS_FLAG ("copyright", USGprintCopyright (); exit (0));

    /*
     * Options starting with ddddddddddddddddddddddddddddddddddddddddddd
     */

    ARGS_OPTION_BEGIN ("d")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("nocleanup", global.cleanup = FALSE);
        ARG_CHOICE ("syscall", global.show_syscall = TRUE);
        ARG_CHOICE ("cccall", global.gen_cccall = TRUE; global.cleanup = FALSE);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("d");

    /*
     * Options starting with eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
     */

    ARGS_OPTION_BEGIN ("E")
    {
        FMGRappendPath (PK_extlib_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("E");

    /*
     * Options starting with ggggggggggggggggggggggggggggggggggggggggggg
     */

    ARGS_FLAG ("g", global.cc_debug = TRUE);

    /*
     * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
     */

    ARGS_FLAG ("h", USGprintUsage (); exit (0));
    ARGS_FLAG ("help", USGprintUsage (); exit (0));

    /*
     * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
     */

    ARGS_OPTION ("incdir", global.inc_dirname = STRcpy (ARG););

    ARGS_OPTION_BEGIN ("I")
    {
        FMGRappendPath (PK_imp_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("I");

    /*
     * Options starting with kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
     */

    /*
     * Options starting with lllllllllllllllllllllllllllllllllllllllllll
     */

    ARGS_FLAG ("ldflags", global.printldflags = TRUE;);

    ARGS_OPTION_BEGIN ("libdir")
    global.lib_dirname = STRcpy (ARG);
    FMGRappendPath (PK_extlib_path, FMGRabsolutePathname (ARG));
    ARGS_OPTION_END ("libdir");

    ARGS_OPTION_BEGIN ("L")
    {
        FMGRappendPath (PK_lib_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("L");

    /*
     * Options starting with mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
     */

    /*
     * Options starting with nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
     */

    /*
     * Options starting with ooooooooooooooooooooooooooooooooooooooooooo
     */

    ARGS_OPTION ("o", global.outfilename = STRcpy (ARG));

    ARGS_OPTION ("O", ARG_RANGE (global.cc_optimize, 0, 3));

    /*
     * Options starting with ppppppppppppppppppppppppppppppppppppppppppp
     */

    /*
     * Options starting with sssssssssssssssssssssssssssssssssssssssssss
     */

    /*
     * Options starting with ttttttttttttttttttttttttttttttttttttttttttt
     */

    /*
     * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
     */

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 5));

    ARGS_FLAG ("V", USGprintVersion (); exit (0));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); exit (0));

    /*
     * DBUG options ####################################################
     */

#ifndef DBUG_OFF

    ARGS_OPTION_BEGIN ("#")
    {
        if (NULL == strchr (ARG, '/')) {
            global.my_dbug = TRUE;
            global.my_dbug_from = PHIfirstPhase ();
            global.my_dbug_to = PHIlastPhase ();
            global.my_dbug_str = STRcpy (ARG);
        } else {
            PHOinterpretDbugOption (ARG);
        }

        if (global.my_dbug_from == PHIfirstPhase ()) {
            DBUG_PUSH (global.my_dbug_str);
            global.my_dbug_active = TRUE;
        }
    }
    ARGS_OPTION_END ("#");

#endif /* DBUG_OFF */

    /*
     * arguments
     */

    ARGS_ARGUMENT (global.exported_modules
                   = STRSadd (STRcpy (ARG), STRS_saclib, global.exported_modules););

    ARGS_UNKNOWN (ARGS_ERROR ("Invalid command line entry"));

    ARGS_END ();

    /*
     * set defaults not altered by arguments
     */
    if (global.outfilename == NULL) {
        global.outfilename = STRcpy ("cwrapper");
    }
    global.optimize.dophm = FALSE;
    global.filetype = F_cmod;
    if (global.printldflags || global.printccflags) {
        global.verbose_level = 0;
    }

    if (global.exported_modules == NULL) {
        CTIabort ("No modules given as argument. See sac4c -h for details.");
    }

    if (global.printldflags && global.printccflags) {
        CTIabort ("-ldflags and -ccflags cannot be used simultaneously.");
    }

    DBUG_VOID_RETURN;
}

static void
AnalyseCommandlineSac2tex (int argc, char *argv[])
{
    DBUG_ENTER ("AnalyseCommandlineSac2tex");

    ARGS_BEGIN (argc, argv);

    /*
     * Options starting with bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
     */

    ARGS_OPTION ("b", PHOinterpretBreakOption (ARG))

    /*
     * Options starting with ccccccccccccccccccccccccccccccccccccccccccc
     */

    ARGS_FLAG ("copyright", USGprintCopyright (); exit (0));

    /*
     * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
     */

    ARGS_FLAG ("h", USGprintUsage (); exit (0));
    ARGS_FLAG ("help", USGprintUsage (); exit (0));

    /*
     * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
     */

    ARGS_OPTION ("incdir", global.inc_dirname = STRcpy (ARG););

    ARGS_OPTION_BEGIN ("I")
    {
        FMGRappendPath (PK_imp_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("I");

    /*
     * Options starting with kkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkkk
     */

    /*
     * Options starting with lllllllllllllllllllllllllllllllllllllllllll
     */

    ARGS_FLAG ("ldflags", global.printldflags = TRUE;);

    ARGS_OPTION ("libdir", global.lib_dirname = STRcpy (ARG););

    ARGS_OPTION_BEGIN ("L")
    {
        FMGRappendPath (PK_lib_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("L");

    /*
     * Options starting with ooooooooooooooooooooooooooooooooooooooooooo
     */

    ARGS_OPTION ("o", global.outfilename = STRcpy (ARG));

    /*
     * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
     */

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 5));

    ARGS_FLAG ("V", USGprintVersion (); exit (0));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); exit (0));

    /*
     * DBUG options ####################################################
     */

#ifndef DBUG_OFF

    ARGS_OPTION_BEGIN ("#")
    {
        if (NULL == strchr (ARG, '/')) {
            global.my_dbug = TRUE;
            global.my_dbug_from = PHIfirstPhase ();
            global.my_dbug_to = PHIlastPhase ();
            global.my_dbug_str = STRcpy (ARG);
        } else {
            PHOinterpretDbugOption (ARG);
        }

        if (global.my_dbug_from == PHIfirstPhase ()) {
            DBUG_PUSH (global.my_dbug_str);
            global.my_dbug_active = TRUE;
        }
    }
    ARGS_OPTION_END ("#");

#endif /* DBUG_OFF */

    /*
     * arguments
     */

    ARGS_ARGUMENT ({
        if (global.sacfilename == NULL) {
            global.sacfilename = STRcpy (ARG);

            global.puresacfilename = strrchr (global.sacfilename, '/');

            if (global.puresacfilename == NULL) {
                global.puresacfilename = global.sacfilename;
            } else {
                global.puresacfilename += 1;
            }
        } else {
            ARGS_ERROR ("Too many source files specified");
        }
    });

    ARGS_UNKNOWN (ARGS_ERROR ("Invalid command line entry"));

    ARGS_END ();

    /*
     * set defaults not altered by arguments
     */
    if (global.outfilename == NULL) {
        global.outfilename = STRcpy ("doc");
    }
    global.optimize.dophm = FALSE;
    if (global.printldflags || global.printccflags) {
        global.verbose_level = 0;
    }

    DBUG_VOID_RETURN;
}

void
OPTanalyseCommandline (int argc, char *argv[])
{
    DBUG_ENTER ("OPTanalyseCommandline");

    switch (global.tool) {
    case TOOL_sac2c:
        AnalyseCommandlineSac2c (argc, argv);
        break;
    case TOOL_sac4c:
        AnalyseCommandlineSac4c (argc, argv);
        break;
    case TOOL_sac2tex:
        AnalyseCommandlineSac2tex (argc, argv);
        break;
    }

    DBUG_VOID_RETURN;
}
