/*
 *
 * $Log$
 * Revision 3.4  2001/11/14 19:16:07  sbs
 * *** empty log message ***
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
 *  C compiler invocations and file handling converted to new
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
 *
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
        if ((profileflag != 0) && (generatelibrary & GENERATELIBRARY_SAC)) {
            SYSWARN (("-p option turned off for module/class compilation"));
            profileflag = 0;
        }

        if (sacfilename[0] != '\0') {
            strcpy (buffer, MODUL_NAME (modul));
            strcat (buffer, ".sac");

            if (0 != strcmp (buffer, puresacfilename)) {
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

    DBUG_RETURN (syntax_tree);
}

#else /* NO_CPP */

node *
ScanParse ()
{
    int i;
    char *pathname;
    char cccallstr[MAX_PATH_LEN];

    DBUG_ENTER ("ScanParse");

    filename = puresacfilename;

    if (sacfilename[0] == '\0') {
        strcpy (cccallstr, config.cpp_stdin);

        strcat (cccallstr, " ");
        strcat (cccallstr, config.opt_D);
        strcat (cccallstr, "SAC_FOR_");
        strcat (cccallstr, target_platform);

        for (i = 0; i < num_cpp_vars; i++) {
            strcat (cccallstr, " ");
            strcat (cccallstr, config.opt_D);
            strcat (cccallstr, cppvars[i]);
        }

        NOTE (("Parsing from stdin ..."));
    } else {
        pathname = FindFile (PATH, sacfilename);

        if (pathname == NULL) {
            SYSABORT (("Unable to open file \"%s\"", sacfilename));
        }

        strcpy (cccallstr, config.cpp_file);

        strcat (cccallstr, " ");
        strcat (cccallstr, config.opt_D);
        strcat (cccallstr, "SAC_FOR_");
        strcat (cccallstr, target_platform);

        for (i = 0; i < num_cpp_vars; i++) {
            strcat (cccallstr, " ");
            strcat (cccallstr, config.opt_D);
            strcat (cccallstr, cppvars[i]);
        }

        strcat (cccallstr, " ");
        strcat (cccallstr, pathname);
        NOTE (("Parsing file \"%s\" ...", pathname));
    }

    if (show_syscall)
        NOTE (("yyin = popen( %s)", cccallstr));

    yyin = popen (cccallstr, "r");

    if (yyin == NULL) {
        SYSABORT (("Unable to start C preprocessor"));
    }

    start_token = PARSE_PRG;
    My_yyparse ();

    pclose (yyin);

    SetFileNames (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#endif /* NO_CPP */
