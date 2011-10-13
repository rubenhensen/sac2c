/*
 *
 * $Id$
 *
 */

/*
 *  This file contains the start functions for the various sac tools.
 */

#include "sactools.h"
#include "types.h"
#include "phase_drivers.h"
#include "options.h"
#include "ctinfo.h"
#include "globals.h"
#include "resource.h"
#include "libstat.h"
#include "filemgr.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include <stdlib.h>
#include <locale.h>

/*
 *  Here we handle special options that do not initiate any
 *  compilation process.
 */

static void
HandleSpecialOptions (void)
{
    DBUG_ENTER ();

    if (global.printConfig != NULL) {
        RSCprintConfigEntry (global.printConfig);
        CTIterminateCompilationSilent ();
    } else if (global.libstat) {
        LIBSprintLibStat ();
        CTIterminateCompilationSilent ();
    }

    DBUG_RETURN ();
}

/*
 *  First, we need to set up the compile infrastructure.
 */

static node *
SetupCompiler (int argc, char *argv[], tool_t tool, char *toolname)
{
    node *syntax_tree = NULL;

    DBUG_ENTER ();

    setlocale (LC_ALL, "en_US");
    CTIinstallInterruptHandlers ();
    OPTcheckPreSetupOptions (argc, argv);

    GLOBinitializeGlobal (argc, argv, tool, toolname);
    OPTanalyseCommandline (argc, argv);

    RSCevaluateConfiguration ();

    GLOBsetupBackend ();

    FMGRsetupPaths ();
    FMGRcreateTmpDir ();

    HandleSpecialOptions ();
    OPTcheckOptionConsistency ();

    CTIabortOnError ();

    DBUG_RETURN (syntax_tree);
}

/*
 *  And now, the main function which triggers the whole compilation.
 */

int
SACrunSac2c (int argc, char *argv[])
{
    node *syntax_tree;

    DBUG_ENTER ();

    syntax_tree = SetupCompiler (argc, argv, TOOL_sac2c, "sac2c");

    syntax_tree = PHDdriveSac2c (syntax_tree);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (0);
}

/*
 *  And now, the main function which triggers the whole compilation.
 */

int
SACrunSac4c (int argc, char *argv[])
{
    node *syntax_tree;

    DBUG_ENTER ();

    syntax_tree = SetupCompiler (argc, argv, TOOL_sac4c, "sac4c");

    syntax_tree = PHDdriveSac4c (syntax_tree);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (0);
}

/*
 *  And now, the main function which triggers the whole compilation.
 */

int
SACrunSac2tex (int argc, char *argv[])
{
    node *syntax_tree;

    DBUG_ENTER ();

    syntax_tree = SetupCompiler (argc, argv, TOOL_sac2tex, "sac2tex");

    syntax_tree = PHDdriveSac2tex (syntax_tree);

    CTIterminateCompilation (syntax_tree);

    DBUG_RETURN (0);
}

#undef DBUG_PREFIX
