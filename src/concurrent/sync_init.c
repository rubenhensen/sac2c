/*
 *
 * $Log$
 * Revision 2.7  1999/08/09 11:32:20  jhs
 * Cleaned up info-macros for concurrent-phase.
 *
 * Revision 2.6  1999/07/21 16:30:27  jhs
 * needed_sync_fold introduced, max_sync_fold_adjusted.
 *
 * Revision 2.5  1999/07/20 16:59:11  jhs
 * Added counting of FOLDCOUNT.
 *
 * Revision 2.4  1999/07/19 14:45:13  jhs
 * Changed signature of MakeSync.
 *
 * Revision 2.3  1999/07/07 15:52:59  jhs
 * Removed SYNC_WITH_PTRS.
 *
 * Revision 2.2  1999/07/01 13:01:41  jhs
 * Added handling of INFO_SYNCI_LAST.
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
#include "globals.h"
#include "internal_lib.h"

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
 *   INFO_SYNCI_(FIRST|LAST)( arg_info) show, whether the next sync-region
 *   would be the (first|last) one of the spmd-region or not.
 *
 ******************************************************************************/

node *
SYNCIassign (node *arg_node, node *arg_info)
{
    node *with, *sync_let, *sync;
    node *withop;
    ids *with_ids;

    DBUG_ENTER ("SYNCIassign");

    sync_let = ASSIGN_INSTR (arg_node);

    /*
     *  contains the current assignment a with-loop??
     */
    if ((NODE_TYPE (sync_let) == N_let)
        && (NODE_TYPE (LET_EXPR (sync_let)) == N_Nwith2)) {

        with_ids = LET_IDS (sync_let);
        with = LET_EXPR (sync_let);

        /*
         * current assignment contains a with-loop
         *  -> create a SYNC-region containing the current assignment only
         *     and insert it into the syntaxtree.
         */
        sync = MakeSync (MakeBlock (MakeAssign (sync_let, NULL), NULL));
        SYNC_FIRST (sync) = INFO_SYNCI_FIRST (arg_info);
        ASSIGN_INSTR (arg_node) = sync;

        withop = NWITH2_WITHOP (with);
        if (NWITHOP_TYPE (withop) == WO_foldfun) {
            SYNC_FOLDCOUNT (sync) = 1;
            needed_sync_fold = MAX (needed_sync_fold, 1);
        } else {
            SYNC_FOLDCOUNT (sync) = 0;
        }

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
        INFO_SYNCI_FIRST (arg_info) = 0;

        /*
         * we only traverse the following assignments to prevent nested
         *  sync-regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);

        SYNC_LAST (sync) = INFO_SYNCI_LAST (arg_info);
        INFO_SYNCI_LAST (arg_info) = 0;
    }

    DBUG_RETURN (arg_node);
}
