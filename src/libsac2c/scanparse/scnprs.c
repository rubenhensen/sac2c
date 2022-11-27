#include <stdio.h>

#include "scnprs.h"
#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"
#include "ctinfo.h"
#include "str.h"
#include "globals.h"
#include "memory.h"
#include "filemgr.h"
#include "build.h"
#include "system.h"

static const char *pathname;

node *
SPdoLocateSource (node *syntax_tree)
{
    DBUG_ENTER ();

    global.filename = global.puresacfilename;

    if (global.sacfilename == NULL) {
        pathname = NULL;
        CTInote (EMPTY_LOC, "Reading from stdin ...");
    } else {
        pathname = FMGRfindFile (PK_path, global.sacfilename);

        if (pathname == NULL) {
            CTIabort (EMPTY_LOC, "Unable to open file \"%s\"", global.sacfilename);
        }

        CTInote (EMPTY_LOC, "Reading from file \"%s\" ...", pathname);
    }

    DBUG_RETURN (syntax_tree);
}

static char *
CreateInfoMacroCommandLine (void)
{
    char *res;

    DBUG_ENTER ();

    res = STRcatn (5 * 3 + 1, " ",
                   "-DSAC_BUILD_STYLE=", build_style, " ",
                   "-DSAC_BACKEND_", global.backend_string[global.backend], " ",
                   "-DSAC_BACKEND_STRING='\"", global.backend_string[global.backend], "\"' ",
                   "-DSAC_TARGET_", global.target_name, " ",
                   "-DSAC_TARGET_STRING='\"", global.target_name, "\"' ");

    DBUG_RETURN (res);
}

node *
SPdoRunPreProcessor (node *syntax_tree)
{
    char *define;
    char *spathname, *stmpdir;

    DBUG_ENTER ();

    global.filename = global.puresacfilename;
    define = CreateInfoMacroCommandLine ();
    spathname = SYSsanitizePath (pathname);
    stmpdir = SYSsanitizePath (global.tmp_dirname);

    /* The sed command is needed to remove a pragma that is inserted by the
       Apple GCC 3.3 on Panther   */

    SYScall ("%s %s %s %s %s > %s/source.tmp && %s '/^#pragma GCC set_debug_pwd/d' < "
             "%s/source.tmp > %s/source",
             (spathname == NULL) ? global.config.cpp_stdin : global.config.cpp_file,
             define, global.config_macros,
             (global.cpp_options == NULL) ? " " : global.cpp_options,
             (spathname == NULL) ? " " : spathname, stmpdir, global.config.sed,
             stmpdir, stmpdir);

    MEMfree (spathname);
    MEMfree (stmpdir);
    MEMfree (define);

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
        CTInote (EMPTY_LOC, "yyin = fopen( \"%s\", \"r\")", cppfile);
    }

    yyin = fopen (cppfile, "r");

    if ((yyin == NULL) || (ferror (yyin))) {
        CTIabort (EMPTY_LOC, "C preprocessing failed");
    }

    global.start_token = PARSE_PRG;

    SPmyYyparse ();

    if (global.show_syscall) {
        CTInote (EMPTY_LOC, "err = fclose( yyin)");
    }

    err = fclose (yyin);
    if (err) {
        CTIabort (EMPTY_LOC, "C preprocessor error");
    }

    if (global.show_syscall) {
        CTInote (EMPTY_LOC, "err = remove( \"%s\")", cppfile);
    }

    err = remove (cppfile);

    cppfile = MEMfree (cppfile);

    if (err) {
        CTIabort (EMPTY_LOC, "Could not delete /tmp-file");
    }

    if (global.syntax_tree == NULL)
        CTIabort (EMPTY_LOC, "Failed to construct a syntax tree for `%s'\n", global.filename);

    FMGRsetFileNames (global.syntax_tree);

    DBUG_RETURN (global.syntax_tree);
}

#undef DBUG_PREFIX
