/*
 * $Id$
 */

#include "types.h"
#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "check.h"
#include "check_mem.h"

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

static compiler_subphase_t firstoptimization;

static bool
BreakAfterEarlierOptimization (compiler_subphase_t phase)
{
    bool res = FALSE;
    compiler_subphase_t sp = firstoptimization;

    DBUG_ENTER ("BreakAfterEarlierOptimization");

    while (sp != phase) {
        if (ILIBstringCompare (global.break_opt_specifier, subphase_specifier[sp])) {
            res = TRUE;
        }
        sp++;
    }

    DBUG_RETURN (res);
}

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

void
PHsetFirstOptimization (compiler_subphase_t subphase)
{
    DBUG_ENTER ("PHsetFirstOptimization");

    firstoptimization = subphase;

    DBUG_VOID_RETURN;
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

#ifdef SHOW_MALLOC
    DBUG_EXECUTE ("MEM_LEAK", MEMdbugMemoryLeakCheck (););
#endif

#ifdef SHOW_MALLOC
    if (global.treecheck && (syntax_tree != NULL)) {
        syntax_tree = CHKdoTreeCheck (syntax_tree);
    }

    if (global.memcheck && (syntax_tree != NULL)) {
        syntax_tree = CHKMdoMemCheck (syntax_tree);
    }
#endif

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

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerSubPhase called with non N_module node");

    global.compiler_subphase = subphase;

    CTIstate ("**** %s ...", PHsubPhaseName (subphase));

    syntax_tree = subphase_fun[subphase](syntax_tree);

    CTIabortOnError ();

#ifdef SHOW_MALLOC

    if ((global.treecheck) && (syntax_tree != NULL)) {
        syntax_tree = CHKdoTreeCheck (syntax_tree);
    }

    if (global.memcheck && (syntax_tree != NULL)) {
        syntax_tree = CHKMdoMemCheck (syntax_tree);
    }
#endif

    if ((global.break_after == global.compiler_phase)
        && (ILIBstringCompare (global.break_specifier, subphase_specifier[subphase]))) {
        CTIterminateCompilation (global.compiler_phase, global.break_specifier,
                                 syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunOptimizationInCycle (compiler_subphase_t subphase, int pass, node *syntax_tree)
{
    DBUG_ENTER ("PHrunOptimizationInCycle");

    if ((global.break_after != global.compiler_phase)
        || (!ILIBstringCompare (global.break_specifier,
                                subphase_specifier[global.compiler_subphase]))
        || (global.break_cycle_specifier == -1) || (pass < global.break_cycle_specifier)
        || ((pass == global.break_cycle_specifier)
            && (!BreakAfterEarlierOptimization (subphase)))) {

        DBUG_EXECUTE ("OPT", CTIstate ("****** %s ...", PHsubPhaseName (subphase)););

        syntax_tree = subphase_fun[subphase](syntax_tree);

        CTIabortOnError ();

        if (NODE_TYPE (syntax_tree) == N_fundef) {
            syntax_tree = TCappendFundef (syntax_tree, DUPgetCopiedSpecialFundefs ());
        } else {
            MODULE_FUNS (syntax_tree)
              = TCappendFundef (MODULE_FUNS (syntax_tree), DUPgetCopiedSpecialFundefs ());
        }
    }

    DBUG_RETURN (syntax_tree);
}

bool
PHbreakAfterCurrentPass (int pass)
{
    bool res;

    DBUG_ENTER ("PHbreakAfterCurrentPass");

    res = (ILIBstringCompare (global.break_specifier,
                              subphase_specifier[global.compiler_subphase])
           && (pass == global.break_cycle_specifier));

    DBUG_RETURN (res);
}

node *
PHdummy (node *syntax_tree)
{
    DBUG_ENTER ("PHdummy");

    DBUG_ASSERT (FALSE, "This function should never be called.");

    DBUG_RETURN (syntax_tree);
}
