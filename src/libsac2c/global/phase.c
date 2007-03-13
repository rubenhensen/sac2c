/*
 * $Id$
 */

#include "phase.h"

#include "dbug.h"
#include "ctinfo.h"
#include "globals.h"
#include "str.h"
#include "memory.h"
#include "tree_basic.h"
#include "DupTree.h"
#include "check.h"
#include "check_mem.h"
#include "phase_drivers.h"
#include "phase_info.h"
#include "statistics.h"

/*
 * static global variables
 */

static int cycle_counter;
static optimize_counter_t oc_global;
static optimize_counter_t oc_pass;

#ifndef DBUG_OFF

static void
CheckEnableDbug (compiler_phase_t phase)
{
    DBUG_ENTER ("CheckEnableDbug");

    if (global.my_dbug && (phase >= global.my_dbug_from) && !global.my_dbug_active) {
        DBUG_PUSH (global.my_dbug_str);
        global.my_dbug_active = TRUE;
    }

    DBUG_VOID_RETURN;
}

static void
CheckDisableDbug (compiler_phase_t phase)
{
    DBUG_ENTER ("CheckDisableDbug");

    if (global.my_dbug && global.my_dbug_active && (phase >= global.my_dbug_to)) {
        DBUG_POP ();
        global.my_dbug_active = FALSE;
    }

    DBUG_VOID_RETURN;
}

#endif /* DBUG_OFF */

node *
PHrunPhase (compiler_phase_t phase, node *syntax_tree, bool cond)
{
    static int phase_num = 0;

    DBUG_ENTER ("PHrunPhase");

    DBUG_ASSERTF (PHIphaseType (phase) == PHT_phase,
                  ("PHrunPhase called with incompatible phase: %s",
                   PHIphaseIdent (phase)));

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunPhase called with non N_module node");

    global.compiler_phase = phase;
    global.compiler_anyphase = phase;
    phase_num += 1;

#ifndef DBUG_OFF
    CheckEnableDbug (phase);
#endif

    CTInote (" ");

    if (cond) {
        CTIstate ("** %2d: %s ...", phase_num, PHIphaseText (phase));
        syntax_tree = PHIphaseFun (phase) (syntax_tree);

        CTIabortOnError ();

#ifdef SHOW_MALLOC
        DBUG_EXECUTE ("MEM_LEAK", MEMdbugMemoryLeakCheck (););
#endif

        if (global.treecheck && (syntax_tree != NULL)) {
            syntax_tree = CHKdoTreeCheck (syntax_tree);
        }

#ifdef SHOW_MALLOC
        if (global.memcheck && (syntax_tree != NULL)) {
            syntax_tree = CHKMdoMemCheck (syntax_tree);
        }
#endif
    } else {
        CTIstate ("** %2d: %s skipped.", phase_num, PHIphaseText (phase));
    }

#ifndef DBUG_OFF
    CheckDisableDbug (phase);
#endif

    if (global.break_after_phase == phase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunSubPhase (compiler_phase_t subphase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ("PHrunSubPhase");

    DBUG_ASSERTF (PHIphaseType (subphase) == PHT_subphase,
                  ("PHrunSubPhase called with incompatible phase: %s",
                   PHIphaseIdent (subphase)));

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunSubPhase called with non N_module node");

    global.compiler_subphase = subphase;
    global.compiler_anyphase = subphase;

#ifndef DBUG_OFF
    CheckEnableDbug (subphase);
#endif

    if (cond) {
        if (PHIphaseType (subphase) != PHT_cycle) {
            CTInote ("**** %s ...", PHIphaseText (subphase));
        }
        syntax_tree = PHIphaseFun (subphase) (syntax_tree);
        CTIabortOnError ();

        if (global.treecheck && (syntax_tree != NULL)) {
            syntax_tree = CHKdoTreeCheck (syntax_tree);
        }

#ifdef SHOW_MALLOC
        if (global.memcheck && (syntax_tree != NULL)) {
            syntax_tree = CHKMdoMemCheck (syntax_tree);
        }
#endif
    }

#ifndef DBUG_OFF
    CheckDisableDbug (subphase);
#endif

    if (global.break_after_subphase == subphase) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCycle (compiler_phase_t cycle, node *syntax_tree, bool cond)
{
    bool go_on;

    DBUG_ENTER ("PHrunCycle");

    DBUG_ASSERTF (PHIphaseType (cycle) == PHT_cycle,
                  ("PHrunCycle called with incompatible phase: %s",
                   PHIphaseIdent (cycle)));

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCycle called with wrong node type.");

    global.compiler_subphase = cycle;
    global.compiler_anyphase = cycle;

    if (cond) {

#ifndef DBUG_OFF
        CheckEnableDbug (cycle);
#endif

        STATcopyCounters (&oc_global, &global.optcounters);
        STATclearCounters (&global.optcounters);

        cycle_counter = 1;
        do {
            CTInote (" ");
            CTInote ("**** %s pass: %i", PHIphaseText (cycle), cycle_counter);
            STATclearCounters (&oc_pass);

            syntax_tree = PHIphaseFun (cycle) (syntax_tree);

            CTIabortOnError ();

            if (global.treecheck && (syntax_tree != NULL)) {
                syntax_tree = CHKdoTreeCheck (syntax_tree);
            }

#ifdef SHOW_MALLOC
            if (global.memcheck && (syntax_tree != NULL)) {
                syntax_tree = CHKMdoMemCheck (syntax_tree);
            }
#endif

            STATaddCounters (&oc_pass, &global.optcounters);
            STATclearCounters (&global.optcounters);

            if (STATdidSomething (&oc_pass)) {
                go_on = TRUE;
                STATaddCounters (&oc_global, &oc_pass);
            } else {
                go_on = FALSE;
                CTInote (" ");
            }

            cycle_counter += 1;

        } while (go_on && (cycle_counter <= global.max_optcycles)
                 && ((cycle_counter <= global.break_cycle_specifier)
                     || (global.break_after_cyclephase > global.compiler_cyclephase)));

        STATcopyCounters (&global.optcounters, &oc_global);

        if (go_on && (cycle_counter == global.max_optcycles)) {
            CTIwarn ("Maximum number of optimization cycles reached");
            global.run_stabilization_cycle = TRUE;
        }

#ifndef DBUG_OFF
        CheckDisableDbug (cycle);
#endif
    }

    if (global.break_after_subphase == cycle) {
        CTIterminateCompilation (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCyclePhase (compiler_phase_t cyclephase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ("PHrunCyclePhase");

    DBUG_ASSERTF (PHIphaseType (cyclephase) == PHT_cyclephase,
                  ("PHrunPhase called with incompatible phase: %s",
                   PHIphaseIdent (cyclephase)));

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCyclePhase called with wrong node type.");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (cycle_counter < global.break_cycle_specifier))) {
        CTInote ("****** %s ...", PHIphaseText (cyclephase));

        syntax_tree = PHIphaseFun (cyclephase) (syntax_tree);

        CTIabortOnError ();
    }

#ifndef DBUG_OFF
    CheckDisableDbug (cyclephase);
#endif

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCycleFun (compiler_phase_t cycle, node *syntax_tree)
{
    node *fundef;

    DBUG_ENTER ("PHrunCycleFun");

    DBUG_ASSERTF (PHIphaseType (cycle) == PHT_cycle_fun,
                  ("PHrunPhase called with incompatible phase: %s",
                   PHIphaseIdent (cycle)));

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCycleFun called with wrong node type.");

    STATaddCounters (&oc_pass, &global.optcounters);
    STATclearCounters (&global.optcounters);

    fundef = MODULE_FUNS (syntax_tree);

    while (fundef != NULL) {
        if (!FUNDEF_ISZOMBIE (fundef) && !FUNDEF_ISTYPEERROR (fundef)
            && FUNDEF_WASOPTIMIZED (fundef)) {
            CTItell (4, " ");

            if (FUNDEF_ISWRAPPERFUN (fundef)) {
                CTInote ("****** Optimizing wrapper function:");
            } else if (FUNDEF_ISDOFUN (fundef)) {
                CTInote ("****** Optimizing loop function:");
            } else if (FUNDEF_ISCONDFUN (fundef)) {
                CTInote ("****** Optimizing conditional function:");
            } else {
                CTInote ("****** Optimizing regular function:");
            }

            CTInote ("******  %s( %s): ...", CTIitemName (fundef), CTIfunParams (fundef));

            FUNDEF_ISINLINECOMPLETED (fundef) = FALSE;

            fundef = PHIphaseFun (cycle) (fundef);

            CTIabortOnError ();

            FUNDEF_WASOPTIMIZED (fundef) = STATdidSomething (&global.optcounters);

            if (FUNDEF_WASOPTIMIZED (fundef)) {
                STATaddCounters (&oc_pass, &global.optcounters);
                STATclearCounters (&global.optcounters);
            }

            DBUG_EXECUTE ("OPT", STATprint (&global.optcounters););
        }

        if (FUNDEF_NEXT (fundef) == NULL) {
            FUNDEF_NEXT (fundef) = DUPgetCopiedSpecialFundefs ();
        }

        fundef = FUNDEF_NEXT (fundef);
    }

    if (global.treecheck && (syntax_tree != NULL)) {
        syntax_tree = CHKdoTreeCheck (syntax_tree);
    }

#ifdef SHOW_MALLOC
    if (global.memcheck && (syntax_tree != NULL)) {
        syntax_tree = CHKMdoMemCheck (syntax_tree);
    }
#endif

    DBUG_RETURN (syntax_tree);
}

node *
PHrunCyclePhaseFun (compiler_phase_t cyclephase, node *arg_node, bool cond)
{
    DBUG_ENTER ("PHrunCyclePhaseFun");

    DBUG_ASSERTF (PHIphaseType (cyclephase) == PHT_cyclephase_fun,
                  ("PHrunCyclePhaseFun called with incompatible phase: %s",
                   PHIphaseIdent (cyclephase)));

    DBUG_ASSERT ((arg_node != NULL) && (NODE_TYPE (arg_node) == N_fundef),
                 "PHrunCyclePhaseFun called with wrong node type.");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (cycle_counter < global.break_cycle_specifier))) {

        CTItell (4, "         %s ...", PHIphaseText (cyclephase));

        arg_node = PHIphaseFun (cyclephase) (arg_node);

        CTIabortOnError ();
    }

#ifndef DBUG_OFF
    CheckDisableDbug (cyclephase);
#endif

    DBUG_RETURN (arg_node);
}
