/*
 *
 * $Log$
 * Revision 2.18  1999/07/20 08:00:44  cg
 * Bug in consistency check fixed: Combination of -mt and -cs
 * now checked correctly.
 *
 * Revision 2.17  1999/07/20 07:53:42  cg
 * Added global variable malloc_align_step.
 *
 * Revision 2.16  1999/07/16 17:04:12  jhs
 * Fixed Bug in CheckOptionConsistency.
 *
 * Revision 2.15  1999/07/09 12:45:32  cg
 * Basic prerequisites for diagnostic heap management introduced.
 *
 * Revision 2.14  1999/07/09 11:52:18  cg
 * Added consistency check for command line options.
 *
 * Revision 2.13  1999/07/09 07:31:24  cg
 * SAC heap manager integrated into sac2c.
 *
 * Revision 2.12  1999/06/28 09:53:32  cg
 * Handling of options -noLUR, -noWLUR, and -noLUS corrected.
 *
 * Revision 2.11  1999/06/11 12:54:46  cg
 * Bug fixed in option -cshost.
 * Added options -csfile and csdir.
 *
 * Revision 2.10  1999/06/10 09:49:03  cg
 * Bug fixed in implementation of option -cshost.
 *
 * Revision 2.9  1999/06/09 08:41:02  rob
 * Introduce support for dynamic shape arrays option "ds".
 *
 * Revision 2.8  1999/06/04 14:33:46  cg
 * Added new option -cshost.
 * Added missing options -noMTO and -noSBE
 *
 * Revision 2.7  1999/06/02 15:39:39  cg
 * Bug fixed in break option: show_idx and show_refcnt are now triggered correctly.
 *
 * Revision 2.6  1999/05/26 14:32:23  jhs
 * Added options MTO and SBE for multi-thread optimsation and
 * synchronisation barrier elimination, both options are by
 * default disabled.
 *
 * Revision 2.5  1999/05/20 14:06:40  cg
 * bug fixed in -check option
 * ccachesim bit mask macros reorganized.
 *
 * Revision 2.4  1999/05/18 13:43:16  cg
 * bug fixed in analysing multi-threaded options.
 *
 * Revision 2.3  1999/05/18 12:52:48  cg
 * File converted to consistently prefixed command line analysis macros.
 * Option -minae renamed to -maxae.
 *
 * Revision 2.2  1999/05/18 08:42:32  cg
 * bug fixed in -o option
 *
 * Revision 2.1  1999/05/12 14:27:24  cg
 * initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:    options.c
 *
 * prefix:
 *
 * description:
 *
 *  This file provides means for the analysis of sac2c command line arguments.
 *  It uses the set macro definitions from main_args.h
 *
 *****************************************************************************/

#include <stdlib.h>

#include "dbug.h"

#define ARGS_ERROR(msg)                                                                  \
    {                                                                                    \
        SYSERROR (("%s: %s %s %s", msg, ARGS_argv[0], NULL == OPT ? "" : OPT,            \
                   NULL == ARG ? "" : ARG));                                             \
    }

#include "main_args.h"

#include "usage.h"
#include "filemgr.h"
#include "globals.h"
#include "internal_lib.h"
#include "free.h"
#include "Error.h"

/******************************************************************************
 *
 * function:
 *   void AnalyseCommandline(int argc, char *argv[])
 *
 * description:
 *
 *   This function analyses the commandline options given to sac2c.
 *   Usually selections made are stored in global variables for later
 *   reference.
 *
 ******************************************************************************/

void
AnalyseCommandline (int argc, char *argv[])
{
    int store_num_threads = 0;

    DBUG_ENTER ("AnalyseCommandline");

    ARGS_BEGIN (argc, argv);

    ARGS_OPTION ("b", {
        char *break_arg = StringCopy (ARG);

        ARG = strtok (ARG, ":");
        ARG_RANGE (break_after, 1, 21);
        switch (break_after) {
        case PH_psiopt:
            show_idx = 1;
            break;
        case PH_refcnt:
            show_refcnt = 1;
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
                    break_cycle_specifier = atoi (ARG + 3);

                    ARG = strtok (NULL, ":");

                    if (ARG == NULL) {
                        ARG = break_arg;
                        ARGS_ERROR ("Break specifier missing");
                    } else {
                        strncpy (break_specifier, ARG, MAX_BREAK_SPECIFIER - 1);
                    }
                }
            } else {
                break_cycle_specifier = 0;

                if (ARG == NULL) {
                    ARG = break_arg;
                    ARGS_ERROR ("Break specifier missing");
                } else {
                    strncpy (break_specifier, ARG, MAX_BREAK_SPECIFIER - 1);
                }
            }
        }
        FREE (break_arg);
    });

    ARGS_OPTION ("check", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', runtimecheck = RUNTIMECHECK_ALL);
        ARG_FLAGMASK ('m', runtimecheck |= RUNTIMECHECK_MALLOC);
        ARG_FLAGMASK ('b', runtimecheck |= RUNTIMECHECK_BOUNDARY);
        ARG_FLAGMASK ('e', runtimecheck |= RUNTIMECHECK_ERRNO);
        ARG_FLAGMASK ('h', runtimecheck |= RUNTIMECHECK_HEAP);
        ARG_FLAGMASK_END ();
    });

    ARGS_FLAG ("copyright", copyright (); exit (0));

    ARGS_OPTION ("csdefaults", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('s', cachesim &= ~CACHESIM_ADVANCED);
        ARG_FLAGMASK ('a', cachesim |= CACHESIM_ADVANCED);
        ARG_FLAGMASK ('g', cachesim &= ~CACHESIM_BLOCK);
        ARG_FLAGMASK ('b', cachesim |= CACHESIM_BLOCK);
        ARG_FLAGMASK ('f', cachesim |= CACHESIM_FILE; cachesim &= ~CACHESIM_PIPE;
                      cachesim &= ~CACHESIM_IMMEDIATE);
        ARG_FLAGMASK ('p', cachesim |= CACHESIM_PIPE; cachesim &= ~CACHESIM_FILE;
                      cachesim &= ~CACHESIM_IMMEDIATE);
        ARG_FLAGMASK ('i', cachesim |= CACHESIM_IMMEDIATE; cachesim &= ~CACHESIM_PIPE;
                      cachesim &= ~CACHESIM_FILE);
        ARG_FLAGMASK_END ();
    });

    ARGS_OPTION ("cshost", strncpy (cachesim_host, ARG, MAX_FILE_NAME - 1));

    ARGS_OPTION ("csfile", strncpy (cachesim_file, ARG, MAX_FILE_NAME - 1));

    ARGS_OPTION ("csdir", strncpy (cachesim_dir, ARG, MAX_FILE_NAME - 1));

    ARGS_FLAG ("cs", cachesim |= CACHESIM_YES);

    ARGS_FLAG ("c", break_after = PH_genccode);

    ARGS_FLAG ("ds", dynamic_shapes = 1);

    ARGS_OPTION ("do", {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("opt", optimize = OPT_ALL);
        ARG_CHOICE ("OPT", optimize = OPT_ALL);

        ARG_CHOICE ("dcr", optimize |= OPT_DCR);
        ARG_CHOICE ("DCR", optimize |= OPT_DCR);

        ARG_CHOICE ("cf", optimize |= OPT_CF);
        ARG_CHOICE ("CF", optimize |= OPT_CF);

        ARG_CHOICE ("lir", optimize |= OPT_LIR);
        ARG_CHOICE ("LIR", optimize |= OPT_LIR);

        ARG_CHOICE ("inl", optimize |= OPT_INL);
        ARG_CHOICE ("INL", optimize |= OPT_INL);

        ARG_CHOICE ("lur", optimize |= OPT_LUR);
        ARG_CHOICE ("LUR", optimize |= OPT_LUR);

        ARG_CHOICE ("wlur", optimize |= OPT_WLUR);
        ARG_CHOICE ("WLUR", optimize |= OPT_WLUR);

        ARG_CHOICE ("lus", optimize |= OPT_LUS);
        ARG_CHOICE ("LUS", optimize |= OPT_LUS);

        ARG_CHOICE ("cse", optimize |= OPT_CSE);
        ARG_CHOICE ("CSE", optimize |= OPT_CSE);

        ARG_CHOICE ("dfr", optimize |= OPT_DFR);
        ARG_CHOICE ("DFR", optimize |= OPT_DFR);

        ARG_CHOICE ("wlt", optimize |= OPT_WLT);
        ARG_CHOICE ("WLT", optimize |= OPT_WLT);

        ARG_CHOICE ("wlf", optimize |= OPT_WLF);
        ARG_CHOICE ("WLF", optimize |= OPT_WLF);

        ARG_CHOICE ("ive", optimize |= OPT_IVE);
        ARG_CHOICE ("IVE", optimize |= OPT_IVE);

        ARG_CHOICE ("ae", optimize |= OPT_AE);
        ARG_CHOICE ("AE", optimize |= OPT_AE);

        ARG_CHOICE ("dlaw", optimize |= OPT_DLAW);
        ARG_CHOICE ("DLAW", optimize |= OPT_DLAW);

        ARG_CHOICE ("rco", optimize |= OPT_RCO);
        ARG_CHOICE ("RCO", optimize |= OPT_RCO);

        ARG_CHOICE ("uip", optimize |= OPT_UIP);
        ARG_CHOICE ("UIP", optimize |= OPT_UIP);

        ARG_CHOICE ("tsi", optimize |= OPT_TSI);
        ARG_CHOICE ("TSI", optimize |= OPT_TSI);

        ARG_CHOICE ("tsp", optimize |= OPT_TSP);
        ARG_CHOICE ("TSP", optimize |= OPT_TSP);

        ARG_CHOICE ("mto", optimize |= OPT_MTO);
        ARG_CHOICE ("MTO", optimize |= OPT_MTO);

        ARG_CHOICE ("sbe", optimize |= OPT_SBE);
        ARG_CHOICE ("SBE", optimize |= OPT_SBE);

        ARG_CHOICE ("phm", optimize |= OPT_PHM);
        ARG_CHOICE ("PHM", optimize |= OPT_PHM);

        ARG_CHOICE_END ();
    });

    ARGS_OPTION ("d", {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("efence", use_efence = 1);
        ARG_CHOICE ("nocleanup", cleanup = 0);
        ARG_CHOICE ("syscall", show_syscall = 1);
        ARG_CHOICE ("cccall", gen_cccall = 1; cleanup = 0);
        ARG_CHOICE_END ();
    });

    ARGS_OPTION ("D", cppvars[num_cpp_vars++] = ARG);

    ARGS_FLAG ("g", cc_debug = 1);

    ARGS_FLAG ("help", usage (); exit (0));
    ARGS_FLAG ("h", usage (); exit (0));

    ARGS_OPTION ("initheap", ARG_NUM (initial_heapsize));

    ARGS_OPTION ("intrinsic", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', intrinsics = INTRINSIC_ALL);
        ARG_FLAGMASK ('+', intrinsics |= INTRINSIC_ADD);
        ARG_FLAGMASK ('-', intrinsics |= INTRINSIC_SUB);
        ARG_FLAGMASK ('x', intrinsics |= INTRINSIC_MUL);
        ARG_FLAGMASK ('/', intrinsics |= INTRINSIC_DIV);
        ARG_FLAGMASK ('t', intrinsics |= INTRINSIC_TAKE);
        ARG_FLAGMASK ('d', intrinsics |= INTRINSIC_DROP);
        ARG_FLAGMASK ('c', intrinsics |= INTRINSIC_CAT);
        ARG_FLAGMASK ('r', intrinsics |= INTRINSIC_ROT);
        ARG_FLAGMASK ('p', intrinsics |= INTRINSIC_PSI);
        ARG_FLAGMASK ('o', intrinsics |= INTRINSIC_TO);
        ARG_FLAGMASK_END ();
    });

    ARGS_OPTION ("I", AppendPath (MODDEC_PATH, AbsolutePathname (ARG)));

    ARGS_FLAG ("libstat", libstat = 1);

    ARGS_OPTION ("l", { ARG_RANGE (linkstyle, 1, 2); });

    ARGS_OPTION ("L", {
        AppendPath (MODIMP_PATH, AbsolutePathname (ARG));
        AppendPath (SYSTEMLIB_PATH, AbsolutePathname (ARG));
    });

    ARGS_OPTION ("maxoptcyc", ARG_NUM (max_optcycles));

    ARGS_OPTION ("maxoptvar", ARG_NUM (optvar));

    ARGS_OPTION ("maxinl", ARG_NUM (inlnum));

    ARGS_OPTION ("maxlur", ARG_NUM (unrnum));

    ARGS_OPTION ("maxwlur", ARG_NUM (wlunrnum));

    ARGS_OPTION ("maxspecialize", ARG_NUM (max_overload));

    ARGS_OPTION ("maxthreads", ARG_NUM (max_threads));

    ARGS_OPTION ("maxsyncfold", ARG_NUM (max_sync_fold));

    ARGS_OPTION ("maxae", ARG_NUM (minarray));

    ARGS_OPTION ("minmtsize", ARG_NUM (min_parallel_size));

    ARGS_FLAG ("mt", {
        gen_mt_code = 1;
        if (store_num_threads > 0) {
            num_threads = store_num_threads;
        } else {
            num_threads = 0;
        }
    });

    ARGS_FLAG ("Mlib", makedeps = 2);

    ARGS_FLAG ("M", makedeps = 1);

    ARGS_OPTION ("numthreads", {
        ARG_RANGE (store_num_threads, 1, max_threads);
        if (gen_mt_code) {
            num_threads = store_num_threads;
        }
    });

    ARGS_OPTION ("no", {
        ARG_CHOICE_BEGIN ();

        ARG_CHOICE ("opt", optimize = OPT_NONE);
        ARG_CHOICE ("OPT", optimize = OPT_NONE);

        ARG_CHOICE ("dcr", optimize &= ~OPT_DCR);
        ARG_CHOICE ("DCR", optimize &= ~OPT_DCR);

        ARG_CHOICE ("cf", optimize &= ~OPT_CF);
        ARG_CHOICE ("CF", optimize &= ~OPT_CF);

        ARG_CHOICE ("lir", optimize &= ~OPT_LIR);
        ARG_CHOICE ("LIR", optimize &= ~OPT_LIR);

        ARG_CHOICE ("inl", optimize &= ~OPT_INL);
        ARG_CHOICE ("INL", optimize &= ~OPT_INL);

        ARG_CHOICE ("lur", optimize &= ~OPT_LUR);
        ARG_CHOICE ("LUR", optimize &= ~OPT_LUR);

        ARG_CHOICE ("wlur", optimize &= ~OPT_WLUR);
        ARG_CHOICE ("WLUR", optimize &= ~OPT_WLUR);

        ARG_CHOICE ("lus", optimize &= ~OPT_LUS);
        ARG_CHOICE ("LUS", optimize &= ~OPT_LUS);

        ARG_CHOICE ("cse", optimize &= ~OPT_CSE);
        ARG_CHOICE ("CSE", optimize &= ~OPT_CSE);

        ARG_CHOICE ("dfr", optimize &= ~OPT_DFR);
        ARG_CHOICE ("DFR", optimize &= ~OPT_DFR);

        ARG_CHOICE ("wlt", optimize &= ~OPT_WLT);
        ARG_CHOICE ("WLT", optimize &= ~OPT_WLT);

        ARG_CHOICE ("wlf", optimize &= ~OPT_WLF);
        ARG_CHOICE ("WLF", optimize &= ~OPT_WLF);

        ARG_CHOICE ("ive", optimize &= ~OPT_IVE);
        ARG_CHOICE ("IVE", optimize &= ~OPT_IVE);

        ARG_CHOICE ("ae", optimize &= ~OPT_AE);
        ARG_CHOICE ("AE", optimize &= ~OPT_AE);

        ARG_CHOICE ("dlaw", optimize &= ~OPT_DLAW);
        ARG_CHOICE ("DLAW", optimize &= ~OPT_DLAW);

        ARG_CHOICE ("rco", optimize &= ~OPT_RCO);
        ARG_CHOICE ("RCO", optimize &= ~OPT_RCO);

        ARG_CHOICE ("uip", optimize &= ~OPT_UIP);
        ARG_CHOICE ("UIP", optimize &= ~OPT_UIP);

        ARG_CHOICE ("tsi", optimize &= ~OPT_TSI);
        ARG_CHOICE ("TSI", optimize &= ~OPT_TSI);

        ARG_CHOICE ("tsp", optimize &= ~OPT_TSP);
        ARG_CHOICE ("TSP", optimize &= ~OPT_TSP);

        ARG_CHOICE ("mto", optimize &= ~OPT_MTO);
        ARG_CHOICE ("MTO", optimize &= ~OPT_MTO);

        ARG_CHOICE ("sbe", optimize &= ~OPT_SBE);
        ARG_CHOICE ("SBE", optimize &= ~OPT_SBE);

        ARG_CHOICE ("phm", optimize &= ~OPT_PHM);
        ARG_CHOICE ("PHM", optimize &= ~OPT_PHM);

        ARG_CHOICE_END ();
    });

    ARGS_OPTION ("o", {
        strcpy (outfilename, ARG);
        /*
         * The option is only stored in outfilename,
         * the correct settings of the global variables
         * outfilename, cfilename, and targetdir will be done
         * in SetFileNames() in scnprs.c. This cannot be done
         * because you have to know the kind of file (program
         * or module/class implementation).
         */
    });

    ARGS_OPTION ("O", ARG_RANGE (cc_optimize, 0, 3));

    ARGS_OPTION ("profile", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', profileflag = PROFILE_ALL);
        ARG_FLAGMASK ('f', profileflag |= PROFILE_FUN);
        ARG_FLAGMASK ('i', profileflag |= PROFILE_INL);
        ARG_FLAGMASK ('l', profileflag |= PROFILE_LIB);
        ARG_FLAGMASK ('w', profileflag |= PROFILE_WITH);
        ARG_FLAGMASK_END ();
    });

    ARGS_OPTION ("target", target_name = ARG);

    ARGS_OPTION ("trace", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', traceflag = TRACE_ALL);
        ARG_FLAGMASK ('m', traceflag |= TRACE_MEM);
        ARG_FLAGMASK ('r', traceflag |= TRACE_REF);
        ARG_FLAGMASK ('f', traceflag |= TRACE_FUN);
        ARG_FLAGMASK ('p', traceflag |= TRACE_PRF);
        ARG_FLAGMASK ('o', traceflag |= TRACE_OWL);
        ARG_FLAGMASK ('w', traceflag |= TRACE_WL);
        ARG_FLAGMASK ('t', traceflag |= TRACE_MT);
        ARG_FLAGMASK_END ();
    });

    ARGS_OPTION ("v", ARG_RANGE (verbose_level, 0, 3));

    ARGS_FLAG ("V", version (); exit (0));

    ARGS_OPTION ("#", {
        if (NULL == strchr (ARG, '/')) {
            my_dbug_str = StringCopy (ARG);
            my_dbug = 1;
        } else {
            char *s;
            my_dbug_from = (compiler_phase_t)strtol (ARG, &s, 10);
            if (*s == '/') {
                s++;
            } else {
                ARGS_ERROR ("Invalid dbug phase specification");
            }

            my_dbug_to = (compiler_phase_t)strtol (s, &s, 10);
            if (*s == '/') {
                s++;
            } else {
                ARGS_ERROR ("Invalid dbug phase specification");
            }

            my_dbug_str = StringCopy (s);
            my_dbug = 1;
        }
    });

    ARGS_ARGUMENT ({
        if (sacfilename[0] == '\0') {
            strcpy (sacfilename, ARG);

            puresacfilename = strrchr (sacfilename, '/');

            if (puresacfilename == NULL) {
                puresacfilename = sacfilename;
            } else {
                puresacfilename += 1;
            }
        } else {
            ARGS_ERROR ("Too many source files specified");
        }
    });

    ARGS_UNKNOWN (ARGS_ERROR ("Invalid command line entry"));

    ARGS_END ();

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   void CheckOptionConsistency()
 *
 * description:
 *
 *   This function is called from main() right after command line arguments
 *   have been analysed. Errors and warnings are produced whenever the user
 *   has selected an incompatible combination of options.
 *
 ******************************************************************************/

void
CheckOptionConsistency ()
{
    DBUG_ENTER ("CheckOptionConsistency");

    if (gen_mt_code) {

        if (cachesim & CACHESIM_YES) {
            SYSERROR (("Cache simulation is not available for multi-threaded "
                       "program execution"));
        }

        if (profileflag != PROFILE_NONE) {
            SYSERROR (("Profiling is not available for multi-threaded "
                       "program execution"));
        }

        if (optimize & OPT_PHM) {
            SYSWARN (("Private heap management is not (yet) available for "
                      "multi-threaded program execution.\n"
                      "Conventional heap management is used instead"));
            optimize &= ~OPT_PHM;
        }
    }

    if ((!(optimize & OPT_PHM)) && (runtimecheck & RUNTIMECHECK_HEAP)) {
        SYSWARN (("Diagnostic heap management is only available in "
                  "conjunction with private heap management"));
    }

    DBUG_VOID_RETURN;
}
