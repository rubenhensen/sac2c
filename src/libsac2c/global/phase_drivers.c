/*
 *
 * $Id$
 *
 */

#include "phase_drivers.h"

#include "dbug.h"
#include "phase.h"
#include "globals.h"

/*
 * Generated function-based cycle driver functions
 */

#define FUNBEGINname(name)                                                               \
    node *PHDdriveCycleFun_##name (node *fundef)                                         \
    {                                                                                    \
        DBUG_ENTER ("PHDdriveCycleFun_" #name);

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle)                               \
    fundef = PHrunCyclePhaseFun (PH_##phase##_##cycle##_##name, fundef, cond);

#define FUNEND(name)                                                                     \
    DBUG_RETURN (fundef);                                                                \
    }

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"

#undef FUNBEGINname
#undef CYCLEPHASEFUN
#undef FUNEND

/*
 * Generated cycle driver functions
 */

#define CYCLEname(name)                                                                  \
    node *PHDdriveCycle_##name (node *syntax_tree)                                       \
    {                                                                                    \
        DBUG_ENTER ("PHDdriveCycle_" #name);

#define CYCLEPHASE(name, text, fun, cond, phase, cycle)                                  \
    syntax_tree = PHrunCyclePhase (PH_##phase##_##cycle##_##name, syntax_tree, cond);

#define FUNBEGIN(name, phase, cycle)                                                     \
    syntax_tree = PHrunCycleFun (PH_##phase##_##cycle##_##name, syntax_tree);

#define ENDCYCLE(name)                                                                   \
    DBUG_RETURN (syntax_tree);                                                           \
    }

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"

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
        DBUG_ENTER ("PHDdrivePhase_" #name);

#define SUBPHASE(name, text, fun, cond, phase)                                           \
    syntax_tree = PHrunSubPhase (PH_##phase##_##name, syntax_tree, cond);

#define CYCLE(name, text, cond, phase)                                                   \
    syntax_tree = PHrunCycle (PH_##phase##_##name, syntax_tree, cond);

#define ENDPHASE(name)                                                                   \
    DBUG_RETURN (syntax_tree);                                                           \
    }

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"

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
  syntax_tree = PHrunPhase( PH_##name, syntax_tree,

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
    DBUG_ENTER ("PHDdriveSac4c");

#define PHASEname(name)                                                                  \
  syntax_tree = PHrunPhase( PH_##name, syntax_tree,

#define PHASEcond(cond)                                                                  \
  cond);

#include "phase_sac4c.mac"

#undef PHASEname
#undef PHASEcond

    DBUG_RETURN (syntax_tree);
}
