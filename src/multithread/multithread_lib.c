/*
 *
 * $Log$
 * Revision 1.8  2000/07/13 08:24:24  jhs
 * Moved DupMask_ InsertBlock, InsertMT and InsertST from blocks_init.[ch]
 * Renamed InsertXX to MUTHInsertXX.
 *
 * Revision 1.7  2000/06/23 15:13:47  dkr
 * signature of DupTree changed
 *
 * Revision 1.6  2000/04/14 17:43:26  jhs
 * Comments ...
 *
 * Revision 1.5  2000/04/10 15:45:08  jhs
 * Added Reduce
 *
 * Revision 1.4  2000/03/09 18:34:40  jhs
 * Additional features.
 *
 * Revision 1.3  2000/03/02 12:59:35  jhs
 * Added MUTHExchangeApplication,
 * added MUTHExpandFundefName,
 * added DBUG_PRINTS and DBUG_ASSERTS.
 *
 * Revision 1.2  2000/02/22 15:48:50  jhs
 * Adapted NODE_TEXT.
 *
 * Revision 1.1  2000/02/21 17:48:22  jhs
 * Initial revision
 *
 */

/*****************************************************************************
 *
 * file:   multithread_lib.c
 *
 * prefix:
 *
 * description:
 *   helper functions for multithread-compilation
 *
 *****************************************************************************/

#include "dbug.h"

#include <string.h>

#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "generatemasks.h"

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   static void MUTHAssertSimpleBlock (node *block)
 *
 * description:
 *   Asserts whether the block is an N_block and has only the attribute
 *   BLOCK_INSTR set. This check is needed at various places to check wether
 *   the block was newly introduced, and has no extra information that cannot
 *   be handled by the calling routines.
 *
 ******************************************************************************/
static void
MUTHAssertSimpleBlock (node *block)
{
    DBUG_ENTER ("MUTHAssertBlock");

    DBUG_PRINT ("MUTH", ("%s", NODE_TEXT (block)));
    DBUG_ASSERT (NODE_TYPE (block) == N_block,
                 "Wrong NODE_TYPE, not a N_block (watch MUTH)");

    /* what is this for??? copied from the old spmd version ??? */
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
 *   node *MUTHBlocksLastInstruction(node *block)
 *
 * description:
 *   Find the last instruction (N_assign) of a block.
 *
 * attention:
 *   One can only step over N_assign until now, N_empty is not handled yet.
 *
 ******************************************************************************/
node *
MUTHBlocksLastInstruction (node *block)
{
    node *result;

    DBUG_ENTER ("MUTHBlocksLastInstruction");
    DBUG_PRINT ("MUTH", ("begin"));

    DBUG_ASSERT ((block != NULL), ("block == NULL"));
    DBUG_ASSERT (NODE_TYPE (block) == N_block, ("Wrong NODE_TYPE of argument"));

    result = BLOCK_INSTR (block);

    DBUG_ASSERT ((result != NULL), ("result == NULL"));
    DBUG_ASSERT (NODE_TYPE (result) == N_assign, "Wrong node for instruction");

    while (ASSIGN_NEXT (result) != NULL) {
        result = ASSIGN_NEXT (result);
        DBUG_ASSERT (NODE_TYPE (result) == N_assign,
                     "Wrong node for further instruction");
    }

    DBUG_PRINT ("MUTH", ("end"));
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * functions:
 *   (1) node *MUTHMeltBlocks (node *first_block, node *second_block)
 *   (2) node *MUTHMeltBlocksOnCopies (node *first_block, node *second_block)
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
MUTHMeltBlocks (node *first_block, node *second_block)
{
    node *result;
    node *lassign;

    DBUG_ENTER ("MeltBlocks");

    MUTHAssertSimpleBlock (first_block);
    MUTHAssertSimpleBlock (second_block);

    lassign = MUTHBlocksLastInstruction (first_block);
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
MUTHMeltBlocksOnCopies (node *first_block, node *second_block)
{
    node *result;
    node *arg_info;

    DBUG_ENTER ("MUTHMeltBlocksOnCopies");

    arg_info = MakeInfo ();
    INFO_DUP_TYPE (arg_info) = DUP_NORMAL;
    INFO_DUP_ALL (arg_info) = TRUE;
    first_block = DupTree (first_block);
    arg_info = FreeTree (arg_info);
    arg_info = MakeInfo ();
    INFO_DUP_TYPE (arg_info) = DUP_NORMAL;
    INFO_DUP_ALL (arg_info) = TRUE;
    second_block = DupTree (second_block);
    arg_info = FreeTree (arg_info);

    result = MUTHMeltBlocks (first_block, second_block);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node* MUTHExchangeApplication (node *arg_node, node *new_fundef)
 *
 * description:
 *   The fundef belonging to a N_ap cannot be changed directly, because
 *   some other features have to bechanged as well.
 *   This function realizes all what has to be done.
 *
 ******************************************************************************/
node *
MUTHExchangeApplication (node *arg_node, node *new_fundef)
{
    DBUG_ENTER ("ExchangeApplication");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_ap), "wrong type of node");

    DBUG_PRINT ("MUTH", ("begin"));

    DBUG_PRINT ("MUTH", ("new_fundef = %s", FUNDEF_NAME (new_fundef)));

    AP_FUNDEF (arg_node) = new_fundef;

    if (AP_NAME (arg_node) != NULL) {
        FREE (AP_NAME (arg_node));
    }
    AP_NAME (arg_node) = StringCopy (FUNDEF_NAME (new_fundef));
    /*  DBUG_ASSERT((AP_MOD( arg_node) != NULL), ("null1!!")); */
    if (AP_MOD (arg_node) != NULL) {
        /*    FREE( AP_MOD( arg_node)); */
    }

    /*  DBUG_ASSERT((FUNDEF_MOD( new_fundef) != NULL), ("null2!!"));  */
    if (FUNDEF_MOD (new_fundef) != NULL) {
        AP_MOD (arg_node) = StringCopy (FUNDEF_MOD (new_fundef));
    } else {
        AP_MOD (arg_node) = NULL;
    }
    DBUG_PRINT ("MUTH", ("end"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MUTHExpandFundefName (node *fundef, char *prefix)
 *
 * description:
 *   Changes the name of the fundef. The prefix and original will will be
 *   concatenated to a new string n and set n is set as FUNDEF_NAME.
 *   The original name will be set free automatically.
 *
 * attention:
 *   THIS DOES NOT CHANGE THE NAMES IN N_AP.
 *   N_ap holds a copy of the name (I don't know why, but this is the fact),
 *   so you have to change *all* the names in corresponding N_ap.
 *
 ******************************************************************************/
node *
MUTHExpandFundefName (node *fundef, char *prefix)
{
    char *old_name;
    char *new_name;

    DBUG_ENTER ("MUTHExpandFundefName");

    old_name = FUNDEF_NAME (fundef);
    new_name = Malloc (strlen (old_name) + strlen (prefix) + 1);
    strcpy (new_name, prefix);
    strcat (new_name, old_name);
    FUNDEF_NAME (fundef) = new_name;
    FREE (old_name);

    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   node *MUTHReduceFundefName( node *fundef, int count)
 *
 * description:
 *   deletes the first "count" chars of the fundef name
 *
 ******************************************************************************/
node *
MUTHReduceFundefName (node *fundef, int count)
{
    int i;

    DBUG_ENTER ("MUTHReduceFundefName");
    DBUG_PRINT ("BARIN", ("begin %i %i", strlen (FUNDEF_NAME (fundef)), count));

    for (i = 0; (i < ((int)strlen (FUNDEF_NAME (fundef)) - count + 1)); i++) {
        DBUG_PRINT ("BARIN", ("%i %i", i, (strlen (FUNDEF_NAME (fundef)) - count + 1)));
        FUNDEF_NAME (fundef)[i] = FUNDEF_NAME (fundef)[i + count];
    }
    /*
        FUNDEF_NAME( fundef)[0] = FUNDEF_NAME( fundef)[0 + count];
    */
    DBUG_PRINT ("BARIN", ("end"));
    DBUG_RETURN (fundef);
}

/******************************************************************************
 *
 * function:
 *   long *DupMask_(long *oldmask, int varno)
 *
 * description:
 *   copies Mask via DupMask, but is able to handle NULL also (returns NULL
 *   when it has to copy a NULL).
 *
 ******************************************************************************/
static long *
DupMask_ (long *oldmask, int varno)
{
    long *result;

    DBUG_ENTER ("DupMask_");

    if (oldmask == NULL) {
        result = NULL;
    } else {
        result = DupMask (oldmask, varno);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   static node *MUTHInsertBlock(node *assign, node *block, node *fundef)
 *
 * description:
 *   Inserts an MT or ST-Block into assign, block contains a MT- or ST-Block
 *   with REGION == NULL, fundef is the actual fundef node.
 *   assign gets the block as new instruction, while the region of the block
 *   will get the old instruction of the assignment.
 *   So the instruction at the assign will be embedded into block.
 *
 ******************************************************************************/
static node *
MUTHInsertBlock (node *assign, node *block, node *fundef)
{
    node *newassign;
    int varno;

    DBUG_ENTER ("MUTHInsertBlock");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), ("assign: N_assign expected"));
    DBUG_ASSERT (((NODE_TYPE (block) == N_mt) || (NODE_TYPE (block) == N_st)),
                 ("block: N_mt or N_st expected"));
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), ("fundef: N_fundef expected"));

    newassign = MakeAssign (ASSIGN_INSTR (assign), NULL);
    L_MT_OR_ST_REGION (block, MakeBlock (newassign, NULL));
    ASSIGN_INSTR (assign) = block;
    varno = FUNDEF_VARNO (fundef);
    ASSIGN_DEFMASK (newassign) = DupMask_ (ASSIGN_DEFMASK (assign), varno);
    ASSIGN_USEMASK (newassign) = DupMask_ (ASSIGN_USEMASK (assign), varno);
    ASSIGN_MRDMASK (newassign) = DupMask_ (ASSIGN_MRDMASK (assign), varno);

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * function:
 *   static node *MUTHInsertMT(node *assign, node *arg_info)
 *
 * description:
 *   inserts mt-block around the instruction of the assignment
 *
 ******************************************************************************/
node *
MUTHInsertMT (node *assign, node *arg_info)
{
    DBUG_ENTER ("MUTHInsertMT");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "assign-node is not a N_assign");

    assign = MUTHInsertBlock (assign, MakeMT (NULL), INFO_MUTH_FUNDEF (arg_info));

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * function:
 *   static node *MUTHInsertST(node *assign, node *arg_info)
 *
 * description:
 *   inserts st-block around the instruction of the assignment
 *
 ******************************************************************************/
node *
MUTHInsertST (node *assign, node *arg_info)
{
    DBUG_ENTER ("MUTHInsertST");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "assign-node is not a N_assign");

    assign = MUTHInsertBlock (assign, MakeST (NULL), INFO_MUTH_FUNDEF (arg_info));

    DBUG_RETURN (assign);
}
