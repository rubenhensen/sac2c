/*
 *
 * $Log$
 * Revision 1.24  1995/02/13 17:21:03  asi
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
#include "ConstantFolding.h"
#include "DeadCodeRemoval.h"
#include "filemgr.h"
#include "import.h"

#include "scnprs.h"
#include <stdlib.h>
#include <string.h>

FILE *outfile;
char *filename = NULL;
int opt_dcr = 1, opt_cf = 1;

MAIN
{
    int set_outfile = 0;
    int breakparse = 0, breakimport = 0, breakflatten = 0, breaktype = 0, optimize = 1;
    char prgname[256];
    char outfilename[256] = "out.txt";

    strcpy (prgname, argv[0]);

    OPT ARG 'h':
    {
        usage (prgname);
        exit (0);
    }
    ARG 'p':
    {
        breakparse = 1;
    }
    ARG 'i':
    {
        breakimport = 1;
    }
    ARG 'f':
    {
        breakflatten = 1;
    }
    ARG 't':
    {
        breaktype = 1;
    }
    ARG 's':
    {
        silent = 1;
    }
    ARG 'n':
    {
        if (strcmp (*argv, "oDCR"))
            opt_dcr = 0;
        if (strcmp (*argv, "oCF"))
            opt_cf = 0;
        if (strcmp (*argv, "oOPT"))
            optimize = 0;
    }
    NEXTOPT
    ARG 'o' : PARM
    {
        strcpy (outfilename, *argv);
        set_outfile = 1;
    }
    NEXTOPT
    OTHER
    {
        fprintf (stderr, "unknown option \"%c\"\n", **argv);
    }
    ENDOPT

    /* First, we set our search paths for the source program, modul declarations,
     * and modul implementations...
     */

    if (AppendEnvVar (MODDEC_PATH, "SAC_DEC_PATH") == 0)
        ERROR2 (1, ("MAX_PATH_LEN too low!/n"));
    if (AppendEnvVar (MODIMP_PATH, "SAC_LIBRARY_PATH") == 0)
        ERROR2 (1, ("MAX_PATH_LEN too low!/n"));
    if (AppendEnvVar (PATH, "SAC_PATH") == 0)
        ERROR2 (1, ("MAX_PATH_LEN too low!/n"));

    if (argc == 1) {
        yyin = FindFile (PATH, *argv);
        filename = *argv;
        if (yyin == NULL) {
            ERROR2 (1, ("Couldn't open file \"%s\"!\n", *argv));
        }
    }

    if (set_outfile) {
        outfile = fopen (outfilename, "w");
        if (outfile == NULL)
            ERROR2 (1, ("Couldn't open Outfile !\n"));
    } else
        outfile = stdout;

    start_token = PARSE_PRG;
    yyparse ();

    if (!breakparse) {
        NOTE (("Resolving Imports: ..."));
        syntax_tree = Import (syntax_tree);
        NOTE (("\n"));
        if (!breakimport) {
            syntax_tree = Flatten (syntax_tree);
            if ((!breakflatten) && (0 == errors)) {
                NOTE (("Typechecking: ..."));
                Typecheck (syntax_tree);
                NOTE (("\n%d Warnings, %d Errors \n", warnings, errors));
                if ((!breaktype) && (errors == 0)) {
                    if (optimize) {
                        NOTE (("Optimizing: ...\n"));
                        syntax_tree = Optimize (syntax_tree);
                    }
                    /*  GenCCode(); */
                }
            } else
                NOTE (("\n%d Warnings, %d Errors \n", warnings, errors));
        }
    }

    Print (syntax_tree);
    FreeTree (syntax_tree);

    return (0);
}
