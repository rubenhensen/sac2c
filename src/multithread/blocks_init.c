/*
 *
 * $Log$
 * Revision 1.2  2000/02/02 12:27:13  jhs
 * Added INFO_MUTH_FUNDEF, improved BLKIN.
 *
 * Revision 1.1  2000/01/28 13:21:35  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   blocks_init.c
 *
 * prefix: BLKIN
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

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *BlocksInit(node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
BlocksInit (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("BlocksInit");

    /* ... */

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static int MustExecuteSingleThreaded(node *arg_node, node *arg_info)
 *
 * description:
 *   arg_node has to be an N_assign
 *   Decides whether an assignment must be executed sequential or not.
 *
 ******************************************************************************/
static int
MustExecuteSingleThreaded (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MustExecuteSingleThreeaded");

    DBUG_RETURN (FALSE);
}

/******************************************************************************
 *
 * function:
 *   static int WillExecuteMultiThreaded(node *arg_node, node *arg_info)
 *
 * description:
 *   arg_node has to be an N_assign
 *   Decides whethter an assignment will definitly be executed multithreaded or
 *   not
 *
 ******************************************************************************/
static int
WillExecuteMultiThreaded (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WillExecuteMultiThreaded");

    DBUG_RETURN (FALSE);
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
 *   static node *InsertBlock(node *assign, node *block, node *fundef)
 *
 * description:
 *   Inserts an MT or ST-Block into assign, block contains a MT- or ST-Block
 *   with REGION == NULL, fundet is the actual fundef node.
 *   assign gets the block as new instruction, while the region of the block
 *   will get the old instruction of the assignment.
 *
 ******************************************************************************/
static node *
InsertBlock (node *assign, node *block, node *fundef)
{
    node *newassign;
    int varno;

    DBUG_ENTER ("InsertBlock");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), ("assign: N_assign expected"));
    DBUG_ASSERT (((NODE_TYPE (block) == N_mt) || (NODE_TYPE (block) == N_st)),
                 ("block: N_mt or N_st expected"));
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), ("fundef: N_fundef expected"));

    newassign = MakeAssign (ASSIGN_INSTR (assign), NULL);
    L_MT_OR_ST_REGION (block, MakeBlock (newassign, NULL));
    ASSIGN_INSTR (assign) = MT_OR_ST_REGION (block);
    varno = FUNDEF_VARNO (fundef);
    ASSIGN_DEFMASK (newassign) = DupMask_ (ASSIGN_DEFMASK (assign), varno);
    ASSIGN_USEMASK (newassign) = DupMask_ (ASSIGN_USEMASK (assign), varno);
    ASSIGN_MRDMASK (newassign) = DupMask_ (ASSIGN_MRDMASK (assign), varno);

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * function:
 *   static node *InsertMT(node *assign, node *arg_info)
 *
 * description:
 *   inserts mt-block
 *
 ******************************************************************************/
static node *
InsertMT (node *assign, node *arg_info)
{
    DBUG_ENTER ("InsertMT");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "assign-node is not a N_assign");

    assign = InsertBlock (assign, MakeMT (NULL), INFO_MUTH_FUNDEF (arg_info));

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * function:
 *   static node *InsertST(node *assign, node *arg_info)
 *
 * description:
 *   inserts st-block
 *
 ******************************************************************************/
static node *
InsertST (node *assign, node *arg_info)
{
    DBUG_ENTER ("InsertST");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), "assign-node is not a N_assign");

    assign = InsertBlock (assign, MakeST (NULL), INFO_MUTH_FUNDEF (arg_info));

    DBUG_RETURN (assign);
}

/******************************************************************************
 *
 * function:
 *   node *BLKINassign(node *arg_node, node *arg_info)
 *
 * description:
 *   Traversal, inserts mt and st-blocks if necessary
 *
 ******************************************************************************/
node *
BLKINassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("BLKINassign");

    if (MustExecuteSingleThreaded (arg_node, arg_info)) {
        /*
         *  Insert ST-Block
         */
        InsertST (arg_node, arg_info);
    } else if (WillExecuteMultiThreaded (arg_node, arg_info)) {
        /*
         *  Insert MT-Block
         */
        InsertMT (arg_node, arg_info);
    } else {
        /*
         *  can be executed sequential or multithreaded
         *  this decision is made later
         */
    }

    DBUG_RETURN (arg_node);
}
