/*
 *
 * $Log$
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

static void
SetFileNames (node *modul)
{
    char buffer[MAX_FILE_NAME];

    DBUG_ENTER ("SetFileNames");

    filetype = MODULE_FILETYPE (modul);

    if (MODULE_FILETYPE (modul) == F_prog) {
        /**
         * Programs are always linked in style 0, i.e. a single
         * C-file is generated and compiled as a whole.
         */
        global.linkstyle = 0;

        global.targetdir[0] = '\0';

        strcpy (global.modulename, MODULE_NAME (modul));

        if (global.outfilename[0] == '\0') {
            strcpy (global.outfilename, "a.out");
            strcpy (global.cfilename, "a.out.c");
        } else {
            strcpy (global.cfilename, global.outfilename);
            strcat (global.cfilename, ".c");
        }
    } else {
        if ((global.profileflag != 0) && (global.generatelibrary & GENERATELIBRARY_SAC)) {
            SYSWARN (("-p option turned off for module/class compilation"));
            global.profileflag = 0;
        }

        if (global.sacfilename[0] != '\0') {
            strcpy (buffer, MODULE_NAME (modul));
            strcat (buffer, ".sac");

            if (strcmp (buffer, global.puresacfilename) != 0) {
                SYSWARN (("Module/class '%s` should be in a file named \"%s\" "
                          "instead of \"%s\"",
                          MODULE_NAME (modul), buffer, global.sacfilename));
            }
        }

        if (global.outfilename[0] == '\0') {
            strcpy (global.targetdir, "./");
        } else {
            strcpy (global.targetdir, global.outfilename);
            strcat (global.targetdir, "/");
        }

        strcpy (global.modulename, MODULE_NAME (modul));

        strcpy (global.cfilename, MODULE_NAME (modul));
        strcat (global.cfilename, ".c");

        strcpy (global.outfilename, MODULE_NAME (modul));
        strcat (global.outfilename, ".lib");

        strcpy (global.targetdir, FMabsolutePathname (global.targetdir));
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
SPdoScanParse ()
{
    char *pathname;

    DBUG_ENTER ("SPdoScanParse");

    global.filename = global.puresacfilename;

    if (global.sacfilename[0] == '\0') {
        yyin = stdin;
        NOTE (("Parsing from stdin ..."));
    } else {
        global.pathname = FMfindFile (PATH, global.sacfilename);

        if (global.pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", global.sacfilename));
        }

        yyin = fopen (global.pathname, "r");
        NOTE (("Parsing file \"%s\" ...", global.pathname));
    }

    start_token = PARSE_PRG;
    linenum = 1;

    SPmyYyparse ();

    fclose (yyin);

    SetFileNames (global.syntax_tree);

    if ((global.break_after == PH_scanparse)
        && (0 == strcmp (global.break_specifier, "yacc"))) {
        goto DONE;
    }

    global.syntax_tree = HDdoEliminateSelDots (global.syntax_tree);

DONE:
    DBUG_RETURN (global.syntax_tree);
}

#else /* NO_CPP */

node *
SPdoScanParse ()
{
    char *pathname;
    char cccallstr[MAX_PATH_LEN];
    char cppfile[MAX_PATH_LEN];
    int err;

    DBUG_ENTER ("ScanParse");

    global.filename = global.puresacfilename;

    /*
     * Create a name for the file containing the CPP's result
     */
    cppfile[0] = '\0';
    strcat (cppfile, global.tmp_dirname);
    strcat (cppfile, "/");

    if (global.sacfilename[0] == '\0') {
        strcat (cppfile, "stdin");
        ILIBcreateCppCallString (global.sacfilename, cccallstr, cppfile);
        NOTE (("Parsing from stdin ..."));
    } else {
        strcat (cppfile, global.filename);
        pathname = FMfindFile (PATH, global.sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", global.sacfilename));
        }

        ILIBcreateCppCallString (pathname, cccallstr, cppfile);

        NOTE (("Parsing file \"%s\" ...", pathname));
    }

    if (global.show_syscall) {
        NOTE (("err = system( \"%s\")", cccallstr));
    }

    err = system (cccallstr);
    if (err) {
        SYSABORT (("Unable to start C preprocessor"));
    }

    if (global.show_syscall) {
        NOTE (("yyin = fopen( \"%s\", \"r\")", cppfile));
    }

    yyin = fopen (cppfile, "r");

    if ((yyin == NULL) || (ferror (yyin))) {
        SYSABORT (("Unable to start C preprocessor"));
    }

    global.start_token = PARSE_PRG;

    SPmyYyparse ();

    if (global.show_syscall) {
        NOTE (("err = fclose( yyin)"));
    }

    err = fclose (yyin);
    if (err) {
        SYSABORT (("C preprocessor error"));
    }

    if (global.show_syscall) {
        NOTE (("err = remove( \"%s\")", cppfile));
    }

    err = remove (cppfile);
    if (err) {
        SYSABORT (("Could not delete /tmp-file"));
    }

    SetFileNames (global.syntax_tree);

    if ((global.break_after == PH_scanparse)
        && (0 == strcmp (global.break_specifier, "yacc"))) {
        goto DONE;
    }

    global.syntax_tree = HDdoEliminateSelDots (global.syntax_tree);

DONE:
    DBUG_RETURN (global.syntax_tree);
}

#endif /* NO_CPP */
