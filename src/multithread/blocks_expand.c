/*
 *
 * $Log$
 * Revision 1.1  2000/02/21 11:02:13  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   blocks_expand.c
 *
 * prefix: BLKEX
 *
 * description:
 *   ####
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *BlocksExpand(node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
BlocksExpand (node *arg_node, node *arg_info)
{
    funtab *old_tab;

    DBUG_ENTER ("BlocksExpand");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "BlocksExpand expects a N_fundef as arg_node");

    old_tab = act_tab;
    act_tab = blkex_tab;

    /* push info ... */

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    /* pop info ... */

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}
