/* $Id$ */

#include "uniqueness_phase.h"

#include "phase.h"
#include "dbug.h"

node *
UPdoUniquenessInference (node *syntax_tree)
{
    DBUG_ENTER ("UPdoUniquenessInference");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_cua, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_iuq, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
