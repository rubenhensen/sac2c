/*
 *
 * $Log$
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

/******************************************************************************
 *
 * function:
 *   void AssertSimpleBlock (node *block)
 *
 * description:
 *   Asserts whether the block is an N_block and has only the attribute
 *   BLOCK_INSTR set. This checkis needed at various places to check wheter
 *   the block was newly introduced, and hos no extra information that cannot
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
 *
 * function:
 *   node *MeltBlocks (node *first_block, node *second_block)
 *   node *MeltBlocksOnCopies (node *first_block, node *second_block)
 *
 * description:
 *   ...
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

/*
 *  see comment above
 */
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
 *   node *MeltSPMDs (node *first_spmd, node *second_spmd)
 *
 * description:
 *   This function takes two N_spmd's and melts them to a new one.
 *
 ******************************************************************************/
node *
MeltSPMDs (node *first_spmd, node *second_spmd)
{
    node *result; /* result value of this function */

    DBUG_ENTER ("MeltSPMDs");

    DBUG_ASSERT (NODE_TYPE (first_spmd) == N_spmd, "First argument not a N_spmd!");
    DBUG_ASSERT (NODE_TYPE (second_spmd) == N_spmd, "Second argument not a N_smpd!");

    /* both static should be the same, otherwise they cannot follow each other on
     * the same level.
     */
    DBUG_ASSERT (SPMD_STATIC (first_spmd) == SPMD_STATIC (second_spmd),
                 "SPMD_STATIC differs in SPMDs to be melted");

    /* build one combined region from the two original regions */
    SPMD_REGION (first_spmd)
      = MeltBlocks (SPMD_REGION (first_spmd), SPMD_REGION (second_spmd));
    SPMD_REGION (second_spmd) = NULL;

    /* melt the masks of used variables */
    /* --->>> not finished, simple version <<<---
      SPMD_IN(first_spmd)    = DFMGenMaskOr (SPMD_IN(first_spmd)   , SPMD_IN(second_spmd)
      ); SPMD_OUT(first_spmd)   = DFMGenMaskOr (SPMD_OUT(first_spmd)  ,
      SPMD_OUT(second_spmd)  ); SPMD_INOUT(first_spmd) = DFMGenMaskOr
      (SPMD_INOUT(first_spmd), SPMD_INOUT(second_spmd)); SPMD_LOCAL(first_spmd) =
      DFMGenMaskOr (SPMD_LOCAL(first_spmd), SPMD_LOCAL(second_spmd));
    */

    SPMD_IN (first_spmd) = DFMGenMaskOr (SPMD_IN (first_spmd), SPMD_IN (second_spmd));
    SPMD_OUT (first_spmd) = DFMGenMaskOr (SPMD_OUT (first_spmd), SPMD_OUT (second_spmd));
    DFMSetMaskOr (SPMD_OUT (first_spmd), SPMD_INOUT (second_spmd));
    /* SPMD_INOUT(first_spmd) = DFMGenMaskSet (SPMD_INOUT(first_spmd)); */
    SPMD_LOCAL (first_spmd)
      = DFMGenMaskOr (SPMD_LOCAL (first_spmd), SPMD_LOCAL (second_spmd));

    DFMSetMaskMinus (SPMD_IN (first_spmd), SPMD_INOUT (first_spmd));

    FreeTree (second_spmd);

    result = first_spmd;
    DBUG_RETURN (result);
}

/*
 *  see comment above
 */

node *
MeltSPMDsOnCopies (node *first_spmd, node *second_spmd)
{
    node *result;

    DBUG_ENTER ("MeltSPMDsOnCopies");

    first_spmd = DupTree (first_spmd, NULL);
    second_spmd = DupTree (second_spmd, NULL);

    result = MeltSPMDs (first_spmd, second_spmd);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *
 ******************************************************************************/

node *
SPMDOspmd (node *arg_node, node *arg_info)
{
    node *result;

    DBUG_ENTER ("SPMDOspmd");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_spmd), "Wrong NODE_TYPE");
    DBUG_ASSERT ((arg_node == ASSIGN_INSTR (INFO_SPMDO_THISASSIGN (arg_info))),
                 "arg_node differs from thisasssign");
    DBUG_ASSERT ((arg_info != NULL), "arg_info is NULL");

    result = arg_node;
    while ((INFO_SPMDO_NEXTASSIGN (arg_info) != NULL)
           && (NODE_TYPE (ASSIGN_INSTR (INFO_SPMDO_NEXTASSIGN (arg_info))) == N_spmd)) {
        /* optimze */

        result = MeltSPMDs (arg_node, ASSIGN_INSTR (INFO_SPMDO_NEXTASSIGN (arg_info)));

        ASSIGN_NEXT (INFO_SPMDO_THISASSIGN (arg_info))
          = ASSIGN_NEXT (INFO_SPMDO_NEXTASSIGN (arg_info));
        ASSIGN_NEXT (INFO_SPMDO_NEXTASSIGN (arg_info)) = NULL;
        ASSIGN_INSTR (INFO_SPMDO_NEXTASSIGN (arg_info)) = NULL;

        FreeTree (INFO_SPMDO_NEXTASSIGN (arg_info));

        INFO_SPMDO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (INFO_SPMDO_THISASSIGN (arg_info));
    } /* while */
      /* else {
          result = arg_node;
        }*/

    DBUG_RETURN (result);
}

node *
SPMDOassign (node *arg_node, node *arg_info)
{
    int own_arg_info;
    node *old_thisassign;
    node *old_nextassign;

    DBUG_ENTER ("SPMDOassign");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "Wrong NODE_TYPE");

    /*  check if there is an arg_info, else create one, and set variable
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
     *  NEXT might have changed during traversal of instrcution!
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
