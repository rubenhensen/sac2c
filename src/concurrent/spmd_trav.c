/*
 *
 * $Log$
 * Revision 2.7  1999/08/09 11:32:20  jhs
 * Cleaned up info-macros for concurrent-phase.
 *
 * Revision 2.6  1999/08/05 13:36:25  jhs
 * Added optimization of sequential assignments between spmd-blocks, main work
 * happens in spmdinit and ist steered by OPT_MTI (default now: off), some
 * traversals were needed and added in spmd_trav.
 *
 * Revision 2.5  1999/08/03 11:44:02  jhs
 * Comments.
 *
 * Revision 2.4  1999/07/30 13:48:12  jhs
 * Added comments.
 *
 * Revision 2.3  1999/07/28 13:07:45  jhs
 * CountOccurences gets fundef now.
 *
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
 *         SPMDTRM - ReduceMasks
 *
 * description:
 *   This file implements some travsersals needed by spmd_opt:
 *
 *   *co -> count occurences*
 *   Count occurences of Variables in certain blocks.
 *
 *   *ro -> reduce occurences*
 *   Reduces occurences in a certain block.
 *
 *   *rm -> reduce masks*
 *   Reduces masks in a certain block.
 *
 *****************************************************************************/

#include "dbug.h"

#include "DataFlowMask.h"
#include "traverse.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "refcount.h" /* FindVardec */

#include "spmd_trav.h"

/******************************************************************************
 *
 * function
 *   void ReduceOccurences (node *block, int *counters, DFMmask_t mask)
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

    INFO_SPMDRO_CHECK (arg_info) = DFMGenMaskCopy (mask);
    INFO_SPMDRO_COUNTERS (arg_info) = counters;

    block = Trav (block, arg_info);

    act_tab = old_tab;

    INFO_SPMDRO_CHECK (arg_info) = DFMRemoveMask (INFO_SPMDRO_CHECK (arg_info));
    INFO_SPMDRO_COUNTERS (arg_info) = NULL;
    FreeTree (arg_info);

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   DFMmask_t ReduceMasks ( node *block, DFMmask_t first_out)
 *
 * description:
 *   #### ???? to be pointed out
 *
 *   reduces the out mask to the variables really used behind two melted
 *   blocks
 *   all the out variables are set somewhere in the block, so there
 *   is at least one left handed occurence
 *
 * attention:
 *   Contents of first_out are changed.
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

    INFO_SPMDRM_RESULT (arg_info) = first_out;
    INFO_SPMDRM_CHECK (arg_info) = DFMGenMaskCopy (first_out);

    /*
     *  now check == result, but lateron result will be a superset of check.
     */
    block = Trav (block, arg_info);

    act_tab = old_tab;

    result = INFO_SPMDRM_RESULT (arg_info);

    INFO_SPMDRM_RESULT (arg_info) = NULL; /* used as function result */
    INFO_SPMDRM_CHECK (arg_info) = DFMRemoveMask (INFO_SPMDRM_CHECK (arg_info));
    FreeTree (arg_info);

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
 * int *CountOccurences (node *block, DFMmask_t which, node* fundef)
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
 *   other constructions will follow ####
 *
 ******************************************************************************/
int *
CountOccurences (node *block, DFMmask_t which, node *fundef)
{
    int *result;
    int occurs;
    node *assign;
    node *withloop;
    node *vardec;
    node *let;

    DBUG_ENTER ("CountOccurences");

    /* check argument types */
    DBUG_ASSERT ((NODE_TYPE (block) == N_block), ("argument block-node of wrong type"));
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 ("argument fundef-node of wrong type"));

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

    result = CreateCM (FUNDEF_VARNO (fundef));
    DBUG_PRINT ("SPMDT", ("fundef_varno %i", FUNDEF_VARNO (fundef)));

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

/* #### */
int
LetWithFunction (node *let)
{
    int result;
    node *arg_info;
    funptr *old_tab;

    DBUG_ENTER ("LetWithFunction");

    old_tab = act_tab;
    act_tab = spmdlc_tab;

    arg_info = MakeInfo ();
    INFO_SPMDLC_APPLICATION (arg_info) = FALSE;

    let = Trav (let, arg_info);

    result = INFO_SPMDLC_APPLICATION (arg_info);

    DBUG_PRINT ("SPMDI", ("lwf result: %i", result));

    Free (arg_info);

    act_tab = old_tab;

    DBUG_RETURN (result);
}

node *
SPMDTLCap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDTLCap");

    INFO_SPMDLC_APPLICATION (arg_info) = TRUE;

    DBUG_PRINT ("SPMDI", ("N_ap hit"));

    DBUG_RETURN (arg_node);
}

node *
DeleteNested (node *arg_node)
{
    node *arg_info;
    funptr *old_tab;

    DBUG_ENTER ("DeleteNested");

    old_tab = act_tab;
    act_tab = spmddn_tab;

    arg_info = MakeInfo ();
    INFO_SPMDDN_NESTED (arg_info) = FALSE;

    arg_node = Trav (arg_node, arg_info);

    Free (arg_info);

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

node *
SPMDDNspmd (node *arg_node, node *arg_info)
{
    node *spmd;

    DBUG_ENTER ("SPMDDNspmd");

    if (INFO_SPMDDN_NESTED (arg_info)) {
        DBUG_PRINT ("SPMDI", ("deleting spmd"));
        spmd = arg_node;
        arg_node = ASSIGN_INSTR (BLOCK_INSTR (SPMD_REGION (spmd)));
        ASSIGN_INSTR (BLOCK_INSTR (SPMD_REGION (spmd))) = NULL;
        Free (spmd);

        arg_node = Trav (arg_node, arg_info);
    } else {
        DBUG_PRINT ("SPMDI", ("first spmd, leaving"));

        INFO_SPMDDN_NESTED (arg_info) = TRUE;

        SPMD_REGION (arg_node) = Trav (SPMD_REGION (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/* attached to arg_info are spmd-masks to be changed ... */
void
ProduceMasks (node *arg_node, node *spmd, node *fundef)
{
    node *arg_info;
    funptr *old_tab;

    DBUG_ENTER ("ProduceMasks");

    old_tab = act_tab;
    act_tab = spmdpm_tab;

    arg_info = MakeInfo ();
    INFO_CONC_FUNDEF (arg_info) = fundef;
    INFO_SPMDPM_IN (arg_info) = SPMD_IN (spmd);
    INFO_SPMDPM_INOUT (arg_info) = SPMD_INOUT (spmd);
    INFO_SPMDPM_OUT (arg_info) = SPMD_OUT (spmd);
    INFO_SPMDPM_LOCAL (arg_info) = SPMD_LOCAL (spmd);
    INFO_SPMDPM_SHARED (arg_info) = SPMD_SHARED (spmd);

    arg_node = Trav (arg_node, arg_info);

    INFO_SPMDPM_IN (arg_info) = NULL;
    INFO_SPMDPM_INOUT (arg_info) = NULL;
    INFO_SPMDPM_OUT (arg_info) = NULL;
    INFO_SPMDPM_LOCAL (arg_info) = NULL;
    INFO_SPMDPM_SHARED (arg_info) = NULL;
    arg_info = FreeTree (arg_info);

    act_tab = old_tab;

    DBUG_VOID_RETURN;
}

node *
SPMDPMlet (node *arg_node, node *arg_info)
{
#if 0
  ids *letids;
#endif

    DBUG_ENTER ("SPMDPMlet");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_let), ("N_let expected"));

#if 0
  letids = LET_IDS( arg_node);
  while (letids != NULL) {
    DFMSetMaskEntrySet( SPMD_OUT( arg_info), IDS_NAME( letids), NULL);
    letids = IDS_NEXT (letids);
  }
#endif

    DBUG_RETURN (arg_node);
}

node *
SPMDPMassign (node *arg_node, node *arg_info)
{
    int i;
    node *vardec;
    node *with;

    DBUG_ENTER ("SPMDPMassign");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), ("N_assign expected"));

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_Nwith2)) {
        DBUG_PRINT ("SPMDPM", ("with pm"));
        with = LET_EXPR (ASSIGN_INSTR (arg_node));

        DFMSetMaskOr (INFO_SPMDPM_IN (arg_info), NWITH2_IN (with));
        DFMSetMaskOr (INFO_SPMDPM_OUT (arg_info), NWITH2_OUT (with));
        DFMSetMaskOr (INFO_SPMDPM_OUT (arg_info), NWITH2_INOUT (with));
        DFMSetMaskOr (INFO_SPMDPM_INOUT (arg_info), NWITH2_INOUT (with));
        DFMSetMaskOr (INFO_SPMDPM_LOCAL (arg_info), NWITH2_LOCAL (with));

    } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_while) {
        DBUG_PRINT ("SPMDPM", ("while pm"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        DBUG_PRINT ("SPMDPM", ("for pm"));
        DBUG_PRINT ("SPMDPM",
                    ("for pm -> %i", FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info))));

        for (i = 0; i < FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)); i++) {
            /*
                  DBUG_PRINT( "SPMDPM", ("for pm step"));
                  DBUG_PRINT( "SPMDPM", ("(%i) use = %i def = %i",
                                         i,
                                         (ASSIGN_USEMASK( arg_node) != NULL) ?
               ASSIGN_USEMASK( arg_node)[i] : -1, (ASSIGN_DEFMASK( arg_node) != NULL) ?
               ASSIGN_DEFMASK( arg_node)[i] : -1));
            */
            if (((ASSIGN_USEMASK (arg_node) != NULL)
                 && (ASSIGN_USEMASK (arg_node)[i] > 0))
                || ((ASSIGN_DEFMASK (arg_node) != NULL)
                    && (ASSIGN_DEFMASK (arg_node)[i] > 0))) {
                vardec = FindVardec (i, INFO_CONC_FUNDEF (arg_info));
                if ((ASSIGN_USEMASK (arg_node) != NULL)
                    && (ASSIGN_USEMASK (arg_node)[i] > 0)) {
                    DBUG_PRINT ("SPMDPM", ("use hit %i", i));
                    DFMSetMaskEntrySet (INFO_SPMDPM_IN (arg_info), NULL, vardec);
                }
                if ((ASSIGN_DEFMASK (arg_node) != NULL)
                    && (ASSIGN_DEFMASK (arg_node)[i] > 0)) {
                    DBUG_PRINT ("SPMDPM", ("def hit %i", i));
                    DFMSetMaskEntrySet (INFO_SPMDPM_OUT (arg_info), NULL, vardec);
                }
            }
        }
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("SPMDPM", ("next pm"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDTCO - SPMD - Traversal to Count Occurences
 **
 ******************************************************************************
 ******************************************************************************/

/* #### not yet implemented as real traversal */

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
        if (DFMTestMaskEntry (INFO_SPMDRO_CHECK (arg_info), NULL, IDS_VARDEC (act_ids))) {
            /*
             *  found ids in check and result.
             */
            IDS_NAIVE_REFCNT (act_ids)
              = IDS_NAIVE_REFCNT (act_ids)
                - INFO_SPMDRO_COUNTERS (arg_info)[IDS_VARNO (act_ids)];
            /*
             *  do not check this one again
             */
            DFMSetMaskEntryClear (INFO_SPMDRO_CHECK (arg_info), NULL,
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
        if (DFMTestMaskEntry (INFO_SPMDRM_CHECK (arg_info), NULL, IDS_VARDEC (act_ids))) {
            /*
             *  found ids in check and result.
             */
            if (IDS_NAIVE_REFCNT (act_ids) == 0) {
                DFMSetMaskEntryClear (INFO_SPMDRM_RESULT (arg_info), NULL,
                                      IDS_VARDEC (act_ids));
            }
            /*
             *  do not check this one again
             */
            DFMSetMaskEntryClear (INFO_SPMDRM_CHECK (arg_info), NULL,
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
