/*
 *
 * $Log$
 * Revision 2.2  1999/06/25 15:36:33  jhs
 * Checked these in just to provide compileabilty.
 *
 * Revision 2.1  1999/06/15 14:15:23  jhs
 * Initial revision generated.
 *
 *
 */

/*****************************************************************************
 *
 * file:   spmd_trav.c
 *
 * prefix: SPMDT
 *
 *         SPMDTCO - CountOccurences
 *         SPMDTRO - ReduceOccurences
 *
 * description:
 *
 *  #### implements three traversals
 *      ro -> reduce ocuurences
 *      rm -> reduce masks
 *      co -> count occurences
 *
 *   This file implements the traversal of N_blocks to find the last usage of
 *   a variable in this block and fetch its reference counter.
 *
 *   This is needed to determine the combinations of two consecutive
 *   DataFlowMasks.
 *
 *****************************************************************************/

#include "dbug.h"

#include "DataFlowMask.h"
#include "traverse.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

#include "spmd_trav.h"

/******************************************************************************
 *
 * function
 *   void ReduceOccurences (mode * block, int * counters)
 *
 * description:
 *   modifies the naive refcounters (occurences) in block.
 *   the last set of a variable (v) will be decremented by the number specified
 *   by counter[varno] (where varno is the varno of v).
 *
 ******************************************************************************/
void
ReduceOccurences (node *block, int *counters, DFMmask_t mask)
{
    node *arg_info;
    funptr *old_tab;

    DBUG_ENTER ("ReduceOccurences");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = spmdrotrav_tab;

    INFO_SPMDT_CHECK (arg_info) = DFMGenMaskCopy (mask);
    INFO_SPMDT_COUNTERS (arg_info) = counters;

    block = Trav (block, arg_info);

    act_tab = old_tab;
    /* #### clear new masks */

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   DFMmask_t ReduceMasks ( node *block, DFMmask_t first_out)
 *
 * description:
 *   ???? to be pointed out
 *
 *   reduces the out mask to the variables really used behind two melted
 *   blocks
 *   all the out variables are set somewhere in the block, so there
 *   is at least one left handed occurence
 *
 ******************************************************************************/
DFMmask_t
ReduceMasks (node *block, DFMmask_t first_out)
{
    node *arg_info;
    funptr *old_tab;
    DFMmask_t result;

    DBUG_ENTER ("ReduceMasks");

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = spmdrmtrav_tab;

    INFO_SPMDT_FIRSTOUT (arg_info) = first_out;
    INFO_SPMDT_RESULT (arg_info) = DFMGenMaskCopy (first_out);
    INFO_SPMDT_CHECK (arg_info) = DFMGenMaskCopy (first_out);

    /*
     *  now check == result, but lateron result will be a superset of check.
     */
    block = Trav (block, arg_info);

    act_tab = old_tab;

    result = INFO_SPMDT_RESULT (arg_info);

    FreeTree (arg_info);
    /* #### all extra masks to be freed here */

    DBUG_RETURN (result);
}

/* CM == CounterMask */
/******************************************************************************
 *
 * function:
 *   int * CreateCM (int varno)
 *
 * description:
 *   reserves memory for a mask of counters.
 *   all values are initialized by 0.
 *
 * related:
 *   DestroyCM
 *
 ******************************************************************************/
int *
CreateCM (int varno)
{
    int i;
    int *result;

    DBUG_ENTER ("CreateCM");

    result = (int *)Malloc (sizeof (int) * varno);
    for (i = 0; i < varno; i++) {
        result[i] = 0;
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   void DestroyCM (int * mask)
 *
 * description:
 *   releases memory used for a mask of counters.
 *
 * related:
 *   CreateCM
 *
 ******************************************************************************/
void
DestroyCM (int *mask)
{
    DBUG_ENTER ("DestroyCM");

    Free (mask);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * int * CountOccurences (node *block, DFMmask_t which)
 *
 * description:
 *   counts how often certain variables are used in a block (N_block) until
 *   they are set again.1
 *
 * ATTENTION:
 *   this routine works only on withloops!!!
 *   so allowed until now are blocks with:
 *   - the block contains only one withloop, but nothing else
 *
 *   other constructions will follow
 *
 ******************************************************************************/
int *
CountOccurences (node *block, DFMmask_t which)
{
    int *result;
    int occurs;
    node *assign;
    node *withloop;
    node *vardec;
    node *let;

    DBUG_ENTER ("CountOccurences");

    /* check: argument block is really a block? */
    DBUG_ASSERT ((NODE_TYPE (block) == N_block), ("argument block-node of wrong type"));

    /* check: block contains only one withloop and nothing else? */
    DBUG_ASSERT ((NODE_TYPE (BLOCK_INSTR (block)) == N_assign),
                 ("block does not contain assign"));
    assign = BLOCK_INSTR (block);
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (assign)) == N_let),
                 ("block does not contain let"));
    let = ASSIGN_INSTR (assign);
    DBUG_ASSERT ((NODE_TYPE (LET_EXPR (let)) == N_Nwith2),
                 ("block does not contain with-loop"));
    withloop = LET_EXPR (let);
    DBUG_ASSERT ((ASSIGN_NEXT (assign) == NULL),
                 ("block does not contain only one with-loop and nothing else"));

    /* instead of 99 the varno of function aroud has to be set!!!! #### */
    result = CreateCM (99);

    /* for each which that occurs in the with-loop return 1, others 0 */
    vardec = DFMGetMaskEntryDeclSet (which);
    while (vardec != NULL) {
        occurs = DFMTestMaskEntry (NWITH2_IN (withloop), NULL, vardec)
                 || DFMTestMaskEntry (NWITH2_INOUT (withloop), NULL, vardec);
        if (occurs) {
            /*
             *  Found. set corresponding value to 1 in result.
             */
            result[VARDEC_VARNO (vardec)] = 1;
        }
        vardec = DFMGetMaskEntryDeclSet (NULL);
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDTRO - SPMD - Traversal to Reduce Occurences
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *SPMDTROblock (node *arg_node, node *arg_info)
 *
 * description:
 *   one needs to traverse the contents of the block (the assignments) only.
 *   that is waht is done here
 *
 ******************************************************************************/
node *
SPMDTROblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDTROblock");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDTROlet (node *arg_node, node *arg_info)
 *
 * description:
 *
 * remarks:
 *
 ******************************************************************************/
node *
SPMDTROlet (node *arg_node, node *arg_info)
{
    ids *act_ids;

    DBUG_ENTER ("SPMDTROlet");

    /*
     *  checking whether any of the let-ids (better: the vardec of such an ids)
     *  is also in the mask.
     */
    act_ids = LET_IDS (arg_node);
    while (act_ids != NULL) {
        if (DFMTestMaskEntry (INFO_SPMDT_CHECK (arg_info), NULL, IDS_VARDEC (act_ids))) {
            /*
             *  found ids in check and result.
             */
            IDS_NAIVE_REFCNT (act_ids)
              = IDS_NAIVE_REFCNT (act_ids)
                - INFO_SPMDT_COUNTERS (arg_info)[IDS_VARNO (act_ids)];
            /*
             *  do not check this one again
             */
            DFMSetMaskEntryClear (INFO_SPMDT_CHECK (arg_info), NULL,
                                  IDS_VARDEC (act_ids));
            DBUG_PRINT ("SPMD", ("reduced %s", VARDEC_NAME (IDS_VARDEC (act_ids))));
        }
        act_ids = IDS_NEXT (act_ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDTROassign (node *arg_node, node *arg_info)
 *
 * description:
 *   this routine implements a bottom-up traversal of the assignment chain,
 *   it will traverse to the next assignment first and after that traverse
 *   into the assignment/instruction itself.
 *
 ******************************************************************************/
node *
SPMDTROassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDROassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDTRM - SPMD - Traversal to Reduce Masks
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *SPMDTRMblock (node *arg_node, node *arg_info)
 *
 * decription:
 *   one needs to traverse the contents of the block (the assignments) only.
 *   that is what is done here.
 *
 ******************************************************************************/
node *
SPMDTRMblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDTRMblock");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDTRMloop (node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
SPMDTRMloop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDTRMloop");

    DBUG_ASSERT (0, "SPMDTRMloop is not implemented, because it should not be reached");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDTRMcond (node *arg_node, node *arg_info)
 *
 * description:
 *
 ******************************************************************************/
node *
SPMDTRMcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDTRMcond");

    DBUG_ASSERT (0, "SPMDTRMcond is not implemented, because it should not be reached");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDTRMid (node *arg_node, node *arg_info)
 *
 * description:
 *   there is no traversal downto/upto N_id, since one is only interested
 *   in variables on the left hand side of lets, and they are not
 *   realized as N_id nodes in the tree, but as ids directly at N_let.
 *
 ******************************************************************************/
node *
SPMDTRMid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDTRMid");

    DBUG_ASSERT (0, "SPMDTRMid is not implemented, because it should not be reached");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDTRMlet (node *arg_node, node *arg_info)
 *
 * description:
 *   the traversal of the tree is done in a way that the last occurence
 *   of let's is been traversed first.
 *   checked are now variables who in the check mask and are set in the let.
 *   if such variables have refcounters with value 0, these variables are
 *   delete from the result mask. (it's rc ist zero the variable is not be
 *   used after the block ends.)
 *   if the rc is 0 or not, the varibales is not removed from the check
 *   mask, because it has been checked, and it shall not be overridden anymore.
 *
 * remarks:
 *   since one is interested only in varibales set,
 *   there is no traversal in the right hand side needed.
 *
 ******************************************************************************/
node *
SPMDTRMlet (node *arg_node, node *arg_info)
{
    ids *act_ids;

    DBUG_ENTER ("SPMDTRMlet");

    /*
     *  checking whether any of the let-ids (better: the vardec of such an ids)
     *  is also in the mask.
     */
    act_ids = LET_IDS (arg_node);
    while (act_ids != NULL) {
        /*
         *  everything in the check-mask is also in the result-mask.
         *  (but not the other way round!!!)
         */
        if (DFMTestMaskEntry (INFO_SPMDT_CHECK (arg_info), NULL, IDS_VARDEC (act_ids))) {
            /*
             *  found ids in check and result.
             */
            if (IDS_NAIVE_REFCNT (act_ids) == 0) {
                DFMSetMaskEntryClear (INFO_SPMDT_RESULT (arg_info), NULL,
                                      IDS_VARDEC (act_ids));
            }
            /*
             *  do not check this one again
             */
            DFMSetMaskEntryClear (INFO_SPMDT_CHECK (arg_info), NULL,
                                  IDS_VARDEC (act_ids));
        }
        act_ids = IDS_NEXT (act_ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDTRMassign (node *arg_node, node *arg_info)
 *
 * description:
 *   this routine implements a bottom-up traversal of the assignment chain,
 *   it will traverse to the next assignment first and after that traverse
 *   into the assignment/instruction itself.
 *
 ******************************************************************************/
node *
SPMDTRMassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
