/*
 *
 * $Log$
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
#include "wltransform.h"
#include "concurrent.h"
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
    size_t num_len;

    int tmp_break = 1;

#ifdef SHOW_MALLOC
    ComputeMallocAlignStep ();
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
        AppendPath (SYSTEMLIB_PATH, AbsolutePathname (*argv));
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
    ARG 'i':
    {
        if (0 == strcmp (*argv, "intrinsic")) {
            if (--argc >= 1) {
                argv++;
                while (**argv) {
                    switch (**argv) {
                    case 'a':
                        intrinsics = INTRINSIC_ALL;
                        break;
                    case '+':
                        intrinsics |= INTRINSIC_ADD;
                        break;
                    case '-':
                        intrinsics |= INTRINSIC_SUB;
                        break;
                    case 'x':
                        intrinsics |= INTRINSIC_MUL;
                        break;
                    case '/':
                        intrinsics |= INTRINSIC_DIV;
                        break;
                    case 'o':
                        intrinsics |= INTRINSIC_TO;
                        break;
                    case 't':
                        intrinsics |= INTRINSIC_TAKE;
                        break;
                    case 'd':
                        intrinsics |= INTRINSIC_DROP;
                        break;
                    case 'c':
                        intrinsics |= INTRINSIC_CAT;
                        break;
                    case 'r':
                        intrinsics |= INTRINSIC_ROT;
                        break;
                    default:
                        SYSWARN (("Unknown intrinsic flag '%c`", **argv));
                    }
                    ++*argv;
                }
            } else {
                SYSERROR (("Missing intrinsic operation specification"));
            }
        } else {
            copyright ();
            exit (0);
        }
    }
    NEXTOPT
    ARG 'b' : PARM
    {
        switch (**argv) {
        case 'u':
            break_after = PH_setup;
            break;
        case 'p':
            break_after = PH_scanparse;
            break;
        case 'i':
        case '3':
            break_after = PH_import;
            break;
        case 'b':
        case '4':
            break_after = PH_readsib;
            break;
        case 'j':
        case '5':
            break_after = PH_objinit;
            break;
        case 'f':
        case '6':
            break_after = PH_flatten;
            break;
        case 't':
        case '7':
            break_after = PH_typecheck;
            break;
        case 'd':
        case '8':
            break_after = PH_checkdec;
            break;
        case 'm':
        case '9':
            break_after = PH_impltype;
            break;
        case 'a':
            break_after = PH_analysis;
            break;
        case 'w':
            break_after = PH_writesib;
            break;
        case 'e':
            break_after = PH_objects;
            break;
        case 'q':
            break_after = PH_uniquecheck;
            break;
        case 'v':
            break_after = PH_rmvoidfun;
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
        case 'n':
            break_after = PH_wltrans;
            break;
        case 'y':
            break_after = PH_spmdregions;
            break;
        case 'l':
            break_after = PH_precompile;
            break;
        case 'c':
            break_after = PH_compile;
            show_icm = 1;
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
                break_after = PH_wltrans;
                tmp_break = 2;
                break;
            case '9':
                break_after = PH_spmdregions;
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
                break_after = PH_precompile;
                tmp_break = 2;
                break;
            case '1':
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
            if (strncmp (*argv + tmp_break + 1, "cyc", 3) == 0) {
                num_len = strspn (*argv + tmp_break + 4, "0123456789");
                strncpy (break_specifier, *argv + tmp_break + 4, num_len);

                break_cycle_specifier = atoi (break_specifier);
                strncpy (break_specifier, *argv + tmp_break + 4 + num_len + 1,
                         MAX_BREAK_SPECIFIER - 1);
            } else {
                break_cycle_specifier = 0;
                strncpy (break_specifier, *argv + tmp_break + 1, MAX_BREAK_SPECIFIER - 1);
            }
        } else {
            /* prevent any unintended breaks if PH_sacopt is set ! */
            break_cycle_specifier = -1;
        }
        DBUG_PRINT ("ARGS", ("break_cycle_specifier set to %d", break_cycle_specifier));
        DBUG_PRINT ("ARGS", ("break_specifier set to %s", break_specifier));
    }
    NEXTOPT
    ARG 't' : PARM
    {
        if (0 == strcmp (*argv, "arget")) {
            if (--argc >= 1)
                strcpy (target_name, *(++argv));
            else
                SYSERROR (("Missing target parameter"));
        } else {
            if (0 == strcmp (*argv, "race")) {
                if (--argc >= 1) {
                    argv++;
                    while (**argv) {
                        switch (**argv) {
                        case 'a':
                            traceflag = TRACE_ALL;
                            break;
                        case 'm':
                            traceflag |= TRACE_MEM;
                            break;
                        case 'r':
                            traceflag |= TRACE_REF;
                            break;
                        case 'f':
                            traceflag |= TRACE_FUN;
                            break;
                        case 'p':
                            traceflag |= TRACE_PRF;
                            break;
                        case 'o':
                            traceflag |= TRACE_OWL;
                            break;
                        case 'w':
                            traceflag |= TRACE_WL;
                            break;
                        case 't':
                            traceflag |= TRACE_MT;
                            break;
                        default:
                            SYSWARN (("Unknown trace flag '%c`", **argv));
                        }
                        ++*argv;
                    }
                } else
                    SYSERROR (("Missing trace specification"));
            }
        }
    }
    NEXTOPT
    ARG 'p' : PARM
    {
        if (0 == strcmp (*argv, "rofile")) {
            if (--argc >= 1) {
                argv++;
                while (**argv) {
                    switch (**argv) {
                    case 'a':
                        profileflag = PROFILE_ALL;
                        break;
                    case 'f':
                        profileflag |= PROFILE_FUN;
                        break;
                    case 'i':
                        profileflag |= PROFILE_INL;
                        break;
                    case 'l':
                        profileflag |= PROFILE_LIB;
                        break;
                    case 'w':
                        profileflag |= PROFILE_WITH;
                        break;
                    default:
                        SYSWARN (("Unknown profile flag '%c`", **argv));
                    }
                    ++*argv;
                }
            } else
                SYSERROR (("Missing profile specification"));
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
            opt_lunr = 0;
            opt_wlunr = 0;
            opt_uns = 0;
            opt_cse = 0;
            opt_wlt = 0;
            opt_wlf = 0;
            opt_ae = 0;
            opt_ive = 0;
            opt_rco = 0;
            opt_tile = 0;
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
        else if (OptCmp (*argv, "oLUNR"))
            opt_lunr = 0;
        else if (OptCmp (*argv, "oWLUNR"))
            opt_wlunr = 0;
        else if (OptCmp (*argv, "oUNS"))
            opt_uns = 0;
        else if (OptCmp (*argv, "oIVE"))
            opt_ive = 0;
        else if (OptCmp (*argv, "oAE"))
            opt_ae = 0;
        else if (OptCmp (*argv, "oCSE"))
            opt_cse = 0;
        else if (OptCmp (*argv, "oWLT")) {
            opt_wlt = 0;
            opt_wlf = 0;
        }

        else if (OptCmp (*argv, "oWLF"))
            opt_wlf = 0;
        else if (OptCmp (*argv, "oRCO"))
            opt_rco = 0;
        else if (OptCmp (*argv, "oUIP"))
            opt_uip = 0;
        else if (OptCmp (*argv, "oTILE"))
            opt_tile = 0;
        else
            SYSWARN (("Unknown compiler option '-n%s`", *argv));
    }
    NEXTOPT
    ARG 'm' : PARM
    {
        if (0 == strncmp (*argv, "axoptvar", 8)) {
            ++argv;
            --argc;
            optvar = atoi (*argv);
        } else if (0 == strncmp (*argv, "axinline", 8)) {
            ++argv;
            --argc;
            inlnum = atoi (*argv);
        } else if (0 == strncmp (*argv, "axunroll", 8)) {
            ++argv;
            --argc;
            unrnum = atoi (*argv);
        } else if (0 == strncmp (*argv, "axwlunroll", 10)) {
            ++argv;
            --argc;
            wlunrnum = atoi (*argv);
        } else if (0 == strncmp (*argv, "axoverload", 10)) {
            ++argv;
            --argc;
            max_overload = atoi (*argv);
        } else if (0 == strncmp (*argv, "axoptcycles", 10)) {
            ++argv;
            --argc;
            max_optcycles = atoi (*argv);
        } else if (0 == strncmp (*argv, "inarray", 10)) {
            ++argv;
            --argc;
            minarray = atoi (*argv);
        } else if (0 == strncmp (*argv, "tstatic", 7)) {
            ++argv;
            --argc;
            num_threads = atoi (*argv);
            max_threads = num_threads;
            gen_mt_code = 1;
        } else if (0 == strncmp (*argv, "tdynamic", 8)) {
            ++argv;
            --argc;
            max_threads = atoi (*argv);
            num_threads = 0;
            gen_mt_code = 1;
        } else if (0 == strncmp (*argv, "tall", 4)) {
            ++argv;
            --argc;
            max_threads = atoi (*argv);
            num_threads = max_threads;
            all_threads = max_threads;
            gen_mt_code = 1;
        } else if (0 == strncmp (*argv, "axsyncfold", 10)) {
            ++argv;
            --argc;
            max_sync_fold = atoi (*argv);
        } else if (0 == strncmp (*argv, "inparsize", 9)) {
            ++argv;
            --argc;
            min_parallel_size = atoi (*argv);
        } else {
            SYSWARN (("Unknown command line option '-m%s`", *argv));
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
        if (0 == strcmp (*argv, "check_boundary"))
            check_boundary = 1;
        else if (0 == strcmp (*argv, "CB"))
            check_boundary = 1;
        else if (0 == strcmp (*argv, "check_malloc"))
            check_malloc = 1;
        else if (0 == strcmp (*argv, "CM"))
            check_malloc = 1;
        else if (0 == strcmp (*argv, "nocleanup"))
            cleanup = 0;
        else if (0 == strcmp (*argv, "NC"))
            cleanup = 0;
        else if (0 == strcmp (*argv, "show_syscall"))
            show_syscall = 1;
        else if (0 == strcmp (*argv, "SC"))
            show_syscall = 1;
        else if (0 == strcmp (*argv, "cccall")) {
            gen_cccall = 1;
            cleanup = 0;
        } else if (0 == strcmp (*argv, "CC")) {
            gen_cccall = 1;
            cleanup = 0;
        } else
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
            if (0 == strncmp (*argv, "ibstat", 6))
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

        puresacfilename = strrchr (sacfilename, '/');

        if (puresacfilename == NULL) {
            puresacfilename = sacfilename;
        } else {
            puresacfilename += 1;
        }
    } else {
        if (argc == 0) {
            puresacfilename = "stdin";
        } else {
            SYSERROR (("Too many source files specified on command line"));
        }
    }

#ifdef LINUX_X86
    if (gen_mt_code) {
        SYSABORT (("Sorry, multi-threaded execution is not yet supported on "
                   "target platform %s",
                   target_platform));
#endif /* LINUX_X86 */

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

        if (gen_mt_code == 1) {
            NOTE_COMPILER_PHASE;
            CHECK_DBUG_START;
            syntax_tree = BuildSpmdRegions (syntax_tree); /* spmd..._tab, sync..._tab */
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

        return (0);

    BREAK:

        if (compiler_phase >= PH_scanparse) {
            if (compiler_phase < PH_genccode) {
                Print (syntax_tree);
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
