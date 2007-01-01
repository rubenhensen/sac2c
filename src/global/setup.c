/*
 * $Id$
 */

#include <locale.h>

#include "setup.h"

#include "types.h"
#include "dbug.h"
#include "phase.h"
#include "globals.h"
#include "ctinfo.h"
#include "DupTree.h"
#include "filemgr.h"
#include "options.h"

void
SETUPdoSetupCompiler (int argc, char *argv[])
{
    DBUG_ENTER ("SETUPdoSetupCompiler");

    setlocale (LC_ALL, "en_US");
    CTIinstallInterruptHandlers ();

    OPTcheckPreSetupOptions ();
    GLOBinitializeGlobal (argc, argv);

    DUPinitDupTree ();

    DBUG_VOID_RETURN;
}

node *
SETUPdoInitializations (node *syntax_tree)
{
    DBUG_ENTER ("SETUPdoInitializations");

    FMGRcreateTmpDir ();

    DBUG_RETURN (syntax_tree);
}

node *
SETUPdriveSetup (node *syntax_tree)
{
    DBUG_ENTER ("SETUPdriveSetup");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_opt, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rsc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_init, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
