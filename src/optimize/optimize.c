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
 * @fn node *OPTrunOptimizationCycle( node *arg_node)
 *
 *   @brief
 *
 *****************************************************************************/

node *
OPTrunOptimizationCycle (node *arg_node)
{
    optimize_counter_t oldoptcounters;
    int loop = 0;
    node *fundef;

    DBUG_ENTER ("OPTrunOptimizationCycle");

    oldoptcounters = global.optcounters;
    global.optcounters = GenerateOptCounters ();

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
                    fundef = PHrunOptimizationInCycleFun (OIC_cse, loop, fundef,
                                                          global.optimize.docse);

                    /*
                     * Insert shape variables
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_saacyc, loop, fundef,
                                                          global.optimize.dosaacyc
                                                            && global.optimize.dodcr);

                    /*
                     * Infer loop invariants
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_ili, loop, fundef, ALWAYS);

                    /*
                     * Type upgrade
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_ntccyc, loop, fundef,
                                                          global.optimize.dotup);
                    fundef = PHrunOptimizationInCycleFun (OIC_eatcyc, loop, fundef,
                                                          global.optimize.dotup);
                    fundef = PHrunOptimizationInCycleFun (OIC_ebtcyc, loop, fundef,
                                                          global.optimize.dotup);

                    /*
                     * Reverse type upgrade
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_rtupcyc, loop, fundef,
                                                          global.optimize.dortup);

                    /*
                     * try to dispatch further function calls
                     */
                    fundef
                      = PHrunOptimizationInCycleFun (OIC_dfccyc, loop, fundef, ALWAYS);

                    /*
                     * apply INL (inlining)
                     */
                    FUNDEF_ISINLINECOMPLETED (fundef) = FALSE;
                    fundef = PHrunOptimizationInCycleFun (OIC_inlcyc, loop, fundef,
                                                          global.optimize.doinl);

                    /*
                     * Withloop Propagation
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wlprop, loop, fundef,
                                                          global.optimize.dowlprop);

                    /*
                     * Constant folding
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_cf, loop, fundef,
                                                          global.optimize.docf);

                    /*
                     * Constant and variable propagation
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_cvp, loop, fundef,
                                                          global.optimize.docvp);

                    /*
                     * With-loop partition generation
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wlpgcyc, loop, fundef,
                                                          global.optimize.dowlpg);

                    /*
                     * With-loop simplification
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wlsimp, loop, fundef,
                                                          global.optimize.dowlsimp);

                    /*
                     * Copy With-loop elimination
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_cwle, loop, fundef,
                                                          global.optimize.docwle);

                    /*
                     * With-loop folding
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wli, loop, fundef,
                                                          global.optimize.dowlf);
                    fundef = PHrunOptimizationInCycleFun (OIC_wlf, loop, fundef,
                                                          global.optimize.dowlf);
                    fundef = PHrunOptimizationInCycleFun (OIC_ssawlf, loop, fundef,
                                                          global.optimize.dowlf);

                    /*
                     * Symbolic with-loop folding
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_swlf, loop, fundef,
                                                          global.optimize.doswlf);

                    /*
                     * Dead code removal (Just the current FUNDEF)
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_dcrcycfun, loop, fundef,
                                                          global.optimize.dodcr);

                    /*
                     * With-loop scalarization
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wls, loop, fundef,
                                                          global.optimize.dowls);

                    /*
                     * Prf unrolling
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_prfunr, loop, fundef,
                                                          global.optimize.doprfunr);

                    /*
                     * Loop unrolling
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_lur, loop, fundef,
                                                          global.optimize.dolur);
                    fundef = PHrunOptimizationInCycleFun (OIC_ssalur, loop, fundef,
                                                          global.optimize.dolur);

                    /*
                     * With-loop unrolling
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wlur, loop, fundef,
                                                          global.optimize.dowlur);
                    fundef = PHrunOptimizationInCycleFun (OIC_ssawlur, loop, fundef,
                                                          global.optimize.dowlur);

                    /*
                     * LAC inlining
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_linlcyc, loop, fundef,
                                                          global.lacinline);

                    /*
                     * In optimization cycle: just with-loop invariant removal
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_wlir, loop, fundef,
                                                          global.optimize.dolir);

                    /*
                     * Eliminate typeconv prfs
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_etc, loop, fundef,
                                                          global.optimize.doetc);

                    /*
                     * Eliminate subtraction and division operations
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_esd, loop, fundef,
                                                          global.optimize.doesd);

                    /*
                     * Associativity optimization
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_al, loop, fundef,
                                                          global.optimize.doal);

                    /*
                     * Distributivity optimization
                     */
                    fundef = PHrunOptimizationInCycleFun (OIC_dl, loop, fundef,
                                                          global.optimize.dodl);
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
        arg_node
          = PHrunOptimizationInCycle (OIC_dcrcyc, loop, arg_node, global.optimize.dodcr);

        /*
         * Signature simplification
         */
        arg_node
          = PHrunOptimizationInCycle (OIC_sisi, loop, arg_node,
                                      global.optimize.dosisi && global.optimize.dodcr);

        /*
         * Lift Optimise and Type Upgrade Flags
         */
        arg_node = PHrunOptimizationInCycle (OIC_lof, loop, arg_node, ALWAYS);

        /*
         * Tree and memory consistency checks
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
             && ((loop < global.break_cycle_specifier)
                 || (global.break_after_optincyc > global.compiler_optincyc)));

    if ((loop == global.max_optcycles) && AnyOptCounterNotZero (global.optcounters)) {
        CTIwarn ("Maximal number of optimization cycles reached");
    } else {
        global.run_stabilization_cycle = FALSE;
    }

    DBUG_RETURN (arg_node);
}

node *
OPTrunStabilizationCycle (node *arg_node)
{
    int loop = 0;
    optimize_counter_t oldoptcounters;
    node *fundef;

    DBUG_ENTER ("OPTrunStabilizationCycle");

    /*
     * to ensure that typeerrors are propagated properly,
     * we continue with a smaller stabilisation cycle
     * until the type information gets stable
     */
    while ((AnyOptCounterNotZero (global.optcounters)) && (loop < global.max_optcycles)
           && ((loop < global.break_cycle_specifier)
               || (global.break_after_optincyc > global.compiler_optincyc))) {
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
                fundef = PHrunOptimizationInCycleFun (OIC_ntcscyc, loop, fundef,
                                                      global.optimize.dotup);
                fundef = PHrunOptimizationInCycleFun (OIC_eatscyc, loop, fundef,
                                                      global.optimize.dotup);
                fundef = PHrunOptimizationInCycleFun (OIC_ebtscyc, loop, fundef,
                                                      global.optimize.dotup);

                /*
                 * try to dispatch further function calls
                 */
                fundef = PHrunOptimizationInCycleFun (OIC_dfcscyc, loop, fundef, ALWAYS);

                FUNDEF_WASOPTIMIZED (fundef) = AnyOptCounterNotZero (global.optcounters);
                DBUG_EXECUTE ("OPT", PrintStatistics (););
                global.optcounters = AddOptCounters (global.optcounters, oc);
            }

            fundef = FUNDEF_NEXT (fundef);
        }

        /*
         * Lift Optimise and Type Upgrade Flags
         */
        arg_node = PHrunOptimizationInCycle (OIC_slof, loop, arg_node, ALWAYS);

        /*
         * Tree and memory consistency checks
         */
#ifdef SHOW_MALLOC
        if ((global.treecheck) && (arg_node != NULL)) {
            arg_node = CHKdoTreeCheck (arg_node);
        }

        if (global.memcheck && (arg_node != NULL)) {
            arg_node = CHKMdoMemCheck (arg_node);
        }
#endif /* SHOW_MALLOC */
    }

    global.optcounters = AddOptCounters (global.optcounters, oldoptcounters);

    DBUG_RETURN (arg_node);
}

/* @} */
