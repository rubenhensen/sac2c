/*
 *
 * $Log$
 * Revision 1.5  1999/09/01 17:14:23  jhs
 * Remove SYNC_SCHEDULING.
 *
 * Revision 1.4  1999/08/27 11:55:37  jhs
 * Adjusted comments.
 *
 * Revision 1.3  1999/08/02 09:48:35  jhs
 * Moved MeltBlocks[OnCopies] from spmd_opt.[ch] to concurrent_lib.[ch].
 *
 * Revision 1.2  1999/07/30 13:45:44  jhs
 * Added initial macros and functions.
 *
 * Revision 1.1  1999/07/30 12:34:50  jhs
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   concurrent_lib.c
 *
 * prefix: CONL
 *
 * description:
 *   helper functions for concurrent-compilation
 *
 *****************************************************************************/

#include "dbug.h"

#include "tree_basic.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"

/******************************************************************************
 *
 * function:
 *   void CONLDisplayMask (char *tag, char *name, DFMmask_t mask)
 *
 * description:
 *   Prints out the names of variables contained in mask.
 *   The printout is coupled to DBUG_PRINT with tag, so prints happen only
 *   with this tag given for debugging.
 *   name is used for specify print out.
 ******************************************************************************/
void
CONLDisplayMask (char *tag, char *name, DFMmask_t mask)
{
    char *varname;

    DBUG_ENTER ("CONLDisplayMask");

    DBUG_PRINT (tag, ("%s-mask begin", name));
    varname = DFMGetMaskEntryNameSet (mask);
    while (varname != NULL) {
        DBUG_PRINT (tag, ("%s", varname));
        varname = DFMGetMaskEntryNameSet (NULL);
    }
    DBUG_PRINT (tag, ("%s-mask end", name));

    DBUG_VOID_RETURN;
}

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

    DBUG_ENTER ("BlocksLastInstruction");

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
    node *arg_info;

    DBUG_ENTER ("MeltBlocksOnCopies");

    arg_info = MakeInfo ();
    INFO_DUP_TYPE (arg_info) = DUP_NORMAL;
    INFO_DUP_ALL (arg_info) = TRUE;
    first_block = DupTree (first_block, NULL);
    arg_info = FreeTree (arg_info);
    arg_info = MakeInfo ();
    INFO_DUP_TYPE (arg_info) = DUP_NORMAL;
    INFO_DUP_ALL (arg_info) = TRUE;
    second_block = DupTree (second_block, NULL);
    arg_info = FreeTree (arg_info);

    result = MeltBlocks (first_block, second_block);

    DBUG_RETURN (result);
}
