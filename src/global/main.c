/*
 *
 * $Log$
 * Revision 1.12  1994/12/16 14:23:10  sbs
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

#include "Error.h"
#include "usage.h"
#include "flatten.h"
#include "print.h"
#include "typecheck.h"
#include "optimize.h"
#include "filemgr.h"
#include "import.h"

#include "scnprs.h"
#include <stdlib.h>
#include <string.h>

FILE *outfile;

MAIN
{
    int set_outfile = 0;
    int breakparse = 0, breakimport = 0, breakflatten = 0, breaktype = 0;
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
        syntax_tree = Import (syntax_tree);
        if (!breakimport) {
            syntax_tree = Flatten (syntax_tree);
            if (!breakflatten) {
                NOTE (("Typechecking: ..."));
                Typecheck (syntax_tree);
                NOTE (("%d Warnings, %d Errors \n", warnings, errors));
                if (!breaktype) {
                    NOTE (("Optimizing: ...\n"));
                    syntax_tree = Optimize (syntax_tree);
                    /*  GenCCode(); */
                }
            }
        }
    }

    Print (syntax_tree);

    /*  GenCCode(); */
    return (0);
}
