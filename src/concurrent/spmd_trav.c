/*
 *
 * $Log$
 * Revision 3.10  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.9  2004/09/28 16:33:12  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.8  2004/09/28 14:09:59  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.7  2004/02/25 08:17:44  cg
 * Elimination of while-loops by conversion into do-loops with
 * leading conditional integrated into flatten.
 * Separate compiler phase while2do eliminated.
 * NO while-loops may occur after flatten.
 * While-loop specific code eliminated.
 *
 * Revision 3.6  2001/05/08 12:49:42  dkr
 * new RC macros used
 *
 * Revision 3.3  2000/12/12 12:12:45  dkr
 * NWITH_INOUT removed
 * interpretation of NWITH_IN changed:
 * the LHS of a with-loop assignment is now longer included in
 * NWITH_IN!!!
 *
 * Revision 3.1  2000/11/20 18:02:33  sacbase
 * new release made
 *
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
 */

/*****************************************************************************
 *
 * file:     spmd_trav.c
 *
 * description:
 *   This file implements some traversals needed in spmd_xxx/sync_xxx phases.
 *   All phases are explained in extra sections in this file.
 *
 *****************************************************************************/

#define NEW_INFO

#include <stdio.h>
#include "dbug.h"
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
    DFMmask_t in;
    DFMmask_t inout;
    DFMmask_t out;
    DFMmask_t local;
    DFMmask_t shared;
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

    result = Malloc (sizeof (info));

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

    info = Free (info);

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
    info *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("DeleteNested");

    old_tab = act_tab;
    act_tab = spmddn_tab;

    arg_info = MakeInfo ();
    INFO_SPMDDN_NESTED (arg_info) = FALSE;

    DBUG_PRINT ("SPMDDN", ("trav into"));
    arg_node = Trav (arg_node, arg_info);
    DBUG_PRINT ("SPMDDN", ("trav from"));

    arg_info = FreeInfo (arg_info);
    DBUG_PRINT ("SPMDI", ("free arg_info"));

    act_tab = old_tab;

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
    info *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("ProduceMasks");

    old_tab = act_tab;
    act_tab = spmdpm_tab;

    arg_info = MakeInfo ();
    INFO_SPMDPM_FUNDEF (arg_info) = fundef;
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
    arg_info = FreeInfo (arg_info);

    act_tab = old_tab;

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
        && (NODE_TYPE (LET_EXPR (ASSIGN_INSTR (arg_node))) == N_Nwith2)) {
        DBUG_PRINT ("SPMDPM", ("with pm"));
        with = LET_EXPR (ASSIGN_INSTR (arg_node));

        DFMSetMaskOr (INFO_SPMDPM_IN (arg_info), NWITH2_IN_MASK (with));
        DFMSetMaskOr (INFO_SPMDPM_OUT (arg_info), NWITH2_OUT_MASK (with));
        DFMSetMaskOr (INFO_SPMDPM_LOCAL (arg_info), NWITH2_LOCAL_MASK (with));

        DBUG_ASSERT ((IDS_NEXT (LET_IDS (ASSIGN_INSTR (arg_node))) == NULL),
                     "more than one ids on LHS of with-loop found");

        /*
         * takings vars from LHS of with-loop assignment into account
         */
        DFMSetMaskEntryClear (INFO_SPMDPM_OUT (arg_info), NULL,
                              LET_VARDEC (ASSIGN_INSTR (arg_node)));
        if ((NWITH2_TYPE (with) == WO_genarray) || (NWITH2_TYPE (with) == WO_modarray)) {
            DFMSetMaskEntryClear (INFO_SPMDPM_INOUT (arg_info), NULL,
                                  LET_VARDEC (ASSIGN_INSTR (arg_node)));
        }
    } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_while) {
        DBUG_PRINT ("SPMDPM", ("while pm"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_do) {
        DBUG_PRINT ("SPMDPM", ("while pm"));
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        DBUG_PRINT ("SPMDPM",
                    ("for pm -> %i", FUNDEF_VARNO (INFO_SPMDPM_FUNDEF (arg_info))));
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        DBUG_PRINT ("SPMDPM", ("next pm"));
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
