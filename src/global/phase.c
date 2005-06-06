/*
 *
 * $Log$
 * Revision 1.4  2005/06/06 13:26:40  jhb
 * added the check traversal after each subphase
 *
 * Revision 1.3  2005/04/20 07:25:18  cg
 * CheckTree is now only called if syntax tree actually exists.
 *
 * Revision 1.2  2005/03/10 09:41:09  cg
 * External declarations of phase driver functions are now created
 * automatically from phase_info.mac
 *
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

#include "phase.h"

#define PHASEfun(it_fun) extern node *it_fun (node *syntax_tree);
#define SUBPHASEfun(it_fun) extern node *it_fun (node *syntax_tree);

#include "phase_info.mac"

#undef PHASEfun
#undef SUBPHASEfun

typedef node *(*phase_fun_p) (node *);

static const char *phase_name[] = {
#define PHASEtext(it_text) it_text,
#include "phase_info.mac"
  ""};

static const phase_fun_p phase_fun[] = {
#define PHASEfun(it_fun) it_fun,
#include "phase_info.mac"
  PHdummy};

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
  PHdummy};

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

    if (global.treecheck && (syntax_tree != NULL)) {
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

    if ((global.treecheck) && (syntax_tree != NULL)) {
        syntax_tree = CHKdoTreeCheck (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHdummy (node *syntax_tree)
{
    DBUG_ENTER ("PHdummy");

    DBUG_ASSERT (FALSE, "This function should never be called.");

    DBUG_RETURN (syntax_tree);
}
