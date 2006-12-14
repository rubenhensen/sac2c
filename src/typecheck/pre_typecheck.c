/*
 * $Id$
 */

#include "resolve_reference_args.h"

#include "phase.h"
#include "dbug.h"

node *
PTCdoPreTypecheck (node *syntax_tree)
{
    DBUG_ENTER ("PTCdoPreTypecheck");

    syntax_tree = PHrunCompilerSubPhase (SUBPH_rst, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_insvd, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_instc, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ses, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_crtwrp, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_oan, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_goi, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rso, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_rra, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ewt, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_lac2fun, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_elf, syntax_tree);
    syntax_tree = PHrunCompilerSubPhase (SUBPH_ssa, syntax_tree);

    DBUG_RETURN (syntax_tree);
}
