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

#define DBUG_PREFIX "STAT_WHY"
#include "debug.h"

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
    DBUG_ENTER ();

#define OPTCOUNTERid(id) oc->id = 0;
#include "optimize.mac"
#undef OPTCOUNTERid

    DBUG_RETURN ();
}

#ifndef DBUG_OFF

/** <!-- ****************************************************************** -->
 * @brief prints, given an optimisation counter, why a function will be
 *        reoptimized in the next cycle round.
 *
 * @param oc optimisation counter of a function
 ******************************************************************************/
static void
WhyItsDone (optimize_counter_t *oc)
{
    DBUG_ENTER ();

#define OPTCOUNTER(id, redo, text)                                                       \
    if (redo && (oc->id != 0)) {                                                         \
        CTItell (0, "Will redo because of %s.", text);                                   \
    }

#include "optimize.mac"
#undef OPTCOUNTER

    DBUG_RETURN ();
}

#endif /* DBUG_OFF */

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

    DBUG_ENTER ();

    res = (FALSE
#define OPTCOUNTER(id, redo, text) || (redo && (oc->id != 0))
#include "optimize.mac"
#undef OPTCOUNTER
    );

    DBUG_EXECUTE (WhyItsDone (oc));

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
    DBUG_ENTER ();

#define OPTCOUNTERid(id) copy->id = orig->id;
#include "optimize.mac"
#undef OPTCOUNTERid

    DBUG_RETURN ();
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
    DBUG_ENTER ();

#define OPTCOUNTERid(id) oc->id += add->id;
#include "optimize.mac"
#undef OPTCOUNTERid

    DBUG_RETURN ();
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
    DBUG_ENTER ();

#define OPTCOUNTER(id, redo, text)                                                       \
    if (oc->id > 0) {                                                                    \
        CTItell (0, "  %zu %s", oc->id, text);                                           \
    }
#include "optimize.mac"
#undef OPTCOUNTER

    DBUG_RETURN ();
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
    DBUG_ENTER ();

    CTItell (0, " ");
    CTItell (0, "***********************************************************");
    CTItell (0, "** Optimisation Feedback                                 **");
    CTItell (0, "***********************************************************");

    STATprint (&global.optcounters);

    CTItell (0, "***********************************************************");

    DBUG_RETURN (syntax_tree);
}

/* @} */

#undef DBUG_PREFIX
