/*****************************************************************************
 *
 * @defgroup
 *
 *
 *   This is a driver module for three transformations aiming at minimizing
 *   the number of host<->device memory transfers. These three transformations
 *   are applied in a cyclic fashion since one optimization might expose more
 *   opportunities for another optimization. The number of cycles is currently
 *   set at 10. However, a better approach would be to stop the cycle when no
 *   changes occur to the AST (Unfortunately, I have yet figurred out how to
 *   do it). For details of each transformation, please refer to the individual
 *   module files.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file minimize_transfers.c
 *
 * Prefix: MTRAN
 *
 *****************************************************************************/
#include "minimize_transfers.h"

#include <stdlib.h>

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "minimize_block_transfers2.h"
#include "annotate_memory_transfers.h"
#include "annotate_cond_transfers.h"
#include "minimize_loop_transfers.h"
#include "minimize_cond_transfers.h"
#include "minimize_cudast_transfers.h"
#include "globals.h"
#include "wl_descalarization.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *MLTRANdoMinimizeLoopTransfers( node *syntax_tree)
 *
 *****************************************************************************/
node *
MTRANdoMinimizeTransfers (node *syntax_tree)
{
    DBUG_ENTER ();

    int i, j;

    if (global.backend == BE_cuda && global.optimize.doexpar) {
        i = 0;
        while (i < 10) {
            /* syntax_tree = MBTRAN2doMinimizeBlockTransfers( syntax_tree); */
            syntax_tree = ACTRANdoAnnotateCondTransfers (syntax_tree);
            syntax_tree = MCTRANdoMinimizeCudastCondTransfers (syntax_tree);
            i++;
        }
    }

    j = 0;
    while (j < 10) {
        syntax_tree = MCSTRANdoMinimizeCudastTransfers (syntax_tree);
        syntax_tree = MBTRAN2doMinimizeBlockTransfers (syntax_tree);
        syntax_tree = ACTRANdoAnnotateCondTransfers (syntax_tree);
        syntax_tree = MCTRANdoMinimizeCondTransfers (syntax_tree);
        /* make sure the lifted transfer are removed when ever
         * possible before minimizing transfers in loops.
         */
        syntax_tree = MBTRAN2doMinimizeBlockTransfers (syntax_tree);
        /*********************************************************/
        syntax_tree = AMTRANdoAnnotateMemoryTransfers (syntax_tree);
        syntax_tree = MLTRANdoMinimizeLoopTransfers (syntax_tree);
        j++;
    }

    /* We perform loop invariant removal here because we found out
     * that that there are certained cases that are ignored by our
     * CUDA specific transfer removal, namely if a transfer is loop
     * invariant. For an example, plese refer to the transfer with
     * regard to array "features" in kmeans.sac in the CUDA Rodinia
     * benchmark suite.
     */
    // syntax_tree = LIRdoLoopInvariantRemoval( syntax_tree);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

#undef DBUG_PREFIX
