/*
 *
 * $Log$
 * Revision 1.3  1994/11/15 13:26:31  sbs
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

#include <stdlib.h>
#include <string.h>

extern FILE *yyin;

FILE *outfile;

MAIN
{
    int prettyprint = 0, set_outfile = 0;
    char filename[256];
    char outfilename[128] = "out.txt";
    char *paths, *path;

    OPT ARG 'h':
    {
        usage (filename);
        exit (0);
    }
    ARG 'p':
    {
        prettyprint = 1;
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
    /*  syntax_tree=Flatten(syntax_tree); */
    if (!prettyprint)
        syntax_tree = Flatten (syntax_tree);
    Print (syntax_tree);

    /*  GenCCode(); */
    return (0);
}
