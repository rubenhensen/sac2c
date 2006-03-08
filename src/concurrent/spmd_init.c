/*****************************************************************************
 *
 * $Id$
 *
 * file:   spmd_init.c
 *
 * prefix: SPMDI
 *
 * description:
 *
 *   This file implements the traversal of a function body in order to
 *   enclose each with-loop suitable for non-sequential execution by
 *   an spmd-block.
 *
 *****************************************************************************/

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "globals.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "internal_lib.h"
#include "InferDFMs.h"

/**
 * INFO structure
 */

struct INFO {
    node *fundef;
    node *let;
    bool parallelize;
    bool hasfoldop;
    node *condition;
    node *sequential;

    dfmask_t *in_mask;
    dfmask_t *inout_mask;
    dfmask_t *out_mask;
    dfmask_t *local_mask;
};

/**
 * INFO macros
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LET(n) (n->let)
#define INFO_PARALLELIZE(n) (n->parallelize)
#define INFO_HASFOLDOP(n) (n->hasfoldop)
#define INFO_CONDITION(n) (n->condition)
#define INFO_SEQUENTIAL(n) (n->sequential)

#define INFO_IN_MASK(n) (n->in_mask)
#define INFO_INOUT_MASK(n) (n->inout_mask)
#define INFO_OUT_MASK(n) (n->out_mask)
#define INFO_LOCAL_MASK(n) (n->local_mask)

/**
 * INFO functions
 */

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_PARALLELIZE (result) = FALSE;
    INFO_HASFOLDOP (result) = FALSE;
    INFO_CONDITION (result) = NULL;
    INFO_SEQUENTIAL (result) = NULL;

    INFO_IN_MASK (result) = NULL;
    INFO_INOUT_MASK (result) = NULL;
    INFO_OUT_MASK (result) = NULL;
    INFO_LOCAL_MASK (result) = NULL;

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
 * @fn SPMDIdoSpmdInit
 *
 *  @brief
 *
 *  @param syntax_tree
 *
 *  @return
 *
 *****************************************************************************/

node *
SPMDIdoSpmdInit (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("SPMDIdoSpmdInit");

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!!!");

    info = MakeInfo ();

    TRAVpush (TR_spmdi);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   bool IsWorthParallel(node *wl, ids *let_var)
 *
 * description:
 *   This function decides whether a with-loop is actually worth to be executed
 *   concurrenly. This is necessary because for small with-loops the most
 *   efficient way of execution is just sequential.
 *
 ******************************************************************************/

static bool
IsWorthParallel (node *withloop, node *let_var)
{
    node *withop;
    bool res;
    int size;

    DBUG_ENTER ("IsWorthParallel");

    res = FALSE;
    withop = WITH2_WITHOP (withloop);
    while (let_var != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            res = TRUE;
        } else {
            if (TUshapeKnown (IDS_NTYPE (let_var))) {
                size = SHgetUnrLen (TYgetShape (IDS_NTYPE (let_var)));
                if (size < global.min_parallel_size) {
                    res = FALSE;
                } else {
                    res = TRUE;
                    break;
                }
            } else {
                res = TRUE;
                break;
            }
        }
        let_var = IDS_NEXT (let_var);
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   int MaybeWorthParallel(node *wl, ids *let_var)
 *
 * description:
 *
 *
 *
 *
 ******************************************************************************/

static node *
MaybeWorthParallel (node *withloop, node *let_var)
{
    DBUG_ENTER ("MaybeWorthParallel");

    DBUG_RETURN (NULL);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDImodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDImodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDImodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIfundef");

    if (!FUNDEF_ISMTFUN (arg_node) && (FUNDEF_BODY (arg_node) != NULL)) {
        /*
         * Infer data flow masks
         */
        arg_node = INFDFMSdoInferDfms (arg_node, HIDE_LOCALS_NEVER);

        INFO_FUNDEF (arg_info) = arg_node;

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
 *   node *SPMDIassign( node *arg_node, info *arg_info)
 *
 * description:
 *   Generates a SPMD-region for each first level with-loop.
 *   Then in SPMD_IN/OUT/INOUT/LOCAL the in/out/inout/local-vars of the
 *   SPMD-region are stored.
 *
 ******************************************************************************/

node *
SPMDIassign (node *arg_node, info *arg_info)
{
    node *spmd;

    DBUG_ENTER ("SPMDIassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PARALLELIZE (arg_info)) {
        spmd
          = TBmakeSpmd (INFO_CONDITION (arg_info),
                        TBmakeBlock (TBmakeAssign (ASSIGN_INSTR (arg_node), NULL), NULL),
                        INFO_SEQUENTIAL (arg_info));

        SPMD_IN (spmd) = INFO_IN_MASK (arg_info);
        SPMD_OUT (spmd) = INFO_OUT_MASK (arg_info);
        SPMD_INOUT (spmd) = INFO_INOUT_MASK (arg_info);
        SPMD_LOCAL (spmd) = INFO_LOCAL_MASK (arg_info);

        INFO_CONDITION (arg_info) = NULL;
        INFO_SEQUENTIAL (arg_info) = NULL;
        INFO_PARALLELIZE (arg_info) = FALSE;

        INFO_IN_MASK (arg_info) = NULL;
        INFO_OUT_MASK (arg_info) = NULL;
        INFO_INOUT_MASK (arg_info) = NULL;
        INFO_LOCAL_MASK (arg_info) = NULL;

        ASSIGN_INSTR (arg_node) = spmd;
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIlet");

    INFO_LET (arg_info) = arg_node;

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    if (INFO_PARALLELIZE (arg_info) && (LET_IDS (arg_node) != NULL)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    INFO_LET (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIids");

    DBUG_ASSERT (INFO_IN_MASK (arg_info) != NULL, "IN mask missing in arg_info node.");
    DBUG_ASSERT (INFO_INOUT_MASK (arg_info) != NULL,
                 "INOUT mask missing in arg_info node.");
    DBUG_ASSERT (INFO_OUT_MASK (arg_info) != NULL, "OUT mask missing in arg_info node.");
    DBUG_ASSERT (INFO_LOCAL_MASK (arg_info) != NULL,
                 "LOCAL mask missing in arg_info node.");

    DFMsetMaskEntrySet (INFO_OUT_MASK (arg_info), NULL, IDS_AVIS (arg_node));

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIwith2( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIwith2 (node *arg_node, info *arg_info)
{
    bool may_parallelize = TRUE;

    DBUG_ENTER ("SPMDIwith2");

    INFO_HASFOLDOP (arg_info) = FALSE;

    if (global.no_fold_parallel) {
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        if (INFO_HASFOLDOP (arg_info)) {
            may_parallelize = FALSE;
        }
    }

    if (may_parallelize) {

        INFO_PARALLELIZE (arg_info)
          = IsWorthParallel (arg_node, LET_IDS (INFO_LET (arg_info)));
        if (!INFO_PARALLELIZE (arg_info)) {
            INFO_CONDITION (arg_info)
              = MaybeWorthParallel (arg_node, LET_IDS (INFO_LET (arg_info)));
            if (INFO_CONDITION (arg_info) != NULL) {
                INFO_PARALLELIZE (arg_info) = TRUE;
                INFO_SEQUENTIAL (arg_info)
                  = TBmakeAssign (DUPdoDupTree (INFO_LET (arg_info)), NULL);
            }
        }
    }

    if (!INFO_PARALLELIZE (arg_info)) {
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
    } else if (INFO_CONDITION (arg_info) != NULL) {
        node *stack_condition = INFO_CONDITION (arg_info);

        INFO_PARALLELIZE (arg_info) = FALSE;
        INFO_CONDITION (arg_info) = NULL;

        INFO_SEQUENTIAL (arg_info) = TRAVdo (INFO_SEQUENTIAL (arg_info), arg_info);

        INFO_PARALLELIZE (arg_info) = TRUE;
        INFO_CONDITION (arg_info) = stack_condition;
    }

    if (INFO_PARALLELIZE (arg_info)) {
        INFO_IN_MASK (arg_info) = DFMgenMaskCopy (WITH2_IN_MASK (arg_node));
        INFO_INOUT_MASK (arg_info)
          = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));
        INFO_OUT_MASK (arg_info) = DFMgenMaskCopy (WITH2_OUT_MASK (arg_node));
        INFO_LOCAL_MASK (arg_info) = DFMgenMaskCopy (WITH2_LOCAL_MASK (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIfold( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIfold");

    INFO_HASFOLDOP (arg_info) = TRUE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIgenarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIgenarray");

    if (GENARRAY_NEXT (arg_node) != NULL) {
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDImodarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDImodarray (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDImodarray");

    if (MODARRAY_NEXT (arg_node) != NULL) {
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
