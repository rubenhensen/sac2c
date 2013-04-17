/** <!--********************************************************************-->
 *
 * @defgroup wlut With-Loop utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            services for manipulating and examining with-loop.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file with_loop_utilities.c
 *
 * Prefix: WLUT
 *
 *****************************************************************************/
#include "with_loop_utilities.h"

#include "globals.h"

#define DBUG_PREFIX "WLUT"

#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "compare_tree.h"
#include "new_types.h"
#include "DupTree.h"
#include "constants.h"
#include "lacfun_utilities.h"
#include "new_typecheck.h"

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisEmptyPartitionCodeBlock( node *partn)
 *
 * @brief Predicate for finding N_part node with no code block.
 * @param N_part
 * @result TRUE if code block is empty
 *
 *****************************************************************************/
bool
WLUTisEmptyPartitionCodeBlock (node *partn)
{
    bool z;

    DBUG_ENTER ();

    z = (NULL == BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (partn))));

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool WLUTisIdsMemberPartition( node *arg_node, node *partn)
 *
 * @brief: Predicate for checking if arg_node's definition point
 *         is within a specified WL partition.
 *
 * @param: arg_node - a WLINTERSECT1/2 node.
 *         partn:   - a WL partition. In our case, it is
 *                    that of the consumerWL.
 *
 * @result: TRUE if arg_node is defined within the partition.
 *
 * @note: This is required because we can produce an inverse
 *        projection that can be used for cube slicing ONLY if said
 *        projection is defined OUTSIDE the current WL.
 *
 *****************************************************************************/
bool
WLUTisIdsMemberPartition (node *arg_node, node *partn)
{
    bool z = FALSE;
    node *nassgns;

    DBUG_ENTER ();

    if (NULL != partn) {
        nassgns = BLOCK_ASSIGNS (CODE_CBLOCK (PART_CODE (partn)));
        while ((NULL != nassgns) && (!z)) {
            z = LFUisAvisMemberIds (ID_AVIS (arg_node), LET_IDS (ASSIGN_STMT (nassgns)));
            nassgns = ASSIGN_NEXT (nassgns);
        }
    }

    DBUG_RETURN (z);
}
