/*
 *
 * $Id$
 *
 */

#include "dbug.h"
#include "types.h"
#include "phase.h"
#include "globals.h"
#include "ctinfo.h"
#include "dependencies.h"
#include "optimize.h"
#include "annotate_fun_calls.h"
#include "type_statistics.h"
#include "tree_basic.h"
#include "check.h"
#include "check_mem.h"
#include "phase_drivers.h"

/*
 * Generated cycle driver functions
 */

#define OPTCYCLEelement(it_element)                                                      \
    node *PHDdriveOptCycle_##it_element (node *syntax_tree)                              \
    {                                                                                    \
        int cycle_counter = 0;                                                           \
        optimize_counter_t oc_global;                                                    \
        optimize_counter_t oc_pass;                                                      \
        bool go_on;                                                                      \
        DBUG_ENTER ("PHDdriveOptCycle_" #it_element);                                    \
        STATcopyCounters (&oc_global, &global.optcounters);                              \
        STATclearCounters (&global.optcounters);                                         \
        do {                                                                             \
            cycle_counter += 1;                                                          \
            CTInote (" ");                                                               \
            CTInote ("****** Cycle pass: %i", cycle_counter);                            \
            STATclearCounters (&oc_pass);

#define OPTCYCLEFUN()                                                                    \
    {                                                                                    \
        node *fundef;                                                                    \
        STATaddCounters (&oc_pass, &global.optcounters);                                 \
        STATclearCounters (&global.optcounters);                                         \
        fundef = MODULE_FUNS (syntax_tree);                                              \
        while (fundef != NULL) {                                                         \
            if ((!FUNDEF_ISZOMBIE (fundef) && !FUNDEF_ISTYPEERROR (fundef)               \
                 && FUNDEF_WASOPTIMIZED (fundef))) {                                     \
                CTItell (4, " ");                                                        \
                CTInote ("****** Optimizing function:");                                 \
                CTInote ("****** %s( %s): ...", CTIitemName (fundef),                    \
                         CTIfunParams (fundef));                                         \
                FUNDEF_ISINLINECOMPLETED (fundef) = FALSE;

#define OPTINCYCFUNelement(it_element)                                                   \
            fundef = PHrunOptimizationInCycleFun( OIC_##it_element, cycle_counter,

#define OPTINCYCFUNcond(it_cond)                                                         \
  fundef, it_cond);

#define ENDOPTCYCLEFUN()                                                                 \
    FUNDEF_WASOPTIMIZED (fundef) = STATdidSomething (&global.optcounters);               \
    if (FUNDEF_WASOPTIMIZED (fundef)) {                                                  \
        STATaddCounters (&oc_pass, &global.optcounters);                                 \
        STATclearCounters (&global.optcounters);                                         \
    }                                                                                    \
    DBUG_EXECUTE ("OPT", STATprint (&global.optcounters););                              \
    }                                                                                    \
    fundef = FUNDEF_NEXT (fundef);                                                       \
    }                                                                                    \
    }

#define OPTINCYCelement(it_element)                                                      \
      syntax_tree = PHrunOptimizationInCycle( OIC_##it_element, cycle_counter,

#define OPTINCYCcond(it_cond)                                                            \
  syntax_tree, it_cond);

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

#define ENDOPTCYCLE(it_element)                                                          \
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
               || (global.break_after_optincyc > global.compiler_optincyc)))             \
        ;                                                                                \
    STATcopyCounters (&global.optcounters, &oc_global);                                  \
    if (go_on && (cycle_counter == global.max_optcycles)) {                              \
        CTIwarn ("Maximum number of optimization cycles reached");                       \
        global.run_stabilization_cycle = TRUE;                                           \
    }                                                                                    \
    DBUG_RETURN (syntax_tree);                                                           \
    }

#include "phase_sac2c.mac"

#undef OPTCYCLEelement
#undef OPTINCYCelement
#undef OPTINCYCcond
#undef OPTINCYCFUNelement
#undef OPTINCYCFUNcond
#undef ENDOPTCYCLE
#undef ENDOPTCYCLEFUN

/*
 * Generated phase driver functions
 */

#define PHASEelement(it_element)                                                         \
    node *PHDdriveCompilerPhase_##it_element (node *syntax_tree)                         \
    {                                                                                    \
        DBUG_ENTER ("PHDdriveCompilerPhase_" #it_element);

#define SUBPHASEelement(it_element)                                                      \
  syntax_tree = PHrunCompilerSubPhase( SUBPH_##it_element, syntax_tree,

#define SUBPHASEcond(it_cond)                                                            \
  it_cond);

#define OPTCYCLEelement(it_element)                                                      \
  syntax_tree = PHrunCompilerSubPhase( SUBPH_##it_element, syntax_tree,

#define OPTCYCLEcond(it_cond)                                                            \
  it_cond);

#define ENDPHASE(it_element)                                                             \
    DBUG_RETURN (syntax_tree);                                                           \
    }

#include "phase_sac2c.mac"

#undef PHASEelement
#undef SUBPHASEelement
#undef SUBPHASEcond
#undef OPTCYCLEelement
#undef OPTCYCLEcond
#undef ENDPHASE

/*
 * Generated tool driver functions
 */

node *
PHDdriveSac2c (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveSac2c");

#define PHASEelement(it_element)                                                         \
  syntax_tree = PHrunCompilerPhase( PH_##it_element, syntax_tree,

#define PHASEcond(it_cond)                                                               \
  it_cond);

#include "phase_sac2c.mac"

#undef SUBPHASEelement
#undef SUBPHASEcond

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveSac4c (node *syntax_tree)
{
    DBUG_ENTER ("PHDdriveSac4c");

#if 0

#define PHASEelement(it_element)                                                         \
  syntax_tree = PHrunCompilerPhase( PH_##it_element, syntax_tree,

#define PHASEcond(it_cond)                                                               \
  it_cond);

#include "phase_sac4c.mac"

#undef SUBPHASEelement
#undef SUBPHASEcond

#endif

    DBUG_RETURN (syntax_tree);
}
