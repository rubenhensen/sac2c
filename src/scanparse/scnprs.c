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

#include "sac.tab.h"

static const char *pathname;

node *
SPdoLocateSource (node *syntax_tree)
{
    DBUG_ENTER ("SPdoRunPreProcessor");

    global.filename = global.puresacfilename;

    if (global.sacfilename == NULL) {
        pathname = NULL;
        CTInote ("Reading from stdin ...");
    } else {
        pathname = FMGRfindFile (PK_path, global.sacfilename);

        if (pathname == NULL) {
            CTIabort ("Unable to open file \"%s\"", global.sacfilename);
        }

        CTInote ("Reading from file \"%s\" ...", pathname);
    }

    DBUG_RETURN (syntax_tree);
}

node *
SPdoRunPreProcessor (node *syntax_tree)
{
    int err;
    char *tmp, *cppcallstr;

    DBUG_ENTER ("SPdoRunPreProcessor");

    global.filename = global.puresacfilename;

    if (pathname == NULL) {
        cppcallstr
          = ILIBstringConcat (global.config.cpp_stdin,
                              global.cpp_options == NULL ? " " : global.cpp_options);
    } else {
        cppcallstr
          = ILIBstringConcat4 (global.config.cpp_file,
                               global.cpp_options == NULL ? " " : global.cpp_options, " ",
                               pathname);
#if 0
    pathname = ILIBfree( pathname);
#endif
    }

    tmp = ILIBstringConcat4 (cppcallstr, " -o ", global.tmp_dirname, "/source");
    cppcallstr = ILIBfree (cppcallstr);
    cppcallstr = tmp;

    if (global.show_syscall) {
        CTInote ("err = system( \"%s\")", cppcallstr);
    }

    err = system (cppcallstr);

    cppcallstr = ILIBfree (cppcallstr);

    if (err) {
        CTIabort ("Unable to run C preprocessor");
    }

    DBUG_RETURN (syntax_tree);
}

node *
SPdoScanParse (node *syntax_tree)
{
    int err;
    char *cppfile;

    DBUG_ENTER ("SPdoScanParse");

    cppfile = ILIBstringConcat (global.tmp_dirname, "/source");

    if (global.show_syscall) {
        CTInote ("yyin = fopen( \"%s\", \"r\")", cppfile);
    }

    yyin = fopen (cppfile, "r");

    if ((yyin == NULL) || (ferror (yyin))) {
        CTIabort ("C preprocessing failed");
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

    cppfile = ILIBfree (cppfile);

    if (err) {
        CTIabort ("Could not delete /tmp-file");
    }

    FMGRsetFileNames (global.syntax_tree);

    DBUG_RETURN (global.syntax_tree);
}
