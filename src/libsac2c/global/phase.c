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
#include "tree_compound.h"
#include "DupTree.h"
#include "check.h"
#include "check_mem.h"
#include "phase_drivers.h"
#include "phase_info.h"

#ifndef DBUG_OFF

static void
CheckEnableDbug (compiler_phase_t phase)
{
    DBUG_ENTER ("CheckEnableDbug");

    if (global.my_dbug && (phase == global.my_dbug_from)) {
        DBUG_PUSH (global.my_dbug_str);
        global.my_dbug_active = TRUE;
    }

    DBUG_VOID_RETURN;
}

static void
CheckDisableDbug (compiler_phase_t phase)
{
    DBUG_ENTER ("CheckDisableDbug");

    if (global.my_dbug && global.my_dbug_active && (phase == global.my_dbug_to)) {
        DBUG_POP ();
        global.my_dbug_active = FALSE;
    }

    DBUG_VOID_RETURN;
}

#endif /* DBUG_OFF */

node *
PHrunCompilerPhase (compiler_phase_t phase, node *syntax_tree, bool cond)
{
    static int phase_num = 0;

    DBUG_ENTER ("PHrunCompilerPhase");

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerPhase called with non N_module node");

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

#ifdef SHOW_MALLOC
        if (global.treecheck && (syntax_tree != NULL)) {
            syntax_tree = CHKdoTreeCheck (syntax_tree);
        }

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
PHrunCompilerSubPhase (compiler_phase_t subphase, node *syntax_tree, bool cond)
{
    DBUG_ENTER ("PHrunCompilerSubPhase");

    DBUG_ASSERT ((syntax_tree == NULL) || (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerSubPhase called with non N_module node");

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

#ifdef SHOW_MALLOC
        if (global.treecheck && (syntax_tree != NULL)) {
            syntax_tree = CHKdoTreeCheck (syntax_tree);
        }

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
PHrunCompilerCyclePhase (compiler_phase_t cyclephase, int pass, node *syntax_tree,
                         bool cond)
{
    DBUG_ENTER ("PHrunCompilerCyclePhase");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    DBUG_ASSERT ((syntax_tree != NULL) && (NODE_TYPE (syntax_tree) == N_module),
                 "PHrunCompilerCyclePhase called with wrong node type.");

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (pass < global.break_cycle_specifier))) {
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
PHrunCompilerCyclePhaseFun (compiler_phase_t cyclephase, int pass, node *arg_node,
                            bool cond)
{
    DBUG_ENTER ("PHrunCompilerCyclePhaseFun");

    global.compiler_cyclephase = cyclephase;
    global.compiler_anyphase = cyclephase;

#ifndef DBUG_OFF
    CheckEnableDbug (cyclephase);
#endif

    DBUG_ASSERT ((arg_node != NULL) && (NODE_TYPE (arg_node) == N_fundef),
                 "PHrunCompilerCyclePhaseFun called with wrong node type.");

    if (cond
        && ((cyclephase <= global.break_after_cyclephase)
            || (pass < global.break_cycle_specifier))) {

        CTItell (4, "         %s ...", PHIphaseText (cyclephase));

        arg_node = PHIphaseFun (cyclephase) (arg_node);

        CTIabortOnError ();
    }

#ifndef DBUG_OFF
    CheckDisableDbug (cyclephase);
#endif

    DBUG_RETURN (arg_node);
}
