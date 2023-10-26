/*****************************************************************************
 *
 * This file contains the all static information derived from the phase
 * macro file(s).
 *****************************************************************************/

#include "phase_info.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "phase.h"
#include "globals.h"
#include "phase_drivers.h"

/*
 * Extern declarations for non-generated functions used to implement subphases
 * and cycle phases.
 */

#define SUBPHASEfun(fun) extern node *fun (node *syntax_tree);
#define CYCLEPHASEfun(fun) extern node *fun (node *syntax_tree);
#define CYCLEPHASEFUNfun(fun) extern node *fun (node *syntax_tree);

#include "phase_sac2c.mac"
#include "phase_sac4c.mac"
#include "phase_sac2tex.mac"

#undef SUBPHASEfun
#undef CYCLEPHASEfun
#undef CYCLEPHASEFUNfun

/****************************************************************************/

/*
 * Dummy phase functions
 */

static node *
DummyPhaseFun (node *syntax_tree)
{
    DBUG_ENTER ();

    DBUG_UNREACHABLE ("This function should never be called.");

    DBUG_RETURN (syntax_tree);
}

/****************************************************************************/

#define PHASEname(name) PHDdrivePhase_##name,
#define SUBPHASEfun(fun) fun,
#define CYCLEname(name) PHDdriveCycle_##name,
#define CYCLEPHASEfun(fun) fun,
#define FUNBEGINname(name) PHDdriveCycleFun_##name,
#define CYCLEPHASEFUNfun(fun) fun,

phase_fun_t
PHIphaseFun (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static const phase_fun_t phase_fun[] = {DummyPhaseFun,
#include "phase_sac2c.mac"
                                            DummyPhaseFun,
#include "phase_sac4c.mac"
                                            DummyPhaseFun,
#include "phase_sac2tex.mac"
                                            DummyPhaseFun};

    DBUG_RETURN (phase_fun[phase]);
}

#undef PHASEname
#undef SUBPHASEfun
#undef CYCLEname
#undef CYCLEPHASEfun
#undef FUNBEGINname
#undef CYCLEPHASEFUNfun

/****************************************************************************/

#define PHASEtext(text) text,
#define SUBPHASEtext(text) text,
#define CYCLEtext(text) text,
#define CYCLEPHASEtext(text) text,
#define FUNBEGINname(name) "",
#define CYCLEPHASEFUNtext(text) text,

const char *
PHIphaseText (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static const char *phase_text[] = {"initial",
#include "phase_sac2c.mac"
                                       "final",
#include "phase_sac4c.mac"
                                       "final",
#include "phase_sac2tex.mac"
                                       "final"};

    DBUG_RETURN (phase_text[phase]);
}

#undef PHASEtext
#undef SUBPHASEtext
#undef CYCLEtext
#undef CYCLEPHASEtext
#undef FUNBEGINname
#undef CYCLEPHASEFUNtext

/****************************************************************************/

#define PHASEname(name) PHT_phase,
#define SUBPHASEname(name) PHT_subphase,
#define CYCLEname(name) PHT_cycle,
#define CYCLEPHASEname(name) PHT_cyclephase,
#define FUNBEGINname(name) PHT_cycle_fun,
#define CYCLEPHASEFUNname(name) PHT_cyclephase_fun,

phase_type_t
PHIphaseType (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static phase_type_t phase_type[] = {PHT_dummy,
#include "phase_sac2c.mac"
                                        PHT_dummy,
#include "phase_sac4c.mac"
                                        PHT_dummy,
#include "phase_sac2tex.mac"
                                        PHT_dummy};

    DBUG_RETURN (phase_type[phase]);
}

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname
#undef FUNBEGINname
#undef CYCLEPHASEFUNname

/****************************************************************************/

#define PHASEname(name) #name,
#define SUBPHASEname(name) #name,
#define CYCLEname(name) #name,
#define CYCLEPHASEname(name) #name,
#define FUNBEGINname(name) #name,
#define CYCLEPHASEFUNname(name) #name,

const char *
PHIphaseName (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static const char *phase_name[] = {"initial",
#include "phase_sac2c.mac"
                                       "final",
#include "phase_sac4c.mac"
                                       "final",
#include "phase_sac2tex.mac"
                                       "final"};

    DBUG_RETURN (phase_name[phase]);
}

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname
#undef FUNBEGINname
#undef CYCLEPHASEFUNname

/****************************************************************************/

#define PHASE(name, text, cond) PH_initial,

#define SUBPHASE(name, text, fun, cond, phase) PHASENAME(phase),

#define CYCLE(name, text, cond, phase, setup) PHASENAME(phase),

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) PHASENAME(phase, cycle),

#define FUNBEGIN(name, phase, cycle) PHASENAME(phase, cycle),

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle) PHASENAME(phase, cycle),

#define CYCLEPHASEFUNOLD(name, text, fun, cond, phase, cycle) PHASENAME(phase, cycle),

compiler_phase_t
PHIphaseParent (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static compiler_phase_t phase_parent[] = {PH_initial,
#include "phase_sac2c.mac"
                                              PH_final,
#include "phase_sac4c.mac"
                                              PH_final_sac4c,
#include "phase_sac2tex.mac"
                                              PH_final_sac2tex};

    DBUG_RETURN (phase_parent[phase]);
}

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE
#undef FUNBEGIN
#undef CYCLEPHASEFUN
#undef CYCLEPHASEFUNOLD

/****************************************************************************/

#define PHASE(name, text, cond) #name,

#define SUBPHASE(name, text, fun, cond, phase) #phase ":" #name,

#define CYCLE(name, text, cond, phase, setup) #phase ":" #name,

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) #phase ":" #cycle ":" #name,

#define FUNBEGIN(name, phase, cycle) #phase ":" #cycle ":" #name,

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle) #phase ":" #cycle ":" #name,

#define CYCLEPHASEFUNOLD(name, text, fun, cond, phase, cycle) #phase ":" #cycle ":" #name,

const char *
PHIphaseIdent (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static const char *phase_ident[] = {"",
#include "phase_sac2c.mac"
                                        "",
#include "phase_sac4c.mac"
                                        "",
#include "phase_sac2tex.mac"
                                        ""};

    DBUG_RETURN (phase_ident[phase]);
}

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE
#undef FUNBEGIN
#undef CYCLEPHASEFUN
#undef CYCLEPHASEFUNOLD

/****************************************************************************/

#define PHASEname(name) FALSE,
#define SUBPHASEname(name) FALSE,
#define CYCLEname(name) FALSE,
#define CYCLEPHASEname(name) FALSE,
#define FUNBEGINname(name) FALSE,
#define CYCLEPHASEFUNname(name) TRUE,

bool
PHIisFunBased (compiler_phase_t phase)
{
    DBUG_ENTER ();

    static bool phase_isfunbased[] = {FALSE,
#include "phase_sac2c.mac"
                                      FALSE,
#include "phase_sac4c.mac"
                                      FALSE,
#include "phase_sac2tex.mac"
                                      FALSE};

    DBUG_RETURN (phase_isfunbased[phase]);
}

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname
#undef FUNBEGINname
#undef CYCLEPHASEFUNname

compiler_phase_t
PHIfirstPhase (void)
{
    compiler_phase_t result = PH_undefined;

    DBUG_ENTER ();

    switch (global.tool) {
    case TOOL_sac2c:
        result = PH_initial;
        break;
    case TOOL_sac4c:
        result = PH_final;
        break;
    case TOOL_sac2tex:
        result = PH_final_sac4c;
        break;
    }

    DBUG_RETURN (result);
}

compiler_phase_t
PHIlastPhase (void)
{
    compiler_phase_t result = PH_undefined;

    DBUG_ENTER ();

    switch (global.tool) {
    case TOOL_sac2c:
        result = PH_final;
        break;
    case TOOL_sac4c:
        result = PH_final_sac4c;
        break;
    case TOOL_sac2tex:
        result = PH_final_sac2tex;
        break;
    }

    DBUG_RETURN (result);
}

#undef DBUG_PREFIX
