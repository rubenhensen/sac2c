/*
 *
 * $Log$
 * Revision 3.13  2004/11/24 19:29:17  skt
 * Compiler Switch during SACDevCampDK 2k4
 *
 * Revision 3.12  2004/11/21 17:32:02  skt
 * make it runable with the new info structure
 *
 * Revision 3.11  2004/10/12 09:59:15  khf
 * initialised a variable to please the compiler
 *
 * Revision 3.10  2004/10/07 15:35:31  khf
 * added support for multioperator WLs
 *
 * Revision 3.9  2004/09/28 16:33:12  ktr
 * cleaned up concurrent (removed everything not working / not working with emm)
 *
 * Revision 3.8  2004/09/28 14:09:59  ktr
 * removed old refcount and generatemasks
 *
 * Revision 3.7  2004/09/18 16:05:48  ktr
 * DFMs are adjusted differently in EMM because memory is allocated explicitly.
 *
 * Revision 3.6  2001/03/05 16:41:53  dkr
 * no macros NWITH???_IS_FOLD used
 *
 * Revision 3.5  2000/12/12 12:11:29  dkr
 * NWITH_INOUT removed
 * interpretation of NWITH_IN changed:
 * the LHS of a with-loop assignment is now longer included in
 * NWITH_IN!!!
 *
 * Revision 3.4  2000/12/07 12:57:09  dkr
 * nothing changed
 *
 * Revision 3.3  2000/12/07 08:54:08  cg
 * Bug fixed in initialization of some variables after
 * warning elimination.
 *
 * Revision 3.2  2000/12/06 19:22:16  cg
 * Removed compiler warnings in production mode.
 *
 * Revision 3.1  2000/11/20 18:02:29  sacbase
 * new release made
 *
 * Revision 2.9  2000/02/22 11:36:00  jhs
 * Adapted NODE_TEXT.
 *
 * Revision 2.8  2000/02/02 12:28:38  jhs
 * Fixed compare.
 *
 * Revision 2.7  1999/08/27 11:56:12  jhs
 * Brushed code. Added handling of do loops. Corrected pulling spmd-blocks
 * over while-loops.
 *
 * Revision 2.6  1999/08/09 11:32:20  jhs
 * Cleaned up info-macros for concurrent-phase.
 *
 * Revision 2.5  1999/08/05 13:36:25  jhs
 * Added optimization of sequential assignments between spmd-blocks, main work
 * happens in spmdinit and ist steered by OPT_MTI (default now: off), some
 * traversals were needed and added in spmd_trav.
 *
 * Revision 2.4  1999/07/28 13:05:52  jhs
 * SPMD_INs are now created only from NWITH2_INs, but not NWITH2_INOUTs anymore.
 *
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
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "DataFlowMask.h"
#include "globals.h"
#include "spmd_trav.h"
#include "concurrent_info.h"

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
/* TODO - make it runnable with AUD and AKD */
static bool
WithLoopIsWorthConcurrentExecution (node *withloop, node *let_var)
{
    node *withop;
    bool res;
    int i, size, target_dim;

    DBUG_ENTER ("WithLoopIsWorthConcurrentExecution");

    res = FALSE;
    withop = WITH2_WITHOP (withloop);
    while (let_var != NULL) {
        if (NODE_TYPE (withop) == N_fold) {
            res = TRUE;
        } else {
            target_dim = VARDEC_DIM (IDS_VARDEC (let_var));
            if (target_dim > 0) {
                size = 1;
                for (i = 0; i < target_dim; i++) {
                    size *= VARDEC_SHAPE (IDS_VARDEC (let_var), i);
                }
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

node *
InsertSPMD (node *assign, node *fundef)
{
    node *instr;
    node *spmd;
    node *with;
    node *newassign;
    node *with_ids;
    int varno;

    DBUG_ENTER ("InsertSPMD");

    DBUG_ASSERT ((NODE_TYPE (assign) == N_assign), ("N_assign expected"));
    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef), ("N_fundef expected"));

    instr = ASSIGN_INSTR (assign);

    /*
     *  - insert spmd between assign and instruction
     *  - delete nested N_spmds
     */
    newassign = TBmakeAssign (instr, NULL);
    spmd = TBmakeSpmd (TBmakeBlock (newassign, NULL));
    DBUG_PRINT ("SPMDI", ("before delete nested"));
    spmd = SPMDDNdoDeleteNested (spmd);
    DBUG_PRINT ("SPMDI", ("after delete nested"));
    ASSIGN_INSTR (assign) = spmd;
    varno = FUNDEF_VARNO (fundef);

    if (NODE_TYPE (instr) == N_let) {
        if (NODE_TYPE (LET_EXPR (instr)) == N_with2) {
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
                with_ids = IDS_NEXT (with_ids);
            }

        } else {
            /* #### ins outs missing ... */
            SPMD_IN (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_OUT (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_INOUT (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_LOCAL (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_SHARED (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));

            DBUG_PRINT ("SPMDI", ("call pm let not with"));
            SPMDPMdoProduceMasks (spmd, spmd, fundef);
            DBUG_PRINT ("SPMDI", ("leave pm let not with"));
        }
    } else {
        /* #### ins outs missing ... */
        SPMD_IN (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_OUT (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_INOUT (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_LOCAL (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_SHARED (spmd) = DFMgenMaskClear (FUNDEF_DFM_BASE (fundef));

        DBUG_PRINT ("SPMDI", ("call pm not let"));
        SPMDPMdoProduceMasks (spmd, spmd, fundef);
        DBUG_PRINT ("SPMDI", ("leave pm not let"));
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

        arg_node = InsertSPMD (arg_node, INFO_CONC_FUNDEF (arg_info));
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
