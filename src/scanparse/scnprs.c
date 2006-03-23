/*
 * $Id$
 */

#include <stdio.h>
#include <string.h>

#include "scnprs.h"
#include "dbug.h"
#include "ctinfo.h"
#include "internal_lib.h"
#include "filemgr.h"
#include "handle_dots.h"

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

node *
SPdoScanParse (node *syntax_tree)
{
    const char *pathname;
    char cccallstr[MAX_PATH_LEN];
    char *cppfile;
    int err;

    DBUG_ENTER ("SPdoScanParse");

    global.compiler_subphase = SUBPH_cpp;

    global.filename = global.puresacfilename;

    /*
     * Create a name for the file containing the CPP's result
     */

    if (global.sacfilename == NULL) {
        cppfile = ILIBstringConcat3 (global.tmp_dirname, "/", "stdin");
        ILIBcreateCppCallString (global.sacfilename, cccallstr, cppfile);
        CTInote ("Parsing from stdin ...");
    } else {
        cppfile = ILIBstringConcat3 (global.tmp_dirname, "/", global.filename);
        pathname = FMGRfindFile (PK_path, global.sacfilename);

        if (pathname == NULL) {
            CTIabort ("Unable to open file \"%s\"", global.sacfilename);
        }

        ILIBcreateCppCallString (pathname, cccallstr, cppfile);

        CTInote ("Parsing file \"%s\" ...", pathname);
    }

    if (global.show_syscall) {
        CTInote ("err = system( \"%s\")", cccallstr);
    }

    err = system (cccallstr);
    if (err) {
        CTIabort ("Unable to start C preprocessor");
    }

    global.compiler_subphase = SUBPH_sp;

    if (global.show_syscall) {
        CTInote ("yyin = fopen( \"%s\", \"r\")", cppfile);
    }

    yyin = fopen (cppfile, "r");

    if ((yyin == NULL) || (ferror (yyin))) {
        CTIabort ("Unable to start C preprocessor");
    }

    global.start_token = PARSE_PRG;

    SPmyYyparse ();

    if (global.show_syscall) {
        CTInote ("err = fclose( yyin)");
    }

    err = fclose (yyin);
    if (err) {
        CTIabort ("C preprocessor error");
    }

    if (global.show_syscall) {
        CTInote ("err = remove( \"%s\")", cppfile);
    }

    err = remove (cppfile);
    if (err) {
        CTIabort ("Could not delete /tmp-file");
    }
    ILIBfree (cppfile);

    FMGRsetFileNames (global.syntax_tree);

    DBUG_RETURN (global.syntax_tree);
}
