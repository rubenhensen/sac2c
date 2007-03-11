/*
 *
 * $Id$
 *
 */

#include "dbug.h"
#include "types.h"
#include "phase.h"
#include "phase_info.h"
#include "statistics.h"
#include "ctinfo.h"
#include "dependencies.h"
#include "annotate_fun_calls.h"
#include "type_statistics.h"
#include "tree_basic.h"
#include "check.h"
#include "check_mem.h"
#include "phase_drivers.h"
#include "globals.h"
#include "DupTree.h"

/*
 * Generated cycle driver functions
 */

#define CYCLEname(name)                                                                  \
    node *PHDdriveCycle_##name (node *syntax_tree)                                       \
    {                                                                                    \
        int cycle_counter = 0;                                                           \
        optimize_counter_t oc_global;                                                    \
        optimize_counter_t oc_pass;                                                      \
        bool go_on;                                                                      \
        node *fundef;                                                                    \
        DBUG_ENTER ("PHDdriveCycle_" #name);                                             \
        STATcopyCounters (&oc_global, &global.optcounters);                              \
        STATclearCounters (&global.optcounters);                                         \
        fundef = NULL;                                                                   \
        do {                                                                             \
            cycle_counter += 1;                                                          \
            CTInote (" ");                                                               \
            CTInote ("**** %s pass: %i", PHIphaseText (global.compiler_subphase),        \
                     cycle_counter);                                                     \
            STATclearCounters (&oc_pass);

#define FUNBEGIN(name)                                                                   \
    {                                                                                    \
        STATaddCounters (&oc_pass, &global.optcounters);                                 \
        STATclearCounters (&global.optcounters);                                         \
        fundef = MODULE_FUNS (syntax_tree);                                              \
        while (fundef != NULL) {                                                         \
            if ((!FUNDEF_ISZOMBIE (fundef) && !FUNDEF_ISTYPEERROR (fundef)               \
                 && FUNDEF_WASOPTIMIZED (fundef))) {                                     \
                CTItell (4, " ");                                                        \
                CTInote ("****** Optimizing function:");                                 \
                CTInote ("******  %s( %s): ...", CTIitemName (fundef),                   \
                         CTIfunParams (fundef));                                         \
                FUNDEF_ISINLINECOMPLETED (fundef) = FALSE;

#define FUNEND(name)                                                                     \
    FUNDEF_WASOPTIMIZED (fundef) = STATdidSomething (&global.optcounters);               \
    if (FUNDEF_WASOPTIMIZED (fundef)) {                                                  \
        STATaddCounters (&oc_pass, &global.optcounters);                                 \
        STATclearCounters (&global.optcounters);                                         \
    }                                                                                    \
    DBUG_EXECUTE ("OPT", STATprint (&global.optcounters););                              \
    }                                                                                    \
    if (FUNDEF_NEXT (fundef) == NULL) {                                                  \
        FUNDEF_NEXT (fundef) = DUPgetCopiedSpecialFundefs ();                            \
    }                                                                                    \
    fundef = FUNDEF_NEXT (fundef);                                                       \
    }                                                                                    \
    }

#define CYCLEPHASE(name, text, fun, cond, phase, cycle)                                  \
    syntax_tree = PHrunCompilerCyclePhase (PH_##phase##_##cycle##_##name, cycle_counter, \
                                           syntax_tree, cond);

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle)                               \
    fundef = PHrunCompilerCyclePhaseFun (PH_##phase##_##cycle##_##name, cycle_counter,   \
                                         fundef, cond);

#ifdef SHOW_MALLOC

#define CHECKS()                                                                         \
    if (global.treecheck && (syntax_tree != NULL)) {                                     \
        syntax_tree = CHKdoTreeCheck (syntax_tree);                                      \
    }                                                                                    \
    if (global.memcheck && (syntax_tree != NULL)) {                                      \
        syntax_tree = CHKMdoMemCheck (syntax_tree);                                      \
    }

#else /* SHOW_MALLOC */

#define CHECKS()

#endif /* SHOW_MALLOC */

#define ENDCYCLE(name)                                                                   \
    CHECKS ()                                                                            \
    STATaddCounters (&oc_pass, &global.optcounters);                                     \
    STATclearCounters (&global.optcounters);                                             \
    if (STATdidSomething (&oc_pass)) {                                                   \
        go_on = TRUE;                                                                    \
        STATaddCounters (&oc_global, &oc_pass);                                          \
    } else {                                                                             \
        go_on = FALSE;                                                                   \
        CTInote (" ");                                                                   \
    }                                                                                    \
    }                                                                                    \
    while (go_on && (cycle_counter < global.max_optcycles)                               \
           && ((cycle_counter < global.break_cycle_specifier)                            \
               || (global.break_after_cyclephase > global.compiler_cyclephase)))         \
        ;                                                                                \
    STATcopyCounters (&global.optcounters, &oc_global);                                  \
    if (go_on && (cycle_counter == global.max_optcycles)) {                              \
        CTIwarn ("Maximum number of optimization cycles reached");                       \
        global.run_stabilization_cycle = TRUE;                                           \
    }                                                                                    \
    DBUG_RETURN (syntax_tree);                                                           \
    }

#include "phase_sac2c.mac"

#undef CYCLEname
#undef CYCLEPHASE
#undef ENDCYCLE
#undef FUNBEGIN
#undef FUNEND
#undef CHECKS

/*
 * Generated phase driver functions
 */

#define PHASEname(name)                                                                  \
    node *PHDdrivePhase_##name (node *syntax_tree)                                       \
    {                                                                                    \
        DBUG_ENTER ("PHDdrivePhase_" #name);

#define SUBPHASE(name, text, fun, cond, phase)                                           \
    syntax_tree = PHrunCompilerSubPhase (PH_##phase##_##name, syntax_tree, cond);

#define CYCLE(name, text, cond, phase)                                                   \
    syntax_tree = PHrunCompilerSubPhase (PH_##phase##_##name, syntax_tree, cond);

#define ENDPHASE(name)                                                                   \
    DBUG_RETURN (syntax_tree);                                                           \
    }

#include "phase_sac2c.mac"

#undef PHASEname
#undef SUBPHASE
#undef CYCLE
#undef ENDPHASE

/*
 * Generated tool driver functions
 */

node *
PHDdriveSac2c (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveSac2c");

#define PHASEname(name)                                                                  \
  syntax_tree = PHrunCompilerPhase( PH_##name, syntax_tree,

#define PHASEcond(cond)                                                                  \
  cond);

#include "phase_sac2c.mac"

#undef SUBPHASEname
#undef SUBPHASEcond

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveSac4c (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveSac4c");

#if 0

#define PHASEname(name)                                                                  \
  syntax_tree = PHrunCompilerPhase( PH_##name, syntax_tree,

#define PHASEcond(cond)                                                                  \
  cond);

#include "phase_sac4c.mac"

#undef SUBPHASEname
#undef SUBPHASEcond

#endif

    DBUG_RETURN (syntax_tree);
}
