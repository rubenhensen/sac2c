/*
 *
 * $Log$
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

/******************************************************************************
 *
 * function:
 *   int WithLoopIsWorthParallelExecution(node *wl)
 *
 * description:
 *
 *   This function decides whether a with-loop is actually to be executed
 *   in parallel. This is necessary because for small with-loops the most
 *   efficient way of execution is just sequential.
 *
 ******************************************************************************/

static int
WithLoopIsWorthParallelExecution (node *wl, ids *let_var)
{
    int res, i, size, target_dim;

    DBUG_ENTER ("WithLoopIsWorthParallelExecution");

    if ((NWITHOP_TYPE (NWITH2_WITHOP (wl)) == WO_foldfun)
        || (NWITHOP_TYPE (NWITH2_WITHOP (wl)) == WO_foldprf)) {
        res = 1;
    } else {
        target_dim = VARDEC_DIM (IDS_VARDEC (let_var));
        if (target_dim > 0) {
            size = 1;
            for (i = 0; i < target_dim; i++) {
                size *= VARDEC_SHAPE (IDS_VARDEC (let_var), i);
            }
            if (size < min_parallel_size) {
                res = 0;
            } else {
                res = 1;
            }
        } else {
            res = 1;
        }
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
        && WithLoopIsWorthParallelExecution (LET_EXPR (spmd_let), LET_IDS (spmd_let))) {

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

        SPMD_IN (spmd) = DFMGenMaskCopy (NWITH2_IN (with));
        SPMD_INOUT (spmd) = DFMGenMaskCopy (NWITH2_INOUT (with));
        SPMD_OUT (spmd) = DFMGenMaskCopy (NWITH2_OUT (with));
        SPMD_LOCAL (spmd) = DFMGenMaskCopy (NWITH2_LOCAL (with));

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
