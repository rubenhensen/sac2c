/*
 *
 * $Log$
 * Revision 1.3  1996/01/02 15:57:43  cg
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
 *  functionname  : CheckModuleName
 *  arguments     : 1) module/class name
 *  description   : checks the length of module name and sets the global
 *                  variables outfilename and cfilename accordingly
 *  global vars   : outfilename, cfilename
 *  internal funs : ---
 *  external funs : strcmp, strncpy, strcat, strlen
 *  macros        : MAX_MODNAME
 *
 *  remarks       :
 *
 */

void
CheckModuleName (char *name)
{
    DBUG_ENTER ("CheckModuleName");

    if (0 != strcmp (outfilename, "a.out")) {
        SYSWARN (("Module/Class name overrides command line option '-o`"));
    }

    if (strlen (name) > MAX_MODNAME) {
        SYSERROR (
          ("Module/Class name must be no longer than %d characters", MAX_MODNAME));
    }

    strncpy (cfilename, name, MAX_MODNAME);
    strcat (cfilename, ".c");

    strncpy (outfilename, name, MAX_MODNAME);
    strcat (outfilename, ".o");

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
        strcpy (sacfilename, "stdin");
    } else {
        pathname = FindFile (PATH, sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", sacfilename));
        }

        yyin = fopen (pathname, "r");
    }

    if (yyin == NULL) {
        SYSABORT (("Unable to open file \"%s\"", *argv));
    }

    NOTE (("Parsing file \"%s\" ...", pathname));

    start_token = PARSE_PRG;
    yyparse ();

    fclose (yyin);

    if ((MODUL_FILETYPE (syntax_tree) == F_modimp)
        || (MODUL_FILETYPE (syntax_tree) == F_classimp)) {
        CheckModuleName (MODUL_NAME (syntax_tree));
    }

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
        strcpy (sacfilename, "stdin");
    } else {
        pathname = FindFile (PATH, sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", sacfilename));
        }

        sprintf (cccallstr, "gcc -E -P -C -x c %s", pathname);
    }

    yyin = popen (cccallstr, "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to open file \"%s\"", sacfilename));
    }

    NOTE (("Parsing file \"%s\" ...", pathname));

    start_token = PARSE_PRG;
    yyparse ();

    pclose (yyin);

    if ((MODUL_FILETYPE (syntax_tree) == F_modimp)
        || (MODUL_FILETYPE (syntax_tree) == F_classimp)) {
        CheckModuleName (MODUL_NAME (syntax_tree));
    }

    DBUG_RETURN (syntax_tree);
}

#endif /* NO_CPP */
