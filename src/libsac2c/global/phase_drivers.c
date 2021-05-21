#include "phase_drivers.h"

#define DBUG_PREFIX "PHD"
#include "debug.h"

#include "phase.h"
#include "traverse_inject.h"
#include "globals.h"
#include "rtspec_modes.h"
#include "str.h"


/*
 * Generated function-based cycle driver functions
 */

#define FUNBEGINname(name)                                                               \
    node *PHDdriveCycleFun_##name (node *fundef)                                         \
    {                                                                                    \
        DBUG_ENTER ();

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle)                               \
        fundef = PHrunCyclePhaseFun (PHASENAME(phase, cycle, name), fundef, cond);

#define CYCLEPHASEFUNOLD(name, text, fun, cond, phase, cycle)                            \
        fundef = PHrunCyclePhaseFunOld (PHASENAME(phase, cycle, name), fundef, cond);

#define FUNEND(name)                                                                     \
        DBUG_RETURN (fundef);                                                            \
    }

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"
#include "phase_sac2tex.mac"

#undef FUNBEGINname
#undef CYCLEPHASEFUN
#undef CYCLEPHASEFUNOLD
#undef FUNEND

/*
 * Generated cycle driver functions
 */

#define CYCLEname(name)                                                                  \
    node *PHDdriveCycle_##name (node *syntax_tree)                                       \
    {                                                                                    \
        DBUG_ENTER ();

#define CYCLEPHASE(name, text, fun, cond, phase, cycle)                                  \
        syntax_tree = PHrunCyclePhase (PHASENAME(phase, cycle, name), syntax_tree, cond);

#define FUNBEGIN(name, phase, cycle)                                                     \
        syntax_tree = PHrunCycleFun (PHASENAME(phase, cycle, name), syntax_tree);

#define ENDCYCLE(name)                                                                   \
        DBUG_RETURN (syntax_tree);                                                       \
    }

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"
#include "phase_sac2tex.mac"

#undef CYCLEname
#undef CYCLEPHASE
#undef FUNBEGIN
#undef ENDCYCLE

/*
 * Generated phase driver functions
 */

#define PHASEname(name)                                                                  \
    node *PHDdrivePhase_##name (node *syntax_tree)                                       \
    {                                                                                    \
        DBUG_ENTER ();

#define ADDPREFUN(name, fun) \
        TRAVaddPreFun (name, fun);

#define ADDPOSTFUN(name, fun) \
        TRAVaddPostFun (name, fun);

#define REMOVEPREFUN(name, fun) \
        TRAVremovePreFun (name, fun);

#define REMOVEPOSTFUN(name, fun) \
        TRAVremovePostFun (name, fun);

#define SUBPHASE(name, text, fun, cond, phase)                                           \
        syntax_tree = PHrunSubPhase (PHASENAME(phase, name), syntax_tree, cond);

#define CYCLE(name, text, cond, phase, reset)                                            \
        syntax_tree = PHrunCycle (PHASENAME(phase, name), syntax_tree, cond, reset);

#define ENDPHASE(name)                                                                   \
        DBUG_RETURN (syntax_tree);                                                       \
    }

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"
#include "phase_sac2tex.mac"

#undef ADDPREFUN
#undef ADDPOSTFUN
#undef REMOVEPREFUN
#undef REMOVEPOSTFUN
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
    DBUG_ENTER ();

#define PHASEname(name)                                                                  \
  syntax_tree = PHrunPhase( PHASENAME(name), syntax_tree,

#define PHASEcond(cond)                                                                  \
  cond);

#include "phase_sac2c.mac"

#undef PHASEname
#undef PHASEcond

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveSac4c (node *syntax_tree)
{
    DBUG_ENTER ();

#define PHASEname(name)                                                                  \
  syntax_tree = PHrunPhase( PHASENAME(name), syntax_tree,

#define PHASEcond(cond)                                                                  \
  cond);

#include "phase_sac4c.mac"

#undef PHASEname
#undef PHASEcond

    DBUG_RETURN (syntax_tree);
}

node *
PHDdriveSac2tex (node *syntax_tree)
{
    DBUG_ENTER ();

#define PHASEname(name)                                                                  \
  syntax_tree = PHrunPhase( PHASENAME(name), syntax_tree,

#define PHASEcond(cond)                                                                  \
  cond);

#include "phase_sac2tex.mac"

#undef PHASEname
#undef PHASEcond

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
