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
#include "options.h"
#include "ctinfo.h"
#include "globals.h"
#include "resource.h"
#include "libstat.h"
#include "filemgr.h"
#include "dbug.h"

#include <stdlib.h>
#include <locale.h>

static node *
SetupCompiler (int argc, char *argv[])
{
    node *syntax_tree = NULL;

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

    CTIabortOnError ();

    DBUG_RETURN (syntax_tree);
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
  syntax_tree = PHrunCompilerPhase( PH_##it_element, syntax_tree,

#define PHASEcond(it_cond)                                                               \
  it_cond);

#include "phase_info.mac"

#undef SUBPHASEelement
#undef SUBPHASEcond

    /*
     * Now, we are done.
     */

    CTIterminateCompilation (syntax_tree);

    return (0);
}
