/*
 *
 * $Log$
 * Revision 1.8  1994/12/09 10:13:19  sbs
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

#include "scnprs.h"
#include <stdlib.h>
#include <string.h>

extern FILE *yyin;

FILE *outfile;

MAIN
{
    int set_outfile = 0;
    int breakparse = 0, breakflatten = 0, breaktype = 0;
    char prgname[256];
    char filename[256];
    char outfilename[128] = "out.txt";
    char *paths, *path;
    char message[80];

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

    if (1 <= argc) {
        yyin = fopen (*argv, "r");
        if (yyin == NULL) {
            paths = getenv ("SAC_LIBRARY_PATH");
            if (paths == NULL)
                Error ("Couldn't open Infile !\n", 1);
            path = strtok (paths, ":");
            while ((yyin == NULL) && (path != NULL)) {
                strcpy (filename, path);
                strcat (filename, "/");
                strcat (filename, *argv);
                DBUG_PRINT ("MAIN", ("trying file %s\n", filename));
                yyin = fopen (filename, "r");
                if (yyin == NULL) {
                    path = strtok (NULL, ":");
                    if (path == NULL)
                        Error ("Couldn't open Infile !\n", 1);
                }
            }
        }
    }

    if (set_outfile) {
        outfile = fopen (outfilename, "w");
        if (outfile == NULL)
            Error ("Couldn't open Outfile !\n", 1);
    } else
        outfile = stdout;

    yyparse ();

    if (!breakparse) {
        syntax_tree = Flatten (syntax_tree);
        if (!breakflatten) {
            NOTE ("Typechecking: ...");
            Typecheck (syntax_tree);
            sprintf (message, "%d Warnings, %d Errors \n", warnings, errors);
            NOTE (message);
            if (!breaktype) {
                NOTE ("Optimizing: ...");
                syntax_tree = Optimize (syntax_tree);
                /*  GenCCode(); */
            }
        }
    }

    Print (syntax_tree);

    /*  GenCCode(); */
    return (0);
}
