/*
 *
 * $Id$
 *
 */

/*
 *  this file contains the main function of the SAC->C compiler!
 */

#include "phase.h"

#include "types.h"
#include "internal_lib.h"
#include "options.h"
#include "ctinfo.h"
#include "globals.h"
#include "resource.h"
#include "libstat.h"
#include "filemgr.h"

#include <stdlib.h>
#include <locale.h>

static node *
SetupCompiler (int argc, char *argv[])
{
    DBUG_ENTER ("SetupCompiler");

    setlocale (LC_ALL, "en_US");
    CTIinstallInterruptHandlers ();
    OPTcheckPreSetupOptions (argc, argv);

    GLOBinitializeGlobal (argc, argv);
    OPTanalyseCommandline (argc, argv);

    RSCevaluateConfiguration ();

    FMGRsetupPaths ();
    FMGRcreateTmpDir ();

    LIBSprintLibStat ();

    DBUG_RETURN (NULL);
}

/*
 *  And now, the main function which triggers the whole compilation.
 */

int
main (int argc, char *argv[])
{
    node *syntax_tree;

    /*
     * We must set up the compiler infrastructure first.
     */

    syntax_tree = SetupCompiler (argc, argv);

    /*
     * The sequence of compiler phases is derived from phase_info.mac.
     */

#define PHASEelement(it_element)                                                         \
    syntax_tree = PHrunCompilerPhase (PH_##it_element, syntax_tree);

#include "phase_info.mac"

#undef SUBPHASEelement

    /*
     * Now, we are done.
     */

    CTIterminateCompilation (PH_final, "", syntax_tree);

    return (0);
}
