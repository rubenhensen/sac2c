/*
 *
 * $Log$
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
    int store_num_threads;

    DBUG_ENTER ("AnalyseCommandline");

    ARGS_BEGIN (argc, argv);

    OPTION ("b", {
        char *break_arg = StringCopy (ARG);

        ARG = strtok (ARG, ":");
        ARG_RANGE (break_after, 1, 21);
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

    OPTION ("check", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', runtimecheck = RUNTIMECHECK_ALL);
        ARG_FLAGMASK ('m', runtimecheck = RUNTIMECHECK_MALLOC);
        ARG_FLAGMASK ('b', runtimecheck = RUNTIMECHECK_BOUNDARY);
        ARG_FLAGMASK ('e', runtimecheck = RUNTIMECHECK_ERRNO);
        ARG_FLAGMASK_END ();
    });

    FLAG ("copyright", copyright (); exit (0));

    OPTION ("csdefaults", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('s', cachesim &= ~CACHESIM_ADVANCED);
        ARG_FLAGMASK ('a', cachesim |= CACHESIM_ADVANCED);
        ARG_FLAGMASK ('g', cachesim &= ~CACHESIM_PRAGMA);
        ARG_FLAGMASK ('b', cachesim |= CACHESIM_PRAGMA);
        ARG_FLAGMASK ('i', cachesim &= ~CACHESIM_FILE; cachesim &= ~CACHESIM_PIPE);
        ARG_FLAGMASK ('f', cachesim |= CACHESIM_FILE; cachesim &= ~CACHESIM_PIPE);
        ARG_FLAGMASK ('p', cachesim |= CACHESIM_PIPE; cachesim &= ~CACHESIM_FILE);
        ARG_FLAGMASK_END ();
    });

    FLAG ("cs", cachesim |= CACHESIM_YES);

    FLAG ("c", break_after = PH_genccode);

    OPTION ("do", {
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

        ARG_CHOICE ("lunr", optimize |= OPT_LUR);
        ARG_CHOICE ("LUNR", optimize |= OPT_LUR);

        ARG_CHOICE ("wlunr", optimize |= OPT_WLUR);
        ARG_CHOICE ("WLUNR", optimize |= OPT_WLUR);

        ARG_CHOICE ("uns", optimize |= OPT_LUS);
        ARG_CHOICE ("UNS", optimize |= OPT_LUS);

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

        ARG_CHOICE_END ();
    });

    OPTION ("d", {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("efence", use_efence = 1);
        ARG_CHOICE ("nocleanup", cleanup = 0);
        ARG_CHOICE ("syscall", show_syscall = 1);
        ARG_CHOICE ("cccall", gen_cccall = 1; cleanup = 0);
        ARG_CHOICE_END ();
    });

    OPTION ("D", cppvars[num_cpp_vars++] = ARG);

    FLAG ("g", cc_debug = 1);

    FLAG ("help", usage (); exit (0));
    FLAG ("h", usage (); exit (0));

    OPTION ("intrinsic", {
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

    OPTION ("I", AppendPath (MODDEC_PATH, AbsolutePathname (ARG)));

    FLAG ("libstat", libstat = 1);

    OPTION ("l", { ARG_RANGE (linkstyle, 1, 2); });

    OPTION ("L", {
        AppendPath (MODIMP_PATH, AbsolutePathname (ARG));
        AppendPath (SYSTEMLIB_PATH, AbsolutePathname (ARG));
    });

    OPTION ("maxoptcyc", ARG_NUM (max_optcycles));

    OPTION ("maxoptvar", ARG_NUM (optvar));

    OPTION ("maxinl", ARG_NUM (inlnum));

    OPTION ("maxlur", ARG_NUM (unrnum));

    OPTION ("maxwlur", ARG_NUM (wlunrnum));

    OPTION ("maxspecialize", ARG_NUM (max_overload));

    OPTION ("maxthreads", ARG_NUM (max_threads));

    OPTION ("maxsyncfold", ARG_NUM (max_sync_fold));

    OPTION ("minae", ARG_NUM (minarray));

    OPTION ("minmtsize", ARG_NUM (min_parallel_size));

    FLAG ("mt", {
        gen_mt_code = 1;
        if (store_num_threads > 0) {
            num_threads = store_num_threads;
        } else {
            num_threads = 0;
        }
    });

    FLAG ("Mlib", makedeps = 2);

    FLAG ("M", makedeps = 1);

    OPTION ("numthreads", {
        ARG_RANGE (store_num_threads, 1, max_threads);
        if (gen_mt_code) {
            num_threads = store_num_threads;
        }
    });

    OPTION ("no", {
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

        ARG_CHOICE ("lunr", optimize &= ~OPT_LUR);
        ARG_CHOICE ("LUNR", optimize &= ~OPT_LUR);

        ARG_CHOICE ("wlunr", optimize &= ~OPT_WLUR);
        ARG_CHOICE ("WLUNR", optimize &= ~OPT_WLUR);

        ARG_CHOICE ("uns", optimize &= ~OPT_LUS);
        ARG_CHOICE ("UNS", optimize &= ~OPT_LUS);

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

        ARG_CHOICE_END ();
    });

    OPTION ("o", {
        strcpy (outfilename, *argv);
        /*
         * The option is only stored in outfilename,
         * the correct settings of the global variables
         * outfilename, cfilename, and targetdir will be done
         * in SetFileNames() in scnprs.c. This cannot be done
         * because you have to know the kind of file (program
         * or module/class implementation).
         */
    });

    OPTION ("O", ARG_RANGE (cc_optimize, 0, 3));

    OPTION ("profile", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', profileflag = PROFILE_ALL);
        ARG_FLAGMASK ('f', profileflag |= PROFILE_FUN);
        ARG_FLAGMASK ('i', profileflag |= PROFILE_INL);
        ARG_FLAGMASK ('l', profileflag |= PROFILE_LIB);
        ARG_FLAGMASK ('w', profileflag |= PROFILE_WITH);
        ARG_FLAGMASK_END ();
    });

    OPTION ("target", target_name = ARG);

    OPTION ("trace", {
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

    OPTION ("v", ARG_RANGE (verbose_level, 0, 3));

    FLAG ("V", version (); exit (0));

    OPTION ("#", {
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

    ARGUMENT ({
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

    ARGS_END ();

    DBUG_VOID_RETURN;
}
