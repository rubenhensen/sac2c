/*
 *
 * $Log$
 * Revision 3.110  2005/09/12 13:56:38  ktr
 * added wlsimplification.o
 *
 * Revision 3.109  2005/09/10 21:10:05  sbs
 * adjusted cycle phase conventions
 *
 * Revision 3.108  2005/09/09 23:39:53  sbs
 * added function dispatch and inlining after TUP in the cycle...
 *
 * Revision 3.107  2005/09/08 09:26:46  ktr
 * minor brushing
 *
 * Revision 3.106  2005/09/04 12:52:11  ktr
 * re-engineered the optimization cycle
 *
 * Revision 3.105  2005/09/02 17:48:59  sah
 * removed ALWAYSMAXOPT again
 *
 * Revision 3.104  2005/09/02 14:25:26  ktr
 * uses liftoptflags
 *
 * Revision 3.103  2005/08/29 11:27:02  ktr
 * As long as TUP is not fixed, the NTC is applied in the opt cycle
 *
 * Revision 3.102  2005/08/26 12:27:31  ktr
 * removed WLT (superseded by WLPG)
 *
 * Revision 3.101  2005/08/24 10:26:01  ktr
 * added wlidxs traversal
 *
 * Revision 3.100  2005/08/20 23:42:47  ktr
 * added IVEI
 *
 * Revision 3.99  2005/08/20 12:06:50  ktr
 * added TypeConvElimination
 *
 * Revision 3.98  2005/07/19 17:08:26  ktr
 * replaced SSADeadCodeRemoval with deadcoderemoval
 *
 * Revision 3.97  2005/07/16 21:14:24  sbs
 * moved dispatch and rmcasts into WLEnhancement.c
 *
 * Revision 3.96  2005/07/15 15:57:02  sah
 * introduced namespaces
 *
 * Revision 3.95  2005/07/15 15:23:08  ktr
 * removed type conversions before and after IVE
 *
 * Revision 3.94  2005/06/28 15:38:25  jhb
 * added phase.h by the includes
 *
 * Revision 3.93  2005/06/06 13:28:40  jhb
 * added PHrunCompilerSubPhase
 *
 * Revision 3.92  2005/06/02 13:42:48  mwe
 * rerun optimization cycle if sisi found optimization cases
 *
 * Revision 3.91  2005/05/31 13:41:38  mwe
 * run sisi only when dcr activated
 *
 * Revision 3.90  2005/05/25 09:52:33  khf
 * optimize normal functions instead of zombie functions
 *
 * Revision 3.89  2005/05/13 16:46:15  ktr
 * lacinlining is now performed in the cycle
 *
 * Revision 3.88  2005/04/20 19:14:31  ktr
 * removed SSArestoreSsaOneFunction after LIR
 *
 * Revision 3.87  2005/04/19 17:59:10  khf
 * removed transformation in ssa-form
 *
 * Revision 3.86  2005/04/19 17:26:09  ktr
 * "lacinl" break specifier introduced
 *
 * Revision 3.85  2005/03/17 19:05:17  sbs
 * IVE still runs on old types.....
 *
 * Revision 3.84  2005/03/17 14:03:11  sah
 * removed global check for function body
 *
 * Revision 3.83  2005/03/10 09:41:09  cg
 * Added #include "DupTree.h"
 *
 * Revision 3.82  2005/03/04 21:21:42  cg
 * Useless conditional eliminated.
 * Integration of silently duplicated LaC funs at the end of the
 * fundef chain added.
 *
 * Revision 3.81  2005/02/16 14:11:09  mwe
 * some renaming done, corrected break specifier
 *
 * Revision 3.80  2005/02/15 14:53:00  mwe
 * changes for esd and uesd
 *
 * Revision 3.79  2005/02/14 11:18:34  cg
 * Old inlining replaced by complete re-implementation.
 *
 * Revision 3.78  2005/02/11 12:13:11  mwe
 * position of sisi changed
 *
 * Revision 3.77  2005/02/03 18:28:22  mwe
 * new counter added
 * order of intrafunctional optimization changed
 * optimization output changed
 * some code beautifying
 *
 * Revision 3.76  2005/02/02 18:09:50  mwe
 * new counter added
 * signature simplification added
 *
 * Revision 3.75  2005/01/27 18:20:30  mwe
 * new counter for type_upgrade added
 *
 * Revision 3.74  2005/01/11 12:58:15  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 3.73  2004/12/09 13:08:54  mwe
 * type_upgrade now running before constant folding
 *
 * Revision 3.72  2004/12/09 10:59:30  mwe
 * support for type_upgrade added
 *
 * Revision 3.71  2004/11/27 01:48:24  jhb
 * comment out WLAdoAccessAnalysis and ApdoArrayPadding
 *
 * Revision 3.70  2004/11/26 18:14:35  mwe
 * change prefix of IVE
 *
 * Revision 3.69  2004/11/26 16:27:36  mwe
 * SacDevCamp
 *
 * ... [elminated] ...
 *
 * Revision 1.1  1994/12/09  10:47:40  sbs
 * Initial revision
 *
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
#include "index_infer.h" /* for IVEIprintPreFun */
#include "new_types.h"   /* for TYtype2String */

/**
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

    /*
     * Intra-functional optimizations
     */
    arg_node = PHrunCompilerSubPhase (SUBPH_intraopt, arg_node);

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
        TRAVsetPreFun (TR_prt, NULL);
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
                        fundef = PHrunOptimizationInCycle (SUBPH_wlf, loop, fundef);
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
                     * Loop invariant removal
                     */
                    if (global.optimize.dolir) {
                        fundef = PHrunOptimizationInCycle (SUBPH_lir, loop, fundef);
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
