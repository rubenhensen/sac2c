/*
 *
 * $Log$
 * Revision 2.7  1999/05/10 11:11:40  bs
 * All functions of the tsi moved to wl_access_analyze.c
 *
 * Revision 2.6  1999/05/03 15:24:14  bs
 * The TSI is printing detailed information about array accesses within a WL.
 *
 * Revision 2.5  1999/04/29 08:00:56  bs
 * print routines modified.
 *
 * Revision 2.4  1999/04/12 18:00:54  bs
 * Two functions added: TSIprintAccesses and TSIprintFeatures.
 *
 * Revision 2.3  1999/04/08 12:49:37  bs
 * The TSI is analysing withloops now.
 *
 * Revision 2.2  1999/03/15 15:49:54  bs
 * access macros changed
 *
 * Revision 2.1  1999/02/23 12:43:17  sacbase
 * new release made
 *
 * Revision 1.1  1999/01/15 15:31:06  cg
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   tile_size_inference.c
 *
 * prefix: TSI
 *
 * description:
 *
 *   This compiler module realizes an inference scheme for the selection
 *   of appropriate tile sizes. This is used by the code generation in
 *   order to create tiled target code.
 *
 *   The following access macros are defined for the info-node:
 *
 *   INFO_TSI_ACCESS(n)     ((access_t*)n->info2)
 *   INFO_TSI_INDEXVAR(n)              (n->node[0])
 *   INFO_TSI_FEATURE(n)    ((feature_t)n->lineno)
 *   INFO_TSI_WOTYPE(n)    ((WithOpType)n->varno)
 *   INFO_TSI_LASTLETIDS(n)            (n->info.ids)
 *   INFO_TSI_BELOWAP(n)               (n->flag)
 *   INFO_TSI_WLLEVEL(n)               (n->counter)
 *   INFO_TSI_ACCESSVEC(n)    ((shpseg*)n->node[1])
 *   INFO_TSI_TMPACCESS(n)  ((access_t*)n->node[2])
 *   INFO_TSI_WLARRAY(n)               (n->node[3])
 *
 *****************************************************************************/

#include <stdlib.h>
#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "globals.h"
#include "print.h"
#include "tile_size_inference.h"

#define CURRENT_A 0
#define TEMP_A 1
#define _EXIT 0
#define _ENTER 1

/******************************************************************************
 *
 * function:
 *   node *TileSizeInference(node *arg_node, node *arg_info)
 *
 * description:
 *
 *   This function initiates the tile size inference scheme, i.e.
 *   act_tab is set to tsi_tab and the traversal mechanism is started.
 *   Just as the other optimization schemes, tile size selection is performed
 *   on single function definitions rather than on the entire syntax tree.
 *
 ******************************************************************************/

node *
TileSizeInference (node *arg_node)
{
    DBUG_ENTER ("TileSizeInference");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith),
                 "Tile size selection not initiated on N_Nwith level");

    DBUG_RETURN (arg_node);
}
