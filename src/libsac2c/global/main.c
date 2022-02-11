/*! \file
    This file contains the start functions for the various sac tools.  */
#include "sactools.h"
#include "types.h"
#include "phase_drivers.h"
#include "options.h"
#include "ctinfo.h"
#include "memory.h"
#include "cctools.h"
#include "globals.h"
#include "resource.h"
#include "libstat.h"
#include "filemgr.h"

#define DBUG_PREFIX "SAC"
#include "debug.h"

#include "config.h"

#include <stdlib.h>
#include <locale.h>

/*! Here we handle special options that do not initiate any
    compilation process.  */
static void
HandleSpecialOptions (void)
{
    DBUG_ENTER ();

    if (global.printConfig != NULL) {
        RSCprintConfigEntry (global.printConfig);
        CTIexit (EXIT_SUCCESS);
    } else if (global.libstat) {
        LIBSprintLibStat ();
        CTIexit (EXIT_SUCCESS);
    } else if (global.do_clink != DO_C_none) {
        CCTperformTask (CCT_clinkonly);
        CTIexit (EXIT_SUCCESS);
    } else if (global.do_ccompile != DO_C_none) {
        CCTperformTask (CCT_ccompileonly);
        CTIexit (EXIT_SUCCESS);
    }

    DBUG_RETURN ();
}

/*! Prepare the compiler: parse options, parse configuration files
    and initialise global values.
    \param argc     Number of arguments in the \a argv list
    \param argv     Argument list
    \param tool     Identifier of the tool (enum values)
    \param toolname The name of the tool, e.g. sac2c  */
static node *
SetupCompiler (int argc, char *argv[], tool_t tool, char *toolname)
{
    node *syntax_tree = NULL;

    DBUG_ENTER ();

    /* Set custom exit function to make DBUG_ASSERT
       macro correctly terminate the compilation
       process.  */
    set_debug_exit_function (CTIexit);

    CTIset_stderr (stderr);

    setlocale (LC_ALL, "en_US");
    CTIinstallInterruptHandlers ();
    OPTcheckPreSetupOptions (argc, argv);

    GLOBinitializeGlobal (argc, argv, tool, toolname);
    OPTanalyseCommandline (argc, argv);

    RSCevaluateConfiguration ();

    GLOBsetupBackend ();

    OPTcheckPostSetupOptions ();

    /* For the distributed memory backend
       determine the communication library setting. */
    if (global.backend == BE_distmem) {
        GLOBsetupDistMemCommLib ();
    }

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

/*! Triggers compilation of sac2c.  */
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

/*! Trigger compilation of sac4c.  */
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

/*! Trigger compilation of sac2tex.  */
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

/**
 * This function is intended to be used only by the binaries sac2c,
 * sac4c, and sac2tex. It is used to do a version comparison between
 * what is set in the binary and what is set here in the library.
 * The idea is that with version checking, we can more smartly decide
 * on whether we should look for/load the globally install version of
 * this library or only load the build version.
 *
 * The use of the `-dirty' postfix given by `git describe' helps us to
 * determine whether the binary is from a non-committed build or not.
 */
char *
getLibsac2cVersion (void)
{
    return SAC2C_VERSION;
}

#undef DBUG_PREFIX
