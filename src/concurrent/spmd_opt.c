/*
 *
 * $Log$
 * Revision 2.8  1999/07/28 13:06:57  jhs
 * CountOccurences gets fundef now.
 *
 * Revision 2.7  1999/07/20 16:58:52  jhs
 * Added comments.
 *
 * Revision 2.6  1999/07/07 15:53:51  jhs
 * Added comments, code brushed and assertions ordered correctly.
 *
 * Revision 2.5  1999/07/02 09:57:39  jhs
 * Removed some memory leaks when combinig masks in MeltSpmds.
 *
 * Revision 2.4  1999/07/01 13:04:24  jhs
 * Added comments.
 *
 * Revision 2.3  1999/06/25 15:36:33  jhs
 * Checked these in just to provide compileabilty.
 *
 * Revision 2.2  1999/05/28 15:31:45  jhs
 * Implemented first steps of spmd-optimisation.
 *
 * Revision 2.1  1999/02/23 12:44:16  sacbase
 * new release made
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_opt.c
 *
 * prefix: SPMDO
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   optimize spmd-blocks, i.e. adjacent compatible spmd-blocks are merged
 *   to single ones.
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

/******************************************************************************
 *
 * function:
 *   void AssertSimpleBlock (node *block)
 *
 * description:
 *   Asserts whether the block is an N_block and has only the attribute
 *   BLOCK_INSTR set. This check is needed at various places to check wether
 *   the block was newly introduced, and has no extra information that cannot
 *   be handled by the calling routines.
 *
 ******************************************************************************/
void
AssertSimpleBlock (node *block)
{
    DBUG_ENTER ("AssertBlock");

    DBUG_ASSERT (NODE_TYPE (block) == N_block, "Wrong NODE_TYPE, not a N_block");

    DBUG_ASSERT (BLOCK_VARDEC (block) == NULL, "BLOCK_VARDEC not NULL");
    DBUG_ASSERT (BLOCK_NEEDFUNS (block) == NULL, "BLOCK_NEEDFUNS not NULL");
    DBUG_ASSERT (BLOCK_NEEDTYPES (block) == NULL, "BLOCK_NEEDTYPES not NULL");
    DBUG_ASSERT (BLOCK_SPMD_PROLOG_ICMS (block) == NULL, "BLOCK_SPMD_... not NULL");
    DBUG_ASSERT (BLOCK_CACHESIM (block) == NULL, "BLOCK_CACHESIM not NULL");
    DBUG_ASSERT (BLOCK_VARNO (block) == 0, "BLOCK_VARNO not 0");

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *BlocksLastInstruction(node *block)
 *
 * description:
 *   Find last instruction of a block
 *
 * attention:
 *   One can only step over N_assign until now, N_empty is not handled yet.
 *
 ******************************************************************************/
node *
BlocksLastInstruction (node *block)
{
    node *result;

    DBUG_ENTER ("FindBlocksLastInstruction");

    DBUG_ASSERT (NODE_TYPE (block) == N_block, "Wrong NODE_TYPE of argument");

    result = BLOCK_INSTR (block);
    DBUG_ASSERT (NODE_TYPE (result) == N_assign, "Wrong node for instruction");

    while (ASSIGN_NEXT (result) != NULL) {
        result = ASSIGN_NEXT (result);
        DBUG_ASSERT (NODE_TYPE (result) == N_assign,
                     "Wrong node for further instruction");
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 * #### used by sync_opt also, move to more global place
 * functions:
 *   (1) node *MeltBlocks (node *first_block, node *second_block)
 *   (2) node *MeltBlocksOnCopies (node *first_block, node *second_block)
 *
 * description:
 *   Melts to N_blocks to one.
 *
 *   (1) normal version:
 *   If the normal version is used the first entry is reused, and the second
 *   one is given free. That means both arguments are modified!
 *
 *   (2) copying version:
 *   Both arguments (and also the two complete trees!) are copied here, and
 *   no harm is done to them, one gets a complety new node as result.
 *
 ******************************************************************************/
node *
MeltBlocks (node *first_block, node *second_block)
{
    node *result;
    node *lassign;

    DBUG_ENTER ("MeltBlocks");

    AssertSimpleBlock (first_block);
    AssertSimpleBlock (second_block);

    lassign = BlocksLastInstruction (first_block);
    ASSIGN_NEXT (lassign) = BLOCK_INSTR (second_block);
    /* cut old connection */
    BLOCK_INSTR (second_block) = NULL;

    FreeTree (second_block);
    result = first_block;

    DBUG_RETURN (result);
}

/******************************************************************************
 *  see comment above
 ******************************************************************************/
node *
MeltBlocksOnCopies (node *first_block, node *second_block)
{
    node *result;

    DBUG_ENTER ("MeltBlocksOnCopies");

    first_block = DupTree (first_block, NULL);
    second_block = DupTree (second_block, NULL);

    result = MeltBlocks (first_block, second_block);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *MeltSPMDs (node *first_spmd, node *second_spmd, node* fundef)
 *
 * description:
 *   This function takes two N_spmd's and melts them to a new one.
 *
 * attention:
 *   There are some sideeffects!
 *   first_spmd will be used to build the result, while second_spmd will be
 *   freed. Use MeltSPMDsOnCopies (see below) to avoid this. That function
 *   first copies both arguments, so no sideeffects occur to the outside.
 *
 * remark:
 *   - A version without sideeffects is also available
 *     (see MeltSPMDsOnCopies below).
 *
 ******************************************************************************/
node *
MeltSPMDs (node *first_spmd, node *second_spmd, node *fundef)
{
    int i;
    node *result;    /* result value of this function            */
    DFMmask_t newin; /* masks to combine old masks               */
    DFMmask_t newout;
    DFMmask_t newshared;
    int *counters; /* mask to save counted occurences          */
    node *vardec;  /* only needed to traverse in debug-section */

    DBUG_ENTER ("MeltSPMDs");

    DBUG_ASSERT (NODE_TYPE (first_spmd) == N_spmd, "First argument not a N_spmd!");
    DBUG_ASSERT (NODE_TYPE (second_spmd) == N_spmd, "Second argument not a N_smpd!");
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "fundef argument not a N_fundef!");

    /* both static should be the same, otherwise they cannot follow each other on
     * the same level.
     */
    DBUG_ASSERT (SPMD_STATIC (first_spmd) == SPMD_STATIC (second_spmd),
                 "SPMD_STATIC differs in SPMDs to be melted");

    /*
     *  Count which variables are consumned how often in second block and ...
     */
    DBUG_PRINT ("SPMD", ("Entering CountOccurences"));
    counters = CountOccurences (SPMD_REGION (second_spmd), SPMD_OUT (first_spmd), fundef);
    DBUG_PRINT ("SPMD", ("Leaving CountOccurences"));

    /*
     *  this is only to debug, it prints out all variables (each with a varno
     *  and the corresponding value of counter[varno].
     */
    vardec = BLOCK_VARDEC (FUNDEF_BODY (fundef));
    for (i = 0; i < 20; i++) {
        if (vardec != NULL) {
            DBUG_PRINT ("SPMD", ("%i %i %s(varno=%i)", i, counters[VARDEC_VARNO (vardec)],
                                 VARDEC_NAME (vardec), VARDEC_VARNO (vardec)));
            vardec = VARDEC_NEXT (vardec);
        } else {
            /* rest will be NULL, so we jump out of here */
            DBUG_PRINT ("SPMD", ("Rest NULL!!!"));
            break;
        }
    }

    /*
     *  ... reduce all this occurences in the first block.
     */
    DBUG_PRINT ("SPMD", ("Entering ReduceOccurences"));
    ReduceOccurences (SPMD_REGION (first_spmd), counters, SPMD_OUT (first_spmd));
    DestroyCM (counters);
    DBUG_PRINT ("SPMD", ("Leaving ReduceOccurences"));

    /* build one combined region from the two original regions */
    SPMD_REGION (first_spmd)
      = MeltBlocks (SPMD_REGION (first_spmd), SPMD_REGION (second_spmd));
    SPMD_REGION (second_spmd) = NULL;

    /* melt the masks of used variables */

    /*  -> IN <- */
    /*  newin = first's ins and second's ins,
     *          but without second's inouts and without first's out
     */
    newin = DFMGenMaskCopy (SPMD_IN (second_spmd));
    DFMSetMaskMinus (newin, SPMD_INOUT (second_spmd));
    DFMSetMaskMinus (newin, SPMD_OUT (first_spmd));
    DFMSetMaskOr (newin, SPMD_IN (first_spmd));

    /* -> OUT <- */
    newout = DFMGenMaskCopy (SPMD_OUT (first_spmd));
    DFMSetMaskOr (newout, SPMD_OUT (second_spmd));
    /*
     *  cut out the ones produced in first but not used beyond second.
     *  (here one runs over both blocks now melted, that makes no difference).
     */
    DBUG_PRINT ("SPMD", ("Entering ReduceMasks"));
    newout = ReduceMasks (SPMD_REGION (first_spmd), newout);
    DBUG_PRINT ("SPMD", ("Leaving ReduceMasks"));

    /* -> SHARED <- */
    /* combination of - (first's outs and second's ins)
     *                - inouts, to share pointer to memory
     *                - already shared's
     */
    newshared = DFMGenMaskAnd (SPMD_OUT (first_spmd), SPMD_IN (second_spmd));
    DFMSetMaskOr (newshared, SPMD_INOUT (second_spmd));
    DFMSetMaskOr (newshared, SPMD_SHARED (first_spmd));
    DFMSetMaskOr (newshared, SPMD_SHARED (second_spmd));

    /* -> IN, OUT, SHARED <- */
    /*
     *  First one destroys the old masks of the first block, and then sets
     *  them to new values.
     */
    SPMD_IN (first_spmd) = DFMRemoveMask (SPMD_IN (first_spmd));
    SPMD_OUT (first_spmd) = DFMRemoveMask (SPMD_OUT (first_spmd));
    SPMD_SHARED (first_spmd) = DFMRemoveMask (SPMD_SHARED (first_spmd));
    SPMD_IN (first_spmd) = newin;
    SPMD_OUT (first_spmd) = newout;
    SPMD_SHARED (first_spmd) = newshared;

    /* -> LOCAL <- */
    /* combination of locals of both blocks */
    DFMSetMaskOr (SPMD_LOCAL (first_spmd), SPMD_LOCAL (second_spmd));

    /* -> INOUT <- */
    /* nothing to be done, only values of first needed */

    FreeTree (second_spmd);

    result = first_spmd;
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node MeltSPMDsOnCopies (node *first_spmd, node *second_spmd)
 *
 * description:
 *   Copies both arguments an returns a new melted SPMD. Both arguments are
 *   not touched during this process (compare MeltSPMDS above), but both
 *   arguments (and alos the complete tree below!) are copied. The return
 *   value is a completly new node.
 *
 * remark:
 *   A simple version without any copying is also available
 *   (see MeltSPMDs above).
 *
 ******************************************************************************/
node *
MeltSPMDsOnCopies (node *first_spmd, node *second_spmd)
{
    node *result;

    DBUG_ENTER ("MeltSPMDsOnCopies");

    first_spmd = DupTree (first_spmd, NULL);
    second_spmd = DupTree (second_spmd, NULL);

    result = MeltSPMDs (first_spmd, second_spmd, NULL);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDOspmd (node* arg_node, node* arg_info)
 *
 * description:
 *   The traversal of N_spmd during SPMDO.
 *
 *   Melts the N_spmd with all *directly* following N_spmds to one remaining
 *   block. The process will be done by checking if the actual N_spmd is
 *   followed *directly* be another N_spmd. These will be melted together and
 *   the resulting N_spmd becomes the new actual N_spmd. The Process lasts
 *   until there is no *directly* following N_spmd to the actual anymore.
 *
 * ATTENTION:
 *   The THISASSIGN and NEXTASSIGN values at the arg_info have to be updated.
 *
 ******************************************************************************/
node *
SPMDOspmd (node *arg_node, node *arg_info)
{
    node *result;

    DBUG_ENTER ("SPMDOspmd");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_spmd), "Wrong NODE_TYPE: N_sync expected");
    DBUG_ASSERT ((arg_info != NULL), ("arg_info is NULL"));
    DBUG_ASSERT ((arg_node == ASSIGN_INSTR (INFO_SPMDO_THISASSIGN (arg_info))),
                 "arg_node differs from thisasssign");

    result = arg_node;
    /*
     *  if this SPMD-Block is followed directly (!) by another SPMD-Block both
     *  blocks are melted here.
     */
    while ((INFO_SPMDO_NEXTASSIGN (arg_info) != NULL)
           && (NODE_TYPE (ASSIGN_INSTR (INFO_SPMDO_NEXTASSIGN (arg_info))) == N_spmd)) {
        DBUG_PRINT ("SPMDO", ("melting spmd-blocks"));
        /*
         *  The actual optimization of SPMD-blocks takes place here.
         */
        result = MeltSPMDs (arg_node, ASSIGN_INSTR (INFO_SPMDO_NEXTASSIGN (arg_info)),
                            INFO_SPMDT_ACTUAL_FUNDEF (arg_info));

        /*
         *  Rearrange the pointers between the two assigments around the assignment
         *  who's spmd-block (ASSIGN_INSTR) has been deleted by melting.
         *  Cut of connections between assignment and the block and delete it.
         */
        ASSIGN_NEXT (INFO_SPMDO_THISASSIGN (arg_info))
          = ASSIGN_NEXT (INFO_SPMDO_NEXTASSIGN (arg_info));
        ASSIGN_NEXT (INFO_SPMDO_NEXTASSIGN (arg_info)) = NULL;
        ASSIGN_INSTR (INFO_SPMDO_NEXTASSIGN (arg_info)) = NULL;

        FreeTree (INFO_SPMDO_NEXTASSIGN (arg_info));

        INFO_SPMDO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (INFO_SPMDO_THISASSIGN (arg_info));
    } /* while */

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDOassign (node* arg_node, node* arg_info)
 *
 * description:
 *   Traversal of N_assign during SPMDO.
 *
 *   Sets some values at arg_info. If no arg_info exists one is created.
 *   The values set are THISASSIGN and NEXTASSIGN, they have to be corrected
 *   be outer routines if the configuration of assignments is changed.
 *   Especially when N_spmds (under a N_assign-node) are deleted (and also
 *   the N_assign-node).
 *
 ******************************************************************************/
node *
SPMDOassign (node *arg_node, node *arg_info)
{
    int own_arg_info;
    node *old_thisassign;
    node *old_nextassign;

    DBUG_ENTER ("SPMDOassign");

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
        old_thisassign = INFO_SPMDO_THISASSIGN (arg_info);
        old_nextassign = INFO_SPMDO_NEXTASSIGN (arg_info);
    }

    INFO_SPMDO_THISASSIGN (arg_info) = arg_node;
    INFO_SPMDO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (arg_node);

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
        INFO_SPMDO_THISASSIGN (arg_info) = old_thisassign;
        INFO_SPMDO_NEXTASSIGN (arg_info) = old_nextassign;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDOfundef (node* arg_node, node* arg_info)
 *
 * description:
 *   Ensures that the actual FUNDEF or the actual function in which the
 *   SPMD-Block are located is known.
 *   The rountine automatically creates an info-node if non exists until
 *   reaching this function.
 *
 ******************************************************************************/
node *
SPMDOfundef (node *arg_node, node *arg_info)
{
    int own_arg_info;
    node *old_fundef;

    DBUG_ENTER ("SPMDOfundef");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef), "Wrong NODE_TYPE");

    /*
     *  check if there is an arg_info, else create one, and set variable
     *  to remember the arg_info has to be destroyed later.
     */
    if (arg_info == NULL) {
        own_arg_info = TRUE;
        arg_info = MakeInfo ();
    } else {
        own_arg_info = FALSE;
        old_fundef = INFO_SPMDT_ACTUAL_FUNDEF (arg_info);
    }

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    if (own_arg_info) {
        FreeTree (arg_info);
    } else {
        INFO_SPMDT_ACTUAL_FUNDEF (arg_info) = old_fundef;
    }

    DBUG_RETURN (arg_node);
}
