/*
 *
 * $Log$
 * Revision 3.9  2004/11/24 19:29:17  skt
 * Compiler Switch during SACDevCampDK 2k4
 *
 * Revision 3.8  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.7  2004/10/07 15:35:31  khf
 * added support for multioperator WLs
 *
 * Revision 3.6  2004/09/28 16:33:12  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.5  2004/09/28 14:09:59  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.4  2004/09/18 16:05:48  ktr
 * DFMs are adjusted differently in EMM because memory is allocated explicitly.
 *
 * Revision 3.3  2001/03/05 16:42:00  dkr
 * no macros NWITH???_IS_FOLD used
 *
 * Revision 3.2  2000/12/12 12:11:43  dkr
 * NWITH_INOUT removed
 * interpretation of NWITH_IN changed:
 * the LHS of a with-loop assignment is now longer included in
 * NWITH_IN!!!
 *
 * Revision 3.1  2000/11/20 18:02:34  sacbase
 * new release made
 *
 * Revision 2.9  2000/01/25 13:42:38  dkr
 * function FindVardec moved to tree_compound.h and renamed to
 * FindVardec_Varno
 *
 * Revision 2.8  1999/08/27 12:48:03  jhs
 * Added comments.
 * Added possibility to inserted SYNC blokcs around non with-loop code.
 *
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

/******************************************************************************
 *
 * function:
 *   node *SYNCIwhile( node *arg_node, info *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/

node *
SYNCIwhile (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SYNCIwhile");

    DBUG_PRINT ("SYNCI", ("trav into while"));
    WHILE_BODY (arg_node) = TRAVdo (WHILE_BODY (arg_node), arg_info);
    DBUG_PRINT ("SYNCI", ("trav from while"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SYNCIdo( node *arg_node, info *arg_info)
 *
 * description:
 *   ####
 ******************************************************************************/

node *
SYNCIdo (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SYNCIdo");

    DBUG_PRINT ("SYNCI", ("trav into do"));
    DO_BODY (arg_node) = TRAVdo (DO_BODY (arg_node), arg_info);
    DBUG_PRINT ("SYNCI", ("trav from do"));

    DBUG_RETURN (arg_node);
}
