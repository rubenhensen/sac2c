/**
 * @file
 *
 * @brief
 *  This file provides means for the analysis of sac2c command line arguments.
 *  It uses the set macro definitions from getoptions.h
 *
 */
#include <sys/param.h>
#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>

#include "config.h"

#define DBUG_PREFIX "OPT"
#include "debug.h"

#include "getoptions.h"

#include "usage.h"
#include "filemgr.h"
#include "globals.h"
#include "str.h"
#include "memory.h"

#include "ctformatting.h"
#include "ctinfo.h"

#include "libstat.h"
#include "stringset.h"
#include "phase_options.h"
#include "phase_info.h"
#include "resource.h"
#include "rtspec_modes.h"
#include "runtime_compiler.h"
#include "str_buffer.h"

/**
 * @brief
 *   Some options require processing as early as possible in the compilation
 *   process. For example, memory checks must be initiated BEFORE any
 *   allocations are done. This requires the corresponding option to be
 *   processed before global variables are initialised.
 *
 *   We deliberately switch off error detection because that would obviously
 *   complain about illegal options and because the compile time information
 *   system has not yet been set up properly.
 *
 * @param argc number of arguments given
 * @param argv argument array
 */
void
OPTcheckPreSetupOptions (int argc, char *argv[])
{
    DBUG_ENTER ();

    ARGS_BEGIN (argc, argv);

#ifndef DBUG_OFF
    // We look for "d " to avoid tripping over "-docf", etc.
    ARGS_OPTION_BEGIN ("d ")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("memcheck", global.memcheck = TRUE);
        ARG_CHOICE ("noclean", global.memclean = FALSE);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("d ");
#endif /* DBUG_OFF */

    ARGS_END ();

    DBUG_RETURN ();
}


/**
 * @brief Ensure integer is a ^2
 *
 * @param arg integer to check
 * @return True if ^2, otherwise false
 */
static bool
OPTpowOf2 (int arg)
{
    int exp = 0;
    int orgArg = arg;
    DBUG_ENTER ();

    DBUG_ASSERT (arg > 0, "Must be positive number");
    while (arg > 0) {
        arg = arg >> 1;
        exp++;
    }
    DBUG_RETURN ((1 << (exp - 1)) == orgArg);
}

/**
 * @brief
 *   This function is called after command line arguments have been
 *   analysed and sac2crc has been read. It propagates SBI-defined
 *   defaults and then prints the help text if requested by the user.
 */
void
OPTcheckPostSetupOptions (void)
{
    DBUG_ENTER ();

    /*
     * PHM enable/disable.
     */
#define CHECKDEFAULT(Opt, Default)                                                       \
    if (global.optimize.Opt != TRUE && global.optimize.Opt != FALSE) {                   \
        global.optimize.Opt = ((unsigned)Default) & 3;                                   \
    }

    CHECKDEFAULT (dophm, global.config.use_phm_api);
    CHECKDEFAULT (doaps, global.config.use_phm_api);
    CHECKDEFAULT (dodpa, global.config.use_phm_api);
    CHECKDEFAULT (domsca, global.config.use_phm_api);

    if (global.print_help_and_exit) {
        USGprintUsage ();
        CTIexit (EXIT_SUCCESS);
    }

    DBUG_RETURN ();
}

/**
 * @brief Check that given name only contains alpha-numeric,
 *        space, and ~.-_/ characters.
 *
 * @param name file (or lib) name
 * @return True if valid name, otherwise false
 */
static bool
OPTisValidName (const char *name)
{
    const char *tmp;
    bool res = true;

    DBUG_ENTER ();

    if (name != NULL)
    {
        tmp = name;
        while (*tmp != '\0')
        {
            if (!(isalnum (*tmp) || isblank (*tmp) || *tmp == '.'
                  || *tmp == '~' || *tmp == '/' || *tmp == '-'
                  || *tmp == '_'))
            {
                res = false;
                break;
            }
            tmp++;
        }
    } else
        res = false;

    DBUG_RETURN (res);
}

/**
 * @brief
 *   This function is called right after command line arguments
 *   have been analysed. Errors and warnings are produced whenever the user
 *   has selected an incompatible combination of options.
 */
void
OPTcheckOptionConsistency (void)
{

    DBUG_ENTER ();

    if (global.sacfilename && !OPTisValidName (global.sacfilename)) {
        CTIerror (EMPTY_LOC, "Input filename can only contain alpha-numeric and space characters!");
    }

    if (global.outfilename && !OPTisValidName (global.outfilename)) {
        CTIerror (EMPTY_LOC, "Argument to `-o' flag can only contain alpha-numeric and space characters!");
    }

    if (global.optimize.dophm && !global.config.use_phm_api) {
        CTIerror (EMPTY_LOC, "Private heap management disabled for this SBI.");
    }

    global.mtmode = MT_none;
    switch (global.config.mt_mode) {
    case 0:
        break;
    case 1:
        global.mtmode = MT_createjoin;
        break;
    case 2:
        global.mtmode = MT_startstop;
        break;
    case 3:
        global.mtmode = MT_mtstblock;
        break;
    default:
        CTIerror (EMPTY_LOC, "Target MT_MODE has an invalid value.");
    }

    if (STReq (global.config.backend, "MUTC")) {

        if (global.mtmode != MT_none) {
            CTIerror (EMPTY_LOC, "The MUTC backend only supports MT_MODE = 0.");
        }

        if ((global.mutc_disable_thread_mem == TRUE)
            && (global.mutc_thread_mem == TRUE)) {
            CTIerror (EMPTY_LOC, "-mutc_thread_mem can not be used with "
                                 "-mutc_disable_thread_mem");
        }

        if ((global.mutc_distribution_mode == MUTC_DMODE_bounded)
            && (global.mutc_distribution_mode_arg == -1)) {
            CTIerror (EMPTY_LOC, "bounded distribution mode requires an argument");
        }

        if ((global.mutc_force_block_size != -1)
            && (global.mutc_static_resource_management)) {
            CTIerror (EMPTY_LOC, "Can only use one method of setting the block size at a time");
        }

        if (!OPTpowOf2 (global.mutc_rc_places)) {
            CTIerror (EMPTY_LOC, "-mutc_rc_places must be a power of 2");
        }

        if ((global.mutc_rc_places != 1) && (global.mutc_rc_indirect == TRUE)) {
            CTIerror (EMPTY_LOC, "-mutc_rc_places can not be used with "
                                 "-mutc_rc_indirect");
        }
    } else {
        if (global.mutc_fun_as_threads == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_fun_threads only works with mutc backend");
        }
        if (global.mutc_thread_mem == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_thread_mem only works with mutc backend");
        }
        if (global.mutc_disable_thread_mem == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_disable_thread_mem only works with mutc backend");
        }
        if (global.mutc_static_resource_management == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_static_resource_management needs mutc backend");
        }
        if (global.mutc_force_block_size != -1) {
            CTIerror (EMPTY_LOC, "-mutc_force_block_size only works with mutc backend");
        }
        if (global.mutc_benchmark == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_benchmark needs mutc backend");
        }
        if (global.mutc_force_spawn_flags != NULL) {
            CTIerror (EMPTY_LOC, "-mutc_force_spawn_flags needs mutc backend");
        }
        if (global.mutc_suballoc_desc_one_level_up == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_suballoc_desc_one_level_up only works with mutc backend");
        }
        if (global.mutc_rc_places != 1) {
            CTIerror (EMPTY_LOC, "-mutc_rc_places only works with mutc backend");
        }
        if (global.mutc_rc_indirect == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_rc_indirect only works with mutc backend");
        }
        if (global.mutc_seq_data_parallel == TRUE) {
            CTIerror (EMPTY_LOC, "-mutc_seq_data_parallel only works with mutc backend");
        }
    }

    if (global.runtimecheck.boundary && global.optimize.doap) {
        global.optimize.doap = FALSE;
        CTIwarn (EMPTY_LOC, "Option -check b requires option -noAP\n"
                            "Array padding disabled");
    }

    if (global.runtimecheck.conformity && !global.insertconformitychecks) {
        global.insertconformitychecks = TRUE;
        CTIwarn (EMPTY_LOC, "Option -check c implies option -ecc.\n"
                            "Insertion of explicit conformity checks has been enabled.");
    }

    /* Checks related to the distributed memory backend */
    if (global.backend == BE_distmem) {
        /* The distributed memory backend is used. Check for incompatible options. */

#if !ENABLE_DISTMEM
        CTIerror (EMPTY_LOC, "The distributed memory backend (-target) is not supported by this "
                  "installation.");
#endif

        if (global.mtmode != MT_none) {
            CTIerror (EMPTY_LOC, "Multi-threaded program execution is not "
                                 "supported when using the distributed memory backend.");
        }

        if (global.optimize.dodmgs && !global.optimize.dodmmls) {
            CTIerror (EMPTY_LOC, "The optimization DMGS requires the optimization DMMLS.");
        }

        if (global.optimize.dodmmls && global.dmgs_max_selects != 0
            && global.dmgs_min_selects > global.dmgs_max_selects) {
            CTIerror (EMPTY_LOC, "dmgs_max_selects must be 0 (unbounded) or >= dmgs_min_selects");
        }

        /* Communication library specific checks */
        switch (global.distmem_commlib) {
        case DISTMEM_COMMLIB_MPI:
#if !ENABLE_DISTMEM_MPI
            CTIerror (EMPTY_LOC, "The MPI communication library (-target) for the distributed "
                      "memory backend is not supported by this installation.");
#endif

            if (global.distmem_cache_outside_dsm) {
                CTIwarn (EMPTY_LOC,
                         "When MPI is used as a communication library, the cache is "
                         "always allocated "
                         "outside of the DSM segment. dsm_cache_outside_seg does not "
                         "have any effect.");
            } else {
                global.distmem_cache_outside_dsm = TRUE;
            }
            break;
        case DISTMEM_COMMLIB_ARMCI:
#if !ENABLE_DISTMEM_ARMCI
            CTIerror (EMPTY_LOC, "The ARMCI communication library (-target) for the distributed "
                      "memory backend is not supported by this installation.");
#endif

            if (global.distmem_cache_outside_dsm) {
                CTIwarn (EMPTY_LOC,
                         "When ARMCI is used as a communication library, the cache is "
                         "always allocated "
                         "outside of the DSM segment. dsm_cache_outside_seg does not "
                         "have any effect.");
            } else {
                global.distmem_cache_outside_dsm = TRUE;
            }
            break;
        case DISTMEM_COMMLIB_GPI:
#if !ENABLE_DISTMEM_GPI
            CTIerror (EMPTY_LOC, "The GPI communication library (-target) for the distributed "
                      "memory backend is not supported by this installation.");
#endif

            if (global.distmem_cache_outside_dsm) {
                CTIwarn (EMPTY_LOC,
                         "When GPI is used as a communication library, the cache is "
                         "always allocated "
                         "within the DSM segment. dsm_cache_outside_seg does not have "
                         "any effect.");
                global.distmem_cache_outside_dsm = FALSE;
            }
            break;
        case DISTMEM_COMMLIB_GASNET:
#if !ENABLE_DISTMEM_GASNET
            CTIerror (EMPTY_LOC, "The GASNet communication library (-target) for the distributed "
                      "memory backend is not supported by this installation.");
#endif

            if (global.distmem_cache_outside_dsm) {
                CTIwarn (EMPTY_LOC,
                         "The cache will be registered outside of the GASNet segment "
                         "which can cause bugs. If you experience the following error "
                         "this is probably the reason: FATAL ERROR: ibv_reg_mr failed in "
                         "firehose_move_callback errno=14 (Bad address). To fix this "
                         "problem, you can deactivate firehose by setting "
                         "GASNET_USE_FIREHOSE=NO but this may affect performance.");
            }
            break;
        default:
            /* The communication library is obligatory when the distributed memory backend
             * is used and a SAC program is compiled. Modules are compiled for the
             * distributed memory backend but not for a specific communication library.
             * The same holds for the prelude. However, we cannot enforce this check here
             * because we do not know yet if we are compiling a module. */
            break;
        }
    } else {
        /* The distributed memory backend is not used. Disable options that don't apply.
         */
        global.runtimecheck.distmem = FALSE;
        global.trace.distmem = FALSE;
        global.profile.distmem = FALSE;
    }

    /* turn on default multithreading if using cuda hybrid backend */
    if (global.backend == BE_cudahybrid && global.mtmode == MT_none) {
        CTIerror (EMPTY_LOC, "Target MT_MODE must be set with BACKEND CudaHybrid.");
    }

    if (global.backend == BE_omp) {
        if (global.mtmode == MT_none) {
            CTIerror (EMPTY_LOC, "Target MT_MODE must be set with BACKEND omp.");
        }
        if (!STReq (global.config.mt_lib, "omp")) {
            CTIerror (EMPTY_LOC, "Target MT_LIB must be set to omp with BACKEND omp.");
        }
    }

    if (global.backend == BE_c99 && global.mtmode != MT_none) {
        /* multithreading requested */
        /* check the mt_lib spec */
        if (!(STReq (global.config.mt_lib, "lpel")
              || STReq (global.config.mt_lib, "pthread"))) {
            global.mtmode = MT_none;
            global.num_threads = 1;
            CTIerror (EMPTY_LOC, "The MT_LIB specification can be either 'pthread' or 'lpel'.");
        }
    }

    if (global.mtmode != MT_none) {
        if (global.docachesim) {
            CTIerror (EMPTY_LOC, "Cache simulation is not available for multi-threaded "
                                 "program execution");
        }

        if (global.doprofile) {
            CTIerror (EMPTY_LOC, "Profiling is not available for multi-threaded "
                                 "program execution");
        }
    }

    if (global.runtimecheck.heap && !global.optimize.dophm) {
        CTIerror (EMPTY_LOC, "Diagnostic heap management is only available in "
                             "conjunction with private heap management.\n"
                             "Diagnostic disabled");
    }

    if (global.fp && global.mtmode == MT_none) {
        CTIerror (EMPTY_LOC, "Functional Parallelism only works when MT is enabled");
    }

    if (STReq (global.config.cuda_alloc, "cumanp") && STRgt("SM_60", global.config.cuda_arch)) {
        CTIwarn (EMPTY_LOC, "Compiling for CC < 6.0 (Pascal), CUDA prefetching is not available. Disabling...");
       global.optimize.docuprf = FALSE;
    }

    if (global.optimize.dosaa && !global.optimize.dodcr) {
        CTIwarn (EMPTY_LOC,
                 "Symbolic array attributes (SAA) require dead coderemoval (DCR).\n"
                 "Symbolic array attributes disabled.");
        global.optimize.dosaa = FALSE;
    }

    if (global.optimize.doive && !global.optimize.dosaa) {
        CTIwarn (EMPTY_LOC,
                 "Index vector elimination (IVE) requires symbolic array "
                 "attributes (SAA).\n"
                 "Index vector elimination disabled.");
        global.optimize.doive = FALSE;
    }

    if (!global.optimize.dowlur) {
        CTIwarn (EMPTY_LOC,
                 "With-Loop unrolling (WLUR) was disabled using the command line. "
                 "However, unrolling of single-trip with-loops is required for "
                 "code generation. Therefore, WLUR will be re-enabled with the  "
                 "maximum number of unrolling steps set to 1.");
        global.wlunrnum = 1;
        global.optimize.dowlur = TRUE;
    }

    // Any polyhedral optimizations enabled
    if ((global.optimize.dopwlf || global.optimize.dopogo || global.optimize.doplur)) {
        if (!global.optimize.dossawl) {
            global.optimize.dossawl = TRUE;
            CTIwarn (EMPTY_LOC,
                     "PLUR/POGO/PWLF is enabled: -dossawl enabled.");
        }
    }

    if (global.optimize.doawlf && (!global.insertconformitychecks)) {
        global.insertconformitychecks = TRUE;
        CTIwarn (EMPTY_LOC, "AWLF is enabled: -ecc enabled.");
    }

    if (global.optimize.doawlf && (!global.doivext)) {
        global.doivext = TRUE;
        CTIwarn (EMPTY_LOC, "AWLF is enabled: -extrema enabled.");
    }

    if (global.optimize.doawlf) {
        global.max_optcycles = global.max_optcycles * 2;
        CTIwarn (EMPTY_LOC, "AWLF is enabled: -maxoptcyc=%d", global.max_optcycles);
    }

    if ((global.mtmode == MT_none) && (global.num_threads != 1)) {
        CTItell (4, "MT_MODE = 0 in target, forcing -numthreads to 1.");
        global.num_threads = 1;
    }

#if !ENABLE_HWLOC
    if (global.cpubindstrategy != HWLOC_off) {
        CTIwarn (EMPTY_LOC,
          "sac2c was compiled without hwloc support, forcing cpubindstrategy to OFF.");
        global.cpubindstrategy = HWLOC_off;
    }
#endif

    if ((global.mt_smart_mode > 0) && (global.min_parallel_size != 0)) {
        CTIwarn (EMPTY_LOC,
                 "Cannot use both -minmtsize and -mt_smart_mode simultaniously, "
                 "setting -minmtsize to 0.");
        global.min_parallel_size = 0;
    }

    if ((global.mt_smart_mode > 0) && (global.mt_smart_filename == NULL)) {
        CTIerror (EMPTY_LOC, "-mt_smart_filename must be set when using -mt_smart_mode. ");
    }

    if ((global.mt_smart_mode > 0) && (global.mt_smart_arch == NULL)) {
        CTIerror (EMPTY_LOC, "-mt_smart_arch must be set when using -mt_smart_mode. ");
    }

    if ((global.mt_smart_filename != NULL)
        && (strstr (global.mt_smart_filename, ".") != NULL)) {
        CTIerror (EMPTY_LOC, "Illegal use of dot (\".\") character in -mt_smart_filename.");
    }

    if ((global.mt_smart_arch != NULL) && (strstr (global.mt_smart_arch, ".") != NULL)) {
        CTIerror (EMPTY_LOC, "Illegal use of dot (\".\") character in -mt_smart_arch.");
    }

    /* validity check of RC_METHOD entry */

    if (STReq (global.config.rc_method, "local")) {
    } else if (STReq (global.config.rc_method, "norc")) {
    } else if (STReq (global.config.rc_method, "async")) {
    } else if (STReq (global.config.rc_method, "local_norc_desc")) {
        if (global.backend != BE_mutc) {
            CTIerror (EMPTY_LOC,
                      "Specified reference counting method %s is currently only "
                      "supported for the backend BE_mutc!",
                      global.config.rc_method);
        }
    } else if (STReq (global.config.rc_method, "local_norc_ptr")) {
        if (global.backend != BE_mutc) {
            CTIerror (EMPTY_LOC,
                      "Specified reference counting method %s is currently only "
                      "supported for the backend BE_mutc!",
                      global.config.rc_method);
        }
    } else if (STReq (global.config.rc_method, "async_norc_copy_desc")) {
        if (global.backend != BE_mutc) {
            CTIerror (EMPTY_LOC,
                      "Specified reference counting method %s is currently only "
                      "supported for the backend BE_mutc!",
                      global.config.rc_method);
        }
    } else if (STReq (global.config.rc_method, "async_norc_two_descs")) {
        CTIerror (EMPTY_LOC,
                  "Specified reference counting method %s is currently not supported!",
                  global.config.rc_method);
    } else if (STReq (global.config.rc_method, "async_norc_ptr")) {
        if (global.backend != BE_mutc) {
            CTIerror (EMPTY_LOC,
                      "Specified reference counting method %s is currently only "
                      "supported for the backend BE_mutc!",
                      global.config.rc_method);
        }
    } else if (STReq (global.config.rc_method, "local_pasync_norc_desc")) {
        CTIwarn (EMPTY_LOC, "Specified reference counting method %s is a work in progress!",
                 global.config.rc_method);
    } else if (STReq (global.config.rc_method, "local_async_norc_ptr")) {
        CTIwarn (EMPTY_LOC, "Specified reference counting method %s is a work in progress!",
                 global.config.rc_method);
    } else if (STReq (global.config.rc_method, "local_pasync")) {
        CTIwarn (EMPTY_LOC, "Specified reference counting method %s is a work in progress!",
                 global.config.rc_method);
    } else {
        CTIerror (EMPTY_LOC, "Illegal reference counting method specified RC_METHOD == %s !",
                  global.config.rc_method);
    }

    /* Setup paths:
     * if compiling a module, -o/-olib point to directories that must be searched first.
     * TREE_OUTPUTDIR and LIB_OUTPUTDIR must be searched second.
     */
    FMGRprependPath (PK_tree_path, global.config.tree_outputdir);
    FMGRprependPath (PK_lib_path, global.config.lib_outputdir);

    DBUG_RETURN ();
}

/**
 * @brief
 *  This function is called after the compiler knows whether
 *  it is compiling a module or a plain program, but before
 *  it starts searching for dependent modules.
 *
 *  @param in_module indicate whether we're compiling a module
 */
void
OPTcheckOptionConsistencyForTarget (bool in_module)
{
    DBUG_ENTER ();

    if (in_module) {

        /* when compiling a module, the directory specified with
           -o must be searched to find dependent modules.
           However this must not be done when compiling plain
           programs, because then -o specifies a binary file, not
           a directory.
        */

        if (global.outfilename != NULL) {
            FMGRprependPath (PK_tree_path, global.outfilename);
            FMGRprependPath (PK_lib_path, global.outfilename);
        }
    }

    if (global.target_modlibdir != NULL) {
        FMGRprependPath (PK_lib_path, global.target_modlibdir);
    }

    DBUG_RETURN ();
}

/**
 * @fn node *OPTanalyseCommandline(  int argc, char *argv[])
 *
 * @brief
 *   This function analyses the commandline options given to sac2c.
 *   Usually selections made are stored in global variables for later
 *   reference.
 *
 * @param argc number of arguments given
 * @param argv argument array
 */

#undef ARGS_ERROR
#define ARGS_ERROR(msg)                                                                  \
    {                                                                                    \
        CTIerror (EMPTY_LOC, "%s: %s %s %s", msg, ARGS_argv[0], STRonNull ("", OPT),     \
                  STRonNull ("", ARG));                                                  \
    }

static void
AnalyseCommandlineSac2c (int argc, char *argv[])
{
    int store_num_threads = 0;

    DBUG_ENTER ();

    str_buf *cppflags_buf = SBUFcreate (1);
    str_buf *cflags_buf = SBUFcreate (1);
    str_buf *ldflags_buf = SBUFcreate (1);
    str_buf *tree_cflags_buf = SBUFcreate (1);
    str_buf *tree_ldflags_buf = SBUFcreate (1);
    str_buf *command_line_buf = SBUFcreate (1);

    for (int i = 1; i < argc; i++) {
        SBUFprintf (command_line_buf, " %s", argv[i]);
    }

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

    ARGS_OPTION ("b", PHOinterpretBreakOption (ARG));

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

    ARGS_FLAG ("copyright", USGprintCopyright (); CTIexit (EXIT_SUCCESS));

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

    ARGS_OPTION ("csdir", strncpy (global.cachesim_dir, ARG, NAME_MAX - 1));

    ARGS_OPTION ("csfile", strncpy (global.cachesim_file, ARG, NAME_MAX - 1));

    ARGS_OPTION ("cshost", strncpy (global.cachesim_host, ARG, NAME_MAX - 1));

    ARGS_OPTION ("cti-message-length", ARG_RANGE (global.cti_message_length,0,255));

    ARGS_FLAG ("cti-no-color", global.cti_no_color = TRUE);

    ARGS_FLAG ("cti-single-line", global.cti_single_line= TRUE);

    ARGS_OPTION ("cti-primary-header-format", global.cti_primary_header_format = STRcpy (ARG));

    ARGS_OPTION ("cti-continuation-header-format", global.cti_continuation_header_format = STRcpy (ARG));

    ARGS_OPTION ("ccflag", CTIwarn (EMPTY_LOC, "Option -ccflag has been replaced by -Xc");
                 SBUFprintf (cflags_buf, " %s", ARG));

    ARGS_FLAG ("cs", global.docachesim = FALSE);

    ARGS_FLAG ("c", global.break_after_phase = PH_cg);

    ARGS_OPTION_BEGIN ("cc")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("ldprog", global.do_clink = DO_C_prog);
        ARG_CHOICE ("ldmod", global.do_clink = DO_C_mod);
        ARG_CHOICE ("ldrmod", global.do_clink = DO_C_rmod);
        ARG_CHOICE ("ccprog", global.do_ccompile = DO_C_prog);
        ARG_CHOICE ("ccmod", global.do_ccompile = DO_C_mod);
        ARG_CHOICE ("ccrmod", global.do_ccompile = DO_C_rmod);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("cc");

    ARGS_OPTION_BEGIN ("cuda_async_mode")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("nosync", global.cuda_async_mode = CUDA_SYNC_NONE);
        ARG_CHOICE ("device", global.cuda_async_mode = CUDA_SYNC_DEVICE);
        ARG_CHOICE ("stream", global.cuda_async_mode = CUDA_SYNC_STREAM);
        ARG_CHOICE ("callback", global.cuda_async_mode = CUDA_SYNC_CALLBACK);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("cuda_async_mode");

    ARGS_FLAG("cuda_gpu_branching", global.cuda_gpu_branching = 1);

    ARGS_OPTION_BEGIN("gpu_mapping_strategy")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("jings_method", global.gpu_mapping_strategy = Jings_method);
        ARG_CHOICE ("jings_method_ext", global.gpu_mapping_strategy = Jings_method_ext);
        ARG_CHOICE ("foldall", global.gpu_mapping_strategy = Foldall);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("gpu_mapping_strategy");

    ARGS_FLAG("gpu_mapping_nocompress", global.gpu_mapping_compress = FALSE);

    ARGS_FLAG("gpu_measure_kernel_time", global.gpu_measure_kernel_time = TRUE);

    /*
     * Options starting with ddddddddddddddddddddddddddddddddddddddddddd
     */
    ARGS_FLAG ("debug_rc", global.debug_rc = TRUE);

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

        ARG_CHOICE ("vab", global.visual_after_break = TRUE);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("do");

    ARGS_OPTION ("dsm_maxmem_mb", ARG_RANGE (global.distmem_max_memory_mb, 0,
                                             1000 * 1024)); /* Max. 1000 GB */

    ARGS_OPTION ("distmem_min_elems",
                 ARG_RANGE (global.distmem_min_elems_per_node, 10, 1000));

    ARGS_OPTION ("distmem_tr_pf_node", ARG_RANGE (global.distmem_tr_pf_node, -1, 1000));

    ARGS_OPTION ("dmgs_min_selects", ARG_RANGE (global.dmgs_min_selects, 3, 100));

    ARGS_OPTION ("dmgs_max_selects", ARG_RANGE (global.dmgs_max_selects, 0, 100));

    ARGS_FLAG ("dsm_cache_outside_seg", global.distmem_cache_outside_dsm = TRUE);

    ARGS_FLAG ("distmem_ptrs_desc", global.distmem_ptrs_desc = TRUE);

    ARGS_FLAG ("distmem_no_ptr_cache", global.distmem_ptr_cache = FALSE);

#ifndef DBUG_OFF

    ARGS_OPTION_BEGIN ("d")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("treecheck", global.treecheck = TRUE);
        ARG_CHOICE ("lacfuncheck", global.lacfuncheck = TRUE);
        ARG_CHOICE ("sancheck", global.sancheck = TRUE);
        ARG_CHOICE ("memcheck", global.memcheck = TRUE);
        ARG_CHOICE ("gpukernel", global.gpukernel = TRUE);
        ARG_CHOICE ("nofree", global.nofree = TRUE);
        ARG_CHOICE ("noclean", global.memclean = FALSE);
        ARG_CHOICE ("nolacinline", global.lacinline = FALSE);
        ARG_CHOICE ("nocleanup", global.cleanup = FALSE);
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
        FMGRappendPath (PK_extlib_path, ARG);
    }
    ARGS_OPTION_END ("E");

    ARGS_FLAG ("enable_structs", global.enable_structs = TRUE);

    ARGS_FLAG ("enforceIEEE", global.enforce_ieee = TRUE);

    ARGS_FLAG ("enforceFloat", global.enforce_float = TRUE);

    /*
     * Options starting with fffffffffffffffffffffffffffffffffffffffffff
     */

    ARGS_OPTION_BEGIN ("feedback")
    {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.feedback = global.feedback_all);

#define FEEDBACK(flag, char, default)                                                       \
    ARG_FLAGMASK (char, global.feedback.flag = TRUE);
#include "flags.mac"

        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("feedback");

    ARGS_FLAG ("force_naive", global.force_naive_with2 = TRUE);
    ARGS_OPTION ("force_desc_size", ARG_NUM (global.force_desc_size));

    ARGS_FLAG ("fp", global.fp = TRUE);
    ARGS_FLAG ("fpnoopt", global.fpnoopt = TRUE);

    ARGS_OPTION ("fVAB", global.visual_format = ARG);

    /*
     * Options starting with ggggggggggggggggggggggggggggggggggggggggggg
     */

    ARGS_FLAG ("g", global.cc_debug = TRUE);
    ARGS_FLAG ("generic", global.cc_tune_generic = TRUE);
    ARGS_FLAG ("gg", global.cc_debug = TRUE; global.cc_debug_extra = TRUE);

    /*
     * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
     */

    ARGS_FLAG ("h", global.print_help_and_exit = TRUE; global.verbose_help = FALSE);
    ARGS_FLAG ("help", global.print_help_and_exit = TRUE; global.verbose_help = TRUE);

    /*
     * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
     */

    ARGS_FLAG ("install", global.install = TRUE);
    ARGS_OPTION ("initmheap", ARG_NUM (global.initial_master_heapsize));
    ARGS_OPTION ("initwheap", ARG_NUM (global.initial_worker_heapsize));
    ARGS_OPTION ("inituheap", ARG_NUM (global.initial_unified_heapsize));
    ARGS_OPTION ("iveo", ARG_NUM (global.iveo));
    ARGS_OPTION ("ive", ARG_NUM (global.ive));
    ARGS_FLAG ("extrema", global.doivext = TRUE);

    ARGS_OPTION_BEGIN ("I")
    {
        FMGRappendPath (PK_imp_path, ARG);
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
        FMGRappendPath (PK_lib_path, ARG);
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

    ARGS_FLAG ("list-targets", global.print_targets_and_exit = TRUE);

    /*
     * Options starting with mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
     */

    ARGS_OPTION_BEGIN ("mt_barrier_type")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("spin", global.mt_barrier_type = 0);
        ARG_CHOICE ("mutex", global.mt_barrier_type = 1);
        ARG_CHOICE ("cond", global.mt_barrier_type = 2);
        ARG_CHOICE ("pthread", global.mt_barrier_type = 3);
        ARG_CHOICE ("futex", global.mt_barrier_type = 4);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("mt_barrier_type");

    /*
     * NB: if you want to add a new strategy, you need to add it to:
     *     types/types.h
     *     global/usage.c
     *     codegen/gen_startup_code.c
     *     libsac/mt/hwloc_data.c
     */
    ARGS_OPTION_BEGIN ("mt_bind")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("off", global.cpubindstrategy = HWLOC_off);
        ARG_CHOICE ("simple", global.cpubindstrategy = HWLOC_simple);
        ARG_CHOICE ("env", global.cpubindstrategy = HWLOC_env);
        ARG_CHOICE ("numa", global.cpubindstrategy = HWLOC_numa);
        ARG_CHOICE ("socket", global.cpubindstrategy = HWLOC_socket);
        ARG_CHOICE ("env_pus_string", global.cpubindstrategy = HWLOC_envString);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("mt_bind");

    ARGS_OPTION_BEGIN ("mt_smart_mode")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("off", global.mt_smart_mode = 0);
        ARG_CHOICE ("train", global.mt_smart_mode = 1);
        ARG_CHOICE ("on", global.mt_smart_mode = 2);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("mt_smart_mode");

    ARGS_OPTION ("mt_smart_filename", global.mt_smart_filename = STRcpy (ARG));

    ARGS_OPTION ("mt_smart_arch", global.mt_smart_arch = STRcpy (ARG));

    ARGS_OPTION ("mt_smart_period", ARG_NUM (global.mt_smart_period));

    ARGS_OPTION ("mt_smart_gradient", ARG_RANGE (global.mt_smart_gradient, 0, 90));

    ARGS_OPTION ("maxnewgens", ARG_NUM (global.max_newgens));

    ARGS_OPTION ("maxoptcyc", ARG_NUM (global.max_optcycles));

    ARGS_OPTION ("maxrecinl", CTIabort (EMPTY_LOC, "Option -maxrecinl de-activated temporarily");
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

    /* mutc options */
    ARGS_FLAG ("mutc_benchmark", global.mutc_benchmark = TRUE);
    ARGS_FLAG ("mutc_disable_thread_mem", global.mutc_disable_thread_mem = TRUE);

    ARGS_OPTION ("mutc_distribute_arg", ARG_NUM (global.mutc_distribution_mode_arg));

    ARGS_OPTION ("mutc_force_block_size", ARG_NUM (global.mutc_force_block_size));

    ARGS_OPTION ("mutc_force_spawn_flags", global.mutc_force_spawn_flags = STRcpy (ARG));

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
    ARGS_FLAG ("mutc_suballoc_desc_one_level_up",
               global.mutc_suballoc_desc_one_level_up = TRUE);
    ARGS_OPTION ("mutc_rc_places", ARG_NUM (global.mutc_rc_places));
    ARGS_FLAG ("mutc_rc_indirect", global.mutc_rc_indirect = TRUE);
    ARGS_FLAG ("mutc_seq_data_parallel", global.mutc_seq_data_parallel = TRUE);

    /*
     * Options starting with nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
     */

    /* Avoid generating Tree file for modules; do only Mod.  */
    ARGS_FLAG ("notree", global.notree = TRUE;);

    ARGS_FLAG ("nofoldfusion", global.no_fold_fusion = TRUE);

    ARGS_FLAG ("nofoldparallel", global.no_fold_parallel = TRUE);

    ARGS_FLAG ("noprelude", global.loadprelude = FALSE);
    ARGS_FLAG ("nosaclibs", global.loadsaclibs = FALSE);

    ARGS_OPTION_BEGIN ("numthreads")
    {
        ARG_RANGE (store_num_threads, 1, global.max_threads);
        global.num_threads = store_num_threads;
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

    ARGS_FLAG ("on_demand_lib", global.on_demand_lib = TRUE);

    /*
     * -o / -olib The option is only stored in outfilename,
     * the correct settings of the global variables
     * outfilename and targetdir will be done
     * in FMGRsetFileNames() in filemgr.c. This cannot be done here
     * because you have to know the kind of file (program
     * or module/class implementation).
     */
    ARGS_OPTION_BEGIN ("o")
    {
        if (global.outfilename != NULL && global.runtime != TRUE) {
            CTIabort (EMPTY_LOC, "-o cannot be specified multiple times.");
        }
        if (global.outfilename == NULL) {
            global.outfilename = STRcpy (ARG);
        }
    }
    ARGS_OPTION_END ("o");

    ARGS_OPTION_BEGIN ("olib")
    {
        if (global.target_modlibdir != NULL)
            CTIabort (EMPTY_LOC, "-olib cannot be specified multiple times.");
        global.target_modlibdir = STRcpy (ARG);
    }
    ARGS_OPTION_END ("olib");

    ARGS_OPTION ("O", ARG_RANGE (global.cc_optimize, 0, 3));

    /*
     * Options starting with ppppppppppppppppppppppppppppppppppppppppppp
     */

    ARGS_FLAG ("plibsac2c", global.print_libsac2c = TRUE);

    ARGS_OPTION ("printstart", PHOinterpretStartPhase (ARG))

    ARGS_OPTION ("printstop", PHOinterpretStopPhase (ARG))

    ARGS_OPTION_BEGIN ("printfunsets")
    {
        global.doprintfunsets = FALSE;

        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.printfunsets = global.printfunsets_all;
                      global.doprintfunsets = TRUE);

#define PRINTFUNSETS(flag, char, default)                                                \
    ARG_FLAGMASK (char, global.printfunsets.flag = TRUE;);
#include "flags.mac"

        ARG_FLAGMASK_END ();

        global.dovisualizefunsets = FALSE;
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', global.visualizefunsets = global.visualizefunsets_all;
                      global.dovisualizefunsets = TRUE);

#define VISUALIZEFUNSETS(flag, char, default)                                            \
    ARG_FLAGMASK (char, global.visualizefunsets.flag = TRUE;);
#include "flags.mac"

        ARG_FLAGMASK_END ();
    }
    ARGS_OPTION_END ("printfunsets");

    /*selective printing implementation*/
    ARGS_OPTION ("printfun", PHOinterpretBreakFunName (ARG));

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

    /* -- Runtime Specialization -- */

    /* Specialization has to be turned of otherwise the compiler will pick the
     * most generic function instead of the wrapper entry function.
     */
    ARGS_OPTION_BEGIN ("rtspec_mode")
    {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("simple", if (global.config.rtspec == FALSE) {
                                  CTIwarn (EMPTY_LOC,
                                           "RTSPEC in sac2c is not set; "
                                           "`-rtspec_mode simple` will be ignored");
                              } else {
                                  global.rtspec_mode = RTSPEC_MODE_SIMPLE;
                              });

#if ENABLE_HASH
        ARG_CHOICE ("hash", if (global.config.rtspec == FALSE) {
                                  CTIwarn (EMPTY_LOC,
                                           "RTSPEC in sac2c is not set; "
                                           "`-rtspec_mode hash` will be ignored");
                              } else {
                                  global.rtspec_mode = RTSPEC_MODE_HASH;
                              });
#else
        ARG_CHOICE ("hash",
                    CTIabort (EMPTY_LOC, "hash support not available in the current build."));
#endif /* ENABLE_HASH */

#if ENABLE_UUID
        ARG_CHOICE ("uuid", if (global.config.rtspec == FALSE) {
                                CTIwarn (EMPTY_LOC,
                                         "RTSPEC in sac2c is not set; "
                                         "`-rtspec_mode uuid` will be ignored");
                            } else {
                                global.rtspec_mode = RTSPEC_MODE_UUID;
                            });
#else
        ARG_CHOICE ("uuid",
                    CTIabort (EMPTY_LOC, "uuid support not available in the current build."));
#endif /* ENABLE_UUID */

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("rtspec_mode");

    ARGS_FLAG ("runtime", global.runtime = TRUE);

    ARGS_OPTION ("rt_old_mod", global.rt_old_module = ARG);

    ARGS_OPTION ("rt_new_mod", global.rt_new_module = ARG);

    ARGS_OPTION ("rttypeinfo", global.rt_type_info = ARG);

    ARGS_OPTION ("rtshapeinfo", global.rt_shape_info = ARG);

    ARGS_OPTION ("rtfunname", global.rt_fun_name = ARG);

    ARGS_OPTION ("rtnewname", global.rt_new_name = ARG);

    /* -- Runtime Specialization -- */

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

    ARGS_OPTION_BEGIN ("specmode")
    {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("aks", global.spec_mode = SS_aks);
        ARG_CHOICE ("akd", global.spec_mode = SS_akd);
        ARG_CHOICE ("aud", global.spec_mode = SS_aud);

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("specmode");

    ARGS_FLAG ("stop", kill (getpid (), SIGSTOP));

    /*
     * Options starting with ttttttttttttttttttttttttttttttttttttttttttt
     */

    ARGS_OPTION ("target", global.target_name = ARG);

#ifdef HAVE_GETTIME
    ARGS_OPTION ("timefreq", global.timefreq = atoi (ARG));
#else
    ARGS_OPTION ("timefreq",
                 CTIerror (EMPTY_LOC, "Timing is not available since you don't have clock_gettime"));
#endif

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

    ARGS_OPTION_BEGIN ("T")
    {
        FMGRappendPath (PK_tree_path, ARG);
    }
    ARGS_OPTION_END ("T");

    /*
     * Options starting with uuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuuu
     */

    ARGS_FLAG ("utrace", global.dousertrace = TRUE);

    /*
     * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
     */

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 5));

    ARGS_FLAG ("V", USGprintVersion (); CTIexit (EXIT_SUCCESS));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); CTIexit (EXIT_SUCCESS));

    /*
     * Options starting with xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
     */

    ARGS_FIXED ("Xp", SBUFprintf (cppflags_buf, " %s", ARG));
    ARGS_FIXED ("Xc", SBUFprintf (cflags_buf, " %s", ARG));
    ARGS_FIXED ("Xl", SBUFprintf (ldflags_buf, " %s", ARG));
    ARGS_FIXED ("Xtc", SBUFprintf (tree_cflags_buf, " %s", ARG));
    ARGS_FIXED ("Xtl", SBUFprintf (tree_ldflags_buf, " %s", ARG));

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

            global.puresacfilename = FMGRbasename (global.sacfilename);
        } else if (global.do_clink) {
            char *tmp = STRcatn (3, global.sacfilename, " ", ARG);
            MEMfree (global.sacfilename);
            global.sacfilename = tmp;
        } else {
            ARGS_ERROR ("Too many source files specified");
        }
    });

    ARGS_UNKNOWN (ARGS_ERROR ("Invalid command line entry"));

    ARGS_END ();

    global.cppflags = SBUF2strAndFree (&cppflags_buf);
    global.cflags = SBUF2strAndFree (&cflags_buf);
    global.ldflags = SBUF2strAndFree (&ldflags_buf);
    global.tree_cflags = SBUF2strAndFree (&tree_cflags_buf);
    global.tree_ldflags = SBUF2strAndFree (&tree_ldflags_buf);
    global.command_line = SBUF2strAndFree (&command_line_buf);

    DBUG_RETURN ();
}

static void
AnalyseCommandlineSac4c (int argc, char *argv[])
{
    int store_num_threads = 0;
    str_buf *cppflags_buf = SBUFcreate (1);
    str_buf *cflags_buf = SBUFcreate (1);
    str_buf *ldflags_buf = SBUFcreate (1);
    str_buf *tree_cflags_buf = SBUFcreate (1);
    str_buf *tree_ldflags_buf = SBUFcreate (1);

    DBUG_ENTER ();

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

    ARGS_OPTION ("ccflag",
                 CTIwarn (EMPTY_LOC,
                          "Option -ccflag is deprecated, consider using -Xc instead.");
                 SBUFprintf (cflags_buf, " %s", ARG));

    ARGS_FLAG ("copyright", USGprintCopyright (); CTIexit (EXIT_SUCCESS));

    ARGS_OPTION ("cti-message-length", ARG_RANGE (global.cti_message_length,0,255));

    ARGS_FLAG ("cti-no-color", global.cti_no_color = TRUE);

    ARGS_FLAG ("cti-single-line", global.cti_single_line= TRUE);

    ARGS_OPTION ("cti-primary-header-format", global.cti_primary_header_format = STRcpy (ARG));

    ARGS_OPTION ("cti-continuation-header-format", global.cti_continuation_header_format = STRcpy (ARG));

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
        FMGRappendPath (PK_extlib_path, ARG);
    }
    ARGS_OPTION_END ("E");

    /*
     * Options starting with fffffffffffffffffffffffffffffffffffffffffff
     */

    ARGS_FLAG ("fortran", global.genfortran = TRUE);

    /*
     * Options starting with ggggggggggggggggggggggggggggggggggggggggggg
     */

    ARGS_FLAG ("g", global.cc_debug = TRUE);

    /*
     * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
     */

    ARGS_FLAG ("h", USGprintUsage (); CTIexit (EXIT_SUCCESS));
    ARGS_FLAG ("help", USGprintUsage (); CTIexit (EXIT_SUCCESS));

    /*
     * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
     */

    ARGS_OPTION ("incdir", global.inc_dirname = STRcpy (ARG););

    ARGS_OPTION_BEGIN ("I")
    {
        FMGRappendPath (PK_imp_path, ARG);
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
    FMGRappendPath (PK_extlib_path, ARG);
    ARGS_OPTION_END ("libdir");

    ARGS_OPTION_BEGIN ("L")
    {
        FMGRappendPath (PK_lib_path, ARG);
    }
    ARGS_OPTION_END ("L");

    /*
     * Options starting with mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm
     */

    /*
     * NB: if you want to add a new strategy, you need to add it to:
     *     types/types.h
     *     global/usage.c
     *     codegen/gen_startup_code.c
     *     libsac/mt/hwloc_data.c
     */
    ARGS_OPTION_BEGIN ("mt_bind")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("off", global.cpubindstrategy = HWLOC_off);
        ARG_CHOICE ("simple", global.cpubindstrategy = HWLOC_simple);
        ARG_CHOICE ("env", global.cpubindstrategy = HWLOC_env);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("mt_bind");

    /*
     * Options starting with nnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnnn
     */

    ARGS_OPTION_BEGIN ("numthreads")
    {
        ARG_RANGE (store_num_threads, 1, global.max_threads);
        global.num_threads = store_num_threads;
    }
    ARGS_OPTION_END ("numthreads");

    /* only the "-nophm" optimization option is supported in sac4c,
     * hence parse it manually here */
    ARGS_FLAG ("nophm", global.optimize.dophm = FALSE;);

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

    ARGS_OPTION_BEGIN ("target")
    {
        if (global.target_name == NULL)
            CTIabort (EMPTY_LOC, "-target is missing an argument.");
        global.target_name = ARG;
    }
    ARGS_OPTION_END ("target");

    ARGS_OPTION_BEGIN ("t")
    {
        if (global.target_name == NULL)
            CTIabort (EMPTY_LOC, "-target is missing an argument.");
        global.target_name = ARG;
    }
    ARGS_OPTION_END ("t");

    ARGS_OPTION_BEGIN ("T")
    {
        FMGRappendPath (PK_tree_path, ARG);
    }
    ARGS_OPTION_END ("T");

    /*
     * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
     */

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 5));

    ARGS_FLAG ("V", USGprintVersion (); CTIexit (EXIT_SUCCESS));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); CTIexit (EXIT_SUCCESS));

    /*
     * Options starting with xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
     */

    ARGS_FIXED ("Xp", SBUFprintf (cppflags_buf, " %s", ARG));
    ARGS_FIXED ("Xc", SBUFprintf (cflags_buf, " %s", ARG));
    ARGS_FIXED ("Xl", SBUFprintf (ldflags_buf, " %s", ARG));
    ARGS_FIXED ("Xtc", SBUFprintf (tree_cflags_buf, " %s", ARG));
    ARGS_FIXED ("Xtl", SBUFprintf (tree_ldflags_buf, " %s", ARG));

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

    global.cppflags = SBUF2strAndFree (&cppflags_buf);
    global.cflags = SBUF2strAndFree (&cflags_buf);
    global.ldflags = SBUF2strAndFree (&ldflags_buf);
    global.tree_cflags = SBUF2strAndFree (&tree_cflags_buf);
    global.tree_ldflags = SBUF2strAndFree (&tree_ldflags_buf);

    /*
     * set defaults not altered by arguments
     */
    if (global.outfilename == NULL) {
        global.outfilename = STRcpy ("cwrapper");
    }
    global.filetype = FT_cmod;
    if (global.printldflags || global.printccflags) {
        global.verbose_level = 0;
    }

    if (global.exported_modules == NULL) {
        CTIabort (EMPTY_LOC, "No modules given as argument. See sac4c -h for details.");
    }

    if (global.printldflags && global.printccflags) {
        CTIabort (EMPTY_LOC, "-ldflags and -ccflags cannot be used simultaneously.");
    }

    if (global.optimize.dophm) {
        CTIwarn (EMPTY_LOC,
          "Private Heap Manager in sac4c is experimental! Use -nophm to disable it.");
    }

    DBUG_RETURN ();
}

static void
AnalyseCommandlineSac2tex (int argc, char *argv[])
{
    DBUG_ENTER ();

    ARGS_BEGIN (argc, argv);

    /*
     * Options starting with bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb
     */

    ARGS_OPTION ("b", PHOinterpretBreakOption (ARG))

    /*
     * Options starting with ccccccccccccccccccccccccccccccccccccccccccc
     */

    ARGS_FLAG ("copyright", USGprintCopyright (); CTIexit (EXIT_SUCCESS));

    ARGS_OPTION ("cti-message-length", ARG_RANGE (global.cti_message_length,0,255));

    ARGS_FLAG ("cti-no-color", global.cti_no_color = TRUE);

    ARGS_FLAG ("cti-single-line", global.cti_single_line= TRUE);

    ARGS_OPTION ("cti-primary-header-format", global.cti_primary_header_format = STRcpy (ARG));

    ARGS_OPTION ("cti-continuation-header-format", global.cti_continuation_header_format = STRcpy (ARG));

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

    /*
     * Options starting with hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh
     */

    ARGS_FLAG ("h", USGprintUsage (); CTIexit (EXIT_SUCCESS));
    ARGS_FLAG ("help", USGprintUsage (); CTIexit (EXIT_SUCCESS));

    /*
     * Options starting with iiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiiii
     */

    ARGS_OPTION ("incdir", global.inc_dirname = STRcpy (ARG););

    ARGS_OPTION_BEGIN ("I")
    {
        FMGRappendPath (PK_imp_path, ARG);
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
        FMGRappendPath (PK_lib_path, ARG);
    }
    ARGS_OPTION_END ("L");

    /*
     * Options starting with ooooooooooooooooooooooooooooooooooooooooooo
     */

    ARGS_OPTION ("o", global.outfilename = STRcpy (ARG));

    /*
     * Options starting with ttttttttttttttttttttttttttttttttttttttttttt
     */

    ARGS_OPTION_BEGIN ("T")
    {
        FMGRappendPath (PK_tree_path, ARG);
    }
    ARGS_OPTION_END ("T");

    /*
     * Options starting with vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
     */

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 5));

    ARGS_FLAG ("V", USGprintVersion (); CTIexit (EXIT_SUCCESS));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); CTIexit (EXIT_SUCCESS));

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

            global.puresacfilename = FMGRbasename (global.sacfilename);
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

    DBUG_RETURN ();
}

void
OPTanalyseCommandline (int argc, char *argv[])
{
    DBUG_ENTER ();

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
    CTFinitialize (); // Reinitialize formatting settings based on command line arguments

    DBUG_RETURN ();
}

#undef DBUG_PREFIX
