/**
 * $Id$
 *
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

#include "liftoptflags.h"
#include "index_infer.h"  /* for IVEIprintPreFun */
#include "new_types.h"    /* for TYtype2String */
#include "SSATransform.h" /* needed after current WLF implementation */

/** <!--********************************************************************-->
 *
 * @fn void GenerateOptCounters()
 *
 *   @brief returns an optcounters structure with all elements set to 0
 *
 ******************************************************************************/
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
 ******************************************************************************/
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
 ******************************************************************************/
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
 * @fn void PrintStatistics()
 *
 *   @brief prints the global optimization statistic
 *
 ******************************************************************************/
static void
PrintStatistics ()
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

    CTInote ("****** Optimizing function:\n******  %s( %s): ...", FUNDEF_NAME (fundef),
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
 * @fn node *OPTdoOptimize( node *arg_node)
 *
 *   @brief governs the whole optimization phase and installs opt_tab.
 *
 *****************************************************************************/

node *
OPTdoOptimize (node *arg_node)
{
    DBUG_ENTER ("OPTdoOptimize");

    /*
     * apply INL (inlining)
     */
    if (global.optimize.doinl) {
        arg_node = PHrunCompilerSubPhase (SUBPH_inl, arg_node);
    }

    /*
     * apply DFR (dead function removal)
     */
    if (global.optimize.dodfr) {
        arg_node = PHrunCompilerSubPhase (SUBPH_dfr, arg_node);
    }

    /*
     * Array elimination (FIX ME!)
     */

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        arg_node = PHrunCompilerSubPhase (SUBPH_dcr, arg_node);
    }

    /**
     * Loop invariant removal
     */
    if (global.optimize.dolir) {
        arg_node = PHrunCompilerSubPhase (SUBPH_lir, arg_node);
    }

    /*
     * Intra-functional optimizations
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_intraopt, arg_node);

    /**
     * Loop invariant removal
     */
    if (global.optimize.dolir) {
        arg_node = PHrunCompilerSubPhase (SUBPH_lir2, arg_node);
    }

    /*
     * UESD
     */
    if (global.optimize.doesd) {
        arg_node = PHrunCompilerSubPhase (SUBPH_uesd, arg_node);
    }

    /*
     * Dead code removal
     */
    if (global.optimize.dodcr) {
        arg_node = PHrunCompilerSubPhase (SUBPH_dcr2, arg_node);
    }

    /*
     * With-loop fusion
     */
    if (global.optimize.dowlfs) {
        arg_node = PHrunCompilerSubPhase (SUBPH_wlfs, arg_node);

        /*
         * Common subexpression elimination
         */
        if (global.optimize.docse) {
            arg_node = PHrunCompilerSubPhase (SUBPH_cse2, arg_node);
        }

        /*
         * Dead code removal
         */
        if (global.optimize.dodcr) {
            arg_node = PHrunCompilerSubPhase (SUBPH_dcr3, arg_node);
        }
    }

    /*
     * !!! If they should ever work again, WLAA, TSI, and AP must run here
     */

    /*
     * Another MANDATORY run of WLPG. This is necessary to prevent AKSIV
     * with-loops to arrive at wltransform
     *
     * with-loop partition generation
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_wlpg2, arg_node);

    /*
     * Withloop reuse candidate inference
     */
    if (global.optimize.douip) {
        arg_node = PHrunCompilerSubPhase (SUBPH_wrci, arg_node);
    }

    /*
     * annotate offset scalars on with-loops
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_wlidx, arg_node);

    /*
     * apply index vector elimination
     */
    if (global.optimize.doive) {
        TRAVsetPreFun (TR_prt, IVEIprintPreFun);
        arg_node = PHrunCompilerSubPhase (SUBPH_ivei, arg_node);
        arg_node = PHrunCompilerSubPhase (SUBPH_ive, arg_node);
        arg_node = PHrunCompilerSubPhase (SUBPH_iveo, arg_node);
        TRAVsetPreFun (TR_prt, NULL);

        /*
         * Constant and variable propagation
         */
        if (global.optimize.docvp) {
            arg_node = PHrunCompilerSubPhase (SUBPH_cvpive, arg_node);
        }

        /*
         * Dead code removal after ive
         */
        if (global.optimize.dodcr) {
            arg_node = PHrunCompilerSubPhase (SUBPH_dcrive, arg_node);
        }
    }

    /*
     * apply USSA (undo ssa transformation)
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_ussa, arg_node);
    arg_node = PHrunCompilerSubPhase (SUBPH_fun2lac, arg_node);
    arg_node = PHrunCompilerSubPhase (SUBPH_lacinl, arg_node);

    CTInote (" ");
    CTInote ("Overall optimization statistics:");
    PrintStatistics ();

    DBUG_RETURN (arg_node);
}

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

        MODULE_FUNS (arg_node) = LOFdoLiftOptFlags (MODULE_FUNS (arg_node));

        fundef = MODULE_FUNS (arg_node);
        while (fundef != NULL) {

            if (!FUNDEF_ISZOMBIE (fundef)) {
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
                     * Infer loop invariants
                     */
                    fundef = PHrunOptimizationInCycle (SUBPH_ili, loop, fundef);

                    /*
                     * Type upgrade
                     */
#ifndef TUPFIXED
                    if (global.optimize.dotup) {
                        fundef = PHrunOptimizationInCycle (SUBPH_ntccyc, loop, fundef);
                    }
#else
                    if (global.optimize.dotup || global.optimize.dortup
                        || global.optimize.dofsp || global.optimize.dosfd) {
                        fundef = PHrunOptimizationInCycle (SUBPH_tup, loop, fundef);
                    }
#endif

                    /*
                     * Reverse type upgrade
                     */
                    if (global.optimize.dortup) {
                        fundef = PHrunOptimizationInCycle (SUBPH_rtup, loop, fundef);
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
                     * With-loop folding
                     */
                    if (global.optimize.dowlf) {
                        fundef = PHrunOptimizationInCycle (SUBPH_wli, loop, fundef);
                        fundef = PHrunOptimizationInCycle (SUBPH_wlf, loop, fundef);
                        fundef = SSATdoTransformOneFundef (fundef);
                    }

                    /*
                     * Dead code removal
                     */
                    if (global.optimize.dodcr) {
                        fundef = PHrunOptimizationInCycle (SUBPH_dcrcyc, loop, fundef);
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
                    if (global.optimize.dolur || global.optimize.dowlur) {
                        fundef = PHrunOptimizationInCycle (SUBPH_lur, loop, fundef);
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
         * Dead function removal
         */
        if (global.optimize.dodfr) {
            arg_node = PHrunOptimizationInCycle (SUBPH_dfrcyc, loop, arg_node);
        }

        /*
         * Signature simplification
         */
        if ((global.optimize.dosisi) && (global.optimize.dodcr)) {
            arg_node = PHrunOptimizationInCycle (SUBPH_sisi, loop, arg_node);
        }
    } while ((AnyOptCounterNotZero (global.optcounters)) && (loop < global.max_optcycles)
             && (!PHbreakAfterCurrentPass (loop)));

    if ((loop == global.max_optcycles) && AnyOptCounterNotZero (global.optcounters)) {
        CTIwarn ("Maximal number of optimization cycles reached");
    }

    global.optcounters = AddOptCounters (global.optcounters, oldoptcounters);

    DBUG_RETURN (arg_node);
}

/* @} */
