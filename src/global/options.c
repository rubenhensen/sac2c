/*
 *
 * $Log$
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
 * Revision 3.32  2002/11/08 13:29:45  cg
 * Removed TRACE_OWL macro since old with-loops left sac2c several
 * years ago.  :-))))
 *
 * Revision 3.31  2002/10/25 16:00:51  mwe
 * option enforce_ieee added
 * rename DLAW to DL
 *
 * Revision 3.30  2002/10/24 13:12:32  ktr
 * level of WLS aggressiveness now controlled by flag -wls_aggressive
 *
 * Revision 3.29  2002/10/18 14:16:50  ktr
 * changed option -wlsx to -wls <level>
 *
 * Revision 3.28  2002/10/17 17:53:08  ktr
 * added option -wlsx for aggressive WLS
 *
 * Revision 3.27  2002/10/09 13:09:18  dkr
 * TAGGED_ARRAYS: warning about RCAO added
 *
 * Revision 3.26  2002/09/26 13:14:38  sbs
 * mt compilation enabled for OSF_ALPHA now!!
 *
 * Revision 3.25  2002/07/24 15:50:06  dkr
 * -mt disabled for TAGGED_ARRAYS
 *
 * Revision 3.24  2002/07/15 19:05:03  dkr
 * -intrinsic flag modified for TAGGED_ARRAYS
 *
 * Revision 3.23  2002/07/03 15:27:39  dkr
 * RUNTIMECHECK_TYPE added (for TAGGED_ARRAYS)
 *
 * Revision 3.21  2002/06/07 17:11:23  mwe
 * OPT_AL added for AssociativeLaw
 *
 * Revision 3.20  2002/03/13 16:03:20  ktr
 * OPT_WLS added for Withloop-Scalarization
 *
 * Revision 3.19  2002/01/18 16:53:17  sacbase
 * PHM disabled for OSX_MAC
 *
 * Revision 3.18  2001/12/10 15:31:55  dkr
 * flag 'xxx' (dkr) removed
 *
 * Revision 3.17  2001/11/21 19:28:28  sbs
 * mt generation on MACs enabled again 8-))
 *
 * Revision 3.16  2001/11/21 19:25:48  sbs
 * mt code generation on MACs disabled 8-(
 *
 * Revision 3.15  2001/06/28 07:46:51  cg
 * Primitive function psi() renamed to sel().
 *
 * Revision 3.14  2001/05/28 09:01:00  nmw
 * SSACSE requieres SSADCR - additional check and warning implemented
 *
 * Revision 3.13  2001/05/17 11:15:59  sbs
 * return value of Free used now 8-()
 *
 * Revision 3.12  2001/05/17 08:35:33  sbs
 * MALLOC/FREE eliminated
 *
 * Revision 3.11  2001/05/07 15:01:42  dkr
 * PAB_YES, PAB_NO replaced by TRUE, FALSE.
 *
 * Revision 3.10  2001/04/24 09:36:35  dkr
 * CHECK_NULL renamed into STR_OR_EMPTY
 *
 * Revision 3.9  2001/03/28 14:51:45  dkr
 * CHECK_NULL used
 *
 * Revision 3.8  2001/02/09 14:39:46  nmw
 * ssa switch added
 *
 * Revision 3.7  2001/01/25 10:18:38  dkr
 * -b21 added
 *
 * Revision 3.6  2000/12/14 17:48:13  dkr
 * internal flag 'dkr' replaced by 'xxx'
 *
 * Revision 3.5  2000/12/12 12:15:57  dkr
 * internal flag 'dkr' added
 *
 * Revision 3.4  2000/12/01 12:32:03  cg
 * Added option -noAPL to disable array placement (default).
 *
 * Revision 3.3  2000/11/27 21:04:38  cg
 * Added general support for new optimization APL,
 * "array placement"
 *
 * Revision 3.2  2000/11/24 16:30:24  nmw
 * -trace c for c library runtime enviroment tracing added
 *
 * Revision 3.1  2000/11/20 17:59:35  sacbase
 * new release made
 *
 * Revision 2.52  2000/11/17 16:21:06  sbs
 * -MM and -MMlib added.
 *
 * Revision 2.51  2000/11/15 20:34:57  sbs
 * -mt disabled for DEC ALPHA
 * if check b and doAP => AP is turned off, warning is given
 * and the program is still compiled.
 *
 * Revision 2.50  2000/10/27 13:23:13  cg
 * Added new command line options -aplimit and -apdiaglimit.
 *
 * Revision 2.49  2000/08/16 09:04:09  dkr
 * minor corrections of error messages done
 *
 * Revision 2.48  2000/08/02 14:22:46  mab
 * added flag "-apdiag"
 *
 * Revision 2.47  2000/08/02 11:14:25  nmw
 * WARNING for MT with genlib c added
 *
 * Revision 2.46  2000/08/01 13:43:58  nmw
 * default disabling of PHM in case of c libraries removed
 *
 * Revision 2.45  2000/07/12 13:05:35  mab
 * bug fixed in option consistency check
 *
 * Revision 2.44  2000/07/12 12:08:58  mab
 * added consistency check for array padding and boundary check
 *
 * Revision 2.43  2000/07/11 16:13:49  dkr
 * -b21 illegal now
 *
 * Revision 2.42  2000/07/11 15:51:03  dkr
 * PH_psiopt removed
 *
 * Revision 2.41  2000/06/13 13:39:02  dkr
 * options 'wltrans' renamed into 'wlpatch'
 *
 * Revision 2.40  2000/06/08 09:04:03  nmw
 * added default opt. and mt. settings for generating c library
 * by default PHM and MT are disabled
 *
 * Revision 2.39  2000/06/07 11:43:24  nmw
 * commandline switch genlib added with options sac and c
 * default forced if not specified: sac library
 *
 * Revision 2.38  2000/05/29 17:22:14  dkr
 * 'show_refcnt' set (to 1) in phase PH_refcnt only
 *
 * Revision 2.37  2000/05/26 14:22:29  sbs
 * noAP doAP added
 *
 * Revision 2.36  2000/03/24 15:20:39  dkr
 * CheckOptionConsistency() extended
 *
 * Revision 2.35  2000/03/23 21:38:51  dkr
 * consistency check for -lac2fun, -fun2lac added
 *
 * Revision 2.34  2000/03/17 20:27:35  dkr
 * option -# without / in parameter: my_dbug_from and my_dbug_to are explicitely
 * set to the first and last phase, respectively.
 *
 * Revision 2.33  2000/03/17 11:41:16  dkr
 * fixed a bug in macro LAC_FUN
 *
 * Revision 2.32  2000/03/17 10:38:50  dkr
 * comment for -lac2fun and -fun2lac added
 *
 * Revision 2.31  2000/03/16 16:22:07  dkr
 * options -lac2fun and -fun2lac extended:
 * activates lac2fun conversion and vice versa for the given compiler
 * phases
 *
 * Revision 2.30  2000/03/02 18:50:04  cg
 * Added new option -lac2fun that activates lac2fun conversion and
 * vice versa between psi optimizations and precompiling.
 *
 * Revision 2.29  2000/02/04 14:45:50  jhs
 * Added -maxrepsize.
 *
 * Revision 2.28  2000/02/03 16:45:11  cg
 * Added new optimization option MSCA.
 *
 * Revision 2.27  2000/01/24 12:23:08  jhs
 * Added options to activate/dactivate printing after a break
 * (-noPAB, -doPAB).
 *
 * Revision 2.26  2000/01/21 13:19:53  jhs
 * Added new mt ... infrastructure expanded ...
 *
 * Revision 2.25  2000/01/17 17:58:45  cg
 * Added new heap manager optimization options
 * APS (arena preselection) and
 * RCAO (reference counter allocation optimization).
 *
 * Revision 2.24  2000/01/17 16:25:58  cg
 * Added new options to control initial heap sizes separately
 * for master's arena of arenas, workers' arena of arenas and the
 * top arena.
 *
 * Revision 2.23  1999/11/30 20:33:12  dkr
 * flag -wlconv added for activation of Old2NewWith()
 *
 * Revision 2.22  1999/11/23 12:17:57  dkr
 * PRINT_RC, PRINT_NRC can be used not only with -b17 but with -b17 ... -b20
 *
 * Revision 2.21  1999/10/04 11:58:34  sbs
 * secret option "sbs" added!
 *
 * Revision 2.20  1999/08/05 13:33:59  jhs
 * Added OPT_MTI.
 *
 * Revision 2.19  1999/07/21 16:28:19  jhs
 * needed_sync_fold introduced, max_sync_fold adjusted, command-line and usage
 * updated.
 *
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
        SYSERROR (                                                                       \
          ("%s: %s %s %s", msg, ARGS_argv[0], STR_OR_EMPTY (OPT), STR_OR_EMPTY (ARG)));  \
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

    ARGS_FLAG ("apdiag", apdiag = TRUE);

    ARGS_OPTION ("aplimit", { ARG_RANGE (padding_overhead_limit, 0, 100); });

    ARGS_OPTION ("apdiagsize", ARG_NUM (apdiag_limit));

    ARGS_OPTION ("b", {
        char *break_arg = StringCopy (ARG);

        ARG = strtok (ARG, ":");
        ARG_RANGE (break_after, 1, 21);
        switch (break_after) {
        case PH_sacopt:
            show_idx = TRUE;
            break;
        case PH_refcnt:
            show_refcnt = TRUE;
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
        break_arg = Free (break_arg);
    });

#ifdef TAGGED_ARRAYS
    ARGS_OPTION ("check", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', runtimecheck = RUNTIMECHECK_ALL);
        ARG_FLAGMASK ('t', runtimecheck |= RUNTIMECHECK_TYPE);
        ARG_FLAGMASK ('b', runtimecheck |= RUNTIMECHECK_BOUNDARY);
        ARG_FLAGMASK ('m', runtimecheck |= RUNTIMECHECK_MALLOC);
        ARG_FLAGMASK ('e', runtimecheck |= RUNTIMECHECK_ERRNO);
        ARG_FLAGMASK ('h', runtimecheck |= RUNTIMECHECK_HEAP);
        ARG_FLAGMASK_END ();
    });
#else
    ARGS_OPTION ("check", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', runtimecheck = RUNTIMECHECK_ALL);
        ARG_FLAGMASK ('b', runtimecheck |= RUNTIMECHECK_BOUNDARY);
        ARG_FLAGMASK ('m', runtimecheck |= RUNTIMECHECK_MALLOC);
        ARG_FLAGMASK ('e', runtimecheck |= RUNTIMECHECK_ERRNO);
        ARG_FLAGMASK ('h', runtimecheck |= RUNTIMECHECK_HEAP);
        ARG_FLAGMASK_END ();
    });
#endif

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

    ARGS_OPTION ("csdir", strncpy (cachesim_dir, ARG, MAX_FILE_NAME - 1));

    ARGS_OPTION ("csfile", strncpy (cachesim_file, ARG, MAX_FILE_NAME - 1));

    ARGS_OPTION ("cshost", strncpy (cachesim_host, ARG, MAX_FILE_NAME - 1));

    ARGS_FLAG ("cs", cachesim |= CACHESIM_YES);

    ARGS_FLAG ("c", break_after = PH_genccode);

    ARGS_FLAG ("ds", dynamic_shapes = TRUE);

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

        ARG_CHOICE ("wls", optimize |= OPT_WLS);
        ARG_CHOICE ("WLS", optimize |= OPT_WLS);

        ARG_CHOICE ("al", optimize |= OPT_AL);
        ARG_CHOICE ("AL", optimize |= OPT_AL);

        ARG_CHOICE ("ive", optimize |= OPT_IVE);
        ARG_CHOICE ("IVE", optimize |= OPT_IVE);

        ARG_CHOICE ("ae", optimize |= OPT_AE);
        ARG_CHOICE ("AE", optimize |= OPT_AE);

        ARG_CHOICE ("dl", optimize |= OPT_DL);
        ARG_CHOICE ("DL", optimize |= OPT_DL);

        ARG_CHOICE ("rco", optimize |= OPT_RCO);
        ARG_CHOICE ("RCO", optimize |= OPT_RCO);

        ARG_CHOICE ("uip", optimize |= OPT_UIP);
        ARG_CHOICE ("UIP", optimize |= OPT_UIP);

        ARG_CHOICE ("tsi", optimize |= OPT_TSI);
        ARG_CHOICE ("TSI", optimize |= OPT_TSI);

        ARG_CHOICE ("ap", optimize |= OPT_AP);
        ARG_CHOICE ("AP", optimize |= OPT_AP);

        ARG_CHOICE ("apl", optimize |= OPT_APL);
        ARG_CHOICE ("APL", optimize |= OPT_APL);

        ARG_CHOICE ("tsp", optimize |= OPT_TSP);
        ARG_CHOICE ("TSP", optimize |= OPT_TSP);

        ARG_CHOICE ("mto", optimize |= OPT_MTO);
        ARG_CHOICE ("MTO", optimize |= OPT_MTO);

        ARG_CHOICE ("mti", optimize |= OPT_MTI);
        ARG_CHOICE ("MTI", optimize |= OPT_MTI);

        ARG_CHOICE ("sbe", optimize |= OPT_SBE);
        ARG_CHOICE ("SBE", optimize |= OPT_SBE);

        ARG_CHOICE ("phm", optimize |= OPT_PHM);
        ARG_CHOICE ("PHM", optimize |= OPT_PHM);

        ARG_CHOICE ("aps", optimize |= OPT_APS);
        ARG_CHOICE ("APS", optimize |= OPT_APS);

        ARG_CHOICE ("rcao", optimize |= OPT_RCAO);
        ARG_CHOICE ("RCAO", optimize |= OPT_RCAO);

        ARG_CHOICE ("msca", optimize |= OPT_MSCA);
        ARG_CHOICE ("MSCA", optimize |= OPT_MSCA);

        ARG_CHOICE ("blir", optimize |= OPT_BLIR);
        ARG_CHOICE ("BLIR", optimize |= OPT_BLIR);

        ARG_CHOICE ("pab", print_after_break = TRUE);
        ARG_CHOICE ("PAB", print_after_break = TRUE);

        ARG_CHOICE_END ();
    });

    ARGS_OPTION ("d", {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("efence", use_efence = TRUE);
        ARG_CHOICE ("nocleanup", cleanup = FALSE);
        ARG_CHOICE ("syscall", show_syscall = TRUE);
        ARG_CHOICE ("cccall", gen_cccall = TRUE; cleanup = FALSE);
        ARG_CHOICE_END ();
    });

    ARGS_OPTION ("D", cppvars[num_cpp_vars++] = ARG);

    ARGS_FLAG ("enforceIEEE", enforce_ieee = TRUE);

    ARGS_OPTION ("genlib", {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("sac", generatelibrary |= GENERATELIBRARY_SAC);
        ARG_CHOICE ("c", generatelibrary |= GENERATELIBRARY_C);
        ARG_CHOICE_END ();
    });

    ARGS_FLAG ("g", cc_debug = TRUE);

    ARGS_FLAG ("h", usage (); exit (0));
    ARGS_FLAG ("help", usage (); exit (0));

    ARGS_OPTION ("initmheap", ARG_NUM (initial_master_heapsize));
    ARGS_OPTION ("initwheap", ARG_NUM (initial_worker_heapsize));
    ARGS_OPTION ("inituheap", ARG_NUM (initial_unified_heapsize));

#ifdef TAGGED_ARRAYS
    ARGS_OPTION ("intrinsic", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', intrinsics = INTRINSIC_ALL);
        ARG_FLAGMASK ('+', intrinsics |= INTRINSIC_ADD);
        ARG_FLAGMASK ('-', intrinsics |= INTRINSIC_SUB);
        ARG_FLAGMASK ('x', intrinsics |= INTRINSIC_MUL);
        ARG_FLAGMASK ('/', intrinsics |= INTRINSIC_DIV);
        ARG_FLAGMASK ('s', intrinsics |= INTRINSIC_SEL);
        ARG_FLAGMASK ('o', intrinsics |= INTRINSIC_TO);
        ARG_FLAGMASK_END ();
    });
#else
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
        ARG_FLAGMASK ('s', intrinsics |= INTRINSIC_SEL);
        ARG_FLAGMASK ('o', intrinsics |= INTRINSIC_TO);
        ARG_FLAGMASK_END ();
    });
#endif

    ARGS_OPTION ("I", AppendPath (MODDEC_PATH, AbsolutePathname (ARG)));

    ARGS_FLAG ("libstat", libstat = TRUE);

#define LAC_FUN(array)                                                                   \
    {                                                                                    \
        int phase;                                                                       \
        char *old_s;                                                                     \
        char *new_s;                                                                     \
                                                                                         \
        for (phase = 0; phase < PH_final; phase++) {                                     \
            array[phase] = FALSE;                                                        \
        }                                                                                \
                                                                                         \
        old_s = ARG;                                                                     \
        while (*old_s) {                                                                 \
            if (*old_s == ':') {                                                         \
                old_s++;                                                                 \
            } else {                                                                     \
                if (old_s != ARG) {                                                      \
                    ARGS_ERROR ("Illegal separation symbol found");                      \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
                                                                                         \
            phase = strtol (old_s, &new_s, 10);                                          \
                                                                                         \
            if (phase >= PH_final) {                                                     \
                ARGS_ERROR ("Illegal phase number found");                               \
                break;                                                                   \
            } else {                                                                     \
                if (new_s != old_s) {                                                    \
                    array[phase] = TRUE;                                                 \
                } else {                                                                 \
                    ARGS_ERROR ("No phase number found");                                \
                    break;                                                               \
                }                                                                        \
            }                                                                            \
                                                                                         \
            old_s = new_s;                                                               \
        }                                                                                \
    }
    /* "-lac2fun 8:14" means: call Lac2fun() before phases 8 and 14 */
    ARGS_OPTION ("lac2fun", LAC_FUN (do_lac2fun));
    /* "-fun2lac 8:21" means: call Fun2lac() after phases 8 and 21 */
    ARGS_OPTION ("fun2lac", LAC_FUN (do_fun2lac));

    ARGS_OPTION ("l", { ARG_RANGE (linkstyle, 1, 2); });

    ARGS_OPTION ("L", {
        AppendPath (MODIMP_PATH, AbsolutePathname (ARG));
        AppendPath (SYSTEMLIB_PATH, AbsolutePathname (ARG));
    });

    ARGS_FLAG ("mtn", {
        gen_mt_code = GEN_MT_NEW;
        if (store_num_threads > 0) {
            num_threads = store_num_threads;
        } else {
            num_threads = 0;
        }
    });

    ARGS_FLAG ("mt", {
        gen_mt_code = GEN_MT_OLD;
        if (store_num_threads > 0) {
            num_threads = store_num_threads;
        } else {
            num_threads = 0;
        }
    });

    ARGS_OPTION ("maxoptcyc", ARG_NUM (max_optcycles));

    ARGS_OPTION ("maxoptvar", ARG_NUM (optvar));

    ARGS_OPTION ("maxinl", ARG_NUM (inlnum));

    ARGS_OPTION ("maxlur", ARG_NUM (unrnum));

    ARGS_OPTION ("maxwlur", ARG_NUM (wlunrnum));

    ARGS_OPTION ("maxspec", ARG_NUM (max_overload));

    ARGS_OPTION ("maxthreads", ARG_NUM (max_threads));

    ARGS_OPTION ("maxsync", ARG_RANGE (max_sync_fold, -1, 64));

    ARGS_OPTION ("maxae", ARG_NUM (minarray));

    ARGS_OPTION ("minmtsize", ARG_NUM (min_parallel_size));

    ARGS_OPTION ("maxrepsize", ARG_NUM (max_replication_size));

    ARGS_OPTION ("minarrayrep", {
        ARG_CHOICE_BEGIN ();
        ARG_CHOICE ("s", min_array_rep = MIN_ARRAY_REP_SCL_AKS);
        ARG_CHOICE ("d", min_array_rep = MIN_ARRAY_REP_SCL_AKD);
        ARG_CHOICE ("+", min_array_rep = MIN_ARRAY_REP_SCL_AUD);
        ARG_CHOICE ("*", min_array_rep = MIN_ARRAY_REP_AUD);
        ARG_CHOICE_END ();
    });

    ARGS_FLAG ("MMlib", makedeps = 4);
    ARGS_FLAG ("MM", makedeps = 3);
    ARGS_FLAG ("Mlib", makedeps = 2);
    ARGS_FLAG ("M", makedeps = 1);

    ARGS_OPTION ("numthreads", {
        ARG_RANGE (store_num_threads, 1, max_threads);
        if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) {
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

        ARG_CHOICE ("wls", optimize &= ~OPT_WLS);
        ARG_CHOICE ("WLS", optimize &= ~OPT_WLS);

        ARG_CHOICE ("al", optimize &= ~OPT_AL);
        ARG_CHOICE ("AL", optimize &= ~OPT_AL);

        ARG_CHOICE ("ive", optimize &= ~OPT_IVE);
        ARG_CHOICE ("IVE", optimize &= ~OPT_IVE);

        ARG_CHOICE ("ae", optimize &= ~OPT_AE);
        ARG_CHOICE ("AE", optimize &= ~OPT_AE);

        ARG_CHOICE ("dl", optimize &= ~OPT_DL);
        ARG_CHOICE ("DL", optimize &= ~OPT_DL);

        ARG_CHOICE ("rco", optimize &= ~OPT_RCO);
        ARG_CHOICE ("RCO", optimize &= ~OPT_RCO);

        ARG_CHOICE ("uip", optimize &= ~OPT_UIP);
        ARG_CHOICE ("UIP", optimize &= ~OPT_UIP);

        ARG_CHOICE ("tsi", optimize &= ~OPT_TSI);
        ARG_CHOICE ("TSI", optimize &= ~OPT_TSI);

        ARG_CHOICE ("ap", optimize &= ~OPT_AP);
        ARG_CHOICE ("AP", optimize &= ~OPT_AP);

        ARG_CHOICE ("apl", optimize &= ~OPT_APL);
        ARG_CHOICE ("APL", optimize &= ~OPT_APL);

        ARG_CHOICE ("tsp", optimize &= ~OPT_TSP);
        ARG_CHOICE ("TSP", optimize &= ~OPT_TSP);

        ARG_CHOICE ("mto", optimize &= ~OPT_MTO);
        ARG_CHOICE ("MTO", optimize &= ~OPT_MTO);

        ARG_CHOICE ("mti", optimize &= ~OPT_MTI);
        ARG_CHOICE ("MTI", optimize &= ~OPT_MTI);

        ARG_CHOICE ("sbe", optimize &= ~OPT_SBE);
        ARG_CHOICE ("SBE", optimize &= ~OPT_SBE);

        ARG_CHOICE ("phm", optimize &= ~OPT_PHM);
        ARG_CHOICE ("PHM", optimize &= ~OPT_PHM);

        ARG_CHOICE ("aps", optimize &= ~OPT_APS);
        ARG_CHOICE ("APS", optimize &= ~OPT_APS);

        ARG_CHOICE ("rcao", optimize &= ~OPT_RCAO);
        ARG_CHOICE ("RCAO", optimize &= ~OPT_RCAO);

        ARG_CHOICE ("msca", optimize &= ~OPT_MSCA);
        ARG_CHOICE ("MSCA", optimize &= ~OPT_MSCA);

        ARG_CHOICE ("blir", optimize &= ~OPT_BLIR);
        ARG_CHOICE ("BLIR", optimize &= ~OPT_BLIR);

        ARG_CHOICE ("pab", print_after_break = FALSE);
        ARG_CHOICE ("PAB", print_after_break = FALSE);

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

    ARGS_FLAG ("sbs", sbs = TRUE);

    ARGS_FLAG ("ssa", use_ssaform = TRUE);

    ARGS_OPTION ("target", target_name = ARG);

    ARGS_OPTION ("trace", {
        ARG_FLAGMASK_BEGIN ();
        ARG_FLAGMASK ('a', traceflag = TRACE_ALL);
        ARG_FLAGMASK ('m', traceflag |= TRACE_MEM);
        ARG_FLAGMASK ('r', traceflag |= TRACE_REF);
        ARG_FLAGMASK ('f', traceflag |= TRACE_FUN);
        ARG_FLAGMASK ('p', traceflag |= TRACE_PRF);
        ARG_FLAGMASK ('w', traceflag |= TRACE_WL);
        ARG_FLAGMASK ('s', traceflag |= TRACE_AA);
        ARG_FLAGMASK ('t', traceflag |= TRACE_MT);
        ARG_FLAGMASK ('c', traceflag |= TRACE_CENV);
        ARG_FLAGMASK_END ();
    });

    ARGS_OPTION ("v", ARG_RANGE (verbose_level, 0, 3));

    ARGS_FLAG ("V", version (); exit (0));

    ARGS_FLAG ("wls_aggressive", wls_aggressive = TRUE);

    ARGS_FLAG ("wlpatch", patch_with = TRUE);

    ARGS_OPTION ("#", {
        if (NULL == strchr (ARG, '/')) {
            my_dbug_str = StringCopy (ARG);
            my_dbug = 1;
            my_dbug_from = PH_initial;
            my_dbug_to = PH_final;
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
    int write_dummyfuns_into_sib;
    int ph_l2f, i;

    DBUG_ENTER ("CheckOptionConsistency");

    if (runtimecheck & RUNTIMECHECK_BOUNDARY && (optimize & OPT_AP)) {
        optimize &= ~OPT_AP;
        SYSWARN (("Boundary check (-checkb) and array padding (AP) may not be used"
                  " simultaneously.\n"
                  "Array padding disabled"));
    }

#ifdef TAGGED_ARRAYS
    if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) {
        gen_mt_code = GEN_MT_NONE;
        num_threads = 1;
        SYSWARN (("Code generation for multi-threaded program execution not"
                  " yet available for TAGGED_ARRAYS.\n"
                  "Code for sequential execution generated instead"));
    }
    if (optimize & OPT_RCAO) {
        SYSWARN (("Refcount allocation optimization (RCAO) of private heap"
                  " management is not yet available for TAGGED_ARRAYS.\n"
                  "RCAO disabled"));
        optimize &= ~OPT_RCAO;
    }
#endif

#ifdef SAC_FOR_OSX_MAC
    if (optimize & OPT_PHM) {
        SYSWARN (("Private heap management is not yet available for Mac OSX.\n"
                  "Conventional heap management is used instead"));
        optimize &= ~OPT_PHM;
    }
#endif

    if ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW)) {
        if (cachesim & CACHESIM_YES) {
            SYSERROR (("Cache simulation is not available for multi-threaded "
                       "program execution"));
        }

        if (profileflag != PROFILE_NONE) {
            SYSERROR (("Profiling is not available for multi-threaded "
                       "program execution"));
        }
    }

    if (use_ssaform && (optimize & OPT_CSE) && (!(optimize & OPT_DCR))) {
        SYSWARN (("Common subexpressions elimination (CSE) in ssaform requires "
                  "dead code removal (DCR) enabled.\n"
                  "CSE disabled"));
        optimize &= ~OPT_CSE;
    }

    if ((!(optimize & OPT_PHM)) && (runtimecheck & RUNTIMECHECK_HEAP)) {
        SYSWARN (("Diagnostic heap management is only available in "
                  "conjunction with private heap management.\n"
                  "Diagnostic disabled"));
        runtimecheck &= ~RUNTIMECHECK_HEAP;
    }

    /* commandline switch for library generation not used, set it to default
       and generate a standard SAC Library */
    if (generatelibrary == GENERATELIBRARY_NOTHING) {
        generatelibrary = GENERATELIBRARY_SAC;
    }
    if ((generatelibrary & GENERATELIBRARY_C)
        && ((gen_mt_code == GEN_MT_OLD) || (gen_mt_code == GEN_MT_NEW))) {
        SYSWARN (("Multithreading is not yet available when compiling for "
                  "a c-library"));
    }

    /*
     * check -lac2fun, -fun2lac options
     */

    /*
     * For the time being the dummy-tag of the lac-functions is lost
     * when these functions are written into the SIB.
     *  -> give a warning if relevant.
     */
    write_dummyfuns_into_sib = FALSE;
    ph_l2f = PH_initial;
    for (i = PH_initial; i <= PH_writesib; i++) {
        if (do_lac2fun[i]) {
            ph_l2f = i;
            write_dummyfuns_into_sib = TRUE;
        }
    }
    for (; i < PH_writesib; i++) {
        if (do_fun2lac[i]) {
            write_dummyfuns_into_sib = FALSE;
        }
    }
    if (write_dummyfuns_into_sib) {
        SYSWARN (("The tag of lac2fun-functions is lost when these functions "
                  "are written into the SIB"));
    }

    /*
     * Between PH_analysis and PH_objects no transformations are allowed!
     */

    for (i = PH_analysis; i < PH_objects; i++) {
        if ((do_lac2fun[i + 1]) || (do_fun2lac[i])) {
            SYSERROR (("No transformation of loops and conditionals into functions"
                       " or vice versa is allowed between phases %i and %i",
                       PH_analysis, PH_objects));
        }
    }

    /*
     * After PH_refcnt no lac2fun transformation is allowed!
     */

    for (i = PH_refcnt + 1; i < PH_final; i++) {
        if (do_lac2fun[i]) {
            SYSERROR (("No transformation of loops and conditionals into functions"
                       " is allowed after phase %i",
                       PH_refcnt));
        }
    }

    DBUG_VOID_RETURN;
}
