/*****************************************************************************
 *
 * $Id$
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

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DataFlowMask.h"
#include "globals.h"
#include "internal_lib.h"
#include "concurrent_info.h"

/******************************************************************************
 *
 * function:
 *   node *SYNCIassign( node *arg_node, info *arg_info)
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
SYNCIassign (node *arg_node, info *arg_info)
{
    node *with, *sync_let, *sync, *withop, *with_ids;
    dfmask_base_t *maskbase;
    int foldcount;

    DBUG_ENTER ("SYNCIassign");

    DBUG_PRINT ("SYNCI", ("frontwards"));

    /*
     *  the maskbase is needed at several spots, and does not change, so one
     *  can initialize it here.
     */
    maskbase = FUNDEF_DFM_BASE (INFO_CONC_FUNDEF (arg_info));

    sync_let = ASSIGN_INSTR (arg_node);

    /*
     *  contains the current assignment a with-loop??
     */
    if ((NODE_TYPE (sync_let) == N_let) && (NODE_TYPE (LET_EXPR (sync_let)) == N_with2)) {
        DBUG_PRINT ("SYNCI", ("build sync-block around with-loop"));

        with = LET_EXPR (sync_let);

        /*
         * current assignment contains a with-loop
         *  -> create a SYNC-region containing the current assignment only
         *     and insert it into the syntaxtree.
         */
        sync = TBmakeSync (TBmakeBlock (TBmakeAssign (sync_let, NULL), NULL));
        SYNC_FIRST (sync) = INFO_SYNCI_FIRST (arg_info);
        ASSIGN_INSTR (arg_node) = sync;

        withop = WITH2_WITHOP (with);
        foldcount = 0;
        while (withop != NULL) {
            if (NODE_TYPE (withop) == N_fold) {
                foldcount++;
            }
            withop = WITHOP_NEXT (withop);
        }

        SYNC_FOLDCOUNT (sync) = foldcount;
        global.needed_sync_fold = MAX (global.needed_sync_fold, foldcount);

        /*
         * get IN/INOUT/OUT/LOCAL from the N_Nwith2 node.
         */
        SYNC_IN (sync) = DFMgenMaskCopy (WITH2_IN_MASK (with));
        SYNC_INOUT (sync) = DFMgenMaskClear (maskbase);
        SYNC_OUT (sync) = DFMgenMaskCopy (WITH2_OUT_MASK (with));
        SYNC_LOCAL (sync) = DFMgenMaskCopy (WITH2_LOCAL_MASK (with));
        SYNC_OUTREP (sync) = DFMgenMaskClear (maskbase);

        withop = WITH2_WITHOP (with);
        with_ids = LET_IDS (sync_let);
        while (withop != NULL) {
            /*
             * add vars from LHS of with-loop assignment
             */
            if ((NODE_TYPE (withop) == N_genarray)
                || (NODE_TYPE (withop) == N_modarray)) {

                DFMsetMaskEntrySet (SYNC_INOUT (sync), NULL,
                                    ID_AVIS (WITHOP_MEM (withop)));
                DFMsetMaskEntryClear (SYNC_IN (sync), NULL,
                                      ID_AVIS (WITHOP_MEM (withop)));
            } else {
                DFMsetMaskEntrySet (SYNC_OUT (sync), NULL, IDS_AVIS (with_ids));
            }
            withop = WITHOP_NEXT (withop);
            with_ids = IDS_NEXT (with_ids);
        }

        /*
         * unset flag: next N_sync node is not the first one in SPMD-region
         */
        INFO_SYNCI_FIRST (arg_info) = 0;
    } else if ((NODE_TYPE (sync_let) == N_while) || (NODE_TYPE (sync_let) == N_do)) {
        DBUG_PRINT ("SYNCI", ("trav into loop"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
        DBUG_PRINT ("SYNCI", ("trav from loop"));
        sync = NULL;
    } else if (NODE_TYPE (sync_let) == N_return) {
        DBUG_PRINT ("SYNCI", ("return reached (no sync-block inserted)"));
        sync = NULL;
    } else {
        DBUG_PRINT ("SYNCI", ("build sync-block around non with-loop"));
        sync = TBmakeSync (TBmakeBlock (TBmakeAssign (sync_let, NULL), NULL));
        SYNC_FIRST (sync) = INFO_SYNCI_FIRST (arg_info);
        ASSIGN_INSTR (arg_node) = sync;
        INFO_SYNCI_FIRST (arg_info) = 0;

        SYNC_IN (sync) = DFMgenMaskClear (maskbase);
        SYNC_INOUT (sync) = DFMgenMaskClear (maskbase);
        SYNC_OUT (sync) = DFMgenMaskClear (maskbase);
        SYNC_OUTREP (sync) = DFMgenMaskClear (maskbase);
        SYNC_LOCAL (sync) = DFMgenMaskClear (maskbase);

        /*
         * unset flag: next N_sync node is not the first one in SPMD-region
         */
        INFO_SYNCI_FIRST (arg_info) = 0;
    }
    DBUG_PRINT ("SYNCI", ("inbetween"));

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("SYNCI", ("into assign next"));
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("SYNCI", ("from assign next"));

        if (sync != NULL) {
            SYNC_LAST (sync) = INFO_SYNCI_LAST (arg_info);
        }
        INFO_SYNCI_LAST (arg_info) = 0;
    } else {
        DBUG_PRINT ("SYNCI", ("turnaround"));
    }

    DBUG_PRINT ("SYNCI", ("backwards"));

    DBUG_RETURN (arg_node);
}
