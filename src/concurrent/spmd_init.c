/*
 *
 * $Log$
 * Revision 2.3  1999/07/21 16:30:27  jhs
 * needed_sync_fold introduced, max_sync_fold_adjusted.
 *
 * Revision 2.2  1999/06/25 15:36:33  jhs
 * Checked these in just to provide compileabilty.
 *
 * Revision 2.1  1999/02/23 12:44:12  sacbase
 * new release made
 *
 * Revision 1.3  1998/10/23 14:29:46  cg
 * added the new command line option -inparsize <no> which allows to
 * specify a minimum generator size for with-loops to be executed in
 * parallel if such execution is enabled.
 * The information stored by the global variable min_parallel_size.
 *
 * Revision 1.2  1998/07/03 10:20:52  cg
 * attribute attribute SPMD_INOUT_IDS no longer needed and set
 *
 * Revision 1.1  1998/06/18 14:35:53  cg
 * Initial revision
 *
 *
 */

/*****************************************************************************
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

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "globals.h"
#include "internal_lib.h"

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
 *   concurrently (by WithLoopIsAllowedConcurrentExecution).
 *
 ******************************************************************************/

static int
WithLoopIsWorthConcurrentExecution (node *withloop, ids *let_var)
{
    int res, i, size, target_dim;

    DBUG_ENTER ("WithLoopIsWorthConcurrentExecution");

    if ((NWITHOP_TYPE (NWITH2_WITHOP (withloop)) == WO_foldfun)
        || (NWITHOP_TYPE (NWITH2_WITHOP (withloop)) == WO_foldprf)) {
        res = TRUE;
    } else {
        target_dim = VARDEC_DIM (IDS_VARDEC (let_var));
        if (target_dim > 0) {
            size = 1;
            for (i = 0; i < target_dim; i++) {
                size *= VARDEC_SHAPE (IDS_VARDEC (let_var), i);
            }
            if (size < min_parallel_size) {
                res = FALSE;
            } else {
                res = TRUE;
            }
        } else {
            res = TRUE;
        }
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
 *   should follow test, whether the with-loop is worth to be executed
 *   concurrently (by WithLoopIsWorthConcurrentExecution).
 *
 ******************************************************************************/

static int
WithLoopIsAllowedConcurrentExecution (node *withloop)
{
    int res;
    node *withop;

    DBUG_ENTER ("WithLoopIsAllowedConcurrentExecution");

    withop = NWITH2_WITHOP (withloop);
    if ((NWITHOP_TYPE (withop) == WO_foldfun) || (NWITHOP_TYPE (withop) == WO_foldprf)) {
        if (max_sync_fold == 0) {
            res = FALSE;
        } else {
            res = TRUE;
        }
    } else {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SPMDIassign( node *arg_node, node *arg_info)
 *
 * description:
 *   Generates a SPMD-region for each first level with-loop.
 *   Then in SPMD_IN/OUT/INOUT/LOCAL the in/out/inout/local-vars of the
 *   SPMD-region are stored.
 *
 ******************************************************************************/

node *
SPMDIassign (node *arg_node, node *arg_info)
{
    node *with, *spmd_let, *spmd;

    DBUG_ENTER ("SPMDIassign");

    spmd_let = ASSIGN_INSTR (arg_node);

    /* contains the current assignment a with-loop?? */
    if ((NODE_TYPE (spmd_let) == N_let) && (NODE_TYPE (LET_EXPR (spmd_let)) == N_Nwith2)
        && WithLoopIsAllowedConcurrentExecution (LET_EXPR (spmd_let))
        && WithLoopIsWorthConcurrentExecution (LET_EXPR (spmd_let), LET_IDS (spmd_let))) {

        with = LET_EXPR (spmd_let);

        /*
         * current assignment contains a with-loop
         *  -> create a SPMD-region containing the current assignment only
         *      and insert it into the syntaxtree.
         */
        spmd = MakeSpmd (MakeBlock (MakeAssign (spmd_let, NULL), NULL));
        ASSIGN_INSTR (arg_node) = spmd;

        /*
         * get IN/INOUT/OUT/LOCAL from the N_Nwith2 node.
         */
        /*
            SPMD_IN   ( spmd) = DFMGenMaskCopy( NWITH2_IN   ( with));
            SPMD_INOUT( spmd) = DFMGenMaskCopy( NWITH2_INOUT( with));
            SPMD_OUT  ( spmd) = DFMGenMaskCopy( NWITH2_OUT  ( with));
            SPMD_LOCAL( spmd) = DFMGenMaskCopy( NWITH2_LOCAL( with));
        */
        SPMD_IN (spmd) = DFMGenMaskOr (NWITH2_IN (with), NWITH2_INOUT (with));
        SPMD_OUT (spmd) = DFMGenMaskOr (NWITH2_OUT (with), NWITH2_INOUT (with));
        SPMD_INOUT (spmd) = DFMGenMaskCopy (NWITH2_INOUT (with));
        SPMD_LOCAL (spmd) = DFMGenMaskCopy (NWITH2_LOCAL (with));
        /*
                                DFMGenMaskClear (FUNDEF_DFM_BASE( SPMD_FUNDEF(
           arg_node)));
        */
        SPMD_SHARED (spmd) = DFMGenMaskCopy (SPMD_LOCAL (spmd));
        DFMSetMaskMinus (SPMD_SHARED (spmd), SPMD_LOCAL (spmd));

        /*
         * we only traverse the following assignments to prevent nested
         *  SPMD-regions
         */
    } else {
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}
