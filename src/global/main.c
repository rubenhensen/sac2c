/*
 *
 * $Log$
 * Revision 1.117  1998/04/03 11:27:31  dkr
 * concregs renamed to concregions
 *
 * Revision 1.116  1998/04/02 16:05:57  dkr
 * new compiler phase:
 *   generating concurrent regions (phase 18)
 *
 * Revision 1.115  1998/03/24 15:30:29  cg
 * #include "profile.h" removed since file no longer exists.
 *
 * Revision 1.114  1998/03/17 14:21:58  cg
 * file src/compile/trace.h removed.
 * definition of symbolic values of global variable traceflag moved to globals.h
 *
 * Revision 1.113  1998/03/13 13:13:28  dkr
 * removed a bug with flag _DBUG
 *
 * Revision 1.112  1998/03/04 16:21:00  cg
 * C compiler invocations and file handling converted to new
 * configuration files.
 *
 * Revision 1.111  1998/03/02 13:58:37  cg
 * Scanning of command line options streamlined.
 *
 * Revision 1.110  1998/02/27 16:28:45  cg
 * added usage of sac2c configuration files for customizing sac2c for various
 * target architectures and C compilers.
 *
 * Revision 1.109  1998/02/25 09:11:41  cg
 * Global variables moved to globals.c
 * Compilation process rigidly streamlined.
 * New break options using break specifiers.
 *
 * Revision 1.108  1998/02/09 15:59:24  srs
 * changed call of optimization functions
 *
 * Revision 1.107  1998/02/06 13:32:20  srs
 * inserted var opt_wlf and switch -noWLF
 *
 * Revision 1.106  1998/02/05 12:43:37  srs
 * added some comments
 *
 * Revision 1.105  1997/12/06 17:16:35  srs
 * added call of compute_malloc_align_step()
 *
 * Revision 1.104  1997/11/23 15:18:26  dkr
 * CC-flag: show_malloc -> SHOW_MALLOC
 *
 * Revision 1.103  1997/11/20 18:37:17  dkr
 * moved call of Old2NewWith().
 * the call is now after compiler-phase 18 (RefCount()) --- use: sac2c -b18 -2 ...
 *
 * Revision 1.102  1997/11/20 14:45:48  dkr
 * added a converter "OldWithLoop -> NewWithLoop" (function Old2NewWith())
 * the new sac2c flag -2 activates the converter
 * sac2c calls the converter between compilerphases 19 and 20 (before Compile())
 *
 * Revision 1.101  1997/10/29 14:18:04  srs
 * removed HAVE_MALLOC_O
 * changed output for memory allocation statistics
 *
 * Revision 1.100  1997/10/10 13:41:57  srs
 * counter for mem allocation
 *
 * Revision 1.99  1997/10/09 13:53:25  srs
 * counter for memory allocation
 *
 * Revision 1.97  1997/08/07 11:11:43  dkr
 * added option -_DBUG<from>/<to>/<string>
 *
 * Revision 1.96  1997/06/03 10:14:46  sbs
 * -D option integrated
 *
 * Revision 1.95  1997/05/28  12:36:51  sbs
 * Profiling integrated
 *
 * Revision 1.94  1997/05/14  08:17:34  sbs
 * analyseflag activated
 *
 * Revision 1.93  1997/04/24  14:05:37  sbs
 * HAVE_MALLOC_O inserted
 *
 * Revision 1.92  1997/04/24  09:54:53  cg
 * added -Mlib option
 * dependencies now printed after import
 *
 * Revision 1.91  1997/03/19  13:36:19  cg
 * Now, the syntax tree is removed before the gcc is started
 *
 * Revision 1.90  1997/03/11  16:32:44  cg
 * compiler option -deps replaced by -M
 *
 * Revision 1.89  1996/09/11  06:15:08  cg
 * Added options -libstat, -deps, -noranlib, -l1, l2, l3
 *
 * Revision 1.88  1996/08/09  16:44:12  asi
 * dead function removal added
 *
 * Revision 1.87  1996/07/14  13:02:19  sbs
 * -opt_rco disabled whith noOPT !
 *
 * Revision 1.86  1996/05/29  14:18:57  sbs
 * inserted noRCO opt_rco!
 *
 * Revision 1.85  1996/01/25  18:37:50  cg
 * added new stop options using compiler phase numbers
 *
 * Revision 1.84  1996/01/22  17:28:30  cg
 * Now, paths are initialized with the current directory
 *
 * Revision 1.83  1996/01/17  16:49:21  asi
 * added common subexpression elimination
 *
 * Revision 1.82  1996/01/16  16:43:14  cg
 * added/modified debug options -dnocleanup, -dcheck_malloc
 * and -dcheck_boundary
 *
 * Revision 1.81  1996/01/05  12:27:29  cg
 * C-compiler call is now an ordinary compilation phase.
 * Module/class implementations are compiled to SAC libraries.
 * The filename handling (-o option) is moved to scnprs.c
 *
 * Revision 1.80  1996/01/02  15:46:31  cg
 * The whole file handling is moved to print.c scnprs.c and cccall.c
 * main.c exclusively evaluates command line options and triggers
 * the compilation process.
 *
 * Revision 1.79  1995/12/29  10:21:43  cg
 * added new compiler phase readsib
 *
 * Revision 1.78  1995/12/21  16:09:20  cg
 * added option -flink_module and global var link_module
 *
 * Revision 1.77  1995/12/01  20:23:56  cg
 * changed compilation sequence: objinit.c now after import.c
 *
 * Revision 1.76  1995/12/01  17:05:47  cg
 * removed global variable 'silent'
 * new compilation phase 'precompile'
 * new break parameter '-bl' to stop after it
 *
 * Revision 1.75  1995/11/16  19:35:48  cg
 * added new compilation phase 'RemoveVoidFunctions'
 * break parameter: -bv
 * activated call of FreeTree at end of compilation to test the
 * new free.c with as many programs as possible.
 *
 * Revision 1.74  1995/11/10  15:01:18  cg
 * verbose_level implemented.
 * modified layout of compile time output
 *
 * Revision 1.73  1995/11/06  14:15:38  cg
 * added compiler phase 'uniqueness check' and new break option -bq
 *
 * Revision 1.72  1995/11/01  16:23:05  cg
 * Information about implicit types will now always be retrieved.
 *
 * Revision 1.71  1995/11/01  09:41:46  cg
 * added new compiler phase for object handling and
 * new break parameter -be to stop right after it.
 *
 * Revision 1.70  1995/10/31  14:46:36  sbs
 * gcc invocation printed;
 *
 * Revision 1.69  1995/10/26  15:57:39  cg
 * compiler phase 'checkdec` moved in between 'typecheck`
 * and 'impltypes`.
 *
 * Revision 1.68  1995/10/24  13:12:39  cg
 * Now, all file names in error messages are written with "
 *
 * Revision 1.67  1995/10/22  17:34:40  cg
 * new break parameter -bd to stop after checkdec
 * new compiler phase checkdec inserted between parse and
 * import
 * some bugs fixed.
 *
 * Revision 1.66  1995/10/20  09:22:51  cg
 * added compiler phase 'analysis`
 *
 * Revision 1.65  1995/10/18  16:47:58  cg
 * some beautifications
 *
 * Revision 1.62  1995/10/16  12:01:20  cg
 * added new compilation phase 'objinit'.
 * added new break parameter '-bj'.
 *
 * Revision 1.61  1995/10/05  16:02:24  cg
 * new break option -bm to stop after resolving implicit types.
 * resolving implicit types started.
 *
 * Revision 1.60  1995/09/01  11:54:49  sbs
 * cpp exchanged by gcc -E -x c!
 * unfortunately not possible to use gcc -E from stdin
 * to be fixed later!!
 *
 * Revision 1.59  1995/09/01  07:45:58  cg
 * writing of SIB-files integrated.
 * new options -bb and -noSIB added.
 *
 * Revision 1.58  1995/08/16  10:57:55  asi
 * added -ba to break compilation after array elimination
 *
 * Revision 1.57  1995/07/24  11:41:35  asi
 * added ArrayElimination and new parameters
 * -noarray_elimination, -noAE and -minarray <nr> for it
 *
 * Revision 1.56  1995/07/24  09:08:55  hw
 * compilation of SAC-modules inserted
 *
 * Revision 1.55  1995/07/19  18:43:49  asi
 * added new parameter -maxoptcycles and variable max_optcycles
 *
 * Revision 1.54  1995/07/07  14:58:38  asi
 * added loop unswitching - basic version
 *
 * Revision 1.53  1995/06/23  13:57:56  hw
 * added new parameter -maxoverload and variable max_overload
 *
 * Revision 1.52  1995/06/23  10:04:25  sacbase
 * CPP invocation inserted.
 *
 * Revision 1.51  1995/06/15  14:57:37  hw
 * call gcc only if no errors occur before
 *
 * Revision 1.50  1995/06/13  08:31:20  asi
 * added parameter -maxunroll and variables unrnum and opt_unr
 *
 * Revision 1.49  1995/06/09  15:27:36  hw
 * new option '-fcheck_boundary'  inserted
 *
 * Revision 1.48  1995/06/07  15:38:28  hw
 * gcc-option -Wno-unused inserted
 *
 * Revision 1.47  1995/06/06  09:50:44  sbs
 * max... opts inserted.
 *
 * Revision 1.46  1995/06/02  16:42:22  sbs
 * show_idx and sac_optimise inserted
 *
 * Revision 1.45  1995/06/02  09:53:31  sbs
 * -bs, -nopsiopt, -noIDE and PsiOpt call inserted
 *
 * Revision 1.44  1995/05/29  10:04:18  asi
 * shortcuts -noCF -noDCR -noPDCR -noLIR -noINL -nnUNR added
 *
 * Revision 1.43  1995/05/26  14:23:42  asi
 * function inlineing and loop unrolling added
 *
 * Revision 1.42  1995/05/22  12:06:24  sbs
 * tr option inserted
 *
 * Revision 1.41  1995/05/15  08:33:03  asi
 * added option -v no ; this options allows the user to fix the maximum
 * number of new variables generated while optimizing.
 *
 * Revision 1.40  1995/05/04  11:40:51  sbs
 * trace option added
 *
 * Revision 1.39  1995/04/10  11:19:36  sbs
 * options I,L,O & g included
 *
 * Revision 1.38  1995/04/05  17:36:39  sbs
 * adapted to new FindFile
 *
 * Revision 1.37  1995/04/05  16:18:41  sbs
 * gcc invocation debugged
 *
 * Revision 1.36  1995/04/05  15:52:38  asi
 * loop invariant removal added
 *
 * Revision 1.35  1995/04/05  15:31:24  sbs
 * linking phase and -c option inserted
 *
 * Revision 1.34  1995/04/03  14:00:43  sbs
 * FreeTree due to bugs commented out
 *
 * Revision 1.33  1995/04/03  06:19:49  sbs
 * options converted to -b[piftorc] and show_icm inserted
 *
 * Revision 1.32  1995/03/29  11:59:41  hw
 * option -c (compile) inserted
 *
 * Revision 1.31  1995/03/24  15:44:19  asi
 * malloc_debug inserted
 *
 * Revision 1.30  1995/03/17  17:40:37  asi
 * added work reduction
 *
 * Revision 1.29  1995/03/17  15:45:44  hw
 * function Typecheck now returns the syntax_tree
 *
 * Revision 1.28  1995/03/13  17:00:06  asi
 * calls Optimize even if no optimization shall be done
 *
 * Revision 1.27  1995/03/10  11:08:29  hw
 * - Refcount inserted
 * - new parameters -noRC, -r (show refcounts while Print) added
 *
 * Revision 1.26  1995/02/27  15:03:14  hw
 * set default value for 'filename'
 *
 * Revision 1.25  1995/02/15  14:32:05  asi
 * PARM-macro added for ARG 'n'
 *
 * Revision 1.24  1995/02/13  17:21:03  asi
 * parmeters noOPT, noCF and noDCR added
 *
 * Revision 1.23  1995/01/18  17:31:48  asi
 * Added FreeTree ad end of program
 *
 * Revision 1.22  1995/01/16  09:32:06  asi
 * moved output for optimizations from main.c to optimize.c
 *
 * Revision 1.21  1995/01/12  13:11:01  asi
 * output for dead code removal added
 *
 * Revision 1.20  1995/01/09  13:58:04  hw
 * call typecheck only if there are no errors before
 *
 * Revision 1.19  1995/01/05  09:55:17  asi
 * bug removed
 *
 * Revision 1.17  1995/01/03  15:00:47  asi
 * No optimization if errors occures while typechecking
 *
 * Revision 1.16  1994/12/21  13:38:41  sbs
 * some category IV work done...
 *
 * Revision 1.15  1994/12/20  14:11:51  hw
 * changed output
 *
 * Revision 1.14  1994/12/20  11:41:52  sbs
 * bug fixed in previously inserted NOTE
 *
 * Revision 1.13  1994/12/20  11:40:33  sbs
 * NOTE for import inserted.
 *
 * Revision 1.12  1994/12/16  14:23:10  sbs
 * Import inserted
 *
 * Revision 1.10  1994/12/13  11:23:54  hw
 * changed call of NOTE
 * changed call of Error to ERROR2
 *
 * Revision 1.9  1994/12/11  17:29:31  sbs
 * -I, -L + environment vars inserted
 *
 * Revision 1.8  1994/12/09  10:13:19  sbs
 * optimize inserted
 *
 * Revision 1.7  1994/12/08  17:55:18  hw
 * put silent into Error.c
 * added output for niumber of errors and number of warnings
 *
 * Revision 1.6  1994/12/02  12:38:35  sbs
 * Options -pfts revised/ prgname inserted
 *
 * Revision 1.5  1994/12/01  17:36:18  hw
 * added function Typecheck
 * new parameter -f for printing after flatten
 *
 * Revision 1.4  1994/11/22  13:48:29  sbs
 * scnprs.h included in main.c
 *
 * Revision 1.3  1994/11/15  13:26:31  sbs
 * changed SAC_LIBRARY into SAC_LIBRARY_PATH and
 * enabled moltiple paths to be specified
 * seperated by ":"
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 *
 */

/*
 *  this file contains the main function of the SAC->C compiler!
 */

#include "main.h"
#include "tree.h"
#include "free.h"
#include "my_debug.h"
#include "globals.h"
#include "Error.h"
#include "usage.h"
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
#include "concregions.h"
#include "precompile.h"
#include "compile.h"
#include "cccall.h"
#include "Old2NewWith.h"
#include "internal_lib.h"
#include "resource.h"

#include <stdlib.h>
#include <string.h>

/*
 *  And now, the main function which triggers the whole compilation.
 */

MAIN
{
    node *syntax_tree;

    int i;

    int tmp_break = 1;

#ifdef SHOW_MALLOC
    compute_malloc_align_step ();
#endif

    /* initializations */
    InitPaths ();

    compiler_phase = PH_setup;

    /*
     *  The command line is written to a single string.
     */

    strcpy (commandline, argv[0]);

    for (i = 1; i < argc; i++) {
        strcat (commandline, " ");
        strcat (commandline, argv[i]);
    }

    /*
     *  First, we evaluate the given command line options...
     */

    OPT ARG '_' : PARM
    {
        if (!strncmp (*argv, "DBUG", 4)) {
            (*argv) += 4;

            if (**argv != 0) {
                my_dbug_from = (compiler_phase_t)strtol (*argv, argv, 10);

                if (**argv == '/') {
                    (*argv)++;

                    if (**argv != 0) {
                        my_dbug_to = (compiler_phase_t)strtol (*argv, argv, 10);

                        if (**argv == '/') {
                            (*argv)++;

                            if (**argv != 0) {
                                my_dbug_str = StringCopy (*argv);

                                if ((my_dbug_str != NULL) && (my_dbug_str != ""))
                                    my_dbug = 1;
                            } else
                                SYSWARN (("Third _DBUG option is missing"));
                        } else
                            SYSWARN (("Unknown _DBUG option '%s'", *argv));
                    } else
                        SYSWARN (("Second _DBUG option is missing"));
                } else
                    SYSWARN (("Unknown _DBUG option '%s'", *argv));
            } else
                SYSWARN (("First _DBUG option is missing"));
        } else
            SYSWARN (("Unknown command line option '_%s`", *argv));
    }
    NEXTOPT
    ARG 'D' : PARM
    {
        cppvars[num_cpp_vars++] = *argv;
    }
    NEXTOPT
    ARG 'I' : PARM
    {
        AppendPath (MODDEC_PATH, AbsolutePathname (*argv));
    }
    NEXTOPT
    ARG 'L' : PARM
    {
        AppendPath (MODIMP_PATH, AbsolutePathname (*argv));
    }
    NEXTOPT
    ARG 'O' : PARM
    {
        switch (**argv) {
        case '0':
            cc_optimize = 0;
            break;
        case '1':
            cc_optimize = 1;
            break;
        case '2':
            cc_optimize = 2;
            break;
        case '3':
            cc_optimize = 3;
            break;
        default:
            SYSWARN (("Unknown optimize parameter '%s`", *argv));
        }
    }
    NEXTOPT
    ARG 'M':
    {
        if (*++*argv == 0) {
            makedeps = 1;
        } else {
            if (strcmp ((*argv), "lib") == 0) {
                makedeps = 2;
            } else {
                SYSWARN (("Unknown compiler option '-M%s`", *argv));
            }
        }
    }
    NEXTOPT
    ARG 'h':
    {
        usage ();
        exit (0);
    }
    ARG 'b' : PARM
    {
        switch (**argv) {
        case 'u':
            break_after = PH_setup;
            break;
        case 'p':
            break_after = PH_scanparse;
            break;
        case 'j':
        case '5':
            break_after = PH_objinit;
            break;
        case 'i':
        case '3':
            break_after = PH_import;
            break;
        case 'f':
        case '6':
            break_after = PH_flatten;
            break;
        case 't':
        case '7':
            break_after = PH_typecheck;
            break;
        case 'o':
            break_after = PH_sacopt;
            break;
        case 's':
            break_after = PH_psiopt;
            show_idx = 1;
            break;
        case 'r':
            break_after = PH_refcnt;
            show_refcnt = 1;
            break;
        case 'c':
            break_after = PH_compile;
            show_icm = 1;
            break;
        case 'w':
            break_after = PH_writesib;
            break;
        case 'b':
        case '4':
            break_after = PH_readsib;
            break;
        case 'd':
        case '8':
            break_after = PH_checkdec;
            break;
        case 'm':
        case '9':
            break_after = PH_impltype;
            break;
        case 'y':
            break_after = PH_analysis;
            break;
        case 'e':
            break_after = PH_objects;
            break;
        case 'v':
            break_after = PH_rmvoidfun;
            break;
        case 'q':
            break_after = PH_uniquecheck;
            break;
        case 'l':
            break_after = PH_precompile;
            break;
        case '1':
            switch (*(*argv + 1)) {
            case '0':
                break_after = PH_analysis;
                tmp_break = 2;
                break;
            case '1':
                break_after = PH_writesib;
                tmp_break = 2;
                break;
            case '2':
                break_after = PH_objects;
                tmp_break = 2;
                break;
            case '3':
                break_after = PH_uniquecheck;
                tmp_break = 2;
                break;
            case '4':
                break_after = PH_rmvoidfun;
                tmp_break = 2;
                break;
            case '5':
                break_after = PH_sacopt;
                tmp_break = 2;
                break;
            case '6':
                break_after = PH_psiopt;
                tmp_break = 2;
                show_idx = 1;
                break;
            case '7':
                break_after = PH_refcnt;
                tmp_break = 2;
                show_refcnt = 1;
                break;
            case '8':
                break_after = PH_concregions;
                tmp_break = 2;
                break;
            case '9':
                break_after = PH_precompile;
                tmp_break = 2;
                break;
            case '\0':
                break_after = PH_setup;
                break;
            default:
                SYSWARN (("Unknown break parameter '%s`", *argv));
            }
            break;
        case '2':
            switch (*(*argv + 1)) {
            case '0':
                break_after = PH_compile;
                tmp_break = 2;
                show_icm = 1;
                break;
            case '\0':
                break_after = PH_scanparse;
                break;
            default:
                SYSWARN (("Unknown break parameter '%s`", *argv));
            }
            break;
        default:
            SYSWARN (("Unknown break parameter '%s`", *argv));
        }

        if (*(*argv + tmp_break) == ':') {
            strncpy (break_specifier, *argv + tmp_break + 1, MAX_BREAK_SPECIFIER - 1);
        }
    }
    NEXTOPT
    ARG 't' : PARM
    {
        if (0 == strcmp (*argv, "arget")) {
            strcpy (target_name, *(argv + 1));
        } else {

            while (**argv) {
                switch (**argv) {
                case 'a':
                    traceflag = TRACE_ALL;
                    break;
                case 'm':
                    traceflag = traceflag | TRACE_MEM;
                    break;
                case 'r':
                    traceflag = traceflag | TRACE_REF;
                    break;
                case 'u':
                    traceflag = traceflag | TRACE_UDF;
                    break;
                case 'p':
                    traceflag = traceflag | TRACE_PRF;
                    break;
                case 'w':
                    traceflag = traceflag | TRACE_WST;
                    break;
                default:
                    SYSWARN (("Unknown trace flag '%c`", **argv));
                }
                ++*argv;
            }
        }
    }

    NEXTOPT
    ARG 'p' : PARM
    {
        while (**argv) {
            switch (**argv) {
            case 'a':
                profileflag = PROFILE_ALL;
                break;
            case 'f':
                profileflag = profileflag | PROFILE_FUN;
                break;
            case 'i':
                profileflag = profileflag | PROFILE_INL;
                break;
            case 'l':
                profileflag = profileflag | PROFILE_LIB;
                break;
            case 'w':
                profileflag = profileflag | PROFILE_WITH;
                break;
            default:
                SYSWARN (("Unknown profile flag '%c`", **argv));
            }
            ++*argv;
        }
    }
    NEXTOPT
    ARG 'c':
    {
        break_after = PH_genccode;
    }
    ARG 'g':
    {
        cc_debug = 1;
    }
    ARG 'v' : PARM
    {
        switch (**argv) {
        case '0':
            verbose_level = 0;
            break;
        case '1':
            verbose_level = 1;
            break;
        case '2':
            verbose_level = 2;
            break;
        case '3':
            verbose_level = 3;
            break;
        default:
            SYSWARN (("Unknown verbose level '%s`", *argv));
        }
    }
    NEXTOPT

    ARG 'n' : PARM
    {
        if (OptCmp (*argv, "oOPT")) {
            optimize = 0;
            opt_dcr = 0;
            opt_dfr = 0;
            opt_cf = 0;
            opt_lir = 0;
            opt_inl = 0;
            opt_unr = 0;
            opt_uns = 0;
            opt_cse = 0;
            opt_wlf = 0;
            opt_ae = 0;
            opt_ive = 0;
            opt_rco = 0;
        }

        else if (OptCmp (*argv, "oDCR"))
            opt_dcr = 0;
        else if (OptCmp (*argv, "oCF"))
            opt_cf = 0;
        else if (OptCmp (*argv, "oDFR"))
            opt_dfr = 0;
        else if (OptCmp (*argv, "oLIR"))
            opt_lir = 0;
        else if (OptCmp (*argv, "oINL"))
            opt_inl = 0;
        else if (OptCmp (*argv, "oUNR"))
            opt_unr = 0;
        else if (OptCmp (*argv, "oUNS"))
            opt_uns = 0;
        else if (OptCmp (*argv, "oIVE"))
            opt_ive = 0;
        else if (OptCmp (*argv, "oAE"))
            opt_ae = 0;
        else if (OptCmp (*argv, "oCSE"))
            opt_cse = 0;
        else if (OptCmp (*argv, "oWLF"))
            opt_wlf = 0;
        else if (OptCmp (*argv, "oRCO"))
            opt_rco = 0;
        else
            SYSWARN (("Unknown compiler option '-n%s`", *argv));
    }
    NEXTOPT
    ARG 'm' : PARM
    {
        if (!strncmp (*argv, "axoptvar", 8)) {
            ++argv;
            --argc;
            optvar = atoi (*argv);
        } else {
            if (!strncmp (*argv, "axinline", 8)) {
                ++argv;
                --argc;
                inlnum = atoi (*argv);
            } else {
                if (!strncmp (*argv, "axunroll", 8)) {
                    ++argv;
                    --argc;
                    unrnum = atoi (*argv);
                } else {
                    if (!strncmp (*argv, "axoverload", 10)) {
                        ++argv;
                        --argc;
                        max_overload = atoi (*argv);
                    } else {
                        if (!strncmp (*argv, "axoptcycles", 10)) {
                            ++argv;
                            --argc;
                            max_optcycles = atoi (*argv);
                        } else {
                            if (!strncmp (*argv, "inarray", 10)) {
                                ++argv;
                                --argc;
                                minarray = atoi (*argv);
                            }
                        }
                    }
                }
            }
        }
    }
    NEXTOPT
    ARG 'o' : PARM
    {
        strcpy (outfilename, *argv);
        /*
         * The option is only stored in outfilename,
         * the correct settings of the global variables
         * outfilename, cfilename, and targetdir will be done
         * in SetFileNames() in scnprs.c. This cannot be done
         * because you have to know the kind of file (program
         * or module/class implementation).
         */
    }
    NEXTOPT
    ARG 'd' : PARM
    {
        if (!strncmp (*argv, "check_boundary", 14))
            check_boundary = 1;
        else if (!strncmp (*argv, "CB", 2))
            check_boundary = 1;
        else if (!strncmp (*argv, "check_malloc", 12))
            check_malloc = 1;
        else if (!strncmp (*argv, "CM", 2))
            check_malloc = 1;
        else if (!strncmp (*argv, "nocleanup", 9))
            cleanup = 0;
        else if (!strncmp (*argv, "NC", 2))
            cleanup = 0;
        else
            SYSWARN (("Unknown debug option '-d%s`", *argv));
    }
    NEXTOPT
    ARG 'l' : PARM
    {
        switch (**argv) {
        case '1':
            linkstyle = 1;
            break;
        case '2':
            linkstyle = 2;
            break;
        default:
            if (!strncmp (*argv, "ibstat", 6))
                libstat = 1;
            else
                SYSWARN (("Unknown command line option '-l%s`", *argv));
        }
    }
    NEXTOPT
    ARG '2':
    {
        Make_Old2NewWith = 1;
    }
    OTHER
    {
        SYSWARN (("Unknown command line option '%s`", *argv));
    }
    ENDOPT

    if (argc == 1) {
        strcpy (sacfilename, *argv);
    }

    /*
     * Now, we read in the sac2c configuration files.
     */

    ABORT_ON_ERROR;

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

    tmp_dirname = tempnam (NULL, "SAC_");

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

    if (Make_Old2NewWith)
        syntax_tree = Old2NewWith (syntax_tree); /* o2nWith_tab */

    NOTE_COMPILER_PHASE;
    CHECK_DBUG_START;
    syntax_tree = ConcRegions (syntax_tree); /* concregions_tab */
    CHECK_DBUG_STOP;
    ABORT_ON_ERROR;

    if (break_after == PH_concregions)
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

    return (0);

BREAK:

    if (compiler_phase >= PH_scanparse)
        Print (syntax_tree);
    else
        RSCShowResources ();

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
