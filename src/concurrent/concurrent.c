/*****************************************************************************
 *
 * $Id$
 *
 * file:   concurrent.c
 *
 * prefix: CONC
 *
 * description:
 *
 *   This file initiates and guides the compilation process of building
 *   SPMD regions within the compiled SAC code. This entire process consists
 *   of 8 consecutive steps:
 *    - building spmd-blocks
 *    - optimizing/enlarging spmd-blocks (not yet implemented)
 *    - creating ST/MT functions
 *    - lifting spmd-blocks to spmd-functions
 *    - consolidating scheduling information
 *    - eliminating spmd-blocks
 *    - lifting conditionals into separate functions (restore SSA form)
 *    - eliminating multiple assignments (restore SSA form)
 *
 *****************************************************************************/

#include "concurrent.h"

#include "dbug.h"
#include "ctinfo.h"
#include "phase.h"
#include "globals.h"

/******************************************************************************
 *
 * function:
 *   node *CONCdoConcurrent(arg_node *syntax_tree)
 *
 * description:
 *
 *   This function starts the process of building spmd- and synchronisation
 *   blocks. Throughout this process arg_info points to an N_info node which
 *   is generated here.
 *
 *   *** This is the process building the old support for multithread. ***
 *
 ******************************************************************************/

node *
CONCdoConcurrent (node *syntax_tree)
{
    DBUG_ENTER ("CONCdoConcurrent");

    switch (global.mtmode) {
    case MT_none:
        CTIstate ("  Multithreaded execution deactivated");
        break;

    case MT_createjoin:
    case MT_startstop:
        if (global.mtmode == MT_createjoin) {
            CTIstate ("  Using create-join version of multithreading (MT1)");
        } else {
            CTIstate ("  Using start-stop version of multithreading (MT2)");
        }

#ifndef BEMT

        /*
         * Init SPMD blocks around with-loops
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdinit, syntax_tree);

        /*
         * Create MT-funs for exported and provided functions in modules
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_createmtfuns, syntax_tree);

        /*
         * Lift SPMD blocks to SPMD functions
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_spmdlift, syntax_tree);

        /*
         * Remove scheduling information outside of SPMD functions AND
         * give each segment specification within an SPMD function scheduling
         * specifications.
         * These are either infered from the context or extracted from
         * wlcomp pragma information.
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_sched, syntax_tree);

        /*
         * Replace SPMD blocks with explicit conditionals
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_rmspmd, syntax_tree);

        /*
         * Lift SPMD conditionals to special fundefs
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_conclac2fun, syntax_tree);

        /*
         * Establish SSA form in SPMD conditional functions
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_concssa, syntax_tree);
#endif
        break;

#ifndef PRODUCTION
    case MT_mtstblock:
        CTIstate ("  Using mt/st-block version of multithreading (MT3)");
        global.executionmodes_available = TRUE;

        /*
         * Tagging execution modes
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_tem, syntax_tree);

        /*
         * Create with in with
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_crwiw, syntax_tree);

        /*
         * Propagate execution modes
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_pem, syntax_tree);

        /*
         * Create data flow graph
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_cdfg, syntax_tree);

        /*
         * Rearrange assignments
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_asmra, syntax_tree);

        /*
         * Create cells
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_crece, syntax_tree);

        /*
         * Cell growth
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_cegro, syntax_tree);

        /*
         * Replicate functions
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_repfun, syntax_tree);

        /*
         * Consolidate execution mode cells
         */
        syntax_tree = PHrunCompilerSubPhase (SUBPH_concel, syntax_tree);

        global.executionmodes_available = FALSE;
        CTIabort ("MT mode 3 cannot be compiled any further!");
        break;
#endif

    default:
        CTIabort ("Illegal multithreading mode!");
        break;
    }

    DBUG_RETURN (syntax_tree);
}
