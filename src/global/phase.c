/*
 *
 * $Log$
 * Revision 1.1  2005/03/07 13:40:22  cg
 * Initial revision
 *
 *
 */

#include "types.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "internal_lib.h"

#include "setup.h"
#include "options.h"
#include "resource.h"

#include "phase.h"

static node *
PhaseDummy (node *syntax_tree)
{
    return (syntax_tree);
}

typedef node *(*phase_fun_p) (node *);

static const char *phase_name[] = {
#define PHASEtext(it_text) it_text,
#include "phase_info.mac"
  ""};

static const phase_fun_p phase_fun[] = {
#define PHASEfun(it_fun) it_fun,
#include "phase_info.mac"
  PhaseDummy};

static const char *subphase_name[] = {
#define SUBPHASEtext(it_text) it_text,
#include "phase_info.mac"
  ""};

static const char *subphase_specifier[] = {
#define SUBPHASEspec(it_spec) it_spec,
#include "phase_info.mac"
  ""};

static const phase_fun_p subphase_fun[] = {
#define SUBPHASEfun(it_fun) it_fun,
#include "phase_info.mac"
  PhaseDummy};

const char *
PHphaseName (compiler_phase_t phase)
{
    DBUG_ENTER ("PHphaseName");

    DBUG_RETURN (phase_name[phase]);
}

const char *
PHsubPhaseName (compiler_subphase_t subphase)
{
    DBUG_ENTER ("PHsubPhaseName");

    DBUG_RETURN (subphase_name[subphase]);
}

node *
PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree)
{
    DBUG_ENTER ("PHrunCompilerPhase");

    global.compiler_phase = phase;

#ifndef DBUG_OFF
    if ((global.my_dbug) && (!global.my_dbug_active)
        && (global.compiler_phase >= global.my_dbug_from)
        && (global.compiler_phase <= global.my_dbug_to)) {
        DBUG_PUSH (global.my_dbug_str);
        global.my_dbug_active = 1;
    }
#endif

    CTIstate (" ");
    CTIstate ("** %d: %s ...", (int)phase, PHphaseName (phase));

    syntax_tree = phase_fun[phase](syntax_tree);

    CTIabortOnError ();

    DBUG_EXECUTE ("MEM_LEAK", ILIBdbugMemoryLeakCheck (););

    if (global.treecheck) {
        syntax_tree = CHKdoTreeCheck (syntax_tree);
    }

    if ((global.my_dbug) && (global.my_dbug_active)
        && (global.compiler_phase >= global.my_dbug_to)) {
        DBUG_POP ();
        global.my_dbug_active = 0;
    }

    if (global.break_after == phase) {
        CTIterminateCompilation (phase, NULL, syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCompilerSubPhase (compiler_subphase_t subphase, node *syntax_tree)
{
    DBUG_ENTER ("PHrunCompilerSubPhase");

    global.compiler_subphase = subphase;

    CTIstate ("**** %s ...", PHsubPhaseName (subphase));

    syntax_tree = subphase_fun[subphase](syntax_tree);

    CTIabortOnError ();

    if (ILIBstringCompare (global.break_specifier, subphase_specifier[subphase])) {
        CTIterminateCompilation (global.compiler_phase, global.break_specifier,
                                 syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}
