/*
 * $Id$
 */

/*****************************************************************************
 *
 * This file contains the all static information derived from the phase
 * macro file(s).
 *****************************************************************************/

#include "phase_info.h"

#include "dbug.h"
#include "phase.h"
#include "phase_drivers.h"

/*
 * Extern declarations for non-generated functions used to implement subphases
 * and cycle phases.
 */

#define SUBPHASEfun(fun) extern node *fun (node *syntax_tree);
#define CYCLEPHASEfun(fun) extern node *fun (node *syntax_tree);
#define CYCLEPHASEFUNfun(fun) extern node *fun (node *syntax_tree);

#include "phase_sac2c.mac"

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
    DBUG_ENTER ("DummyPhaseFun");

    DBUG_ASSERT (FALSE, "This function should never be called.");

    DBUG_RETURN (syntax_tree);
}

/****************************************************************************/

#define PHASEname(name) PHDdrivePhase_##name,
#define SUBPHASEfun(fun) fun,
#define CYCLEname(name) PHDdriveCycle_##name,
#define CYCLEPHASEfun(fun) fun,
#define CYCLEPHASEFUNfun(fun) fun,

phase_fun_t
PHIphaseFun (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIphaseFun");

    static const phase_fun_t phase_fun[] = {DummyPhaseFun,
#include "phase_sac2c.mac"
                                            DummyPhaseFun};

    DBUG_RETURN (phase_fun[phase]);
}

#undef PHASEfun
#undef SUBPHASEfun
#undef CYCLEfun
#undef CYCLEPHASEfun
#undef CYCLEPHASEFUNfun

/****************************************************************************/

#define PHASEtext(text) text,
#define SUBPHASEtext(text) text,
#define CYCLEtext(text) text,
#define CYCLEPHASEtext(text) text,
#define CYCLEPHASEFUNtext(text) text,

const char *
PHIphaseText (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIphaseText");

    static const char *phase_text[] = {"initial",
#include "phase_sac2c.mac"
                                       "final"};

    DBUG_RETURN (phase_text[phase]);
}

#undef PHASEtext
#undef SUBPHASEtext
#undef CYCLEtext
#undef CYCLEPHASEtext
#undef CYCLEPHASEFUNtext

/****************************************************************************/

#define PHASEname(name) PHT_phase,
#define SUBPHASEname(name) PHT_subphase,
#define CYCLEname(name) PHT_cycle,
#define CYCLEPHASEname(name) PHT_cyclephase,
#define CYCLEPHASEFUNname(name) PHT_cyclephase,

phase_type_t
PHIphaseType (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIphaseType");

    static phase_type_t phase_type[] = {PHT_dummy,
#include "phase_sac2c.mac"
                                        PHT_dummy};

    DBUG_RETURN (phase_type[phase]);
}

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname
#undef CYCLEPHASEFUNname

/****************************************************************************/

#define PHASEname(name) #name,
#define SUBPHASEname(name) #name,
#define CYCLEname(name) #name,
#define CYCLEPHASEname(name) #name,
#define CYCLEPHASEFUNname(name) #name,

const char *
PHIphaseName (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIphaseName");

    static const char *phase_name[] = {"initial",
#include "phase_sac2c.mac"
                                       "final"};

    DBUG_RETURN (phase_name[phase]);
}

#undef PHASEname
#undef SUBPHASEname
#undef CYCLEname
#undef CYCLEPHASEname
#undef CYCLEPHASEFUNname

/****************************************************************************/

#define PHASE(name, text, cond) PH_initial,

#define SUBPHASE(name, text, fun, cond, phase) PH_##phase,

#define CYCLE(name, text, cond, phase) PH_##phase,

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) PH_##phase##_##cycle,

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle) PH_##phase##_##cycle,

compiler_phase_t
PHIphaseParent (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIphaseParent");

    static compiler_phase_t phase_parent[] = {PH_initial,
#include "phase_sac2c.mac"
                                              PH_initial};

    DBUG_RETURN (phase_parent[phase]);
}

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE
#undef CYCLEPHASEFUN

/****************************************************************************/

#define PHASE(name, text, cond) #name,

#define SUBPHASE(name, text, fun, cond, phase) #phase ":" #name,

#define CYCLE(name, text, cond, phase) #phase ":" #name,

#define CYCLEPHASE(name, text, fun, cond, phase, cycle) #phase ":" #cycle ":" #name,

#define CYCLEPHASEFUN(name, text, fun, cond, phase, cycle) #phase ":" #cycle ":" #name,

const char *
PHIphaseIdent (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIphaseIdent");

    static const char *phase_ident[] = {"",
#include "phase_sac2c.mac"
                                        ""};

    DBUG_RETURN (phase_ident[phase]);
}

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE
#undef CYCLEPHASEFUN

/****************************************************************************/

#define PHASEname(name) FALSE,
#define SUBPHASEname(name) FALSE,
#define CYCLEname(name) FALSE,
#define CYCLEPHASEname(name) FALSE,
#define CYCLEPHASEFUNname(name) TRUE,

bool
PHIisFunBased (compiler_phase_t phase)
{
    DBUG_ENTER ("PHIisFunBased");

    static bool phase_isfunbased[] = {FALSE,
#include "phase_sac2c.mac"
                                      FALSE};

    DBUG_RETURN (phase_isfunbased[phase]);
}

#undef PHASE
#undef SUBPHASE
#undef CYCLE
#undef CYCLEPHASE
#undef CYCLEPHASEFUN
