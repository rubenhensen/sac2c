/*****************************************************************************
 *
 *
 * file:   minimize_transfers.c
 *
 * prefix: MTRAN
 *
 * description:
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

#include "minimize_transfers.h"

#include <stdlib.h>
#include "dbug.h"
#include "minimize_block_transfers.h"
#include "annotate_memory_transfers.h"
#include "minimize_loop_transfers.h"

node *
MTRANdoMinimizeTransfers (node *syntax_tree)
{
    DBUG_ENTER ("MTRANdoMinimizeTransfers");

    int i = 0;
    while (i < 10) {
        syntax_tree = MBTRANdoMinimizeBlockTransfers (syntax_tree);
        syntax_tree = AMTRANdoAnnotateMemoryTransfers (syntax_tree);
        syntax_tree = MLTRANdoMinimizeLoopTransfers (syntax_tree);
        i++;
    }
    DBUG_RETURN (syntax_tree);
}
