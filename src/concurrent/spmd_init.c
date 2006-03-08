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
};

/**
 * INFO macros
 */
#define INFO_FUNDEF(n) (n->fundef)

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
 *   int WithLoopIsWorthConcurrentExecution(node *wl, ids *let_var)
 *
 * description:
 *   This function decides whether a with-loop is actually worth to be executed
 *   concurrenly. This is necessary because for small with-loops the most
 *   efficient way of execution is just sequential.
 *
 * attention:
 *   Each test whether a with-loop is worth to be executed concurrently
 *   has to follow a test, whether the with-loop is allowed to be executed
 *   concurrently (by WithLoopIsAllowedConcurrentExecution, see below).
 *
 ******************************************************************************/
static bool
WithLoopIsWorthConcurrentExecution (node *withloop, node *let_var)
{
    node *withop;
    bool res;
    int size;

    DBUG_ENTER ("WithLoopIsWorthConcurrentExecution");

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
 *   int WithLoopIsAllowedConcurrentExecution(node *withloop)
 *
 * description:
 *   This function decides whether a with-loop is actually allowed to be
 *   executed concurrently.
 *
 * attention:
 *   Each test whether a with-loop is allowed to be executed concurrently
 *   should follow a test, whether the with-loop is worth to be executed
 *   concurrently (by WithLoopIsWorthConcurrentExecution, above).
 *
 ******************************************************************************/

static bool
WithLoopIsAllowedConcurrentExecution (node *withloop)
{
    node *withop;
    bool res = TRUE;

    DBUG_ENTER ("WithLoopIsAllowedConcurrentExecution");

    withop = WITH2_WITHOP (withloop);
    while (withop != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            if (global.max_sync_fold == 0) {
                res = FALSE;
                break;
            }
        }
        withop = WITHOP_NEXT (withop);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *InsertSPMD (node *assign, node *fundef)
 *
 * description:
 *   Inserts an spmd-block between assign and it's instruction (ASSIGN_INSTR),
 *   all the masks are inferred and attached to this new spmd-block.
 *
 ******************************************************************************/
static node *
InsertSPMD (node *assign, node *fundef)
{
    node *instr;
    node *spmd;
    node *with;
    node *newassign;
    node *with_ids;

    DBUG_ENTER ("InsertSPMD");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), ("N_assign expected"));
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), ("N_fundef expected"));
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_INSTR (assign)) == N_let), "N_let expected");
    DBUG_ASSERT ((NODE_TYPE (ASSIGN_RHS (assign)) == N_with2), "N_with2 expected");

    instr = ASSIGN_INSTR (assign);

    /*
     *  - insert spmd between assign and instruction
     *  - delete nested N_spmds
     */
    newassign = TBmakeAssign (instr, NULL);

#if 0
  spmd = TBmakeSpmd( TBmakeBlock( newassign, NULL));
  DBUG_PRINT( "SPMDI", ("before delete nested"));
  spmd = SPMDDNdoDeleteNested (spmd);
  DBUG_PRINT( "SPMDI", ("after delete nested"));
#endif

    ASSIGN_INSTR (assign) = spmd;

    /*
     * current assignment contains a with-loop
     *  -> create a SPMD-region containing the current assignment only
     *      and insert it into the syntaxtree.
     */

    /*
     * get masks from the N_with2 node.
     */
    with = LET_EXPR (instr);
    SPMD_IN (spmd) = DFMgenMaskCopy (WITH2_IN_MASK (with));
    SPMD_OUT (spmd) = DFMgenMaskCopy (WITH2_OUT_MASK (with));
    SPMD_INOUT (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
    SPMD_LOCAL (spmd) = DFMgenMaskCopy (WITH2_LOCAL_MASK (with));
    SPMD_SHARED (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));

    /*
     * add vars from LHS of with-loop assignment
     */
    with_ids = LET_IDS (instr);
    while (with_ids != NULL) {
        DFMsetMaskEntrySet (SPMD_OUT (spmd), NULL, IDS_AVIS (with_ids));
        AVIS_SSAASSIGN (IDS_AVIS (with_ids)) = newassign;
        with_ids = IDS_NEXT (with_ids);
    }

    DBUG_PRINT ("SPMDI", ("inserted new spmd-block"));

    DBUG_RETURN (assign);
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
    node *spmd_let;

    DBUG_ENTER ("SPMDIassign");

    spmd_let = ASSIGN_INSTR (arg_node);

    /*
     *  Contains the current assignment a with-loop that should be executed
     *  concurrently??
     */
    if ((NODE_TYPE (spmd_let) == N_let) && (NODE_TYPE (LET_EXPR (spmd_let)) == N_with2)
        && WithLoopIsAllowedConcurrentExecution (LET_EXPR (spmd_let))
        && WithLoopIsWorthConcurrentExecution (LET_EXPR (spmd_let), LET_IDS (spmd_let))) {

        arg_node = InsertSPMD (arg_node, INFO_FUNDEF (arg_info));
        DBUG_PRINT ("SPMDI", ("inserted spmd"));

    } else if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
               || (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return)) {
        DBUG_PRINT ("SPMDI",
                    ("ignoring traversal of %s", NODE_TEXT (ASSIGN_INSTR (arg_node))));
    } else {
        DBUG_PRINT ("SPMDI", ("traverse into instruction %s",
                              NODE_TEXT (ASSIGN_INSTR (arg_node))));

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        DBUG_PRINT ("SPMDI", ("traverse from instruction %s",
                              NODE_TEXT (ASSIGN_INSTR (arg_node))));
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("SPMDI", ("turnaround"));
    }

    DBUG_RETURN (arg_node);
}
