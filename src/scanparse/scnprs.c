/*
 *
 * $Log$
 * Revision 3.22  2005/01/11 13:52:12  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.21  2004/11/27 05:04:01  ktr
 * dk
 *
 * Revision 3.20  2004/11/25 22:28:51  sbs
 * compiles
 *
 * Revision 3.19  2004/11/25 22:14:17  cg
 * some ismop
 *
 * Revision 3.18  2004/11/23 21:24:34  sbs
 * some stuff done
 *
 * Revision 3.17  2004/11/21 20:25:08  sbs
 * SACDevCamp2004
 *
 * Revision 3.16  2004/11/07 16:11:19  ktr
 * Temporary files are now stored in tmp_dirname
 *
 * Revision 3.15  2004/11/07 14:28:45  ktr
 * Replaced piped communication with CPP with true file IO in order to
 * be able to use gdb under Mac OS X.
 *
 * Revision 3.14  2003/06/19 08:15:25  sbs
 * checks success of cpp run now and aborts in case of failure!
 *
 * Revision 3.13  2003/03/24 16:37:42  sbs
 * CreateCppCallString used .
 *
 * Revision 3.12  2003/03/21 18:01:46  sbs
 * generic preprocessor flags SAC_FOR_xxx eliminated again 8-)
 *
 * Revision 3.11  2002/08/21 13:33:35  sah
 * dot elimination enabled again;)
 *
 * Revision 3.10  2002/08/13 14:54:07  dkr
 * stephan's dot elimination completely disabled now ;-)
 *
 * Revision 3.9  2002/08/13 14:45:59  sbs
 * Stephan's dot elimination disabled due to scrambled code on several examples
 * ... 8-(
 *
 * Revision 3.7  2002/07/09 12:52:35  sbs
 * dots integrated in the other branch as well 8-))
 *
 * Revision 3.6  2002/07/09 12:51:13  sbs
 * break specifyer "yacc" added and EliminateSelDots called
 *
 * Revision 3.5  2002/04/16 18:41:47  dkr
 * bug in SetFileNames() fixed:
 * F_prog -> 'modulename' is set correctly now
 *
 * Revision 3.3  2001/11/14 19:10:00  sbs
 * generic preprocessor-flag -DSAC_FOR_OSxxx inserted.
 *
 * Revision 3.2  2001/03/28 14:36:59  dkr
 * include of internal_lib.h added
 *
 * Revision 3.1  2000/11/20 17:59:51  sacbase
 * new release made
 *
 * Revision 2.4  2000/08/02 10:14:40  nmw
 * profiling for c library added
 *
 * Revision 2.3  1999/10/07 13:39:17  sbs
 * when run with -dsysccall, now a NOTE is issued for the preprocessor call
 *
 * Revision 2.2  1999/05/06 15:38:46  sbs
 * call of yyparse changed to My_yyparse.
 *
 * Revision 2.1  1999/02/23 12:40:36  sacbase
 * new release made
 *
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
SPdoScanParse ()
{
    char *pathname;
    char cccallstr[MAX_PATH_LEN];
    char *cppfile;
    int err;

    DBUG_ENTER ("SPdoScanParse");

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

    if (!((global.break_after == PH_scanparse)
          && (0 == strcmp (global.break_specifier, "yacc")))) {
        global.syntax_tree = HDdoEliminateSelDots (global.syntax_tree);
    }

    DBUG_RETURN (global.syntax_tree);
}
