/*
 *
 * $Log$
 * Revision 2.4  1999/07/21 12:28:56  jhs
 * Checking of max_sync_fold adjusted.
 *
 * Revision 2.3  1999/07/20 16:59:44  jhs
 * Added counting and checking of FOLDCOUNT.
 *
 * Revision 2.2  1999/07/07 15:55:57  jhs
 * Added SYNCO[sync|assign], first steps to melt sync-blocks.
 *
 * Revision 2.1  1999/02/23 12:44:21  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   sync_opt.c
 *
 * prefix: SYNCO
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   optimize synchronisation blocks, i.e. adjacent synchronisation blocks
 *   are merged into a single one where data dependencies do not require
 *   a barrier synchronisation in between.
 *
 *   It implements the traversal functions of *syncopt_tab*.
 *
 *****************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "spmd_trav.h"
#include "spmd_opt.h"
#include "globals.h"

#include "sync_opt.h"

/* #### better name and comment -> move to DataFlowMask.[ch] */
int
Disjunctive (DFMmask_t mask1, DFMmask_t mask2)
{
    int result;
    DFMmask_t andmask;

    DBUG_ENTER ("Disjunctive");

    andmask = DFMGenMaskAnd (mask1, mask2);
    if (DFMTestMask (andmask)) {
        result = FALSE;
    } else {
        result = TRUE;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   ####
 *
 * description:
 *   Tests whether the two syncs specified can be melted together, i.e. whether
 *   their dataflow maske are disjunctive and do not contain too much
 *   fold-with-loops.
 *
 *   Returns TRUE if both blocks can be melted together, FALSE otherwise.
 *
 *   Blocks can be melted if
 *   - The out-mask of the first block is disjunct to the in-mask of the
 *     second block. This includes implicitly the inout-masks.
 *   ####
 *
 ******************************************************************************/
int
MeltableSYNCs (node *first_sync, node *second_sync /* , ... */)
{
    int result;

    DBUG_ENTER ("MeltableSYNCs");

    /* ... #### */

    result = 1;
    result = result & Disjunctive (SYNC_IN (first_sync), SYNC_INOUT (second_sync));
    result = result & Disjunctive (SYNC_IN (first_sync), SYNC_OUT (second_sync));
    result = result & Disjunctive (SYNC_INOUT (first_sync), SYNC_IN (second_sync));
    result = result & Disjunctive (SYNC_INOUT (first_sync), SYNC_INOUT (second_sync));
    result = result & Disjunctive (SYNC_INOUT (first_sync), SYNC_OUT (second_sync));
    result = result & Disjunctive (SYNC_OUT (first_sync), SYNC_IN (second_sync));
    result = result & Disjunctive (SYNC_OUT (first_sync), SYNC_INOUT (second_sync));
    result = result & Disjunctive (SYNC_OUT (first_sync), SYNC_OUT (second_sync));

    if (result) {
        DBUG_PRINT ("SYNCO", ("disjunctive"));
    } else {
        DBUG_PRINT ("SYNCO", ("non-disjunctive"));
    }

    result
      = result
        & ((SYNC_FOLDCOUNT (first_sync) + SYNC_FOLDCOUNT (second_sync)) <= max_sync_fold);

    if (result) {
        DBUG_PRINT ("SYNCO", ("folds ok"));
    } else {
        DBUG_PRINT ("SYNCO", ("too much folds"));
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   ####
 *
 * description:
 *   melts blocks with sideeffects
 *   ####
 *
 * attention:
 *   - Both SYNCs have to be meltable, or this function will fail!!!
 *     Test if meltable before calling this function!
 *   - There are some sideeffects!
 *     first_sync will be used to build the result, while second_sync will be
 *     freed. Use MeltSYNCsOnCopies (see below) to avoid this. That function
 *     first copies both arguments, so no sideeffects occur to the outside.
 *
 * remark:
 *   A version without sideeffects is also available
 *   (see MeltSYNCsOnCopies below).
 *
 ******************************************************************************/
node *
MeltSYNCs (node *first_sync, node *second_sync /*,  ...  #### */)
{
    node *result; /* result value of this function */

    DBUG_ENTER ("MeltSYNCs");

    DBUG_ASSERT ((NODE_TYPE (first_sync) == N_sync), "First argument not a N_sync!");
    DBUG_ASSERT ((NODE_TYPE (second_sync) == N_sync), "Second argument not a N_sync!");

    DBUG_ASSERT (MeltableSYNCs (first_sync, second_sync),
                 "sync-blocks overhanded are not meltable");

    /*
     *  Combine the mask to new ones.
     */
    SYNC_REGION (first_sync)
      = MeltBlocks (SYNC_REGION (first_sync), SYNC_REGION (second_sync));
    SYNC_REGION (second_sync) = NULL;

    /*
     *  Melt masks of used variables
     */

    DBUG_PRINT ("SYNCO", ("melting masks now ... "));

    DFMSetMaskOr (SYNC_IN (first_sync), SYNC_IN (second_sync));
    DFMSetMaskOr (SYNC_INOUT (first_sync), SYNC_INOUT (second_sync));
    DFMSetMaskOr (SYNC_OUT (first_sync), SYNC_OUT (second_sync));
    DFMSetMaskOr (SYNC_LOCAL (first_sync), SYNC_LOCAL (second_sync));

    SYNC_FOLDCOUNT (first_sync)
      = SYNC_FOLDCOUNT (first_sync) + SYNC_FOLDCOUNT (second_sync);

    FreeTree (second_sync);

    result = first_sync;
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   ####
 *
 * description:
 *   Same functinality as MeltSYNCs, but the arguments are not touched, because
 *   they are copied before actual melting starts.
 *
 * remarks:
 *   A simple version without any copying is also available
 *   (see MeltSYNCs above).
 *
 ******************************************************************************/
node *
MeltSYNCsOnCopies (node *first_sync, node *second_sync /* , ...  #### */)
{
    node *result;

    DBUG_ENTER ("MeltSYNCsOnCopies");

    first_sync = DupTree (first_sync, NULL);
    second_sync = DupTree (second_sync, NULL);

    result = MeltSYNCs (first_sync, second_sync /* , ... #### */);

    DBUG_RETURN (result);
}

/* #### comment missing */
node *
SYNCOsync (node *arg_node, node *arg_info)
{
    node *result;

    DBUG_ENTER ("SYNCOsync");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_sync), "Wrong NODE_TYPE: N_sync expected");
    DBUG_ASSERT ((arg_info != NULL), ("arg_info is NULL"));
    DBUG_ASSERT ((arg_node == ASSIGN_INSTR (INFO_SYNCO_THISASSIGN (arg_info))),
                 "arg_node differs from thisasssign");

    result = arg_node;
    /*
     *  if this SYNC-block is followed directly (!) by another SYNC-block
     *  and both blocks are meltable, i.e. their masks are disjunctive,
     *  blocks are melted together here.
     */
    DBUG_PRINT ("SYNCO", ("try melting sync-blocks"));
    if (SYNC_FIRST (arg_node)) {
        DBUG_PRINT ("SYNCO", ("cannot melt first sync-block *<:("));
    }
    while (
      (!SYNC_FIRST (arg_node)) && (INFO_SYNCO_NEXTASSIGN (arg_info) != NULL)
      && (NODE_TYPE (ASSIGN_INSTR (INFO_SYNCO_NEXTASSIGN (arg_info))) == N_sync)
      && (MeltableSYNCs (arg_node, ASSIGN_INSTR (INFO_SYNCO_NEXTASSIGN (arg_info))))) {
        DBUG_PRINT ("SYNCO", ("melting sync-blocks"));
        /*
         *  The actual optimazation of SYNC-blocks takes place here.
         */
        result = MeltSYNCs ( arg_node,
                         ASSIGN_INSTR( INFO_SYNCO_NEXTASSIGN( arg_info)) /* ,
                         ... ####  */ );

        DBUG_PRINT ("SYNCO", ("rearranging assigments around sync-blocks"));
        /*
         *  Rearrange the pointers between the two assigments around the assignment
         *  who's spmd-block (ASSIGN_INSTR) has been deleted by melting.
         *  Cut of connections between assignment and the block and delete it.
         */
        ASSIGN_NEXT (INFO_SYNCO_THISASSIGN (arg_info))
          = ASSIGN_NEXT (INFO_SYNCO_NEXTASSIGN (arg_info));
        ASSIGN_NEXT (INFO_SYNCO_NEXTASSIGN (arg_info)) = NULL;
        ASSIGN_INSTR (INFO_SYNCO_NEXTASSIGN (arg_info)) = NULL;

        FreeTree (INFO_SYNCO_NEXTASSIGN (arg_info));

        INFO_SYNCO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (INFO_SYNCO_THISASSIGN (arg_info));
    } /* while */

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SYNCOassign (node* arg_node, node* arg_info)
 *
 * description:
 *   Traversal of N_assign during SYNCO.
 *
 *   Sets some values at arg_info. If no arg_info exists one is created.
 *   The values set are THISASSIGN and NEXTASSIGN, they have to be corrected
 *   be outer routines if the configuration of assignments is changed.
 *   Especially when N_spmds (under a N_assign-node) are deleted (and also
 *   the N_assign-node).
 *
 ******************************************************************************/
node *
SYNCOassign (node *arg_node, node *arg_info)
{
    int own_arg_info;
    node *old_thisassign;
    node *old_nextassign;

    DBUG_ENTER ("SYNCOassign");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "Wrong NODE_TYPE");

    /*
     *  check if there is an arg_info, else create one, and set variable
     *  to remember the arg_info has to be destroyed later.
     */
    if (arg_info == NULL) {
        own_arg_info = TRUE;
        arg_info = MakeInfo ();
    } else {
        own_arg_info = FALSE;
        old_thisassign = INFO_SYNCO_THISASSIGN (arg_info);
        old_nextassign = INFO_SYNCO_NEXTASSIGN (arg_info);
    }

    INFO_SYNCO_THISASSIGN (arg_info) = arg_node;
    INFO_SYNCO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (arg_node);

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    /*
     *  NEXT might have changed during traversal of instruction!
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    if (own_arg_info) {
        FreeTree (arg_info);
    } else {
        INFO_SYNCO_THISASSIGN (arg_info) = old_thisassign;
        INFO_SYNCO_NEXTASSIGN (arg_info) = old_nextassign;
    }

    DBUG_RETURN (arg_node);
}
