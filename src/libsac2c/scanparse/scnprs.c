#include <stdio.h>

#include "scnprs.h"
#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"
#include "ctinfo.h"
#include "str.h"
#include "globals.h"
#include "memory.h"
#include "filemgr.h"
#include "handle_dots.h"
#include "build.h"

static const char *pathname;

node *
SPdoLocateSource (node *syntax_tree)
{
    DBUG_ENTER ();

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

static char *
CreateInfoMacroCommandLine (void)
{
    char *res;

    DBUG_ENTER ();

    res = STRcatn (3 * 4 + 1, " ", "-DSAC_REVISION=", build_rev, " ",
                   "-DSAC_REVISION_SHORT=", build_srev, " ",
                   "-DSAC_BUILD_STYLE=", build_style, " ", "-DSAC_BACKEND_",
                   global.backend_string[global.backend], " ");

    DBUG_RETURN (res);
}

node *
SPdoRunPreProcessor (node *syntax_tree)
{
    int err;
    char *tmp, *cppcallstr;
    char *define;

    DBUG_ENTER ();

    global.filename = global.puresacfilename;
    define = CreateInfoMacroCommandLine ();

    if (pathname == NULL) {
        cppcallstr = STRcatn (3, global.config.cpp_stdin, define,
                              global.cpp_options == NULL ? " " : global.cpp_options);
    } else {
        cppcallstr = STRcatn (5, global.config.cpp_file, define,
                              global.cpp_options == NULL ? " " : global.cpp_options, " ",
                              pathname);
#if 0
    pathname = MEMfree( pathname);
#endif
    }

    /* The sed command is needed to remove a pragma that is inserted by the
       Apple GCC 3.3 on Panther   */
    tmp = STRcatn (4, cppcallstr, " | sed '/^#pragma GCC set_debug_pwd/d' > ",
                   global.tmp_dirname, "/source");

    cppcallstr = MEMfree (cppcallstr);
    cppcallstr = tmp;

    if (global.show_syscall) {
        CTInote ("err = system( \"%s\")", cppcallstr);
    }

    err = system (cppcallstr);

    cppcallstr = MEMfree (cppcallstr);

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

    DBUG_ENTER ();

    cppfile = STRcat (global.tmp_dirname, "/source");

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

    cppfile = MEMfree (cppfile);

    if (err) {
        CTIabort ("Could not delete /tmp-file");
    }

    if (global.syntax_tree == NULL)
        CTIabort ("Failed to construct a syntax tree for `%s'\n", global.filename);

    FMGRsetFileNames (global.syntax_tree);

    DBUG_RETURN (global.syntax_tree);
}

#undef DBUG_PREFIX
