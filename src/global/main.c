/*
 *
 * $Log$
 * Revision 1.79  1995/12/29 10:21:43  cg
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
 * Revision 1.64  1995/10/18  13:45:49  cg
 * *** empty log message ***
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
 * Revision 1.18  1995/01/03  15:09:02  asi
 * *** empty log message ***
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
 * Revision 1.11  1994/12/15  16:37:49  asi
 * *** empty log message ***
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
#include "trace.h"
#include "compile.h"
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
#include "precompile.h"

#include <stdlib.h>
#include <string.h>

extern int malloc_debug (int level);

FILE *outfile;

char filename[MAX_FILE_NAME];         /* used for error messages and others */
char sibfilename[MAX_FILE_NAME] = ""; /* used for error messages and others */

int optimize = 1;
int sac_optimize = 1;
int opt_dcr = 1, opt_cf = 1, opt_wr = 1, opt_lir = 1, opt_inl = 1, opt_unr = 1,
    opt_uns = 1, opt_ae = 1;

int optvar = 50;
int inlnum = 1;
int unrnum = 2;
int minarray = 4;

int max_overload = 10;
int max_optcycles = 4;
int psi_optimize = 1;
int psi_opt_ive = 1;

int show_refcnt = 0;
int show_idx = 0;
int show_icm = 0;
int traceflag = 0;

int check_boundary = 0;
int linkstyle = 0;
int breakae = 0;

MAIN
{
    int set_outfile = 0;
    int Ccodeonly = 0;
    int breakparse = 0, breakimport = 0, breakflatten = 0, breaktype = 0, breakopt = 0,
        breakpsiopt = 0, breakref = 0, breakreadsib = 0, breakwritesib = 0,
        breakimpltype = 0, breakobjinit = 0, breakanalysis = 0, breakcheckdec = 0,
        breakobjects = 0, breakuniquecheck = 0, breakrmvoidfun = 0, breakprecompile = 0;

    char prgname[MAX_FILE_NAME];
    char outfilename[MAX_FILE_NAME];
    char cfilename[MAX_FILE_NAME];
    char cccallstr[MAX_PATH_LEN];
    char ccflagsstr[MAX_FILE_NAME] = "";
    char *pathname = NULL;

    malloc_debug (0);
    strcpy (prgname, argv[0]);
    strcpy (filename, prgname);

    /*
     *  First, we scan the given options...
     */

    OPT ARG 'I' : PARM
    {
        AppendPath (MODDEC_PATH, *argv);
    }
    NEXTOPT
    ARG 'L' : PARM
    {
        AppendPath (MODIMP_PATH, *argv);
    }
    NEXTOPT
    ARG 'O' : PARM
    {
        switch (**argv) {
        case '1':
            strcat (ccflagsstr, "-O1 ");
            break;
        case '2':
            strcat (ccflagsstr, "-O2 ");
            break;
        case '3':
            strcat (ccflagsstr, "-O3 ");
            break;
        default:
            SYSWARN (("Unknown optimize parameter '%s`", *argv));
        }
    }
    NEXTOPT
    ARG 'h':
    {
        usage (prgname);
        exit (0);
    }
    ARG 'b' : PARM
    {
        Ccodeonly = 1;
        switch (**argv) {
        case 'p':
            breakparse = 1;
            break;
        case 'j':
            breakobjinit = 1;
            break;
        case 'i':
            breakimport = 1;
            break;
        case 'f':
            breakflatten = 1;
            break;
        case 't':
            breaktype = 1;
            break;
        case 'o':
            breakopt = 1;
            break;
        case 'a':
            breakae = 1;
            breakopt = 1;
            break;
        case 's':
            breakpsiopt = 1;
            show_idx = 1;
            break;
        case 'r':
            breakref = 1;
            show_refcnt = 1;
            break;
        case 'c':
            show_icm = 1;
            break;
        case 'w':
            breakwritesib = 1;
            break;
        case 'b':
            breakreadsib = 1;
            break;
        case 'd':
            breakcheckdec = 1;
            break;
        case 'm':
            breakimpltype = 1;
            break;
        case 'y':
            breakanalysis = 1;
            break;
        case 'e':
            breakobjects = 1;
            break;
        case 'v':
            breakrmvoidfun = 1;
            break;
        case 'q':
            breakuniquecheck = 1;
            break;
        case 'l':
            breakprecompile = 1;
            break;
        default:
            SYSWARN (("Unknown break parameter '%s`", *argv));
        }
    }
    NEXTOPT
    ARG 't' : PARM
    {
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
    NEXTOPT
    ARG 'c':
    {
        Ccodeonly = 1;
    }
    ARG 'g':
    {
        strcat (ccflagsstr, "-g ");
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
    ARG 'l' : PARM
    {
        switch (**argv) {
        case '0':
            linkstyle = 0;
            break;
        case '1':
            linkstyle = 1;
            break;
        case '2':
            linkstyle = 2;
            break;
        default:
            SYSWARN (("Unknown link style '%s`", *argv));
        }
    }
    NEXTOPT

    ARG 'n' : PARM
    {
        if (!strncmp (*argv, "oDCR", 4))
            opt_dcr = 0;
        if (!strncmp (*argv, "odead_code_removal", 18))
            opt_dcr = 0;
        if (!strncmp (*argv, "oCF", 3))
            opt_cf = 0;
        if (!strncmp (*argv, "oconstant_folding", 17))
            opt_cf = 0;
        if (!strncmp (*argv, "oPDCR", 5))
            opt_wr = 0;
        if (!strncmp (*argv, "opartial_dead_code_removal", 26))
            opt_wr = 0;
        if (!strncmp (*argv, "oSACOPT", 7))
            sac_optimize = 0;
        if (!strncmp (*argv, "osacopt", 7))
            sac_optimize = 0;
        if (!strncmp (*argv, "oOPT", 4)) {
            optimize = 0;
            sac_optimize = 0;
            psi_optimize = 0;
        }

        if (!strncmp (*argv, "oopt", 4)) {
            optimize = 0;
            sac_optimize = 0;
            psi_optimize = 0;
        }

        if (!strncmp (*argv, "oLIR", 4))
            opt_lir = 0;
        if (!strncmp (*argv, "oloop_invariant_removal", 23))
            opt_lir = 0;
        if (!strncmp (*argv, "oINL", 4))
            opt_inl = 0;
        if (!strncmp (*argv, "oinline_functions", 17))
            opt_inl = 0;
        if (!strncmp (*argv, "oUNR", 4))
            opt_unr = 0;
        if (!strncmp (*argv, "ounroll_loops", 13))
            opt_unr = 0;
        if (!strncmp (*argv, "oUNS", 4))
            opt_uns = 0;
        if (!strncmp (*argv, "ounswitch_loops", 15))
            opt_uns = 0;
        if (!strncmp (*argv, "oPSIOPT", 7))
            psi_optimize = 0;
        if (!strncmp (*argv, "opsiopt", 7))
            psi_optimize = 0;
        if (!strncmp (*argv, "oindex_vect_elimination", 23))
            psi_opt_ive = 0;
        if (!strncmp (*argv, "oIVE", 3))
            psi_opt_ive = 0;
        if (!strncmp (*argv, "oarray_elimination", 19))
            opt_ae = 0;
        if (!strncmp (*argv, "oAE", 3))
            opt_ae = 0;
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
        strcpy (cfilename, *argv);
        strcat (cfilename, ".c");
        set_outfile = 1;
    }
    NEXTOPT
    ARG 'f' : PARM
    {
        if (!strncmp (*argv, "check_boundary", 13))
            check_boundary = 1;
        if (!strncmp (*argv, "CB", 2))
            check_boundary = 1;
    }
    NEXTOPT
    OTHER
    {
        SYSWARN (("Unknown command line option '%s`", *argv));
    }
    ENDOPT

    /* Now, we set our search paths for the source program, modul declarations,
     * and modul implementations...
     */

    NOTE_COMPILER_PHASE;

    if (AppendEnvVar (MODDEC_PATH, "SAC_DEC_PATH") == 0)
        SYSABORT (("MAX_PATH_LEN too low"));
    if (AppendEnvVar (MODIMP_PATH, "SAC_LIBRARY_PATH") == 0)
        SYSABORT (("MAX_PATH_LEN too low"));
    if (AppendEnvVar (PATH, "SAC_PATH") == 0)
        SYSABORT (("MAX_PATH_LEN too low"));

    strcpy (filename, "stdin"); /*default value */

#ifdef NO_CPP

    if (argc == 1) {
        pathname = FindFile (PATH, *argv);
        yyin = fopen (pathname, "r");
        strcpy (filename, *argv);
        if (yyin == NULL) {
            SYSABORT (("Unable to open file \"%s\"", *argv));
        }
    }

#else /* NO_CPP */

    if (argc == 1) {
        pathname = FindFile (PATH, *argv);
        sprintf (cccallstr, "gcc -E -P -C -x c %s", pathname);
        strcpy (filename, *argv);
    } else {
        sprintf (cccallstr, "cpp -P -C ");
    }

    yyin = popen (cccallstr, "r");

#endif /* NO_CPP */

    DBUG_EXECUTE ("MEMVERIFY", malloc_debug (2););

    if (Ccodeonly) {
        if (set_outfile) {
            outfile = fopen (outfilename, "w");
            if (outfile == NULL) {
                SYSABORT (("Unable to open file \"%s\" for writing", outfilename));
            }
        } else
            outfile = stdout;
    }

    else {
        if (!set_outfile) {
            strcpy (outfilename, "a.out");
            strcpy (cfilename, "a.out.c");
        }
        outfile = fopen (cfilename, "w");
        if (outfile == NULL)
            SYSABORT (("Unable to open file \"%s\" for writing", cfilename));
    }

    ABORT_ON_ERROR;

    if (pathname != NULL) {
        NOTE (("Parsing file \"%s\" ...", pathname));
    }

    start_token = PARSE_PRG;
    yyparse ();
    ABORT_ON_ERROR;
    compiler_phase++;

    if (!breakparse) {
        if (MODUL_IMPORTS (syntax_tree) != NULL) {
            NOTE_COMPILER_PHASE;
            syntax_tree = Import (syntax_tree);
            ABORT_ON_ERROR;
        }
        compiler_phase++;

        if (!breakimport) {
            if (MODUL_STORE_IMPORTS (syntax_tree) != NULL) {
                NOTE_COMPILER_PHASE;
                syntax_tree = ReadSib (syntax_tree);
                ABORT_ON_ERROR;
            }
            compiler_phase++;

            if (!breakreadsib) {
                if (MODUL_OBJS (syntax_tree) != NULL) {
                    NOTE_COMPILER_PHASE;
                    syntax_tree = objinit (syntax_tree);
                    ABORT_ON_ERROR;
                }
                compiler_phase++;

                if (!breakobjinit) {
                    NOTE_COMPILER_PHASE;
                    syntax_tree = Flatten (syntax_tree);
                    ABORT_ON_ERROR;
                    compiler_phase++;

                    if (!breakflatten) {
                        NOTE_COMPILER_PHASE;
                        syntax_tree = Typecheck (syntax_tree);
                        ABORT_ON_ERROR;
                        compiler_phase++;

                        if (!breaktype) {
                            if (MODUL_FILETYPE (syntax_tree) != F_prog) {
                                NOTE_COMPILER_PHASE;
                                syntax_tree = CheckDec (syntax_tree);
                                ABORT_ON_ERROR;
                            }
                            compiler_phase++;

                            if (!breakcheckdec) {
                                NOTE_COMPILER_PHASE;
                                syntax_tree = RetrieveImplicitTypeInfo (syntax_tree);
                                ABORT_ON_ERROR;
                                compiler_phase++;

                                if (!breakimpltype) {
                                    NOTE_COMPILER_PHASE;
                                    syntax_tree = Analysis (syntax_tree);
                                    ABORT_ON_ERROR;
                                    compiler_phase++;

                                    if (!breakanalysis) {
                                        if (MODUL_FILETYPE (syntax_tree) != F_prog) {
                                            NOTE_COMPILER_PHASE;
                                            syntax_tree = WriteSib (syntax_tree);
                                            ABORT_ON_ERROR;
                                        }
                                        compiler_phase++;

                                        if (!breakwritesib) {
                                            NOTE_COMPILER_PHASE;
                                            syntax_tree = HandleObjects (syntax_tree);
                                            ABORT_ON_ERROR;
                                            compiler_phase++;

                                            if (!breakobjects) {
                                                NOTE_COMPILER_PHASE;
                                                syntax_tree
                                                  = UniquenessCheck (syntax_tree);
                                                ABORT_ON_ERROR;
                                                compiler_phase++;

                                                if (!breakuniquecheck) {
                                                    NOTE_COMPILER_PHASE;
                                                    syntax_tree
                                                      = RemoveVoidFunctions (syntax_tree);
                                                    ABORT_ON_ERROR;
                                                    compiler_phase++;

                                                    if (!breakrmvoidfun) {
                                                        if (sac_optimize) {
                                                            NOTE_COMPILER_PHASE;
                                                            syntax_tree
                                                              = Optimize (syntax_tree);
                                                            ABORT_ON_ERROR;
                                                        }
                                                        compiler_phase++;

                                                        if (!breakopt) {
                                                            if (psi_optimize) {
                                                                NOTE_COMPILER_PHASE;
                                                                syntax_tree
                                                                  = PsiOpt (syntax_tree);
                                                                ABORT_ON_ERROR;
                                                            }
                                                            compiler_phase++;

                                                            if (!breakpsiopt) {
                                                                NOTE_COMPILER_PHASE;
                                                                syntax_tree = Refcount (
                                                                  syntax_tree);
                                                                ABORT_ON_ERROR;
                                                                compiler_phase++;

                                                                if (!breakref) {
                                                                    NOTE_COMPILER_PHASE;
                                                                    syntax_tree
                                                                      = precompile (
                                                                        syntax_tree);
                                                                    ABORT_ON_ERROR;
                                                                    compiler_phase++;

                                                                    if (
                                                                      !breakprecompile) {
                                                                        NOTE_COMPILER_PHASE;
                                                                        syntax_tree
                                                                          = Compile (
                                                                            syntax_tree);
                                                                        ABORT_ON_ERROR;
                                                                        compiler_phase++;

                                                                        if (!Ccodeonly) {
                                                                            NOTE_COMPILER_PHASE;
                                                                            if (
                                                                              MODUL_FILETYPE (
                                                                                syntax_tree)
                                                                              == F_prog) {
                                                                                sprintf (
                                                                                  cccallstr,
                                                                                  "gcc "
                                                                                  "%s-"
                                                                                  "Wall "
                                                                                  "-Wno-"
                                                                                  "unused"
                                                                                  " -I "
                                                                                  "$RCSRO"
                                                                                  "OT/"
                                                                                  "src/"
                                                                                  "compil"
                                                                                  "e/"
                                                                                  " -o "
                                                                                  "%s %s "
                                                                                  "%s",
                                                                                  ccflagsstr,
                                                                                  outfilename,
                                                                                  cfilename,
                                                                                  GenLinkerList ());
                                                                            } else {
                                                                                if (
                                                                                  (MODUL_FILETYPE (
                                                                                     syntax_tree)
                                                                                   == F_modimp)
                                                                                  || (MODUL_FILETYPE (
                                                                                        syntax_tree)
                                                                                      == F_classimp)) {
                                                                                    sprintf (
                                                                                      cccallstr,
                                                                                      "gc"
                                                                                      "c "
                                                                                      "%s"
                                                                                      "-W"
                                                                                      "al"
                                                                                      "l "
                                                                                      "-W"
                                                                                      "no"
                                                                                      "-u"
                                                                                      "nu"
                                                                                      "se"
                                                                                      "d "
                                                                                      "-I"
                                                                                      " $"
                                                                                      "RC"
                                                                                      "SR"
                                                                                      "OO"
                                                                                      "T/"
                                                                                      "sr"
                                                                                      "c/"
                                                                                      "co"
                                                                                      "mp"
                                                                                      "il"
                                                                                      "e/"
                                                                                      " -"
                                                                                      "o "
                                                                                      "%s"
                                                                                      ".o"
                                                                                      " -"
                                                                                      "c "
                                                                                      "%"
                                                                                      "s",
                                                                                      ccflagsstr,
                                                                                      (NULL
                                                                                       != module_name)
                                                                                        ? module_name
                                                                                        : outfilename,
                                                                                      cfilename);
                                                                                } else {
                                                                                    DBUG_ASSERT (
                                                                                      0,
                                                                                      "wr"
                                                                                      "on"
                                                                                      "g "
                                                                                      "va"
                                                                                      "lu"
                                                                                      "e "
                                                                                      "of"
                                                                                      " k"
                                                                                      "in"
                                                                                      "d_"
                                                                                      "of"
                                                                                      "_f"
                                                                                      "il"
                                                                                      "e"
                                                                                      " ");
                                                                                }
                                                                            }
                                                                            ABORT_ON_ERROR;
                                                                            compiler_phase
                                                                              = 0;
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    Print (syntax_tree);

    if (outfile != stdout) {
        fclose (outfile);
    }

    FreeTree (syntax_tree);

    NEWLINE (2);
    NOTE2 (("*** Compilation successful ***"));
    NOTE2 (("*** Exit code 0"));
    NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
    NEWLINE (2);

    if (!Ccodeonly) {
        NOTE2 (("*** Invoking C-compiler:"));
        NOTE2 (("%s", cccallstr));
        NEWLINE (2);

        system (cccallstr);
    }

    return (0);
}
