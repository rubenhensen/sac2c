/**
 *
 * $Id: $
 *
 * @file removespmd.c
 *
 * This traversal removes spmd blocks. Depending on whether the spmd
 * block was conditional or not, this results in either just the contents
 * of the spmd block's Region, or a real conditional calling either the
 * Region or the Sequential version of the code. Either way the spmd
 * block is gone afterwards.
 */

#include "removespmd.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "shape.h"
#include "new_types.h"
#include "DataFlowMask.h"

/*
 * INFO structure
 */
struct INFO {
    node *fundef;
    node *preassign;
};

/*
 * INFO macros
 */
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_PREASSIGN(n) ((n)->preassign)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDdoRemoveSpmdBlocks( node *syntax_tree)
 *
 * @brief starting point of Remove Spmd blocks traversal
 *
 * @param syntax_tree
 *
 * @return modified syntax_tree.
 *
 *****************************************************************************/
node *
RMSPMDdoRemoveSpmdBlocks (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("RMSPMDdoRemoveSpmdBlocks");

    info = MakeInfo ();

    global.valid_ssaform = FALSE;
    /*
     * Wrapper code is created in non-SSA form and later on transformed into
     * SSA form using the standard transformation modules lac2fun and
     * ssa_transform. Therefore, we adjust the global control flag.
     */

    TRAVpush (TR_rmspmd);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * Remove Spmd blocks traversal traversal ( rmspmd_tab)
 *
 * prefix: RMSPMD
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
RMSPMDmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMSPMDmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
RMSPMDfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMSPMDfundef");

    if (FUNDEF_ISSTFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Only ST funs may contain SPMD blocks.
         */
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    if (FUNDEF_DFM_BASE (arg_node) != NULL) {
        FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDassign( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
RMSPMDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RMSPMDassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PREASSIGN (arg_info) != NULL) {
        arg_node = TCappendAssign (INFO_PREASSIGN (arg_info), arg_node);
        INFO_PREASSIGN (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMSPMDspmd( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
RMSPMDspmd (node *arg_node, info *arg_info)
{
    node *letap;
    node *predavis;
    node *pred, *thenblock, *elseblock;

    DBUG_ENTER ("RMSPMDspmd");

    if (SPMD_COND (arg_node) == NULL) {
        /*
         * The spmd block is unconditional.
         * We replace the entire block by the application of the corresponding
         * spmd function.
         */
        letap = ASSIGN_INSTR (BLOCK_INSTR (SPMD_REGION (arg_node)));

        DBUG_ASSERT (NODE_TYPE (letap) == N_let,
                     "First assignment in spmd block is *not* application of spmd "
                     "function.");

        DBUG_ASSERT (NODE_TYPE (LET_EXPR (letap)) == N_ap,
                     "First assignment in spmd block is *not* application of spmd "
                     "function.");

        DBUG_ASSERT (FUNDEF_ISSPMDFUN (AP_FUNDEF (LET_EXPR (letap))),
                     "First assignment in spmd block is *not* application of spmd "
                     "function.");

        SPMD_REGION (arg_node) = TBmakeBlock (TBmakeEmpty (), NULL);
        /*
         * We must restore a correct N_spmd node for later de-allocation.
         */

        arg_node = FREEdoFreeNode (arg_node);

        arg_node = letap;
    } else {
        predavis = TBmakeAvis (ILIBtmpVar (),
                               TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (predavis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        INFO_PREASSIGN (arg_info)
          = TBmakeAssign (TBmakeLet (TBmakeIds (predavis, NULL), SPMD_COND (arg_node)),
                          NULL);
        SPMD_COND (arg_node) = NULL;

        pred = TBmakeId (predavis);

        thenblock = SPMD_REGION (arg_node);
        elseblock = SPMD_SEQUENTIAL (arg_node);

        SPMD_REGION (arg_node) = TBmakeBlock (TBmakeEmpty (), NULL);
        /*
         * We must restore a correct N_spmd node for later de-allocation.
         */

        SPMD_SEQUENTIAL (arg_node) = NULL;

        /*
         * Free spmd block
         */
        arg_node = FREEdoFreeNode (arg_node);

        /*
         * return conditional
         */
        arg_node = TBmakeCond (pred, thenblock, elseblock);
    }

    DBUG_RETURN (arg_node);
}
