/*****************************************************************************
 *
 * $Id$
 *
 * file:   sync_opt.c
 *
 * prefix: SYNCO
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   optimize synchronisation blocks, i.e. adjacent synchronisation blocks
 *   are merged into a single one where data dependencies do not require
 *   a barrier synchronisation in between.
 *
 *   It implements the traversal functions of *syncopt_tab*.
 *
 *****************************************************************************/

#include "tree_basic.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "sync_opt.h"
#include "ctinfo.h"
#include "dbug.h"
#include "internal_lib.h"

/*
 * INFO structure
 */

struct INFO {
    node *thisassign;
    node *nextassign;
};

/* INFO macros */
#define INFO_THISASSIGN(n) (n->thisassign)
#define INFO_NEXTASSIGN(n) (n->nextassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_THISASSIGN (result) = NULL;
    INFO_NEXTASSIGN (result) = NULL;

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
 *
 * @fn SYNCOdoSyncOpt
 *
 *  @brief
 *
 *  @param syntax_tree
 *
 *  @return
 *
 *****************************************************************************/
node *
SYNCOdoSyncOpt (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SYNCOdoSyncOpt");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

    info = MakeInfo ();

    TRAVpush (TR_synco);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *SYNCOfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
SYNCOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SYNCOfundef");

    if ((FUNDEF_BODY (arg_node) != NULL) && (!FUNDEF_ISFOLDFUN (arg_node))
        && (FUNDEF_ISSPMDFUN (arg_node))) {
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   bool Disjoint (dfmask_t mask1, dfmask_t mask2)
 *
 * description:
 *   Tests whether both masks are disjoint (return TRUE) or not (return FALSE).
 *
 ******************************************************************************/
static bool
Disjoint (dfmask_t *mask1, dfmask_t *mask2)
{
    bool result;
    dfmask_t *andmask;

    DBUG_ENTER ("Disjoint");

    andmask = DFMgenMaskAnd (mask1, mask2);
    if (DFMtestMask (andmask) > 0) {
        result = FALSE;
    } else {
        result = TRUE;
    }
    DFMremoveMask (andmask);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   bool MeltableSYNCs (node *first_sync, node *second_sync)
 *
 * description:
 *   Tests whether the two syncs specified can be melted together, i.e. whether
 *   their dataflow masks (except for the two IN-masks) are disjunctive and do
 *   not contain too much fold-with-loops if limited by max_sync_fold.
 *
 *   Returns TRUE if both blocks can be melted together, FALSE otherwise.
 *
 ******************************************************************************/
static bool
MeltableSYNCs (node *first_sync, node *second_sync)
{
    bool result;

    DBUG_ENTER ("MeltableSYNCs");

    DBUG_ASSERT (NODE_TYPE (first_sync) == N_sync, ("first_sync not a N_sync"));
    DBUG_ASSERT (NODE_TYPE (second_sync) == N_sync, ("second_sync not a N_sync"));

    result = TRUE;
    result = result & Disjoint (SYNC_IN (first_sync), SYNC_INOUT (second_sync));
    result = result & Disjoint (SYNC_IN (first_sync), SYNC_OUT (second_sync));
    result = result & Disjoint (SYNC_INOUT (first_sync), SYNC_IN (second_sync));
    result = result & Disjoint (SYNC_INOUT (first_sync), SYNC_INOUT (second_sync));
    result = result & Disjoint (SYNC_INOUT (first_sync), SYNC_OUT (second_sync));
    result = result & Disjoint (SYNC_OUT (first_sync), SYNC_IN (second_sync));
    result = result & Disjoint (SYNC_OUT (first_sync), SYNC_INOUT (second_sync));
    result = result & Disjoint (SYNC_OUT (first_sync), SYNC_OUT (second_sync));
    result = result & Disjoint (SYNC_OUTREP (first_sync), SYNC_INOUT (second_sync));

    if (result) {
        DBUG_PRINT ("SYNCO", ("disjoint"));
    } else {
        DBUG_PRINT ("SYNCO", ("non-disjoint"));
    }

    if (result) {
        if (global.max_sync_fold == -1) {
            /* auto-inferation */
            global.needed_sync_fold
              = MAX (global.needed_sync_fold,
                     SYNC_FOLDCOUNT (first_sync) + SYNC_FOLDCOUNT (second_sync));
            DBUG_PRINT ("SYNCO", ("folds ok (auto-inferation"));
        } else {
            DBUG_PRINT ("SYNCO",
                        ("foldcounts are: %i + %i = %i", SYNC_FOLDCOUNT (first_sync),
                         SYNC_FOLDCOUNT (second_sync),
                         SYNC_FOLDCOUNT (first_sync) + SYNC_FOLDCOUNT (second_sync)));
            /* limited by max_sync_fold */
            if ((SYNC_FOLDCOUNT (first_sync) + SYNC_FOLDCOUNT (second_sync))
                <= global.max_sync_fold) {
                DBUG_PRINT ("SYNCO", ("folds ok (<= global.max_sync_fold %i)",
                                      global.max_sync_fold));
            } else {
                result = FALSE;
                DBUG_PRINT ("SYNCO", ("too much folds (global.max_sync_fold %i reached)",
                                      global.max_sync_fold));
                CTIwarnLine (NODE_LINE (second_sync),
                             "Maximum number of fold-with-loops per sync-block (%i) "
                             "reached",
                             global.max_sync_fold);
            }
        }
    }
    DBUG_RETURN (result);
}

/******************************** **********************************************
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
    DBUG_ENTER ("AssertSimpleBlock");

    DBUG_ASSERT (NODE_TYPE (block) == N_block, "Wrong NODE_TYPE, not a N_block");

    DBUG_ASSERT (BLOCK_VARDEC (block) == NULL, "BLOCK_VARDEC not NULL");
    DBUG_ASSERT (BLOCK_SPMD_PROLOG_ICMS (block) == NULL, "BLOCK_SPMD_... not NULL");
    DBUG_ASSERT (BLOCK_CACHESIM (block) == NULL, "BLOCK_CACHESIM not NULL");

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
 *   node *MeltBlocks (node *first_block, node *second_block)
 *
 * description:
 *   Melts to N_blocks to one.
 *
 *   If the normal version is used the first entry is reused, and the second
 *   one is given free. That means both arguments are modified!
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

    FREEdoFreeTree (second_block);
    result = first_block;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *MeltSYNCs (node *first_sync, node *second_sync)
 *
 * description:
 *   Melts sync-blocks with sideeffects. Builds one new sync-block from the
 *   two sync-blocks handed over. Both arguments are overwritten or destroyed.
 *
 * attention:
 *   - Both SYNCs have to be meltable, or this function will fail!!!
 *     Test if meltable before calling this function!
 *   - There are some sideeffects!
 *     first_sync will be used to build the result, while second_sync will be
 *     freed. Use MeltSYNCsOnCopies (see below) to avoid this. That function
 *     first copies both arguments, so no sideeffects occur to the outside.
 *
 * remark:
 *   A version without sideeffects is also available
 *   (see MeltSYNCsOnCopies below).
 *
 ******************************************************************************/
node *
MeltSYNCs (node *first_sync, node *second_sync)
{
    node *result; /* result value of this function */

    DBUG_ENTER ("MeltSYNCs");

    DBUG_ASSERT ((NODE_TYPE (first_sync) == N_sync), "First argument not a N_sync!");
    DBUG_ASSERT ((NODE_TYPE (second_sync) == N_sync), "Second argument not a N_sync!");

    DBUG_ASSERT (MeltableSYNCs (first_sync, second_sync),
                 "sync-blocks overhanded are not meltable");

    /*
     *  Combine the mask to new ones.
     */
    SYNC_REGION (first_sync)
      = MeltBlocks (SYNC_REGION (first_sync), SYNC_REGION (second_sync));
    SYNC_REGION (second_sync) = NULL;

    /*
     *  Melt masks of used variables
     */

    DBUG_PRINT ("SYNCO", ("melting masks now ... "));

    DFMsetMaskOr (SYNC_IN (first_sync), SYNC_IN (second_sync));
    DFMsetMaskOr (SYNC_INOUT (first_sync), SYNC_INOUT (second_sync));
    DFMsetMaskOr (SYNC_OUT (first_sync), SYNC_OUT (second_sync));
    DFMsetMaskOr (SYNC_LOCAL (first_sync), SYNC_LOCAL (second_sync));

    SYNC_FOLDCOUNT (first_sync)
      = SYNC_FOLDCOUNT (first_sync) + SYNC_FOLDCOUNT (second_sync);

    FREEdoFreeTree (second_sync);

    result = first_sync;
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *MeltSYNCsOnCopies (node *first_sync, node *second_sync)
 *
 * description:
 *   Same functinality as MeltSYNCs, but the arguments are not touched, because
 *   they are copied before actual melting starts.
 *
 * remarks:
 *   A simple version without any copying is also available
 *   (see MeltSYNCs above).
 *
 ******************************************************************************/
node *
MeltSYNCsOnCopies (node *first_sync, node *second_sync)
{
    node *result;

    DBUG_ENTER ("MeltSYNCsOnCopies");

    first_sync = DUPdoDupTree (first_sync);
    second_sync = DUPdoDupTree (second_sync);

    result = MeltSYNCs (first_sync, second_sync);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SYNCOsync( node *arg_node, info *arg_info)
 *
 * description:
 *   Stops on an sync-block, ans melts all *directly* following sync-blocks
 *   with this one.
 *
 ******************************************************************************/
node *
SYNCOsync (node *arg_node, info *arg_info)
{
    node *result;

    DBUG_ENTER ("SYNCOsync");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_sync), "Wrong NODE_TYPE: N_sync expected");
    DBUG_ASSERT ((arg_info != NULL), ("arg_info is NULL"));
    DBUG_ASSERT ((arg_node == ASSIGN_INSTR (INFO_THISASSIGN (arg_info))),
                 "arg_node differs from thisasssign");

    result = arg_node;
    /*
     *  while this SYNC-block is followed directly (!) by another SYNC-block
     *  and both blocks are meltable, i.e. their masks are disjunctive,
     *  blocks are melted together here.
     */
    DBUG_PRINT ("SYNCO", ("try melting sync-blocks"));
    while ((INFO_NEXTASSIGN (arg_info) != NULL)
           && (NODE_TYPE (ASSIGN_INSTR (INFO_NEXTASSIGN (arg_info))) == N_sync)
           && (MeltableSYNCs (arg_node, ASSIGN_INSTR (INFO_NEXTASSIGN (arg_info))))) {
        DBUG_PRINT ("SYNCO", ("melting sync-blocks"));
        /*
         *  The actual optimazation of SYNC-blocks takes place here.
         */
        result = MeltSYNCs (arg_node, ASSIGN_INSTR (INFO_NEXTASSIGN (arg_info)));

        DBUG_PRINT ("SYNCO", ("rearranging assigments around sync-blocks"));
        /*
         *  Rearrange the pointers between the two assigments around the assignment
         *  who's spmd-block (ASSIGN_INSTR) has been deleted by melting.
         *  Cut of connections between assignment and the block and delete it.
         */
        ASSIGN_NEXT (INFO_THISASSIGN (arg_info))
          = ASSIGN_NEXT (INFO_NEXTASSIGN (arg_info));
        ASSIGN_NEXT (INFO_NEXTASSIGN (arg_info)) = NULL;
        ASSIGN_INSTR (INFO_NEXTASSIGN (arg_info)) = NULL;

        FREEdoFreeTree (INFO_NEXTASSIGN (arg_info));

        INFO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (INFO_THISASSIGN (arg_info));
    } /* while */

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SYNCOassign (node* arg_node, info *arg_info)
 *
 * description:
 *   Traversal of N_assign during SYNCO.
 *
 *   Sets some values at arg_info. If no arg_info exists one is created.
 *   The values set are THISASSIGN and NEXTASSIGN, they have to be corrected
 *   be outer routines if the configuration of assignments is changed.
 *   Especially when N_spmds (under a N_assign-node) are deleted (and also
 *   the N_assign-node).
 *
 ******************************************************************************/
node *
SYNCOassign (node *arg_node, info *arg_info)
{
    node *old_thisassign = NULL;
    node *old_nextassign = NULL;

    DBUG_ENTER ("SYNCOassign");

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_assign, "Wrong NODE_TYPE");

    old_thisassign = INFO_THISASSIGN (arg_info);
    old_nextassign = INFO_NEXTASSIGN (arg_info);

    INFO_THISASSIGN (arg_info) = arg_node;
    INFO_NEXTASSIGN (arg_info) = ASSIGN_NEXT (arg_node);

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    /*
     *  NEXT might have changed during traversal of instruction!
     */
    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    INFO_THISASSIGN (arg_info) = old_thisassign;
    INFO_NEXTASSIGN (arg_info) = old_nextassign;

    DBUG_RETURN (arg_node);
}
