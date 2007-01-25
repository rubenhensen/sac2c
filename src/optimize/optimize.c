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

#include "optimize.h"

#include <string.h>

#include "tree_basic.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "ctinfo.h"
#include "dbug.h"
#include "traverse.h"
#include "phase.h"
#include "check.h"
#include "check_mem.h"

#include "liftoptflags.h"
#include "new_types.h" /* for TYtype2String */

/** <!--********************************************************************-->
 *
 * @fn void GenerateOptCounters()
 *
 *   @brief returns an optcounters structure with all elements set to 0
 *
 *****************************************************************************/
static optimize_counter_t
GenerateOptCounters ()
{
    optimize_counter_t res;

    DBUG_ENTER ("GenerateOptCounters");

#define OPTCOUNTERid(id) res.id = 0;
#include "optimize.mac"

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool AnyOptCounterNotZero( optimize_counter_t oc)
 *
 *   @brief returns whether any optimization counter is not zero.
 *
 *****************************************************************************/
static bool
AnyOptCounterNotZero (optimize_counter_t oc)
{
    bool res;

    DBUG_ENTER ("AnyOptCounterNotZero");

    res = (FALSE
#define OPTCOUNTERid(id) || (oc.id != 0)
#include "optimize.mac"
    );

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn optimize_counter_t AddOptCounters( optimize_counter_t o,
 *                                        optimize_counter_t p)
 *
 *   @brief returns the sum of two optimization counter structures
 *
 *****************************************************************************/
static optimize_counter_t
AddOptCounters (optimize_counter_t o, optimize_counter_t p)
{
    optimize_counter_t r;

    DBUG_ENTER ("AddOptCounters");

#define OPTCOUNTERid(id) r.id = o.id + p.id;
#include "optimize.mac"

    DBUG_RETURN (r);
}

/** <!--********************************************************************-->
 *
 * @fn void PrintStatistics( void)
 *
 *   @brief prints the optimization statistic
 *
 *****************************************************************************/

static void
PrintStatistics (void)
{
    DBUG_ENTER ("PrintStatistics");

#define OPTCOUNTER(id, text)                                                             \
    if (global.optcounters.id > 0) {                                                     \
        CTInote ("%d %s", global.optcounters.id, text);                                  \
    }
#include "optimize.mac"

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 *
 * @fn node *OPTdoPrintStatistics( node *syntax_tree)
 *
 *   @brief prints the global optimization statistic
 *
 *****************************************************************************/

node *
OPTdoPrintStatistics (node *syntax_tree)
{
    DBUG_ENTER ("OPTdoPrintStatistics");

    CTInote (" ");
    CTInote ("Overall optimization statistics:");

    PrintStatistics ();

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn void PrintFundefInformation( node *fundef)
 *
 *****************************************************************************/
static void
PrintFundefInformation (node *fundef)
{
    node *arg;
    char *tmp_str;
    int tmp_str_size;

    static char argtype_buffer[80];
    static int buffer_space;

    DBUG_ENTER ("PrintFundefInformation");

    strcpy (argtype_buffer, "");
    buffer_space = 77;

    arg = FUNDEF_ARGS (fundef);
    while ((arg != NULL) && (buffer_space > 5)) {

        tmp_str = TYtype2String (AVIS_TYPE (ARG_AVIS (arg)), TRUE, 0);
        tmp_str_size = strlen (tmp_str);

        if ((tmp_str_size + 3) <= buffer_space) {
            strcat (argtype_buffer, tmp_str);
            buffer_space -= tmp_str_size;
            if (ARG_NEXT (arg) != NULL) {
                strcat (argtype_buffer, ", ");
                buffer_space -= 2;
            }
        } else {
            strcat (argtype_buffer, "...");
            buffer_space = 0;
        }

        tmp_str = ILIBfree (tmp_str);
        arg = ARG_NEXT (arg);
    }

    CTInote ("****** Optimizing function:\n******  %s( %s): ...", CTIitemName (fundef),
             argtype_buffer);

    DBUG_VOID_RETURN;
}

/*@}*/

/**
 *
 * @name Entry Function for Applying High-Level Optimizations:
 *
 * @{
 */

/** <!--********************************************************************-->
 *
 * @fn node *OPTdoIntraFunctionalOptimizations( node *arg_node)
 *
 *   @brief
 *
 *****************************************************************************/
node *
OPTdoIntraFunctionalOptimizations (node *arg_node)
{
    optimize_counter_t oldoptcounters;
    int loop = 0;
    node *fundef;

    DBUG_ENTER ("OPTdoIntraFunctionalOptimizations");

    oldoptcounters = global.optcounters;
    global.optcounters = GenerateOptCounters ();

    /*
     * Tell the phase subsystem where to start to look for unambigous
     * break specifiers
     */
    PHsetFirstOptimization (SUBPH_cse);

    do {
        loop++;
        CTInote (" ");
        CTInote ("****** Cycle pass: %i", loop);

        oldoptcounters = AddOptCounters (global.optcounters, oldoptcounters);
        global.optcounters = GenerateOptCounters ();

        fundef = MODULE_FUNS (arg_node);
        while (fundef != NULL) {

            /*
             * Zombies and Type Error functions need not to be optimised
             * as they have no body anyways...
             */
            if ((!FUNDEF_ISZOMBIE (fundef) && !FUNDEF_ISTYPEERROR (fundef))) {
                optimize_counter_t oc = global.optcounters;
                global.optcounters = GenerateOptCounters ();

                if (FUNDEF_WASOPTIMIZED (fundef)) {
                    /*
                     * Print function name
                     */
                    PrintFundefInformation (fundef);

                    /*
                     * Common subexpression elimination
                     */
                    if (global.optimize.docse) {
                        fundef = PHrunOptimizationInCycle (SUBPH_cse, loop, fundef);
                    }

                    /*
                     * Insert shape variables
                     */
                    if (global.optimize.dosaacyc && global.optimize.dodcr) {
                        fundef = PHrunOptimizationInCycle (SUBPH_saacyc, loop, fundef);
                    }

                    /*
                     * Infer loop invariants
                     */
                    fundef = PHrunOptimizationInCycle (SUBPH_ili, loop, fundef);

                    /*
                     * Type upgrade
                     */
                    if (global.optimize.dotup) {
                        fundef = PHrunOptimizationInCycle (SUBPH_ntccyc, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_eatcyc, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_ebtcyc, loop, fundef);
                    }

                    /*
                     * Reverse type upgrade
                     */
                    if (global.optimize.dortup) {
                        fundef = PHrunOptimizationInCycle (SUBPH_rtupcyc, loop, fundef);
                    }

                    /*
                     * try to dispatch further function calls
                     */
                    fundef = PHrunOptimizationInCycle (SUBPH_dfccyc, loop, fundef);

                    /*
                     * apply INL (inlining)
                     */
                    if (global.optimize.doinl) {
                        FUNDEF_ISINLINECOMPLETED (fundef) = FALSE;
                        fundef = PHrunOptimizationInCycle (SUBPH_inlcyc, loop, fundef);
                    }

                    /*
                     * Withloop Propagation
                     */
                    if (global.optimize.dowlprop) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wlprop, loop, fundef);
                    }

                    /*
                     * Constant folding
                     */
                    if (global.optimize.docf) {
                        fundef = PHrunOptimizationInCycle (SUBPH_cf, loop, fundef);
                    }

                    /*
                     * Constant and variable propagation
                     */
                    if (global.optimize.docvp) {
                        fundef = PHrunOptimizationInCycle (SUBPH_cvp, loop, fundef);
                    }

                    /*
                     * With-loop partition generation
                     */
                    if (global.optimize.dowlpg) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wlpgcyc, loop, fundef);
                    }

                    /*
                     * With-loop simplification
                     */
                    if (global.optimize.dowlsimp) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wlsimp, loop, fundef);
                    }

                    /*
                     * Copy With-loop elimination
                     */
                    if (global.optimize.docwle) {
                        fundef = PHrunOptimizationInCycle (SUBPH_cwle, loop, fundef);
                    }

                    /*
                     * With-loop folding
                     */
                    if (global.optimize.dowlf) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wli, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_wlf, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_ssawlf, loop, fundef);
                    }

                    /*
                     * Symbolic with-loop folding
                     */
                    if (global.optimize.doswlf) {
                        fundef = PHrunOptimizationInCycle (SUBPH_swlf, loop, fundef);
                    }

                    /*
                     * Dead code removal (Just the current FUNDEF)
                     */
                    if (global.optimize.dodcr) {
                        fundef = PHrunOptimizationInCycle (SUBPH_dcrcycfun, loop, fundef);
                    }

                    /*
                     * With-loop scalarization
                     */
                    if (global.optimize.dowls) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wls, loop, fundef);
                    }

                    /*
                     * Prf unrolling
                     */
                    if (global.optimize.doprfunr) {
                        fundef = PHrunOptimizationInCycle (SUBPH_prfunr, loop, fundef);
                    }

                    /*
                     * Loop unrolling
                     */
                    if (global.optimize.dolur) {
                        fundef = PHrunOptimizationInCycle (SUBPH_lur, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_ssalur, loop, fundef);
                    }

                    /*
                     * With-loop unrolling
                     */
                    if (global.optimize.dowlur) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wlur, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_ssawlur, loop, fundef);
                    }

                    /*
                     * LAC inlining
                     */
                    if (global.lacinline) {
                        fundef = PHrunOptimizationInCycle (SUBPH_linlcyc, loop, fundef);
                    }

                    /*
                     * In optimization cycle: just with-loop invariant removal
                     */
                    if (global.optimize.dolir) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wlir, loop, fundef);
                    }

                    /*
                     * Eliminate typeconv prfs
                     */
                    if (global.optimize.doetc) {
                        fundef = PHrunOptimizationInCycle (SUBPH_etc, loop, fundef);
                    }

                    /*
                     * Eliminate subtraction and division operations
                     */
                    if (global.optimize.doesd) {
                        fundef = PHrunOptimizationInCycle (SUBPH_esd, loop, fundef);
                    }

                    /*
                     * Associativity optimization
                     */
                    if (global.optimize.doal) {
                        fundef = PHrunOptimizationInCycle (SUBPH_al, loop, fundef);
                    }

                    /*
                     * Distributivity optimization
                     */
                    if (global.optimize.dodl) {
                        fundef = PHrunOptimizationInCycle (SUBPH_dl, loop, fundef);
                    }
                }

                FUNDEF_WASOPTIMIZED (fundef) = AnyOptCounterNotZero (global.optcounters);
                DBUG_EXECUTE ("OPT", PrintStatistics (););
                global.optcounters = AddOptCounters (global.optcounters, oc);
            }

            fundef = FUNDEF_NEXT (fundef);
        }

        /*
         * Dead code removal
         */
        if (global.optimize.dodcr) {
            arg_node = PHrunOptimizationInCycle (SUBPH_dcrcyc, loop, arg_node);
        }

        /*
         * Signature simplification
         */
        if ((global.optimize.dosisi) && (global.optimize.dodcr)) {
            arg_node = PHrunOptimizationInCycle (SUBPH_sisi, loop, arg_node);
        }

        /*
         * Lift Optimise and Type Upgrade Flags
         */
        MODULE_FUNS (arg_node) = LOFdoLiftOptFlags (MODULE_FUNS (arg_node));

        /*
         *
         */
#ifdef SHOW_MALLOC
        if ((global.treecheck) && (arg_node != NULL)) {
            arg_node = CHKdoTreeCheck (arg_node);
        }

        if (global.memcheck && (arg_node != NULL)) {
            arg_node = CHKMdoMemCheck (arg_node);
        }
#endif /* SHOW_MALLOC */

    } while ((AnyOptCounterNotZero (global.optcounters)) && (loop < global.max_optcycles)
             && (!PHbreakAfterCurrentPass (loop)));

    if ((loop == global.max_optcycles) && AnyOptCounterNotZero (global.optcounters)) {
        CTIwarn ("Maximal number of optimization cycles reached");
    }

    /*
     * to ensure that typeerrors are propagated properly,
     * we continue with a smaller stabilisation cycle
     * until the type information gets stable
     */
    while ((AnyOptCounterNotZero (global.optcounters))
           && (!PHbreakAfterCurrentPass (loop))) {
        loop++;
        CTInote (" ");
        CTInote ("****** Cycle pass (stabilisation): %i", loop);

        oldoptcounters = AddOptCounters (global.optcounters, oldoptcounters);
        global.optcounters = GenerateOptCounters ();

        fundef = MODULE_FUNS (arg_node);
        while (fundef != NULL) {
            /*
             * Zombies and Type Error functions need not to be optimised
             * as they have no body anyways...
             */
            if ((!FUNDEF_ISZOMBIE (fundef) && !FUNDEF_ISTYPEERROR (fundef))) {
                optimize_counter_t oc = global.optcounters;
                global.optcounters = GenerateOptCounters ();
                /*
                 * Print function name
                 */
                PrintFundefInformation (fundef);

                /*
                 * Type upgrade
                 */
                if (global.optimize.dotup) {
                    fundef = PHrunOptimizationInCycle (SUBPH_ntccyc, loop, fundef);
                    fundef = PHrunOptimizationInCycle (SUBPH_eatcyc, loop, fundef);
                    fundef = PHrunOptimizationInCycle (SUBPH_ebtcyc, loop, fundef);
                }

                /*
                 * try to dispatch further function calls
                 */
                fundef = PHrunOptimizationInCycle (SUBPH_dfccyc, loop, fundef);

                FUNDEF_WASOPTIMIZED (fundef) = AnyOptCounterNotZero (global.optcounters);
                DBUG_EXECUTE ("OPT", PrintStatistics (););
                global.optcounters = AddOptCounters (global.optcounters, oc);
            }

            fundef = FUNDEF_NEXT (fundef);
        }

        /*
         * Lift Optimise and Type Upgrade Flags
         */
        MODULE_FUNS (arg_node) = LOFdoLiftOptFlags (MODULE_FUNS (arg_node));
    }

    global.optcounters = AddOptCounters (global.optcounters, oldoptcounters);

    DBUG_RETURN (arg_node);
}

/* @} */
