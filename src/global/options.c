/*
 *
 * $Log$
 * Revision 3.87  2005/04/15 13:23:04  ktr
 * added switch -d nolacinline
 *
 * Revision 3.86  2005/04/12 15:15:36  sah
 * cleaned up module system compiler args
 * and sac2crc parameters
 *
 * Revision 3.85  2005/04/07 16:16:27  cg
 * Textual arguments to command line options are now parsed case insensitive.
 * This allows us to write -noopt or -noOPT without specifying two different
 * options in options.c
 *
 * Revision 3.84  2005/03/10 09:41:09  cg
 * Removed options to control application of fun2lac and lac2fun
 * conversion.
 *
 * Revision 3.83  2005/02/18 15:52:38  sbs
 * strgcpy changed into ILIBstringCopy
 * i.e., no more SEGFAULT on -o option
 *
 * Revision 3.82  2005/02/11 14:40:30  jhb
 * added options for treecheck
 *
 * Revision 3.81  2005/01/29 21:40:38  mwe
 * sigspec compiler switch added
 *
 * Revision 3.80  2005/01/07 19:54:13  cg
 * Converted compile time output from Error.h to ctinfo.c
 *
 * Revision 3.79  2004/11/25 17:53:48  cg
 * SacDevCamp 04
 *
 * Revision 3.78  2004/11/24 16:02:03  cg
 * file streamlined.
 *
 * Revision 3.77  2004/10/28 16:58:43  khf
 * support for max_newgens and no_fold_fusion added
 *
 * Revision 3.76  2004/10/23 12:00:31  ktr
 * Added switches for static reuse / static free.
 *
 * Revision 3.75  2004/09/28 16:32:19  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.74  2004/09/28 14:07:30  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.73  2004/09/24 12:53:00  ktr
 * MT&EMM reactivated (Once more, fingers crossing).
 *
 * Revision 3.72  2004/09/24 11:25:20  ktr
 * Combination of EMM/MT deactivated again (due to to new problems).
 *
 * Revision 3.71  2004/09/22 18:41:24  ktr
 * EMM is now activated for MT, too. (crossing fingers)
 *
 * Revision 3.70  2004/08/30 13:02:12  skt
 * enabled emm for MT_mtstblock
 *
 * Revision 3.69  2004/08/26 14:02:36  cg
 * Enabled emm (new refcounting) by default.
 *
 * Revision 3.68  2004/08/12 12:04:46  ktr
 * replaced flag reuse with flag noreuse.
 *
 * Revision 3.67  2004/08/10 16:13:42  ktr
 * reuse inference in EMM can now be activated using -reuse.
 *
 * Revision 3.66  2004/08/04 12:04:20  ktr
 * substituted eacc by emm
 *
 * Revision 3.65  2004/07/23 15:53:50  ktr
 * - removed OPT_BLIR
 * - removed -ktr
 * - added -emm -do/noeacc
 *
 * Revision 3.64  2004/07/19 13:08:04  ktr
 * adjusted break specifiers arg range to 1..25
 *
 * Revision 3.63  2004/07/15 13:36:59  ktr
 * show_refcount is only true during PH_oldrefcount
 *
 * Revision 3.62  2004/07/14 23:23:37  sah
 * removed all old ssa optimizations and the use_ssaform flag
 *
 * Revision 3.61  2004/04/30 13:21:03  ktr
 * Nothing really changed.
 *
 * Revision 3.60  2004/03/26 14:36:23  khf
 * support for wlpg added
 *
 * Revision 3.59  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.58  2004/03/02 16:49:15  mwe
 * support for cvp added
 *
 * Revision 3.57  2004/02/25 13:02:15  khf
 * added option -khf
 *
 * Revision 3.56  2004/02/05 10:37:14  cg
 * Re-factorized handling of different modes in multithreaded code
 * generation:
 * - Added enumeration type for representation of modes
 * - Renamed mode identifiers to more descriptive names.
 *
 * Revision 3.55  2003/12/10 17:33:16  khf
 * OPT_WLFS added for with-loop fusion
 *
 * Revision 3.54  2003/12/10 16:07:14  skt
 * changed compiler flag from -mtn to -mtmode and expanded mt-versions by one
 *
 * Revision 3.53  2003/10/14 12:21:51  cg
 * -mt for new backend activated (for testing only!!)
 *
 * Revision 3.52  2003/10/09 16:52:12  dkrHH
 * DAO for new backend activated
 *
 * Revision 3.51  2003/09/30 21:56:49  dkrHH
 * no changes done
 *
 * Revision 3.50  2003/09/17 18:54:32  dkr
 * RCAO renamed into DAO for new backend
 *
 * Revision 3.48  2003/09/16 16:10:49  sbs
 * specmode option added.
 *
 * Revision 3.47  2003/08/21 17:01:31  cg
 * Detection of tagged arrays backend now uses the dedicated
 * compiler option -has_tagged_backend.
 *
 * Revision 3.46  2003/08/16 08:38:03  ktr
 * SelectionPropagation added. Must currently be activated with -dosp.
 *
 * Revision 3.45  2003/08/05 11:36:19  ktr
 * Support for maxwls added.
 *
 * Revision 3.44  2003/08/04 18:07:31  dkr
 * -mt reactivated for new backend
 *
 * Revision 3.43  2003/07/28 15:35:06  cg
 * Added short version identification option (-V).
 * Full version information is now available with -VV
 * (verbose version).
 *
 * Revision 3.42  2003/05/21 16:38:02  ktr
 * added option -ktr
 *
 * Revision 3.41  2003/04/15 21:16:15  dkr
 * -DTAGGED_ARRAYS created only once
 *
 * Revision 3.40  2003/04/15 14:16:05  dkr
 * -DTAGGED_ARRAYS added for new backend
 *
 * Revision 3.39  2003/03/24 16:36:52  sbs
 * cppI added
 *
 * Revision 3.38  2003/03/20 14:02:29  sbs
 * config.h included; DISABLE_MT and DISABLE_PHM used.
 *
 * Revision 3.37  2003/03/13 17:18:26  dkr
 * -minarrayrep activated for new backend only
 *
 * Revision 3.36  2003/03/13 17:02:21  dkr
 * flags ordered correctly now
 *
 * Revision 3.35  2003/03/13 15:48:57  dkr
 * option -minarrayrep added
 *
 * Revision 3.34  2003/03/09 19:15:43  dkr
 * TRACE_AA added
 *
 * Revision 3.33  2003/03/09 17:13:54  ktr
 * added basic support for BLIR.
 *
 *  [...]
 *
 * Revision 2.1  1999/05/12 14:27:24  cg
 * initial revision
 *
 */

/*****************************************************************************
 *
 * file:    options.c
 *
 * prefix:
 *
 * description:
 *  This file provides means for the analysis of sac2c command line arguments.
 *  It uses the set macro definitions from main_args.h
 *
 *****************************************************************************/

#include <stdlib.h>

#include "config.h"
#include "dbug.h"

#define ARGS_ERROR(msg)                                                                  \
    {                                                                                    \
        CTIerror ("%s: %s %s %s", msg, ARGS_argv[0], STR_OR_EMPTY (OPT),                 \
                  STR_OR_EMPTY (ARG));                                                   \
    }

#include "main_args.h"

#include "usage.h"
#include "filemgr.h"
#include "globals.h"
#include "internal_lib.h"
#include "ctinfo.h"
#include "libstat.h"

void
OPTcheckSpecialOptions (int argc, char *argv[])
{
    DBUG_ENTER ("OPTcheckSpecialOptions");

    ARGS_BEGIN (argc, argv);

    ARGS_FLAG ("copyright", USGprintCopyright (); exit (0));

    ARGS_FLAG ("h", USGprintUsage (); exit (0));
    ARGS_FLAG ("help", USGprintUsage (); exit (0));

    ARGS_OPTION ("libstat", LIBSprintLibStat (ARG));

    ARGS_OPTION ("v", ARG_RANGE (global.verbose_level, 0, 3));

    ARGS_FLAG ("V", USGprintVersion (); exit (0));

    ARGS_FLAG ("VV", USGprintVersionVerbose (); exit (0));

    ARGS_END ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CheckOptionConsistency()
 *
 * description:
 *   This function is called from main() right after command line arguments
 *   have been analysed. Errors and warnings are produced whenever the user
 *   has selected an incompatible combination of options.
 *
 ******************************************************************************/

static void
CheckOptionConsistency ()
{
    DBUG_ENTER ("CheckOptionConsistency");

    if (global.runtimecheck.boundary && global.optimize.doap) {
        global.optimize.doap = FALSE;
        CTIwarn ("Boundary check (-check b) and array padding (AP) may not be used"
                 " simultaneously.\n"
                 "Array padding disabled");
    }

#ifdef DISABLE_MT
    if (global.mtmode != MT_none) {
        global.mtmode = MT_none;
        global.num_threads = 1;
        CTIwarn ("Code generation for multi-threaded program execution not"
                 " yet available for " ARCH " running " OS ".\n"
                 "Code for sequential execution generated instead");
    }
#endif

#ifdef DISABLE_PHM
    if (global.optimize.dophm) {
        CTIwarn ("Private heap management is not yet available for " ARCH " running " OS
                 ".\n"
                 "Conventional heap management is used instead");
        global.optimize.dophm = FALSE;
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
        CTIwarn ("Diagnostic heap management is only available in "
                 "conjunction with private heap management.\n"
                 "Diagnostic disabled");
        global.runtimecheck.heap = FALSE;
    }

    /*
     * commandline switch for library generation not used,
     * set it to default and generate a standard SAC Library
     */
    if (!global.genlib.c && !global.genlib.sac) {
        global.genlib.sac = TRUE;
    }

    if (global.genlib.c && (global.mtmode != MT_none)) {
        CTIwarn ("Multithreading is not yet available when compiling for "
                 "a C-library.\n"
                 "Generation of C-library disabled");
        global.genlib.c = FALSE;
    }

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *OPTanalyseCommandline( node *syntax_tree)
 *
 * description:
 *   This function analyses the commandline options given to sac2c.
 *   Usually selections made are stored in global variables for later
 *   reference.
 *
 *   The non-obvious signature is to obey the compiler subphase standard.
 *
 ******************************************************************************/

node *
OPTanalyseCommandline (node *syntax_tree)
{
    int store_num_threads = 0;
    mtmode_t store_mtmode = MT_none;

    DBUG_ENTER ("OPTanalyseCommandline");

    ARGS_BEGIN (global.argc, global.argv);

    ARGS_FLAG ("apdiag", global.apdiag = TRUE);

    ARGS_OPTION ("aplimit", { ARG_RANGE (global.padding_overhead_limit, 0, 100); });

    ARGS_OPTION ("apdiagsize", ARG_NUM (global.apdiag_limit));

    ARGS_OPTION_BEGIN ("b")
    {
        char *break_arg = ILIBstringCopy (ARG);

        ARG = strtok (ARG, ":");
        ARG_RANGE (global.break_after, 1, 25);
        switch (global.break_after) {
        case PH_sacopt:
            global.show_idx = TRUE;
            break;
        default:
            break;
        }

        ARG = strtok (NULL, ":");
        if (ARG != NULL) {
            if (0 == strncmp (ARG, "cyc", 3)) {
                if (strspn (ARG + 3, "0123456789") != strlen (ARG + 3)) {
                    ARG = break_arg;
                    ARGS_ERROR ("Break cycle specifier not a number");
                } else {
                    global.break_cycle_specifier = atoi (ARG + 3);

                    ARG = strtok (NULL, ":");

                    if (ARG == NULL) {
                        ARG = break_arg;
                        ARGS_ERROR ("Break specifier missing");
                    } else {
                        global.break_specifier = ILIBstringCopy (ARG);
                    }
                }
            } else {
                global.break_cycle_specifier = 0;

                if (ARG == NULL) {
                    ARG = break_arg;
                    ARGS_ERROR ("Break specifier missing");
                } else {
                    global.break_specifier = ILIBstringCopy (ARG);
                }
            }
        }
        break_arg = ILIBfree (break_arg);
    }
    ARGS_OPTION_END ("b");

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

    ARGS_OPTION ("cppI", global.cpp_incs[global.num_cpp_incs++] = ILIBstringCopy (ARG));

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

    ARGS_FLAG ("c", global.break_after = PH_genccode);

    ARGS_FLAG ("ds", global.dynamic_shapes = TRUE);

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

    ARGS_OPTION_BEGIN ("d")
    {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("treecheck", global.treecheck = TRUE);
        ARG_CHOICE ("efence", global.use_efence = TRUE);
        ARG_CHOICE ("nocleanup", global.cleanup = FALSE);
        ARG_CHOICE ("nolacinline", global.lacinline = FALSE);
        ARG_CHOICE ("syscall", global.show_syscall = TRUE);
        ARG_CHOICE ("cccall", global.gen_cccall = TRUE; global.cleanup = FALSE);
        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("d");

    ARGS_OPTION ("D", global.cpp_vars[global.num_cpp_vars++] = ILIBstringCopy (ARG));

    ARGS_FLAG ("enforceIEEE", global.enforce_ieee = TRUE);

    ARGS_OPTION_BEGIN ("genlib")
    {
        ARG_CHOICE_BEGIN ();

#define GENLIB(flag, str, default) ARG_CHOICE (str, global.genlib.flag = default);
#include "flags.mac"

        ARG_CHOICE_END ();
    }
    ARGS_OPTION_END ("genlib");

    ARGS_FLAG ("g", global.cc_debug = TRUE);

    ARGS_OPTION ("initmheap", ARG_NUM (global.initial_master_heapsize));
    ARGS_OPTION ("initwheap", ARG_NUM (global.initial_worker_heapsize));
    ARGS_OPTION ("inituheap", ARG_NUM (global.initial_unified_heapsize));

    ARGS_OPTION_BEGIN ("L")
    {
        FMGRappendPath (PK_mod_path, FMGRabsolutePathname (ARG));
        FMGRappendPath (PK_systemlib_path, FMGRabsolutePathname (ARG));
    }
    ARGS_OPTION_END ("L");

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

    ARGS_FLAG ("nofoldfusion", global.no_fold_fusion = TRUE);

    ARGS_OPTION ("maxoptcyc", ARG_NUM (global.max_optcycles));

    ARGS_OPTION ("maxoptvar", ARG_NUM (global.optvar));

    ARGS_OPTION ("maxinl", ARG_NUM (global.inlnum));

    ARGS_OPTION ("maxlur", ARG_NUM (global.unrnum));

    ARGS_OPTION ("maxwlur", ARG_NUM (global.wlunrnum));

    ARGS_OPTION ("maxspec", ARG_NUM (global.maxspec));

    ARGS_OPTION ("maxthreads", ARG_NUM (global.max_threads));

    ARGS_OPTION ("maxsync", ARG_RANGE (global.max_sync_fold, -1, 64));

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

    ARGS_OPTION ("o", global.outfilename = ILIBstringCopy (ARG));
    /*
     * The option is only stored in outfilename,
     * the correct settings of the global variables
     * outfilename, cfilename, and targetdir will be done
     * in SetFileNames() in scnprs.c. This cannot be done here
     * because you have to know the kind of file (program
     * or module/class implementation).
     */

    ARGS_OPTION ("O", ARG_RANGE (global.cc_optimize, 0, 3));

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

    ARGS_OPTION ("target", global.target_name = ARG);

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

    ARGS_OPTION ("v", );
    /*
     * The -v option has allready been processed by OPTcheckSpecialOptions().
     * However, it must be repeated here with empty action part to avoid an
     * illegal command line option error.
     */

    ARGS_FLAG ("wls_aggressive", global.wls_aggressive = TRUE);

    ARGS_OPTION_BEGIN ("#")
    {
        if (NULL == strchr (ARG, '/')) {
            global.my_dbug_str = ILIBstringCopy (ARG);
            global.my_dbug = 1;
            global.my_dbug_from = PH_initial;
            global.my_dbug_to = PH_final;
        } else {
            char *s;
            global.my_dbug_from = (compiler_phase_t)strtol (ARG, &s, 10);
            if (*s == '/') {
                s++;
            } else {
                ARGS_ERROR ("Invalid dbug phase specification");
            }

            global.my_dbug_to = (compiler_phase_t)strtol (s, &s, 10);
            if (*s == '/') {
                s++;
            } else {
                ARGS_ERROR ("Invalid dbug phase specification");
            }

            global.my_dbug_str = ILIBstringCopy (s);
            global.my_dbug = 1;
        }
    }
    ARGS_OPTION_END ("#");

    ARGS_ARGUMENT ({
        if (global.sacfilename == NULL) {
            global.sacfilename = ILIBstringCopy (ARG);

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

    CheckOptionConsistency ();

    DBUG_RETURN (syntax_tree);
}
