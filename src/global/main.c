/*
 *
 * $Log$
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
#include "my_debug.h"

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
#include "profile.h"
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
#include "cccall.h"

#include <stdlib.h>
#include <string.h>

/*
 *  Global variables to store command line parameters
 */

FILE *outfile;
/* stream to write result to */

char sacfilename[MAX_FILE_NAME] = "";
/* name of file to be compiled */

char prgname[MAX_FILE_NAME];
/* name of the compiler, e.g. sac2c */

char outfilename[MAX_FILE_NAME] = "";
/* name of executable    */

char modulename[MAX_FILE_NAME] = "";
/* name of module/class which is compiled    */

char cfilename[MAX_FILE_NAME];
/* name of C source code file       */

char targetdir[MAX_FILE_NAME];
/* name of C source code file       */

char ccflagsstr[MAX_FILE_NAME] = "";
/* flags which are handed to gcc    */

char commandline[MAX_PATH_LEN] = "";
/* command line, used for diagnostic output (status report)  */

file_type filetype;
/* kind of file: F_PROG, F_MODIMP or F_CLASSIMP */

char *cppvars[MAX_CPP_VARS];
int num_cpp_vars = 0;

int Ccodeonly = 0;
int break_compilation = 0;

int optimize = 1;
int sac_optimize = 1;
int opt_dcr = 1, opt_cf = 1, opt_wr = 1, opt_lir = 1, opt_inl = 1, opt_unr = 1,
    opt_uns = 1, opt_ae = 1, opt_cse = 1, opt_dfr = 1;

int optvar = 50;
int inlnum = 1;
int unrnum = 2;
int minarray = 4;

int max_overload = 10;
int max_optcycles = 4;
int psi_optimize = 1;
int psi_opt_ive = 1;

int opt_rco = 1;
int show_refcnt = 0;
int show_idx = 0;
int show_icm = 0;
int traceflag = 0;
int profileflag = 0;
int check_malloc = 0;
int breakae = 0;
int check_boundary = 0;
int cleanup = 1;
int linkstyle = 2;
int useranlib = 1;
int libstat = 0;
int makedeps = 0;

int print_objdef_for_header_file = 0;
/*
 *  This global variable serves a very special purpose.
 *  When generating separate C-files for functions and global variables,
 *  a header file is required which contains declarations of them all.
 *  In this case the ICM ND_KS_DECL_GLOBAL_ARRAY must be written
 *  differently. This global variable triggers the respective print
 *  function defined in icm2c.c. It is set by PrintModul.
 */

int function_counter = 1;
/*
 *  This global variable is used whenever the functions of a module or
 *  class are written to separate files.
 */

deps *dependencies = NULL;
/*
 *  This global variable is used to store dependencies during the
 *  whole compilation process.
 *
 *  The dependency table is built by import.c and readsib.c and used
 *  for several purposes, such as generating makefiles or link lists.
 */

/*
 *  And now, the main function which triggers the whole compilation.
 */

MAIN
{
    int breakparse = 0, breakimport = 0, breakflatten = 0, breaktype = 0, breakopt = 0,
        breakpsiopt = 0, breakref = 0, breakreadsib = 0, breakwritesib = 0,
        breakimpltype = 0, breakobjinit = 0, breakanalysis = 0, breakcheckdec = 0,
        breakobjects = 0, breakuniquecheck = 0, breakrmvoidfun = 0, breakprecompile = 0,
        breakcompile = 0;

    node *syntax_tree;

    int i;

    InitPaths ();

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

    /*  NOTE_COMPILER_PHASE; */

    strcpy (prgname, argv[0]);

    OPT ARG 'D' : PARM
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
        usage (prgname);
        exit (0);
    }
    ARG 'b' : PARM
    {
        break_compilation = 1;
        Ccodeonly = 1;
        switch (**argv) {
        case 'p':
        case '2':
            breakparse = 1;
            break;
        case 'j':
        case '5':
            breakobjinit = 1;
            break;
        case 'i':
        case '3':
            breakimport = 1;
            break;
        case 'f':
        case '6':
            breakflatten = 1;
            break;
        case 't':
        case '7':
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
            breakcompile = 1;
            show_icm = 1;
            break;
        case 'w':
            breakwritesib = 1;
            break;
        case 'b':
        case '4':
            breakreadsib = 1;
            break;
        case 'd':
        case '8':
            breakcheckdec = 1;
            break;
        case 'm':
        case '9':
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
        case '1':
            switch (*(*argv + 1)) {
            case '0':
                breakanalysis = 1;
                break;
            case '1':
                breakwritesib = 1;
                break;
            case '2':
                breakobjects = 1;
                break;
            case '3':
                breakuniquecheck = 1;
                break;
            case '4':
                breakrmvoidfun = 1;
                break;
            case '5':
                breakopt = 1;
                break;
            case '6':
                breakpsiopt = 1;
                show_idx = 1;
                break;
            case '7':
                breakref = 1;
                show_refcnt = 1;
                break;
            case '8':
                breakprecompile = 1;
                break;
            case '9':
                breakcompile = 1;
                show_icm = 1;
                break;
            default:
                SYSWARN (("Unknown break parameter '%s`", *argv));
            }
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

    ARG 'n' : PARM
    {
        if (!strncmp (*argv, "oDCR", 4))
            opt_dcr = 0;
        else if (!strncmp (*argv, "odead_code_removal", 18))
            opt_dcr = 0;
        else if (!strncmp (*argv, "oCF", 3))
            opt_cf = 0;
        else if (!strncmp (*argv, "oconstant_folding", 17))
            opt_cf = 0;
        else if (!strncmp (*argv, "oPDCR", 5))
            opt_wr = 0;
        else if (!strncmp (*argv, "odead_function_removal", 22))
            opt_dfr = 0;
        else if (!strncmp (*argv, "oSACOPT", 7))
            sac_optimize = 0;
        else if (!strncmp (*argv, "osacopt", 7))
            sac_optimize = 0;
        else if (!strncmp (*argv, "oOPT", 4)) {
            optimize = 0;
            sac_optimize = 0;
            psi_optimize = 0;
            opt_rco = 0;
        } else if (!strncmp (*argv, "oopt", 4)) {
            optimize = 0;
            sac_optimize = 0;
            psi_optimize = 0;
            opt_rco = 0;
        } else if (!strncmp (*argv, "oLIR", 4))
            opt_lir = 0;
        else if (!strncmp (*argv, "oloop_invariant_removal", 23))
            opt_lir = 0;
        else if (!strncmp (*argv, "oINL", 4))
            opt_inl = 0;
        else if (!strncmp (*argv, "oinline_functions", 17))
            opt_inl = 0;
        else if (!strncmp (*argv, "oUNR", 4))
            opt_unr = 0;
        else if (!strncmp (*argv, "ounroll_loops", 13))
            opt_unr = 0;
        else if (!strncmp (*argv, "oUNS", 4))
            opt_uns = 0;
        else if (!strncmp (*argv, "ounswitch_loops", 15))
            opt_uns = 0;
        else if (!strncmp (*argv, "oPSIOPT", 7))
            psi_optimize = 0;
        else if (!strncmp (*argv, "opsiopt", 7))
            psi_optimize = 0;
        else if (!strncmp (*argv, "oindex_vect_elimination", 23))
            psi_opt_ive = 0;
        else if (!strncmp (*argv, "oIVE", 3))
            psi_opt_ive = 0;
        else if (!strncmp (*argv, "oarray_elimination", 19))
            opt_ae = 0;
        else if (!strncmp (*argv, "oAE", 3))
            opt_ae = 0;
        else if (!strncmp (*argv, "ocse", 4))
            opt_cse = 0;
        else if (!strncmp (*argv, "oCSE", 4))
            opt_cse = 0;
        else if (!strncmp (*argv, "oDFR", 4))
            opt_dfr = 0;
        else if (!strncmp (*argv, "orefcount_opt", 3))
            opt_rco = 0;
        else if (!strncmp (*argv, "oRCO", 3))
            opt_rco = 0;
        else if (!strncmp (*argv, "oranlib", 7))
            useranlib = 0;
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
        if (!strncmp (*argv, "check_boundary", 13))
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
    OTHER
    {
        SYSWARN (("Unknown command line option '%s`", *argv));
    }
    ENDOPT

    if (argc == 1) {
        strcpy (sacfilename, *argv);
    }

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
     * which is defined in filemgr.c.
     */

    CreateTmpDirectories ();

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
    compiler_phase++;

    /*
     *  Now, we reset some debugging tools.
     */

#ifdef HAVE_MALLOC_O

    malloc_debug (0);

    DBUG_EXECUTE ("MEMVERIFY", malloc_debug (2););

#endif

    filename = sacfilename;

    /*
     *  Finally the compilation process is started.
     */

    NOTE_COMPILER_PHASE;
    syntax_tree = ScanParse ();

    ABORT_ON_ERROR;
    compiler_phase++;

    if (!breakparse) {
        if (MODUL_IMPORTS (syntax_tree) != NULL) {
            NOTE_COMPILER_PHASE;
            syntax_tree = Import (syntax_tree);
            ABORT_ON_ERROR;
        }
        compiler_phase++;

        if (!breakimport && !makedeps) {
            if (MODUL_STORE_IMPORTS (syntax_tree) != NULL) {
                NOTE_COMPILER_PHASE;
                syntax_tree = ReadSib (syntax_tree);
                ABORT_ON_ERROR;
            }
            compiler_phase++;

            if (!breakreadsib) {
                NOTE_COMPILER_PHASE;
                syntax_tree = objinit (syntax_tree);
                ABORT_ON_ERROR;
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

    if (makedeps) {
        compiler_phase = 23;
        NOTE_COMPILER_PHASE;
        PrintDependencies (dependencies, makedeps);
        ABORT_ON_ERROR;
    } else {
        if (!break_compilation) {
            NOTE_COMPILER_PHASE;
            Print (syntax_tree);
            ABORT_ON_ERROR;
            compiler_phase++;
        } else {
            Print (syntax_tree);
        }

        /*
         *  After the C file has been written, the syntax tree may be released.
         */

        FreeTree (syntax_tree);

        if (!Ccodeonly) {
            NOTE_COMPILER_PHASE;
            InvokeCC ();
            ABORT_ON_ERROR;
            compiler_phase++;

            if (filetype != F_prog) {
                NOTE_COMPILER_PHASE;
                CreateLibrary ();
                ABORT_ON_ERROR;
            }
            compiler_phase++;
        }
    }

    /*
     *  Finally, we do some clean up.
     */

    CleanUp ();

    /*
     *  After all, a success message is displayed.
     */

    if (makedeps) {
        NEWLINE (2);
        NOTE2 (("*** Dependency Detection successful ***"));
        NOTE2 (("*** Exit code 0"));
        NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
        NEWLINE (2);
    } else {
        NEWLINE (2);
        NOTE2 (("*** Compilation successful ***"));

        if (break_compilation) {
            NOTE2 (("*** BREAK after: %s", compiler_phase_name[compiler_phase - 1]));
        }

        NOTE2 (("*** Exit code 0"));
        NOTE2 (("*** 0 error(s), %d warning(s)", warnings));
        NEWLINE (2);
    }

    return (0);
}
