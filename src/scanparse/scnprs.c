/*
 *
 * $Log$
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
 * Revision 1.18  1999/01/20 09:06:53  cg
 * Added check whether piping of input through preprocessor
 * is successful.
 *
 * Revision 1.17  1998/05/27 11:19:44  cg
 * global variable 'filename' which contains the current file name in order
 * to provide better error messages is now handled correctly.
 *
 * Revision 1.16  1998/03/04 16:23:27  cg
 * C compiler invocations and file handling converted to new
 * to usage of new  configuration files.
 *
 * Revision 1.15  1998/02/27 16:31:40  cg
 * added support for correct setting of file names for diagnostic output
 * while parsing (global variable 'filename'.
 *
 * Revision 1.14  1997/08/08 08:49:14  sbs
 * warning that -p is turned off in modules is now only given, iff -p is
 * actually set!
 *
 * Revision 1.13  1997/06/03 13:51:54  sbs
 * # line - capability integrated into sac2c.
 * Now, the line-number in error messages should be correct 8-)))
 *
 * Revision 1.12  1997/06/03  10:14:09  sbs
 * -D option integrated
 *
 * Revision 1.11  1997/05/28  12:36:27  sbs
 * Profiling integrated
 *
 * Revision 1.10  1997/05/16  09:54:45  sbs
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
 */

#include <stdio.h>
#include <string.h>

#include "types.h"
#include "tree_basic.h"

#include "internal_lib.h"
#include "Error.h"
#include "dbug.h"
#include "filemgr.h"
#include "globals.h"
#include "resource.h"

#include "scnprs.h"
#include "handle_dots.h"

/******************************************************************************
 *
 * Function:
 *   void SetFileNames( node *modul)
 *
 * Description:
 *   Sets the global variables
 *     modulename, outfilename, cfilename, targetdir
 *   according to the kind of file and the -o command line option.
 *
 ******************************************************************************/

void
SetFileNames (node *modul)
{
    char buffer[MAX_FILE_NAME];

    DBUG_ENTER ("SetFileNames");

    filetype = MODUL_FILETYPE (modul);

    if (MODUL_FILETYPE (modul) == F_prog) {
        linkstyle = 0; /* Programs are always linked in style 0, i.e. a single
                          C-file is generated and compiled as a whole. */

        targetdir[0] = '\0';

        strcpy (modulename, MODUL_NAME (modul));

        if (outfilename[0] == '\0') {
            strcpy (outfilename, "a.out");
            strcpy (cfilename, "a.out.c");
        } else {
            strcpy (cfilename, outfilename);
            strcat (cfilename, ".c");
        }
    } else {
        if ((profileflag != 0) && (generatelibrary & GENERATELIBRARY_SAC)) {
            SYSWARN (("-p option turned off for module/class compilation"));
            profileflag = 0;
        }

        if (sacfilename[0] != '\0') {
            strcpy (buffer, MODUL_NAME (modul));
            strcat (buffer, ".sac");

            if (strcmp (buffer, puresacfilename) != 0) {
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

    filename = puresacfilename;

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
    linenum = 1;

    My_yyparse ();

    fclose (yyin);

    SetFileNames (syntax_tree);

    if ((break_after == PH_scanparse) && (0 == strcmp (break_specifier, "yacc"))) {
        goto DONE;
    }

    syntax_tree = EliminateSelDots (syntax_tree);

DONE:
    DBUG_RETURN (syntax_tree);
}

#else /* NO_CPP */

node *
ScanParse ()
{
    char *pathname;
    char cccallstr[MAX_PATH_LEN];
    char cppfile[MAX_PATH_LEN];
    int err;

    DBUG_ENTER ("ScanParse");

    filename = puresacfilename;

    /*
     * Create a name for the file containing the CPP's result
     */
    cppfile[0] = '\0';
    strcat (cppfile, tmp_dirname);
    strcat (cppfile, "/");

    if (sacfilename[0] == '\0') {
        strcat (cppfile, "stdin");
        CreateCppCallString (sacfilename, cccallstr, cppfile);
        NOTE (("Parsing from stdin ..."));
    } else {
        strcat (cppfile, filename);
        pathname = FindFile (PATH, sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", sacfilename));
        }

        CreateCppCallString (pathname, cccallstr, cppfile);

        NOTE (("Parsing file \"%s\" ...", pathname));
    }

    if (show_syscall) {
        NOTE (("err = system( \"%s\")", cccallstr));
    }

    err = system (cccallstr);
    if (err) {
        SYSABORT (("Unable to start C preprocessor"));
    }

    if (show_syscall) {
        NOTE (("yyin = fopen( \"%s\", \"r\")", cppfile));
    }

    yyin = fopen (cppfile, "r");

    if ((yyin == NULL) || (ferror (yyin))) {
        SYSABORT (("Unable to start C preprocessor"));
    }

    start_token = PARSE_PRG;

    My_yyparse ();

    if (show_syscall) {
        NOTE (("err = fclose( yyin)"));
    }

    err = fclose (yyin);
    if (err) {
        SYSABORT (("C preprocessor error"));
    }

    if (show_syscall) {
        NOTE (("err = remove( \"%s\")", cppfile));
    }

    err = remove (cppfile);
    if (err) {
        SYSABORT (("Could not delete /tmp-file"));
    }

    SetFileNames (syntax_tree);

    if ((break_after == PH_scanparse) && (0 == strcmp (break_specifier, "yacc"))) {
        goto DONE;
    }

    syntax_tree = EliminateSelDots (syntax_tree);

DONE:
    DBUG_RETURN (syntax_tree);
}

#endif /* NO_CPP */
