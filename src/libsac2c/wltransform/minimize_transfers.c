
#include "minimize_transfers.h"

#include <stdlib.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "globals.h"
#include "dbug.h"
#include "ctinfo.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "print.h"
#include "DataFlowMask.h"
#include "NameTuplesUtils.h"
#include "scheduling.h"
#include "wl_bounds.h"
#include "new_types.h"
#include "user_types.h"
#include "shape.h"
#include "LookUpTable.h"
#include "convert.h"
#include "math_utils.h"
#include "types.h"
#include "deadcoderemoval.h"
#include "compare_tree.h"

#include "minimize_sequential_transfers.h"
#include "minimize_loop_transfers.h"
#include "annotate_memory_transfers.h"

node *
MTRANdoMinimizeTransfers (node *syntax_tree)
{
    bool seq_nochange;
    bool loop_nochange;

    DBUG_ENTER ("MTRANdoMinimizeTransfers");

    int i = 0;

    while (i < 10) {
        // seq_nochange = TRUE;
        // loop_nochange = TRUE;
        syntax_tree = MSTRANdoMinimizeTransfers (syntax_tree, &seq_nochange);
        // if( i != 1)

        syntax_tree = AMTRANdoAnnotateMemoryTransfers (syntax_tree);
        syntax_tree = MLTRANdoMinimizeLoopTransfers (syntax_tree, &loop_nochange);
        i++;
        printf ("[%d,%d]\n", seq_nochange, loop_nochange);
    }
    // while( !seq_nochange || !loop_nochange);

    DBUG_RETURN (syntax_tree);
}
