/*
 *
 * $Log$
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
#include "my_debug.h"
#include "spmd_trav.h"
#include "generatemasks.h"
#include "spmd_opt.h"

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
 *   should follow a test, whether the with-loop is worth to be executed
 *   concurrently (by WithLoopIsWorthConcurrentExecution, above).
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

long *
DupMask_ (long *oldmask, int varno)
{
    long *result;

    DBUG_ENTER ("DupMask_");

    if (oldmask == NULL) {
        result = NULL;
    } else {
        result = DupMask (oldmask, varno);
    }

    DBUG_RETURN (result);
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
    int varno;

    DBUG_ENTER ("InsertSPMD");

    DBUG_ASSERT ((NODE_TYPE (assign) = N_assign), ("N_assign expected"));
    DBUG_ASSERT ((NODE_TYPE (fundef) = N_fundef), ("N_fundef expected"));

    instr = ASSIGN_INSTR (assign);

    /*
     *  - insert spmd between assign and instruction
     *  - delete nested N_spmds
     */
    newassign = MakeAssign (instr, NULL);
    spmd = MakeSpmd (MakeBlock (newassign, NULL));
    DBUG_PRINT ("SPMDI", ("before delete nested"));
    spmd = DeleteNested (spmd);
    DBUG_PRINT ("SPMDI", ("after delete nested"));
    ASSIGN_INSTR (assign) = spmd;
    varno = FUNDEF_VARNO (fundef);
    ASSIGN_DEFMASK (newassign) = DupMask_ (ASSIGN_DEFMASK (assign), varno);
    ASSIGN_USEMASK (newassign) = DupMask_ (ASSIGN_USEMASK (assign), varno);
    ASSIGN_MRDMASK (newassign) = DupMask_ (ASSIGN_MRDMASK (assign), varno);

    if (NODE_TYPE (instr) == N_let) {
        if (NODE_TYPE (LET_EXPR (instr)) == N_Nwith2) {
            /*
             * current assignment contains a with-loop
             *  -> create a SPMD-region containing the current assignment only
             *      and insert it into the syntaxtree.
             */

            /*
             * get masks from the N_Nwith2 node.
             */
            with = LET_EXPR (instr);
            SPMD_IN (spmd) = DFMGenMaskCopy (NWITH2_IN (with));
            SPMD_OUT (spmd) = DFMGenMaskOr (NWITH2_OUT (with), NWITH2_INOUT (with));
            SPMD_INOUT (spmd) = DFMGenMaskCopy (NWITH2_INOUT (with));
            SPMD_LOCAL (spmd) = DFMGenMaskCopy (NWITH2_LOCAL (with));
            SPMD_SHARED (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
        } else {
            /* #### ins outs missing ... */
            SPMD_IN (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_OUT (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_INOUT (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_LOCAL (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
            SPMD_SHARED (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));

            DBUG_PRINT ("SPMDI", ("call pm let not with"));
            ProduceMasks (spmd, spmd, fundef);
            DBUG_PRINT ("SPMDI", ("leave pm let not with"));
        }
    } else {
        /* #### ins outs missing ... */
        SPMD_IN (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_OUT (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_INOUT (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_LOCAL (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));
        SPMD_SHARED (spmd) = DFMGenMaskClear (FUNDEF_DFM_BASE (fundef));

        DBUG_PRINT ("SPMDI", ("call pm not let"));
        ProduceMasks (spmd, spmd, fundef);
        DBUG_PRINT ("SPMDI", ("leave pm not let"));
    }

    DBUG_PRINT ("SPMDI", ("inserted new spmd-block"));

    DBUG_RETURN (assign);
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
    node *spmd_let;
    int old_lastspmd;
    int old_nextspmd;
    nodetype old_context;
    int old_expandcontext;
    int old_expandstep;
    int pullable;
    nodetype dummy;
    node *block1;
    node *assign1;
    node *assign2;
    node *block2;
    node *assign3;

    DBUG_ENTER ("SPMDIassign");

    spmd_let = ASSIGN_INSTR (arg_node);

    /*
     *  Contains the current assignment a with-loop that should be executed
     *  concurrently??
     */
    if ((NODE_TYPE (spmd_let) == N_let) && (NODE_TYPE (LET_EXPR (spmd_let)) == N_Nwith2)
        && WithLoopIsAllowedConcurrentExecution (LET_EXPR (spmd_let))
        && WithLoopIsWorthConcurrentExecution (LET_EXPR (spmd_let), LET_IDS (spmd_let))) {

        arg_node = InsertSPMD (arg_node, INFO_CONC_FUNDEF (arg_info));
        DBUG_PRINT ("SPMDI", ("inserted spmd"));

    } else if ((NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)
               || (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_return)) {
        if (optimize & OPT_MTI) {
            /*
             *  Check if multithreaed execution would be possible ...
             */
            dummy = NODE_TYPE (ASSIGN_INSTR (arg_node));
            DBUG_PRINT ("SPMDI", ("border1"));
            if (dummy == N_let) {
                pullable = (!LetWithFunction (ASSIGN_INSTR (arg_node)));
            } else if (dummy == N_return) {
                pullable = FALSE;
            } else {
                pullable = FALSE;
            }

            DBUG_PRINT ("SPMDI", ("border2"));
            INFO_SPMDI_EXPANDSTEP (arg_info)
              = INFO_SPMDI_EXPANDSTEP (arg_info) && pullable;
            INFO_SPMDI_EXPANDCONTEXT (arg_info)
              = INFO_SPMDI_EXPANDCONTEXT (arg_info) && pullable;

            DBUG_PRINT ("SPMDI", ("ignoring traversal of %s; step %i; context %i",
                                  mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))],
                                  INFO_SPMDI_EXPANDSTEP (arg_info),
                                  INFO_SPMDI_EXPANDCONTEXT (arg_info)));
        } else {
            DBUG_PRINT ("SPMDI", ("ignoring traversal of %s",
                                  mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))]));
        } /* if (optimize & OPT_MTI) */
    } else {
        DBUG_PRINT ("SPMDI", ("traverse into instruction %s",
                              mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))]));

        if (optimize & OPT_MTI) {
            dummy = NODE_TYPE (ASSIGN_INSTR (arg_node));
            DBUG_ASSERT (((dummy == N_while) || (dummy = N_do)), "unexpected node-type");

            old_lastspmd = INFO_SPMDI_LASTSPMD (arg_info);
            old_nextspmd = INFO_SPMDI_NEXTSPMD (arg_info);
            old_context = INFO_SPMDI_CONTEXT (arg_info);
            old_expandcontext = INFO_SPMDI_EXPANDCONTEXT (arg_info);
            old_expandstep = INFO_SPMDI_EXPANDSTEP (arg_info);
            INFO_SPMDI_LASTSPMD (arg_info) = -1;
            INFO_SPMDI_NEXTSPMD (arg_info) = -1;
            INFO_SPMDI_CONTEXT (arg_info) = dummy;
            INFO_SPMDI_EXPANDCONTEXT (arg_info) = TRUE;
            INFO_SPMDI_EXPANDSTEP (arg_info) = FALSE;
        } else {
            dummy = N_empty;
        }

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        if (optimize & OPT_MTI) {
            if ((dummy == N_while) || (dummy == N_do)) {
                if ((INFO_SPMDI_EXPANDCONTEXT (arg_info))
                    && ((INFO_SPMDI_LASTSPMD (arg_info) != -1)
                        || (INFO_SPMDI_NEXTSPMD (arg_info) != -1))) {
                    /*
                     *  The complete interior of the while/do-loop can be executed
                     *  concurrently, so expansions over the while/do-loop happens now.
                     */
                    DBUG_PRINT ("SPMDI", ("EXPAND OVER WHILE!!!"));
                    /*
                     *  After last steps the while/do-loop contains only spmd-blocks
                     *  as assignments in it's first level. These are combined
                     *  to one spmd-block.
                     */
                    if (dummy == N_while) {
                        WHILE_BODY (ASSIGN_INSTR (arg_node))
                          = SPMDoptimize (WHILE_BODY (ASSIGN_INSTR (arg_node)),
                                          INFO_CONC_FUNDEF (arg_info));
                    } else {
                        /* N_do */
                        DO_BODY (ASSIGN_INSTR (arg_node))
                          = SPMDoptimize (DO_BODY (ASSIGN_INSTR (arg_node)),
                                          INFO_CONC_FUNDEF (arg_info));
                    }
                    assign1 = arg_node;
                    if (dummy == N_while) {
                        block1 = WHILE_BODY (ASSIGN_INSTR (assign1));
                    } else {
                        block1 = DO_BODY (ASSIGN_INSTR (assign1));
                    }
                    assign2 = BLOCK_INSTR (block1);
                    block2 = SPMD_REGION (ASSIGN_INSTR (assign2));
                    assign3 = BLOCK_INSTR (block2);
                    /*
                     *  The while/do-loop should contain only one spmd now,
                     *  that means assign2 has no next.
                     */
                    DBUG_ASSERT ((ASSIGN_NEXT (assign2) == NULL), ("what???"));
                    /*
                     *  spmd-block and while/do-loop are exchanged now.
                     *  Do not forget to change the next!
                     */
                    arg_node = assign2;
                    ASSIGN_NEXT (assign2) = ASSIGN_NEXT (assign1);
                    ASSIGN_NEXT (assign1) = NULL;
                    BLOCK_INSTR (block2) = assign1;
                    BLOCK_INSTR (block1) = assign3;
                }
            }

            INFO_SPMDI_LASTSPMD (arg_info) = old_lastspmd;
            INFO_SPMDI_NEXTSPMD (arg_info) = old_nextspmd;
            INFO_SPMDI_CONTEXT (arg_info) = old_context;
            INFO_SPMDI_EXPANDCONTEXT (arg_info)
              = old_expandcontext & INFO_SPMDI_EXPANDCONTEXT (arg_info);
            INFO_SPMDI_EXPANDSTEP (arg_info)
              = old_expandstep & INFO_SPMDI_EXPANDCONTEXT (arg_info);
        } /* if (optimize & OPT_MTI) */

        DBUG_PRINT ("SPMDI", ("traverse from instruction %s",
                              mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))]));
    }

    if (optimize & OPT_MTI) {
        if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_spmd) {
            old_lastspmd = INFO_SPMDI_LASTSPMD (arg_info);
            old_expandstep = INFO_SPMDI_EXPANDSTEP (arg_info);
            INFO_SPMDI_LASTSPMD (arg_info) = 0;
            INFO_SPMDI_EXPANDSTEP (arg_info) = TRUE;
        }
        if (INFO_SPMDI_LASTSPMD (arg_info) != -1) {
            INFO_SPMDI_LASTSPMD (arg_info) = INFO_SPMDI_LASTSPMD (arg_info) + 1;
        }
    }

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    } else {
        DBUG_PRINT ("SPMDI", ("turnaround"));
    }

    if (optimize & OPT_MTI) {
        if (INFO_SPMDI_LASTSPMD (arg_info) != -1) {
            INFO_SPMDI_LASTSPMD (arg_info) = INFO_SPMDI_LASTSPMD (arg_info) - 1;
        }
        if (INFO_SPMDI_NEXTSPMD (arg_info) != -1) {
            INFO_SPMDI_NEXTSPMD (arg_info) = INFO_SPMDI_NEXTSPMD (arg_info) + 1;
        }
        if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_spmd) {
            INFO_SPMDI_NEXTSPMD (arg_info) = 0;
        }

        DBUG_PRINT ("SPMDI",
                    ("context %s; instr %s; << %i; >> %i; step %i; context %i",
                     mdb_nodetype[INFO_SPMDI_CONTEXT (arg_info)],
                     mdb_nodetype[NODE_TYPE (ASSIGN_INSTR (arg_node))],
                     INFO_SPMDI_LASTSPMD (arg_info), INFO_SPMDI_NEXTSPMD (arg_info),
                     INFO_SPMDI_EXPANDSTEP (arg_info),
                     INFO_SPMDI_EXPANDCONTEXT (arg_info)));

        if (INFO_SPMDI_CONTEXT (arg_info) == N_fundef) {
            if ((INFO_SPMDI_LASTSPMD (arg_info) > 0)
                && (INFO_SPMDI_NEXTSPMD (arg_info) > 0)) {
                if ((INFO_SPMDI_LASTSPMD (arg_info) + INFO_SPMDI_NEXTSPMD (arg_info)
                     <= 12)) {
                    if (INFO_SPMDI_EXPANDSTEP (arg_info)) {
                        DBUG_PRINT ("SPMDI", ("  would expand to spmd here (normal)"));
                        arg_node = InsertSPMD (arg_node, INFO_CONC_FUNDEF (arg_info));
                        old_lastspmd = INFO_SPMDI_LASTSPMD (arg_info);
                        old_expandstep = INFO_SPMDI_EXPANDSTEP (arg_info);

                    } else {
                        DBUG_PRINT ("SPMDI", ("  cannot expand here (step forbidden)"));
                    }
                }
            }
        } else if ((INFO_SPMDI_CONTEXT (arg_info) == N_while)
                   || (INFO_SPMDI_CONTEXT (arg_info) == N_do)) {
            if ((INFO_SPMDI_LASTSPMD (arg_info) > 0)
                || (INFO_SPMDI_NEXTSPMD (arg_info) > 0)) {
                if (INFO_SPMDI_EXPANDSTEP (arg_info)) {
                    arg_node = InsertSPMD (arg_node, INFO_CONC_FUNDEF (arg_info));
                    old_lastspmd = INFO_SPMDI_LASTSPMD (arg_info);
                    old_expandstep = INFO_SPMDI_EXPANDSTEP (arg_info);
                    DBUG_PRINT ("SPMDI", ("  expand to spmd here (while-step)"));
                } else if (INFO_SPMDI_EXPANDCONTEXT (arg_info)) {
                    /*
                     *  surrounding context will be expanded, so this one will be
                     *  expanded anyway, so one could leave it on it's on, but
                     *  block-masks are needed for optimization.
                     */
                    arg_node = InsertSPMD (arg_node, INFO_CONC_FUNDEF (arg_info));
                    old_lastspmd = INFO_SPMDI_LASTSPMD (arg_info);
                    old_expandstep = INFO_SPMDI_EXPANDSTEP (arg_info);
                    DBUG_PRINT ("SPMDI", ("  expand to spmd here (while-context)"));
                }
            }
        }

        if (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_spmd) {
            INFO_SPMDI_LASTSPMD (arg_info) = old_lastspmd;
            INFO_SPMDI_EXPANDSTEP (arg_info) = old_expandstep;
        }
    } else {
        DBUG_PRINT ("SPMDI", ("no mti"));
    }

    DBUG_RETURN (arg_node);
}
