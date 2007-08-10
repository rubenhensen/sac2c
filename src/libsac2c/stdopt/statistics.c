/*
 * $Id$
 *
 */

/*
 * @defgroup opt Optimizations
 *
 * This group contains all those files/ modules that apply optimizations
 * on the level of SaC code.
 *
 * @{
 */

/**
 *
 * @file optimize.c
 *
 * This file contains the code for steering the sub phases of the high-level
 * optimizations.
 *
 */

/**
 *
 * @name Functions for optimization statistics:
 *
 *@{
 */

#include "statistics.h"

#include "memory.h"
#include "globals.h"
#include "ctinfo.h"
#include "dbug.h"
#include "tree_basic.h"
#include "type_statistics.h"
#include "constraint_statistics.h"

/** <!--********************************************************************-->
 *
 * @fn void STATclearCounters()
 *
 *   @brief returns an optcounters structure with all elements set to 0
 *
 *****************************************************************************/

void
STATclearCounters (optimize_counter_t *oc)
{
    DBUG_ENTER ("STATclearCounters");

#define OPTCOUNTERid(id) oc->id = 0;
#include "optimize.mac"
#undef OPTCOUNTERid

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn bool STATdidSomething( optimize_counter_t oc)
 *
 *   @brief returns whether any optimization counter is not zero.
 *
 *****************************************************************************/

bool
STATdidSomething (optimize_counter_t *oc)
{
    bool res;

    DBUG_ENTER ("STATdidSomething");

    res = (FALSE
#define OPTCOUNTERid(id) || (oc->id != 0)
#include "optimize.mac"
#undef OPTCOUNTERid
    );

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn optimize_counter_t STATcopyCounters( optimize_counter_t *copy,
 *                                             optimize_counter_t *orig)
 *
 *   @brief returns the sum of two optimization counter structures
 *
 *****************************************************************************/

void
STATcopyCounters (optimize_counter_t *copy, optimize_counter_t *orig)
{
    DBUG_ENTER ("STATcopyCounters");

#define OPTCOUNTERid(id) copy->id = orig->id;
#include "optimize.mac"
#undef OPTCOUNTERid

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn optimize_counter_t STATaddCounters( optimize_counter_t *oc,
 *                                         optimize_counter_t *add)
 *
 *   @brief returns the sum of two optimization counter structures
 *
 *****************************************************************************/

void
STATaddCounters (optimize_counter_t *oc, optimize_counter_t *add)
{
    DBUG_ENTER ("STATaddCounters");

#define OPTCOUNTERid(id) oc->id += add->id;
#include "optimize.mac"
#undef OPTCOUNTERid

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn void STATprint(  optimize_counter_t *oc)
 *
 *   @brief prints the optimization statistics
 *
 *****************************************************************************/

void
STATprint (optimize_counter_t *oc)
{
    DBUG_ENTER ("STATprint");

#define OPTCOUNTER(id, text)                                                             \
    if (oc->id > 0) {                                                                    \
        CTInote ("  %d %s", oc->id, text);                                               \
    }
#include "optimize.mac"
#undef OPTCOUNTER

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *STATdoPrintStatistics( node *syntax_tree)
 *
 *   @brief prints the global optimization statistics
 *
 *****************************************************************************/

node *
STATdoPrintStatistics (node *syntax_tree)
{
    DBUG_ENTER ("OPTdoPrintStatistics");

    CTInote (" ");
    CTInote ("***********************************************************");

    STATprint (&global.optcounters);

    CTInote ("***********************************************************");

    syntax_tree = TSdoPrintTypeStatistics (syntax_tree);

    CTInote ("***********************************************************");

    syntax_tree = CSdoPrintConstraintStatistics (syntax_tree);

    CTInote ("***********************************************************");

    DBUG_RETURN (syntax_tree);
}

/* @} */
