/*
 *
 * $Log$
 * Revision 2.3  1999/07/07 15:52:59  jhs
 * Removed SYNC_WITH_PTRS.
 *
 * Revision 2.2  1999/07/01 13:01:41  jhs
 * Added handling of INFO_SPMD_LAST.
 *
 * Revision 2.1  1999/02/23 12:44:19  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sync_init.c
 *
 * prefix: SYNCI
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   embrace each assignment within an spmd-function by a synchronisation
 *   block.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"

/******************************************************************************
 *
 * function:
 *   node *SYNCIassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Generates a sync-region for each first level with-loop.
 *   Then in SYNC_IN/OUT/INOUT/LOCAL the in/out/inout/local-vars of the
 *   sync-region are stored.
 *
 * remarks:
 *   INFO_SPMD_FIRST( arg_info) shows, weather the next sync-region would be
 *   the first one of the SPMD_region or not.
 *
 ******************************************************************************/

node *
SYNCIassign (node *arg_node, node *arg_info)
{
    node *with, *sync_let, *sync;
    ids *with_ids;

    DBUG_ENTER ("SYNCIassign");

    sync_let = ASSIGN_INSTR (arg_node);

    /* contains the current assignment a with-loop?? */
    if ((NODE_TYPE (sync_let) == N_let)
        && (NODE_TYPE (LET_EXPR (sync_let)) == N_Nwith2)) {

        with_ids = LET_IDS (sync_let);
        with = LET_EXPR (sync_let);

        /*
         * current assignment contains a with-loop
         *  -> create a SYNC-region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        sync = MakeSync (MakeBlock (MakeAssign (sync_let, NULL), NULL),
                         INFO_SPMD_FIRST (arg_info));
        ASSIGN_INSTR (arg_node) = sync;

        /*
         * get IN/INOUT/OUT/LOCAL from the N_Nwith2 node.
         */
        SYNC_IN (sync) = DFMGenMaskCopy (NWITH2_IN (with));
        SYNC_INOUT (sync) = DFMGenMaskCopy (NWITH2_INOUT (with));
        SYNC_OUT (sync) = DFMGenMaskCopy (NWITH2_OUT (with));
        SYNC_LOCAL (sync) = DFMGenMaskCopy (NWITH2_LOCAL (with));

        /*
         * unset flag: next N_sync node is not the first one in SPMD-region
         */
        INFO_SPMD_FIRST (arg_info) = 0;

        /*
         * we only traverse the following assignments to prevent nested
         *  sync-regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        SYNC_LAST (sync) = INFO_SPMD_LAST (arg_info);
        INFO_SPMD_LAST (arg_info) = 0;
    }

    DBUG_RETURN (arg_node);
}
