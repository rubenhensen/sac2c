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
    char *path;

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
            path = getenv ("SAC_LIBRARY");
            if (path == NULL)
                Error ("Couldn't open Infile !\n", 1);
            strcpy (filename, path);
            strcat (filename, "/");
            strcat (filename, *argv);
            yyin = fopen (filename, "r");
            if (yyin == NULL)
                Error ("Couldn't open Infile !\n", 1);
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
