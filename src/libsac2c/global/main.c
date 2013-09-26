/*
 *  This file contains the start functions for the various sac tools.
 */

#include "sactools.h"
#include "types.h"
#include "phase_drivers.h"
#include "options.h"
#include "ctinfo.h"
#include "cctools.h"
#include "globals.h"
#include "resource.h"
#include "libstat.h"
#include "filemgr.h"

#define DBUG_PREFIX "SAC"
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
    } else if (global.do_clink != DO_C_none) {
        CCTperformTask (CCT_clinkonly);
        CTIterminateCompilationSilent ();
    } else if (global.do_ccompile != DO_C_none) {
        CCTperformTask (CCT_ccompileonly);
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

    /* Set custom exit function to make DBUG_ASSERT
       macro correctly terminate the compilation
       process.  */
#ifndef DBUG_OFF
    set_debug_exit_function (CTIexit);
#endif

    setlocale (LC_ALL, "en_US");
    CTIinstallInterruptHandlers ();
    OPTcheckPreSetupOptions (argc, argv);

    GLOBinitializeGlobal (argc, argv, tool, toolname);
    OPTanalyseCommandline (argc, argv);

    RSCevaluateConfiguration ();

    OPTcheckPostSetupOptions ();

    GLOBsetupBackend ();

    FMGRsetupPaths ();
    FMGRcreateTmpDir ();

    OPTcheckOptionConsistency ();
    HandleSpecialOptions ();

    CTIabortOnError ();

    DBUG_RETURN (syntax_tree);
}

#ifdef __cplusplus
extern "C" {
int SACrunSac2c (int argc, char *argv[]);
int SACrunSac4c (int argc, char *argv[]);
int SACrunSac2tex (int argc, char *argv[]);
}
#endif

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
