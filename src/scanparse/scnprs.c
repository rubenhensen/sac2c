/*
 *
 * $Log$
 * Revision 1.1  1995/12/29 17:22:05  cg
 * Initial revision
 *
 *
 *
 */

#include "types.h"

#include "Error.h"
#include "dbug.h"

#include "filemgr.h"

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
 *  functionname  : ScanParse
 *  arguments     : ---
 *  description   : searches the file given by the global variable filename
 *                  in the path and parses it.
 *  global vars   : yyin, filename, syntax_tree
 *  internal funs : ---
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
    char cccallstr[MAX_PATH_LEN];

    DBUG_ENTER ("ScanParse");

    if (filename[0] == '\0') {
        yyin = stdin;
        strcpy (filename, "stdin");
    } else {
        pathname = FindFile (PATH, filename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", filename));
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

    DBUG_RETURN (syntax_tree);
}

#else /* NO_CPP */

node *
ScanParse ()
{
    char *pathname;
    char cccallstr[MAX_PATH_LEN];

    DBUG_ENTER ("ScanParse");

    if (filename[0] == '\0') {
        sprintf (cccallstr, "cpp -P -C ");
        strcpy (filename, "stdin");
    } else {
        pathname = FindFile (PATH, filename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", filename));
        }

        sprintf (cccallstr, "gcc -E -P -C -x c %s", pathname);
    }

    yyin = popen (cccallstr, "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to open file \"%s\"", filename));
    }

    NOTE (("Parsing file \"%s\" ...", pathname));

    start_token = PARSE_PRG;
    yyparse ();

    pclose (yyin);

    DBUG_RETURN (syntax_tree);
}

#endif /* NO_CPP */
