/*
 *
 * $Log$
 * Revision 1.10  1997/05/16 09:54:45  sbs
 * ANALSE-TOOL extended to function-application specific timing
 *
 * Revision 1.9  1997/03/19  13:43:59  cg
 * New global variable filetype set.
 * targetdir converted to absolute pathname
 *
 * Revision 1.8  1996/09/11  06:19:49  cg
 * Now, the global variable outfilename is always set correctly.
 *
 * Revision 1.7  1996/01/23  09:57:28  cg
 * Now, the length of a module name is checked
 *
 * Revision 1.6  1996/01/23  09:03:10  cg
 * Now, we check if a module/class implementation is in the correct file
 *
 * Revision 1.5  1996/01/07  16:56:38  cg
 * bug fixed in initializing sacfilename
 *
 * Revision 1.4  1996/01/05  12:32:56  cg
 * Now, the global variables outfilename, cfilename, and
 * targetdir are set here
 *
 * Revision 1.3  1996/01/02  15:57:43  cg
 * added function CheckModuleName
 * The global variables cfilename and outfilename will be set here.
 * The length of module names is limited to 12 characters due to
 * later handling by 'ar'.
 *
 * Revision 1.2  1996/01/02  08:16:29  cg
 * first compilable revision
 *
 * Revision 1.1  1995/12/29  17:22:05  cg
 * Initial revision
 *
 *
 *
 */

#include <stdio.h>

#include "types.h"
#include "tree_basic.h"

#include "Error.h"
#include "dbug.h"
#include "filemgr.h"
#include "globals.h"

#include "scnprs.h"

/*
 *  Since the module/class name is used as basis for generating file names,
 *  its length must be limited because ar accepts only file names with a
 *  maximum of 15 characters.
 */

#define MAX_MODNAME 12

/*
 *
 *  functionname  :
 *  arguments     :
 *  description   :
 *  global vars   :
 *  internal funs :
 *  external funs :
 *  macros        :
 *
 *  remarks       :
 *
 */

/*
 *
 *  functionname  : SetFileNames
 *  arguments     : 1) module/class name
 *  description   : sets the global
 *                  variables outfilename, cfilename, and targetdir according
 *                  to the kind of file and the -o command line option.
 *  global vars   : outfilename, cfilename, targetdir, filetype
 *  internal funs : ---
 *  external funs : strcmp, strcpy, strcat
 *  macros        :
 *
 *  remarks       :
 *
 */

void
SetFileNames (node *modul)
{
    char buffer[MAX_FILE_NAME];

    DBUG_ENTER ("SetFileNames");

    filetype = MODUL_FILETYPE (modul);

    if (MODUL_FILETYPE (modul) == F_prog) {
        linkstyle = 0; /* Programs are always linked in style 0, i.e. a single
                          C-file is generated and compiled as a whole. */

        if (outfilename[0] == '\0') {
            strcpy (outfilename, "a.out");
            strcpy (cfilename, "a.out.c");
        } else {
            strcpy (cfilename, outfilename);
            strcat (cfilename, ".c");
        }
    } else {
        SYSWARN (("-a option turned off for module/class compilation"));
        analyseflag = 0;

        if (sacfilename[0] != '\0') {
            strcpy (buffer, MODUL_NAME (modul));
            strcat (buffer, ".sac");

            if (0 != strcmp (buffer, sacfilename)) {
                SYSWARN (("Module/class '%s` should be in a file named \"%s\" "
                          "instead of \"%s\"",
                          MODUL_NAME (modul), buffer, sacfilename));
            }
        }

        if (outfilename[0] == '\0') {
            strcpy (targetdir, "./");
        } else {
            strcpy (targetdir, outfilename);
            strcat (targetdir, "/");
        }

        strcpy (modulename, MODUL_NAME (modul));

        strcpy (cfilename, MODUL_NAME (modul));
        strcat (cfilename, ".c");

        strcpy (outfilename, MODUL_NAME (modul));
        strcat (outfilename, ".lib");

        strcpy (targetdir, AbsolutePathname (targetdir));
    }

    DBUG_VOID_RETURN;
}

/*
 *
 *  functionname  : ScanParse
 *  arguments     : ---
 *  description   : searches the file given by the global variable filename
 *                  in the path and parses it.
 *                  The global variables outfilename and cfilename are
 *                  set according to a module/class name, which will be
 *                  abbreviated to a maximum of 13 characters.
 *  global vars   : yyin, filename, syntax_tree
 *  internal funs : CheckModuleName
 *  external funs : FindFile, yyparse, strcpy
 *  macros        : MAX_PATH_LEN, ERROR
 *
 *  remarks       : two different versions with and without invoking
 *                  the C-preprocessor.
 *
 */

#ifdef NO_CPP /* don't use C-preprocessor */

node *
ScanParse ()
{
    char *pathname;

    DBUG_ENTER ("ScanParse");

    if (sacfilename[0] == '\0') {
        yyin = stdin;
        NOTE (("Parsing from stdin ..."));
    } else {
        pathname = FindFile (PATH, sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", sacfilename));
        }

        yyin = fopen (pathname, "r");
        NOTE (("Parsing file \"%s\" ...", pathname));
    }

    start_token = PARSE_PRG;
    yyparse ();

    fclose (yyin);

    SetFileNames (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#else /* NO_CPP */

node *
ScanParse ()
{
    char *pathname;
    char cccallstr[MAX_PATH_LEN];

    DBUG_ENTER ("ScanParse");

    if (sacfilename[0] == '\0') {
        sprintf (cccallstr, "cpp -P -C ");
        NOTE (("Parsing from stdin ..."));
    } else {
        pathname = FindFile (PATH, sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", sacfilename));
        }

        sprintf (cccallstr, "gcc -E -P -C -x c %s", pathname);
        NOTE (("Parsing file \"%s\" ...", pathname));
    }

    yyin = popen (cccallstr, "r");

    start_token = PARSE_PRG;
    yyparse ();

    pclose (yyin);

    SetFileNames (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#endif /* NO_CPP */
