/**
 * @file
 * @defgroup mtran Minimize Transfers
 * @ingroup cuda
 *
 * This is a driver module for several transformations aiming at minimizing
 * the number of host<->device memory transfers. These three transformations
 * are applied in a cyclic fashion since one optimization might expose more
 * opportunities for another optimization. The number of cycles is currently
 * set at max_optcycles (globals), but will termiate early if we've reached a
 * fixed point.
 *
 * @{
 */
#include "minimize_transfers.h"

#include "phase.h"
#include "traverse_optcounter.h"

#define DBUG_PREFIX "MTRAN"
#include "debug.h"

#include "minimize_block_transfers2.h"
#include "annotate_memory_transfers.h"
#include "annotate_cond_transfers.h"
#include "minimize_loop_transfers.h"
#include "minimize_cond_transfers.h"
#include "minimize_cudast_transfers.h"
#include "minimize_emr_transfers.h"
#include "loop_invariant_removal.h"
#include "globals.h"
#include "wl_descalarization.h"

/**
 * @name Entry functions
 * @{
 */

/**
 * @brief Applies various optimisation to the syntax tree, to minimize CUDA
 *        memcpy operations.
 *
 * @param syntax_tree
 * @return the syntax tree
 */
node *
MTRANdoMinimizeTransfers (node *syntax_tree)
{
    int i;
    bool done = false;

    TOC_SETUP(1, COUNT_TRL)

    DBUG_ENTER ();

    DBUG_PRINT ("Performaing CUDA Minimize Transfers Optimistions");

    if (global.optimize.doexpar) {
        DBUG_PRINT ("Doing expar optimisation cycle:");
        for (i = 1; i < global.max_optcycles; i++) {
            /* XXX disabled for some reason, further investigation needed */
            TOC_RUNOPT ("MBTRAN2", false, COUNT_TRL,
                    global.optcounters.cuda_min_trans,
                    syntax_tree, MBTRAN2doMinimizeBlockTransfers)
            TOC_RUNOPT ("ACTRAN", true, TOC_IGNORE, 0,
                    syntax_tree, ACTRANdoAnnotateCondTransfers)
            TOC_RUNOPT ("MCTRAN", true, COUNT_TRL,
                    global.optcounters.cuda_min_trans,
                    syntax_tree, MCTRANdoMinimizeCondTransfers)

            TOC_COMPARE (done)

            if (done) {
                break;
            }
        }
        DBUG_PRINT ("Completed expar optimisation cycle after %d cycles", i);
    }

    /* reset counters for next cycle */
    TOC_RESETCOUNTERS ()
    done = false;

    DBUG_PRINT ("Doing general optimisation cycle:");
    for (i = 1; i < global.max_optcycles; i++) {

        TOC_RUNOPT ("MCSTRAN", true, COUNT_TRL, global.optcounters.cuda_min_trans,
                syntax_tree, MCSTRANdoMinimizeCudastTransfers)
        TOC_RUNOPT ("MBTRAN2", true, COUNT_TRL, global.optcounters.cuda_min_trans,
                syntax_tree, MBTRAN2doMinimizeBlockTransfers)
        TOC_RUNOPT ("ACTRAN", true, TOC_IGNORE, 0,
                syntax_tree, ACTRANdoAnnotateCondTransfers)
        TOC_RUNOPT ("MCTRAN", true, COUNT_TRL, global.optcounters.cuda_min_trans,
                syntax_tree, MCTRANdoMinimizeCondTransfers)
        /* make sure the lifted transfer are removed when ever
         * possible before minimizing transfers in loops.
         */
        TOC_RUNOPT ("MBTRAN2", true, COUNT_TRL, global.optcounters.cuda_min_trans,
                syntax_tree, MBTRAN2doMinimizeBlockTransfers)
        TOC_RUNOPT ("AMTRAN", true, TOC_IGNORE, 0,
                syntax_tree, AMTRANdoAnnotateMemoryTransfers)
        TOC_RUNOPT ("MLTRAN", true, COUNT_TRL, global.optcounters.cuda_min_trans,
                syntax_tree, MLTRANdoMinimizeLoopTransfers)

        TOC_COMPARE (done)

        DBUG_PRINT ("Counter: Lift -> %zu",
                    (global.optcounters.cuda_min_trans - TOC_GETCOUNTER (COUNT_TRL)));

        if (done) {
            break;
        }
    }
    DBUG_PRINT ("Completed general optimisation cycle after %d cycles", i);

    /* For any EMR lifted allocations which are H2Ds within a do-loop,
     * we artificially lift these out, similar to MLTRAN above. We assume
     * that because of the buffer-swapping, there is always a suitable
     * device type to pass in as part of recursive call within the do-loop.
     */
    if (global.optimize.doemrci && global.optimize.domemrt) {
        syntax_tree = MEMRTdoMinimizeEMRTransfers (syntax_tree);
    }

    /* We perform loop invariant removal here because we found out
     * that that there are certained cases that are ignored by our
     * CUDA specific transfer removal, namely if a transfer is loop
     * invariant. For an example, plese refer to the transfer with
     * regard to array "features" in kmeans.sac in the CUDA Rodinia
     * benchmark suite.
     */
    // syntax_tree = DLIRdoLoopInvariantRemoval (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** @} */
/** @} */
#undef DBUG_PREFIX
