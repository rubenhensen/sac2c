/*
 *
 * $Log$
 * Revision 1.6  2000/02/11 16:21:01  jhs
 * Expanded traversals ...
 *
 * Revision 1.5  2000/02/04 14:43:55  jhs
 * Improved MustExecuteSingleThreaded.
 *
 * Revision 1.4  2000/02/03 15:17:31  jhs
 * First step of MustExecuteSingleThreaded implemented.
 *
 * Revision 1.3  2000/02/02 17:22:47  jhs
 * BlocksInit improved.
 *
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
#include "globals.h"

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
    funtab *old_tab;

    DBUG_ENTER ("BlocksInit");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_fundef),
                 "ScheduleInit expects a N_fundef as arg_node");

    old_tab = act_tab;
    act_tab = blkin_tab;

    /* push info ... */

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    /* pop info ... */

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   static int CheckLHSforBigArrays(node* let, int max_small_size)
 *
 * description:
 *   test whether any result on the lhs is an array, with more elements
 *   as max_small_size.
 *
 ******************************************************************************/
static int
CheckLHSforBigArrays (node *let, int max_small_size)
{
    int i;
    int result;
    int sum;
    ids *letids;
    node *vardec;
    types *type;

    DBUG_ENTER ("CheckLHSforBigArrays");

    result = FALSE;
    /* run through ids-chain */
    letids = LET_IDS (let);
    while (letids != NULL) {
        vardec = IDS_VARDEC (letids);
        type = VARDEC_TYPE (vardec);

        if ((KNOWN_SHAPE (type)) && (TYPES_DIM (type) > 0)) {
            sum = 1;
            for (i = 0; i < TYPES_DIM (type); i++) {
                sum = sum * TYPES_SHAPE (type, i);
            }
            result = (result || (sum > max_small_size));
        } else {
            /* nothing happens #### error? warning?  */
        }
        letids = IDS_NEXT (letids);
    }
    DBUG_RETURN (result);
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
    int result;
    node *instr;

    DBUG_ENTER ("MustExecuteSingleThreeaded");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is not a N_assign");

    instr = ASSIGN_INSTR (arg_node);

    if (NODE_TYPE (instr) == N_let) {
        if ((NODE_TYPE (LET_EXPR (instr)) == N_ap)
            && (FUNDEF_BODY (AP_FUNDEF (LET_EXPR (instr))) == NULL)) {
            /*
             *  it is a function with unknown body,
             *  is any array-result > threshold?
             */
            result = CheckLHSforBigArrays (instr, max_replication_size);
        } else if (NODE_TYPE (LET_EXPR (instr)) == N_prf) {
            /*
             *  it is a primitive function with no body
             *  is any array-result > threshold?
             */
            result = CheckLHSforBigArrays (instr, max_replication_size);
        } else if (NODE_TYPE (LET_EXPR (instr)) == N_array) {
            /*
             *  it is a let, assigning a constant
             *  is any array-result > threshold?
             */
            result = CheckLHSforBigArrays (instr, max_replication_size);
        } else {
            result = FALSE;
        }
    } else if (NODE_TYPE (instr) == N_while) {
        /* ??? #### */
        result = FALSE;
    } else if (NODE_TYPE (instr) == N_do) {
        /* ??? #### */
        result = FALSE;
    } else if (NODE_TYPE (instr) == N_cond) {
        /* ??? #### */
        result = FALSE;
    } else if (NODE_TYPE (instr) == N_return) {
        /* ??? #### */
        result = FALSE;
    } else {
        DBUG_ASSERT (0, "unknown type of instr");
        result = FALSE;
    }

    DBUG_RETURN (result);
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
    int result;
    node *instr;

    DBUG_ENTER ("WillExecuteMultiThreaded");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), "arg_node is not a N_assign");

    instr = ASSIGN_INSTR (arg_node);

    if (NODE_TYPE (instr) == N_let) {
        if (NODE_TYPE (LET_EXPR (instr)) == N_Nwith2) {
            if (NWITH2_ISSCHEDULED (LET_EXPR (instr))) {
                NOTE (("hit wemt 1 t"));
                result = TRUE;
            } else {
                NOTE (("hit wemt 4 f"));
                result = FALSE;
            }
        } else {
            NOTE (("hit wemt 2 f"));
            result = FALSE;
        }
    } else {
        NOTE (("hit wemt 3 f"));
        result = FALSE;
    }

    DBUG_RETURN (result);
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

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
