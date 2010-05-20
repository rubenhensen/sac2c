/*
 *
 * $Id$
 *
 */

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "dbug.h"
#include "new_types.h"

#include "gdb_utils.h"

/******************************************************************************
 *
 * function: GDBbreakAtNid( node *arg_node, char *nm)
 *
 * description:
 *
 * This function is intended to assist users of ddd/gdb in
 * making the sac2c compiler stop when a particular node appears
 * in some function. E.g., suppose we want to drop in CFlet
 * when performing:   res = _add_SxS( x, y);
 *
 * Do this in gdb/ddd:
 *
 *     break CFlet
 * Breakpoint 3 at ...
 *     condition 3  BreakAtNid( arg_node, "res")
 *
 * The gdb break will be ignored until the first LET_IDS
 * is "res", at which point the break will be honored.
 *
 ******************************************************************************/
bool
GDBbreakAtNid (node *arg_node, char *nm)
{
    bool z;

    if (NULL == arg_node) {
        z = FALSE;
    } else {
        switch (NODE_TYPE (arg_node)) {

        case N_id:
            z = (0 == strcmp (nm, AVIS_NAME (ID_AVIS (arg_node))));
            break;

        case N_ids:
            z = (0 == strcmp (nm, AVIS_NAME (IDS_AVIS (arg_node))));
            break;

        case N_let:
            z = (0 == strcmp (nm, AVIS_NAME (IDS_AVIS (LET_IDS (arg_node)))));
            break;

        case N_avis:
            z = (0 == strcmp (nm, AVIS_NAME (arg_node)));
            break;

        case N_fundef:
            z = (0 == strcmp (nm, FUNDEF_NAME (arg_node)));
            break;

        default:
            z = FALSE;
            break;
        }
    }

    return (z);
}
