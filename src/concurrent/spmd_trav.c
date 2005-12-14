/*****************************************************************************
 *
 * file:     spmd_trav.c
 *
 * description:
 *   This file implements some traversals needed in spmd_xxx/sync_xxx phases.
 *   All phases are explained in extra sections in this file.
 *
 *****************************************************************************/

#include <stdio.h>
#include "DataFlowMask.h"
#include "traverse.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "spmd_trav.h"

/*
 * INFO structure
 */

struct INFO {
    dfmask_t *in;
    dfmask_t *inout;
    dfmask_t *out;
    dfmask_t *local;
    dfmask_t *shared;
    bool nested;
    node *fundef;
};

/* INFO macros */
#define INFO_SPMDDN_NESTED(n) (n->nested)
#define INFO_SPMDPM_IN(n) (n->in)
#define INFO_SPMDPM_INOUT(n) (n->inout)
#define INFO_SPMDPM_OUT(n) (n->out)
#define INFO_SPMDPM_LOCAL(n) (n->local)
#define INFO_SPMDPM_SHARED(n) (n->shared)
#define INFO_SPMDDN_NESTED(n) (n->nested)
#define INFO_SPMDPM_FUNDEF(n) (n->fundef)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_SPMDPM_IN (result) = NULL;
    INFO_SPMDPM_INOUT (result) = NULL;
    INFO_SPMDPM_OUT (result) = NULL;
    INFO_SPMDPM_LOCAL (result) = NULL;
    INFO_SPMDPM_SHARED (result) = NULL;
    INFO_SPMDDN_NESTED (result) = FALSE;
    INFO_SPMDPM_FUNDEF (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
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
 *   node *SPMDDNdoDeleteNested (node *arg_node)
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
 * in call "SPMDDNdoDeleteNested(a)" a, b, c will survive, e and f not!
 *
 ******************************************************************************/
node *
SPMDDNdoDeleteNested (node *arg_node)
{
    info *arg_info;

    DBUG_ENTER ("SPMDDNdoDeleteNested");

    arg_info = MakeInfo ();
    INFO_SPMDDN_NESTED (arg_info) = FALSE;

    DBUG_PRINT ("SPMDDN", ("trav into"));
    TRAVpush (TR_spmddn);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();
    DBUG_PRINT ("SPMDDN", ("trav from"));

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDDNspmd (node *arg_node, info *arg_info)
 *
 * descriptiom:
 *   - If a spmd-block was already hit, this function deletes the actual spmd,
 *     and hangs the contained assignments at the father-node.
 *   - If not all spmd-blocks in this block will be deleted, but this
 *     spmd-block willsurvive.
 *
 ******************************************************************************/
node *
SPMDDNspmd (node *arg_node, info *arg_info)
{
    node *spmd;

    DBUG_ENTER ("SPMDDNspmd");

    if (INFO_SPMDDN_NESTED (arg_info)) {
        DBUG_PRINT ("SPMDI", ("deleting spmd"));
        spmd = arg_node;
        arg_node = ASSIGN_INSTR (BLOCK_INSTR (SPMD_REGION (spmd)));
        ASSIGN_INSTR (BLOCK_INSTR (SPMD_REGION (spmd))) = NULL;
        spmd = FREEdoFreeTree (spmd);

        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        DBUG_PRINT ("SPMDI", ("first spmd, leaving as is"));

        INFO_SPMDDN_NESTED (arg_info) = TRUE;

        DBUG_PRINT ("SPMDI", ("first spmd, into trav"));
        SPMD_REGION (arg_node) = TRAVdo (SPMD_REGION (arg_node), arg_info);
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
 *   void SPMDPMdoProduceMasks (node *arg_node, node *spmd, node* fundef);
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
SPMDPMdoProduceMasks (node *arg_node, node *spmd, node *fundef)
{
    info *arg_info;
    trav_t traversaltable;

    DBUG_ENTER ("SPMDPMdoProduceMasks");

    TRAVpush (TR_spmdpm);

    arg_info = MakeInfo ();
    INFO_SPMDPM_FUNDEF (arg_info) = fundef;
    INFO_SPMDPM_IN (arg_info) = SPMD_IN (spmd);
    INFO_SPMDPM_INOUT (arg_info) = SPMD_INOUT (spmd);
    INFO_SPMDPM_OUT (arg_info) = SPMD_OUT (spmd);
    INFO_SPMDPM_LOCAL (arg_info) = SPMD_LOCAL (spmd);
    INFO_SPMDPM_SHARED (arg_info) = SPMD_SHARED (spmd);

    arg_node = TRAVdo (arg_node, arg_info);

    INFO_SPMDPM_IN (arg_info) = NULL;
    INFO_SPMDPM_INOUT (arg_info) = NULL;
    INFO_SPMDPM_OUT (arg_info) = NULL;
    INFO_SPMDPM_LOCAL (arg_info) = NULL;
    INFO_SPMDPM_SHARED (arg_info) = NULL;
    arg_info = FreeInfo (arg_info);

    traversaltable = TRAVpop ();
    DBUG_ASSERT ((traversaltable == TR_spmdpm), "Popped incorrect traversal table");

    DBUG_VOID_RETURN;
}

/* #### */
/* #### i do not belive this traversal works for more than one assignment!!!
        to do that, one must check whether are variables is set and do not count
        further uses as in-variables ...  */
node *
SPMDPMassign (node *arg_node, info *arg_info)
{
    node *with;

    DBUG_ENTER ("SPMDPMassign");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_assign), ("N_assign expected"));

    if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_with2)) {
        DBUG_PRINT ("SPMDPM", ("with pm"));
        with = LET_EXPR (ASSIGN_INSTR (arg_node));

        DFMsetMaskOr (INFO_SPMDPM_IN (arg_info), WITH2_IN_MASK (with));
        DFMsetMaskOr (INFO_SPMDPM_OUT (arg_info), WITH2_OUT_MASK (with));
        DFMsetMaskOr (INFO_SPMDPM_LOCAL (arg_info), WITH2_LOCAL_MASK (with));

        DBUG_ASSERT ((IDS_NEXT (LET_IDS (ASSIGN_INSTR (arg_node))) == NULL),
                     "more than one ids on LHS of with-loop found");

        /*
         * takings vars from LHS of with-loop assignment into account
         */
        DFMsetMaskEntryClear (INFO_SPMDPM_OUT (arg_info), NULL,
                              IDS_AVIS (LET_IDS (ASSIGN_INSTR (arg_node))));
        if ((NODE_TYPE (WITH2_WITHOP (with)) == N_genarray)
            || (NODE_TYPE (WITH2_WITHOP (with)) == N_modarray)) {
            DFMsetMaskEntryClear (INFO_SPMDPM_INOUT (arg_info), NULL,
                                  IDS_AVIS (LET_IDS (ASSIGN_INSTR (arg_node))));
        }
    } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_do) {
        DBUG_PRINT ("SPMDPM", ("while pm"));
        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("SPMDPM", ("next pm"));
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
