/*
 *
 * $Log$
 * Revision 2.10  2000/01/26 17:25:08  dkr
 * type of traverse-function-table changed.
 *
 * Revision 2.9  2000/01/25 13:42:25  dkr
 * function FindVardec moved to tree_compound.h and renamed to
 * FindVardec_Varno
 *
 * Revision 2.8  1999/08/27 12:46:43  jhs
 * Added Traversal for CountOccurences.
 * Commented and rearranged functions.
 *
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
 * file:     spmd_trav.c
 *
 * prefixes: SPMDCO - CountOccurences
 *           SPMDRO - ReduceOccurences
 *           SPMDRM - ReduceMasks
 *
 * description:
 *   This file implements some traversals needed in spmd_xxx/sync_xxx phases.
 *   All phases are explained in extra sections in this file.
 *
 *****************************************************************************/

#include <stdio.h>

#include "dbug.h"

#include "DataFlowMask.h"
#include "traverse.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"

#include "spmd_trav.h"

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   counter-mask (cm) helpers
 **   functions to create & destroy counter masks.
 **
 ******************************************************************************
 ******************************************************************************/

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
 *   int *DestroyCM (int *mask)
 *
 * description:
 *   releases memory used for a mask of counters.
 *
 * related:
 *   CreateCM
 *
 ******************************************************************************/
int *
DestroyCM (int *mask)
{
    DBUG_ENTER ("DestroyCM");

    Free (mask);

    DBUG_RETURN ((int *)NULL);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **  #### no good name found ...
 **
 ******************************************************************************
 ******************************************************************************/

int
LetWithFunction (node *let)
{
    int result;
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("LetWithFunction");

    old_tab = act_tab;
    act_tab = spmdlc_tab;

    arg_info = MakeInfo ();
    INFO_SPMDLC_APPLICATION (arg_info) = FALSE;

    let = Trav (let, arg_info);

    result = INFO_SPMDLC_APPLICATION (arg_info);

    DBUG_PRINT ("SPMDI", ("lwf result: %i", result));

    arg_info = FreeTree (arg_info);

    act_tab = old_tab;

    DBUG_RETURN (result);
}

node *
SPMDLCap (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDLCap");

    INFO_SPMDLC_APPLICATION (arg_info) = TRUE;

    DBUG_PRINT ("SPMDI", ("N_ap hit"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDDN - Delete Nested SPMDs
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   node *DeleteNested (node *arg_node)
 *
 * description:
 *   Deletes all spmd-blocks nested in the first spmd-blocks found while
 *   traversing from arg_node on. Ideal usage is to hand over an spmd-block
 *   as arg_node, whose inner spmd-blocks then will be deleteD. But one could
 *   also hand over an node, containing several spmd-blocks "directly" which
 *   are not nested in each other, and deleted the ones nested in them, but
 *   all "directly" connected blocks will survive!!!
 *
 * exapmle:
 *   a: assign -> assign -> assign -> ...
 *        |         |         |
 *   b:  spmd     c: spmd   d: spmd
 *        |                   |
 *   e:  spmd               f: spmd
 *
 * in call "DeleteNested(a)" a, b, c will survive, e and f not!
 *
 ******************************************************************************/
node *
DeleteNested (node *arg_node)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("DeleteNested");

    old_tab = act_tab;
    act_tab = spmddn_tab;

    arg_info = MakeInfo ();
    INFO_SPMDDN_NESTED (arg_info) = FALSE;

    DBUG_PRINT ("SPMDDN", ("trav into"));
    arg_node = Trav (arg_node, arg_info);
    DBUG_PRINT ("SPMDDN", ("trav from"));

    arg_info = FreeTree (arg_info);
    DBUG_PRINT ("SPMDI", ("free arg_info"));

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDDNspmd (node *arg_node, node *arg_info)
 *
 * descriptiom:
 *   - If a spmd-block was already hit, this function deletes the actual spmd,
 *     and hangs the contained assignments at the father-node.
 *   - If not all spmd-blocks in this block will be deleted, but this
 *     spmd-block willsurvive.
 *
 ******************************************************************************/
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
        spmd = FreeTree (spmd);

        arg_node = Trav (arg_node, arg_info);
    } else {
        DBUG_PRINT ("SPMDI", ("first spmd, leaving as is"));

        INFO_SPMDDN_NESTED (arg_info) = TRUE;

        DBUG_PRINT ("SPMDI", ("first spmd, into trav"));
        SPMD_REGION (arg_node) = Trav (SPMD_REGION (arg_node), arg_info);
        DBUG_PRINT ("SPMDI", ("first spmd, from trav"));

        /*
         *  following spmd-blocks are not nested, seen fron the start-point
         *  of this traversal!!!
         */
        INFO_SPMDDN_NESTED (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDPM - Produce mask for non-with-loops
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   void ProduceMasks (node *arg_node, node *spmd, node* fundef);
 *
 * description:
 *   inferres the masks needed for the overhanded spmd-block and
 *   sets these values to the mask.
 *
 * attention:
 *   the masks of the spmd-block should be be reserved and cleared before
 *   calling this function. The functions itself does not care about that.
 *
 ******************************************************************************/
void
ProduceMasks (node *arg_node, node *spmd, node *fundef)
{
    node *arg_info;
    funtab *old_tab;

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

/* #### */
/* #### i do not belive this traversal works for more than one assignment!!!
        to do that, one must check whether are variables is set and do not count
        further uses as in-variables ...  */
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
    } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_do) {
        DBUG_PRINT ("SPMDPM", ("while pm"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        DBUG_PRINT ("SPMDPM",
                    ("for pm -> %i", FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info))));

        for (i = 0; i < FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)); i++) {
            /*
                  DBUG_PRINT( "SPMDPM", ("for pm (%i) use = %i def = %i",
                                         i,
                                         (ASSIGN_USEMASK( arg_node) != NULL)
                                           ? ASSIGN_USEMASK( arg_node)[i] : -1,
                                         (ASSIGN_DEFMASK( arg_node) != NULL)
                                           ? ASSIGN_DEFMASK( arg_node)[i] : -1));
            */
            if (((ASSIGN_USEMASK (arg_node) != NULL)
                 && (ASSIGN_USEMASK (arg_node)[i] > 0))
                || ((ASSIGN_DEFMASK (arg_node) != NULL)
                    && (ASSIGN_DEFMASK (arg_node)[i] > 0))) {
                vardec = FindVardec_Varno (i, INFO_CONC_FUNDEF (arg_info));
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
 **   SPMDCO - SPMD - Traversal to Count Occurences
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * int *CountOccurences (node *block, DFMmask_t which, node* fundef)
 *
 * description:
 *   counts how often certain variables are used in a block (N_block) until
 *   they are set again.
 *
 ******************************************************************************/
int *
CountOccurences (node *block, DFMmask_t which, node *fundef)
{
    int *result;
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("CountOccurences");

    /* check argument types */
    DBUG_ASSERT ((NODE_TYPE (block) == N_block), ("argument block-node of wrong type"));
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 ("argument fundef-node of wrong type"));

    arg_info = MakeInfo ();
    INFO_CONC_FUNDEF (arg_info) = fundef;
    INFO_SPMDCO_WHICH (arg_info) = DFMGenMaskCopy (which);
    INFO_SPMDCO_RESULT (arg_info) = CreateCM (FUNDEF_VARNO (fundef));

    old_tab = act_tab;
    act_tab = spmdco_tab;

    DBUG_PRINT ("SPMDCO", ("co into trav"));
    block = Trav (block, arg_info);
    DBUG_PRINT ("SPMDCO", ("co from trav"));

    act_tab = old_tab;

    result = INFO_SPMDCO_RESULT (arg_info);
    INFO_SPMDCO_WHICH (arg_info) = DFMRemoveMask (INFO_SPMDCO_WHICH (arg_info));
    INFO_CONC_FUNDEF (arg_info) = NULL;
    arg_info = FreeTree (arg_info);

    DBUG_RETURN (result);
}

/* #### tests if assign is let and with-loop */
int
isWith (node *assign)
{
    int result;

    DBUG_ENTER ("isWith");

    result = FALSE;
    if (NODE_TYPE (assign) == N_assign) {
        if (ASSIGN_INSTR (assign) != NULL) {
            if (NODE_TYPE (ASSIGN_INSTR (assign)) == N_let) {
                if (LET_EXPR (ASSIGN_INSTR (assign)) != NULL) {
                    if (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (assign))) == N_Nwith2) {
                        result = TRUE;
                    }
                }
            }
        }
    }

    DBUG_RETURN (result);
}

/* #### comment */
node *
SPMDCOassign (node *arg_node, node *arg_info)
{
    node *vardec;
    int count;
    int i;
    int *dump;

    DBUG_ENTER ("SPMDCOassign");
    DBUG_PRINT ("SPMDCO", ("SPMDCOassign"));

    if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_while) {

        dump = CreateCM (FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)));
        for (i = 0; i < FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)); i++) {
            dump[i] = INFO_SPMDCO_RESULT (arg_info)[i];
        }

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        for (i = 0; i < FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)); i++) {
            if ((INFO_SPMDCO_RESULT (arg_info)[i] - dump[i]) > 0) {
                INFO_SPMDCO_RESULT (arg_info)[i] = dump[i] + 1;
            }
        }
        dump = DestroyCM (dump);

    } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_do) {

        dump = CreateCM (FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)));
        for (i = 0; i < FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)); i++) {
            dump[i] = INFO_SPMDCO_RESULT (arg_info)[i];
        }

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        for (i = 0; i < FUNDEF_VARNO (INFO_CONC_FUNDEF (arg_info)); i++) {
            if ((INFO_SPMDCO_RESULT (arg_info)[i] - dump[i]) > 0) {
                INFO_SPMDCO_RESULT (arg_info)[i] = dump[i] + 1;
            }
        }
        dump = DestroyCM (dump);

    } else {
        vardec = DFMGetMaskEntryDeclSet (INFO_SPMDCO_WHICH (arg_info));
        while (vardec != NULL) {
            DBUG_PRINT ("SPMDCO", ("checking varno %i", VARDEC_VARNO (vardec)));
            if (ASSIGN_USEMASK (arg_node) != NULL) {
                if (isWith (arg_node)) {
                    /* all references to one variable in a with-loop count only 1 */
                    DBUG_PRINT ("SPMDCO", ("withhit"));
                    if (ASSIGN_USEMASK (arg_node)[VARDEC_VARNO (vardec)]) {
                        count = 1;
                    } else {
                        count = 0;
                    }
                } else {
                    count = ASSIGN_USEMASK (arg_node)[VARDEC_VARNO (vardec)];
                }
            } else {
                count = 0;
            }
            DBUG_PRINT ("SPMDCO", ("count = %i", count));
            INFO_SPMDCO_RESULT (arg_info)
            [VARDEC_VARNO (vardec)]
              = INFO_SPMDCO_RESULT (arg_info)[VARDEC_VARNO (vardec)] + count;

            DBUG_PRINT ("SPMDCO", ("change which1"));
            if (ASSIGN_DEFMASK (arg_node) != NULL) {
                DBUG_PRINT ("SPMDCO", ("change which2"));
                if (ASSIGN_DEFMASK (arg_node)[VARDEC_VARNO (vardec)] > 0) {
                    DBUG_PRINT ("SPMDCO", ("change which3"));
                    DFMSetMaskEntryClear (INFO_SPMDCO_WHICH (arg_info), NULL, vardec);
                }
            }
            vardec = DFMGetMaskEntryDeclSet (NULL);
        }
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 ******************************************************************************
 **
 ** section:
 **   SPMDRO - SPMD - Traversal to Reduce Occurences
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function
 *   void ReduceOccurences (node *block, int *counters, DFMmask_t mask)
 *
 * description:
 *   Modifies the naive refcounters (occurences) in the actual spmd-block.
 *   Handed over here as block.
 *   The last set of a variable (v) will be decremented by the number specified
 *   by counter[varno] (where varno is the varno of v).
 *   If counters is produced by CountOccurences before, this traversal will
 *   change the meaning of left-handed refcounters. They will afterwards tell,
 *   how often a varaible is used beyond this actual spmd-block.
 *   This feature is used by reduce masks.
 *
 * ATTENTION:
 *   To be clear here:
 *   ***THIS TRAVERSAL CHANGES THE MEANING OF NAIVE-REFCOUNTERS***
 *
 ******************************************************************************/
void
ReduceOccurences (node *block, int *counters, DFMmask_t mask)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("ReduceOccurences");

    old_tab = act_tab;
    act_tab = spmdrotrav_tab;

    arg_info = MakeInfo ();

    INFO_SPMDRO_CHECK (arg_info) = DFMGenMaskCopy (mask);
    INFO_SPMDRO_COUNTERS (arg_info) = counters;

    DBUG_PRINT ("SPMDRO", ("ro into trav"));
    block = Trav (block, arg_info);
    DBUG_PRINT ("SPMDRO", ("ro from trav"));

    INFO_SPMDRO_CHECK (arg_info) = DFMRemoveMask (INFO_SPMDRO_CHECK (arg_info));
    INFO_SPMDRO_COUNTERS (arg_info) = NULL;
    arg_info = FreeTree (arg_info);

    act_tab = old_tab;

    DBUG_VOID_RETURN;
}

/******************************************************************************
 *
 * function:
 *   node *SPMDROblock (node *arg_node, node *arg_info)
 *
 * description:
 *   one needs to traverse the contents of the block (the assignments) only.
 *   that is waht is done here
 *
 ******************************************************************************/
node *
SPMDROblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDROblock");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDROlet (node *arg_node, node *arg_info)
 *
 * description:
 *   The mask INFO_SPMDRO_CHECK contains the variables to be checked in this
 *   traversal, if such a variables is found on the left hand, one reduces the
 *   naive-refcounters provided by the the INFO_SPMDRO_COUNTER mask
 *   (earlier created by CountOccurences).
 *   After that, one eliminates the variable from the mask. Because this is a
 *   bottom-up traversal, it is achieved that the last occurences in the actual
 *   spmd-block is found and decremented. The values of the left-handed
 *   naive-refcounters will then tell how often the value set to this variable
 *   is used beyaond this block.
 *
 ******************************************************************************/
node *
SPMDROlet (node *arg_node, node *arg_info)
{
    ids *act_ids;

    DBUG_ENTER ("SPMDROlet");

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
            DBUG_PRINT ("SPMDRO",
                        ("reduced %s by %i is now %i", VARDEC_NAME (IDS_VARDEC (act_ids)),
                         INFO_SPMDRO_COUNTERS (arg_info)[IDS_VARNO (act_ids)],
                         IDS_NAIVE_REFCNT (act_ids)));
        }
        act_ids = IDS_NEXT (act_ids);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDROassign (node *arg_node, node *arg_info)
 *
 * description:
 *   this routine implements a bottom-up traversal of the assignment chain,
 *   it will traverse to the next assignment first and after that traverse
 *   into the assignment/instruction itself.
 *
 ******************************************************************************/
node *
SPMDROassign (node *arg_node, node *arg_info)
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
 **   SPMDRM - SPMD - Traversal to Reduce Masks
 **
 ******************************************************************************
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *   DFMmask_t ReduceMasks ( node *block, DFMmask_t first_out)
 *
 * description:
 *   ReduceMasks reduces the mask handed over to the values really uses
 *   outside of the actual spmd-block.
 *
 * How is this done??? It very easy:
 *   While ReduceOccurences has reduced all left-handed naive-refcounters to
 *   the number of occurences this variable is used beyond the end of the
 *   actual spmd-block, ReduceMasks uses exactly this information.
 *   (By the way: Reduce Occurences has not changed anz right-haned occurence.)
 *   ReduceMasks searches the last left-handed-occurence of the varibales
 *   in the mask (botom-up-traversal), if this last occurence has a
 *   naive-refcount of 0, this variable is not uesd beyond the actual
 *   spmd-block anymore, so it must not  be in the out-mask (in german: "Die
 *   Variable darf nicht in der Maske sein"), so the variable is deleted in
 *   this mask.
 *
 * attention:
 *   Contents of first_out are changed as side-effect.
 *
 ******************************************************************************/
DFMmask_t
ReduceMasks (node *block, DFMmask_t first_out)
{
    node *arg_info;
    funtab *old_tab;
    DFMmask_t result;

    DBUG_ENTER ("ReduceMasks");

    DBUG_PRINT ("SPMDRM", ("Entering ReduceMasks"));

    arg_info = MakeInfo ();

    old_tab = act_tab;
    act_tab = spmdrmtrav_tab;

    INFO_SPMDRM_RESULT (arg_info) = first_out;
    INFO_SPMDRM_CHECK (arg_info) = DFMGenMaskCopy (first_out);

    /*
     *  Now check == result, but lateron result (due reduces)
     *  will be a superset of check.
     */
    block = Trav (block, arg_info);

    act_tab = old_tab;

    result = INFO_SPMDRM_RESULT (arg_info);

    INFO_SPMDRM_RESULT (arg_info) = NULL; /* used as function result */
    INFO_SPMDRM_CHECK (arg_info) = DFMRemoveMask (INFO_SPMDRM_CHECK (arg_info));
    arg_info = FreeTree (arg_info);

    DBUG_PRINT ("SPMDRM", ("Leaveing ReduceMasks"));

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDRMblock (node *arg_node, node *arg_info)
 *
 * decription:
 *   one needs to traverse the contents of the block (the assignments) only.
 *   that is what is done here.
 *
 ******************************************************************************/
node *
SPMDRMblock (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMblock");

    BLOCK_INSTR (arg_node) = Trav (BLOCK_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDRMwhile (node *arg_node, node *arg_info)
 *
 * description:
 *   while-loops does not make any known problems here, we only need to
 *   traverse the assignments.
 *
 ******************************************************************************/
node *
SPMDRMwhile (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMwhile");

    DBUG_PRINT ("SPMDRM", ("trav into while"));
    WHILE_BODY (arg_node) = Trav (WHILE_BODY (arg_node), arg_info);
    DBUG_PRINT ("SPMDRM", ("trav from while"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDRMloop (node *arg_node, node *arg_info)
 *
 * description:
 *   do-loops does not make any known problems here, we only need to
 *   traverse the assignments.
 *
 ******************************************************************************/
node *
SPMDRMloop (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMloop");

    DBUG_PRINT ("SPMDRM", ("trav into do"));
    DO_BODY (arg_node) = Trav (DO_BODY (arg_node), arg_info);
    DBUG_PRINT ("SPMDRM", ("trav from do"));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDRMcond (node *arg_node, node *arg_info)
 *
 * description:
 *   #### implementation not done yet ...
 *
 ******************************************************************************/
node *
SPMDRMcond (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMcond");

    DBUG_ASSERT (0, "SPMDRMcond is not implemented, because it should not be reached");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node SPMDRMid (node *arg_node, node *arg_info)
 *
 * description:
 *   there is no traversal downto/upto N_id, since one is only interested
 *   in variables on the left hand side of lets, and they are not
 *   realized as N_id nodes in the tree, but as ids directly at N_let.
 *
 ******************************************************************************/
node *
SPMDRMid (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMid");

    DBUG_ASSERT (0, "SPMDRMid is not implemented, because it should not be reached");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDRMlet (node *arg_node, node *arg_info)
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
 *   since one is interested only in varibales set (left-handed),
 *   there is no traversal in the right hand side needed.
 *
 ******************************************************************************/
node *
SPMDRMlet (node *arg_node, node *arg_info)
{
    ids *act_ids;

    DBUG_ENTER ("SPMDRMlet");

    /*
     *  checking whether any of the let-ids (better: the vardec of such an ids)
     *  is also in the mask.
     */
    act_ids = LET_IDS (arg_node);
    while (act_ids != NULL) {
        /*
         *  everything in the check-mask is also in the result-mask ...
         *  (but not the other way round!!!)
         */
        if (DFMTestMaskEntry (INFO_SPMDRM_CHECK (arg_info), NULL, IDS_VARDEC (act_ids))) {
            /*
             *  ... so if we land here, we found ids in check AND ALSO in result!
             */
            DBUG_PRINT ("SPMDRM", ("ids: name: %s, rc: %i nrc: %i", IDS_NAME (act_ids),
                                   IDS_REFCNT (act_ids), IDS_NAIVE_REFCNT (act_ids)));

            /*
             *  a negative naive-refcounter would mean it is -1 and that means the
             *  variable has not be naive-refcounted, but exactly this information
             *  is needed by this point of this optimization.
             *  So if naive-refcounter not available: fix the refcounting!!!
             */
            DBUG_ASSERT ((IDS_NAIVE_REFCNT (act_ids) > -1),
                         ("NAIVE_REFCNT is not positive or null"));

            if (IDS_NAIVE_REFCNT (act_ids) == 0) {
                DBUG_PRINT ("SPMDRM", ("erased %s", IDS_NAME (act_ids)));
                DFMSetMaskEntryClear (INFO_SPMDRM_RESULT (arg_info), NULL,
                                      IDS_VARDEC (act_ids));
            }
            /*
             *  do not check this one again, delete it from the mask of variables
             *  to be checked.
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
 *   node *SPMDRMassign (node *arg_node, node *arg_info)
 *
 * description:
 *   this routine implements a bottom-up traversal of the assignment chain,
 *   it will traverse to the next assignment first and after that traverse
 *   into the assignment/instruction itself.
 *
 ******************************************************************************/
node *
SPMDRMassign (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("SPMDRMassign");

    /*
     *  This is a bottom-up-traversal, so one traverses to the next assignments
     *  first, than does the work on the instruction of this assignment.
     */

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("SPMDRM", ("rm trav into next"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        DBUG_PRINT ("SPMDRM", ("rm trav from next"));
    } else {
        DBUG_PRINT ("SPMDRM", ("rm turnaround"));
    }

    /*
     *  now the work on the instruction (or it's let-left-side) is done.
     */
    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}
