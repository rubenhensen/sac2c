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
 *   Upon encountering a withloop (N_with), we first traverse the withops.
 *   Each of the withop nodes then decides whether paralellizing it is
 *   worthwile, not worthwile, or unknown.
 *
 *   Depending on the result of this decision, the withop is placed inside
 *   a N_spmd block: This is done for positive and unknown decisions, and
 *   not done for negative decisions. The spmd block has three sons: Cond,
 *   Region and Sequential:
 *     - Cond is the condition upon which the spmd block is to be executed
 *       in parallel. This only exists if the decision is unknown.
 *     - Region is the parallel version of the withloop.
 *     - Sequential is the sequential version of the withloop. This only
 *       exists if the decision is unknown.
 *
 *   BEFORE:
 *     N_assign -> N_let -> N_ids ...
 *        |          |
 *        |          \----> N_with2  -> ....
 *     N_assign...
 *
 *   AFTER:
 *     N_assign -> N_spmd -> N_expr
 *        |          |
 *        |          |-----> N_block -> N_assign -> N_let -> N_ids ...
 *        |          |                                |
 *        |          \-----> N_block -> ...           \----> N_with2
 *     N_assign...
 *
 *   This traversal also makes three dataflow masks for every spmd block
 *   it creates: IN, OUT and LOCAL. All variables in withloop-containing
 *   functions are set in these dataflow masks. These masks are later used
 *   by spmd_lift to lift the spmd Region into a separate function, to
 *   determine function args, rets and vardecs respectively.
 *
 *****************************************************************************/

#include "spmd_init.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "DataFlowMask.h"
#include "globals.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"

/**
 * State used for withop traversal
 */
enum {
    WITHOPMODE_DECIDE,  /* Decide whether this withloop is parallellizable */
    WITHOPMODE_DATAFLOW /* Collect dataflow information */
};

/**
 * INFO structure
 */

struct INFO {
    node *fundef;
    node *let;
    node *letids;
    bool parallelize;
    bool belowwith;
    bool maypar;
    bool isworth;
    node *condition;
    node *sequential;
    int withopmode;

    dfmask_t *in_mask;
    dfmask_t *out_mask;
    dfmask_t *local_mask;
};

/**
 * INFO macros
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LET(n) (n->let)
#define INFO_LETIDS(n) (n->letids)
#define INFO_PARALLELIZE(n) (n->parallelize)
#define INFO_BELOWWITH(n) (n->belowwith)
#define INFO_ISWORTH(n) (n->isworth)
#define INFO_MAYPAR(n) (n->maypar)
#define INFO_CONDITION(n) (n->condition)
#define INFO_SEQUENTIAL(n) (n->sequential)
#define INFO_WITHOPMODE(n) (n->withopmode)

#define INFO_IN_MASK(n) (n->in_mask)
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

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LET (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_PARALLELIZE (result) = FALSE;
    INFO_BELOWWITH (result) = FALSE;
    INFO_ISWORTH (result) = FALSE;
    INFO_MAYPAR (result) = FALSE;
    INFO_CONDITION (result) = NULL;
    INFO_SEQUENTIAL (result) = NULL;
    INFO_WITHOPMODE (result) = WITHOPMODE_DECIDE;

    INFO_IN_MASK (result) = NULL;
    INFO_OUT_MASK (result) = NULL;
    INFO_LOCAL_MASK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

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

    if ((global.mtmode == MT_createjoin) || (global.mtmode == MT_startstop)) {
        info = MakeInfo ();

        TRAVpush (TR_spmdi);
        syntax_tree = TRAVdo (syntax_tree, info);
        TRAVpop ();

        info = FreeInfo (info);
    }

    DBUG_RETURN (syntax_tree);
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

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#ifdef BEMT

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
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
 *   Then in SPMD_IN/OUT/LOCAL the in/out/local-vars of the
 *   SPMD-region are stored.
 *
 ******************************************************************************/

node *
SPMDIassign (node *arg_node, info *arg_info)
{
    node *new_instr;

    DBUG_ENTER ("SPMDIassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_CONDITION (arg_info) != NULL) {
        new_instr
          = TBmakeCond (INFO_CONDITION (arg_info),
                        TBmakeBlock (TBmakeAssign (ASSIGN_INSTR (arg_node), NULL), NULL),
                        TBmakeBlock (TBmakeAssign (INFO_SEQUENTIAL (arg_info), NULL),
                                     NULL));
        ASSIGN_INSTR (arg_node) = new_instr;

        INFO_CONDITION (arg_info) = NULL;
        INFO_SEQUENTIAL (arg_info) = NULL;
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

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETIDS (arg_info) = NULL;

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

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIid");

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
    DBUG_ENTER ("SPMDIwith2");

    INFO_MAYPAR (arg_info) = TRUE;
    INFO_ISWORTH (arg_info) = FALSE;
    INFO_CONDITION (arg_info) = NULL;

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    if (INFO_MAYPAR (arg_info)) {
        if (INFO_ISWORTH (arg_info)) {
            WITH2_MT (arg_node) = TRUE;
        } else {
            if (INFO_CONDITION (arg_info) != NULL) {
                INFO_SEQUENTIAL (arg_info)
                  = TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                               DUPdoDupTree (arg_node));

                WITH2_MT (arg_node) = TRUE;
            }
        }
    } else {
        /* Traverse code to find nested withloops which we may want
         * to parallellize */
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
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

    if (global.no_fold_parallel) {
        /*
         * We decided not to parallelize fold-with-loops.
         * Therefore, we stop traversal of with-ops, reset the flags and delete
         * a potential conditions derived from a previous with-op.
         */
        INFO_MAYPAR (arg_info) = FALSE;
        INFO_ISWORTH (arg_info) = FALSE;
        if (INFO_CONDITION (arg_info) != NULL) {
            INFO_CONDITION (arg_info) = FREEdoFreeTree (INFO_CONDITION (arg_info));
        }
    } else {
        if (FOLD_NEXT (arg_node) != NULL) {
            INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
            FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
        } else {
            if (INFO_MAYPAR (arg_info) && !INFO_ISWORTH (arg_info)) {
                /*
                 * This with-loop only consists of fold operations. As long as we do
                 * not have a proper condition on fold-with-loops, we parallelize
                 * always.
                 */
                INFO_ISWORTH (arg_info) = TRUE;
            }
        }
    }

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
    int size;
    bool size_static;
    node *arg1, *arg2;

    DBUG_ENTER ("SPMDIgenarray");

    size_static = TUshapeKnown (IDS_NTYPE (INFO_LETIDS (arg_info)));

    if (size_static) {
        size = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LETIDS (arg_info))));
        if (size >= global.min_parallel_size) {
            /*
             * We statically know the size of the result array and its beyond the
             * threshold. We parallelize unconditionally and eliminate a condition
             * created before.
             */
            INFO_ISWORTH (arg_info) = TRUE;
            if (INFO_CONDITION (arg_info) != NULL) {
                INFO_CONDITION (arg_info) = FREEdoFreeTree (INFO_CONDITION (arg_info));
            }
        } else {
            /*
             * We statically know the size of the result array and its *not* beyond
             * the threshold.
             */
            if (INFO_ISWORTH (arg_info)) {
                /*
                 * We previously considered the with-loop to be worth parallelization.
                 * We stick to this initial decision.
                 */
            } else {
                /*
                 * We now know that the with-loop is not worth parallelization.
                 * So, we eliminate a potential condition.
                 */
                if (INFO_CONDITION (arg_info) != NULL) {
                    INFO_CONDITION (arg_info)
                      = FREEdoFreeTree (INFO_CONDITION (arg_info));
                }
            }
        }
    } else {
        /*
         * We do not know the size statically.
         * Nevertheless, we do not construct a condition here. We first continue
         * the with-op traversal because we may find a modarray with-loop, which
         * allows us to build a simpler condition.
         */
    }

    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    if (!size_static && (INFO_CONDITION (arg_info) == NULL)) {
        /*
         * We would like to give this with-loop a try and obviously there have been
         * no modarray with-ops. So we must build a genarray parallelization criterion.
         */

        if (NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id) {
            arg1 = TBmakeId (ID_AVIS (GENARRAY_SHAPE (arg_node)));
        } else {
            arg1 = DUPdoDupNode (GENARRAY_SHAPE (arg_node));
        }

        if (NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id) {
            arg2 = TBmakeId (ID_AVIS (GENARRAY_DEFAULT (arg_node)));
        } else {
            arg2 = DUPdoDupNode (GENARRAY_DEFAULT (arg_node));
        }

        INFO_CONDITION (arg_info) = TCmakePrf2 (F_run_mt_genarray, arg1, arg2);
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
    node *arg;
    int size;

    DBUG_ENTER ("SPMDImodarray");

    if (TUshapeKnown (IDS_NTYPE (INFO_LETIDS (arg_info)))) {
        size = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LETIDS (arg_info))));
        if (size >= global.min_parallel_size) {
            /*
             * We statically know the size of the result array and its beyond the
             * threshold. We parallelize unconditionally and eliminate a condition
             * created before.
             */
            INFO_ISWORTH (arg_info) = TRUE;
            if (INFO_CONDITION (arg_info) != NULL) {
                INFO_CONDITION (arg_info) = FREEdoFreeTree (INFO_CONDITION (arg_info));
            }
        } else {
            /*
             * We statically know the size of the result array and its *not* beyond
             * the threshold.
             */
            if (INFO_ISWORTH (arg_info)) {
                /*
                 * We previously considered the with-loop to be worth parallelization.
                 * We stick to this initial decision.
                 */
            } else {
                /*
                 * We now know that the with-loop is not worth parallelization.
                 * So, we eliminate a potential condition.
                 */
                if (INFO_CONDITION (arg_info) != NULL) {
                    INFO_CONDITION (arg_info)
                      = FREEdoFreeTree (INFO_CONDITION (arg_info));
                }
            }
        }
    } else {
        /*
         * We do not know the size statically. So, we give it a try and generate
         * a condition if no condition exists so far. Otherwise, we stick to the
         * existing condition.
         */
        if (INFO_CONDITION (arg_info) == NULL) {
            arg = TBmakeId (ID_AVIS (MODARRAY_ARRAY (arg_node)));
            INFO_CONDITION (arg_info) = TCmakePrf1 (F_run_mt_modarray, arg);
        }
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

#else

/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/
/*****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {
        if (FUNDEF_DFM_BASE (arg_node) != NULL) {
            FUNDEF_DFM_BASE (arg_node) = DFMremoveMaskBase (FUNDEF_DFM_BASE (arg_node));
        }

        FUNDEF_DFM_BASE (arg_node)
          = DFMgenMaskBase (FUNDEF_ARGS (arg_node), FUNDEF_VARDEC (arg_node));
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
 *   Then in SPMD_IN/OUT/LOCAL the in/out/local-vars of the
 *   SPMD-region are stored.
 *
 ******************************************************************************/

node *
SPMDIassign (node *arg_node, info *arg_info)
{
    node *spmd;

    DBUG_ENTER ("SPMDIassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_PARALLELIZE (arg_info) && !INFO_BELOWWITH (arg_info)) {
        spmd
          = TBmakeSpmd (INFO_CONDITION (arg_info),
                        TBmakeBlock (TBmakeAssign (ASSIGN_INSTR (arg_node), NULL), NULL),
                        TBmakeBlock (TBmakeAssign (INFO_SEQUENTIAL (arg_info), NULL),
                                     NULL));

        INFO_CONDITION (arg_info) = NULL;
        INFO_SEQUENTIAL (arg_info) = NULL;
        INFO_PARALLELIZE (arg_info) = FALSE;

        SPMD_IN (spmd) = INFO_IN_MASK (arg_info);
        SPMD_OUT (spmd) = INFO_OUT_MASK (arg_info);
        SPMD_LOCAL (spmd) = INFO_LOCAL_MASK (arg_info);

        INFO_IN_MASK (arg_info) = NULL;
        INFO_OUT_MASK (arg_info) = NULL;
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

    if (LET_IDS (arg_node) != NULL) {
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

    if (INFO_BELOWWITH (arg_info)) {

        DBUG_ASSERT (INFO_IN_MASK (arg_info) != NULL,
                     "IN mask missing in arg_info node.");
        DBUG_ASSERT (INFO_OUT_MASK (arg_info) != NULL,
                     "OUT mask missing in arg_info node.");
        DBUG_ASSERT (INFO_LOCAL_MASK (arg_info) != NULL,
                     "LOCAL mask missing in arg_info node.");

        DFMsetMaskEntrySet (INFO_LOCAL_MASK (arg_info), NULL, IDS_AVIS (arg_node));
    } else {
        if (INFO_OUT_MASK (arg_info) != NULL) {
            DFMsetMaskEntrySet (INFO_OUT_MASK (arg_info), NULL, IDS_AVIS (arg_node));
        }
    }

    if (IDS_NEXT (arg_node) != NULL) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SPMDIid( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
SPMDIid (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SPMDIid");

    if (INFO_BELOWWITH (arg_info)) {

        DBUG_ASSERT (INFO_IN_MASK (arg_info) != NULL,
                     "IN mask missing in arg_info node.");
        DBUG_ASSERT (INFO_OUT_MASK (arg_info) != NULL,
                     "OUT mask missing in arg_info node.");
        DBUG_ASSERT (INFO_LOCAL_MASK (arg_info) != NULL,
                     "LOCAL mask missing in arg_info node.");

        if (!DFMtestMaskEntry (INFO_LOCAL_MASK (arg_info), NULL, ID_AVIS (arg_node))) {
            DFMsetMaskEntrySet (INFO_IN_MASK (arg_info), NULL, ID_AVIS (arg_node));
        }
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
    DBUG_ENTER ("SPMDIwith2");

    if (INFO_BELOWWITH (arg_info)) {
        /*
         * We are already below a parallel with-loop. Hence, we only collect
         * data flow information.
         */

        int oldmode = INFO_WITHOPMODE (arg_info);
        INFO_WITHOPMODE (arg_info) = WITHOPMODE_DATAFLOW;

        WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

        INFO_WITHOPMODE (arg_info) = oldmode;
    } else {

        INFO_MAYPAR (arg_info) = TRUE;
        INFO_ISWORTH (arg_info) = FALSE;
        INFO_CONDITION (arg_info) = NULL;
        INFO_LETIDS (arg_info) = LET_IDS (INFO_LET (arg_info));

        INFO_WITHOPMODE (arg_info) = WITHOPMODE_DECIDE;
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

        if (INFO_MAYPAR (arg_info)) {

            if (INFO_ISWORTH (arg_info) || (INFO_CONDITION (arg_info) != NULL)) {
                INFO_PARALLELIZE (arg_info) = TRUE;
                INFO_SEQUENTIAL (arg_info) = DUPdoDupTree (INFO_LET (arg_info));

                WITH2_MT (arg_node) = TRUE;

                INFO_IN_MASK (arg_info)
                  = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));
                INFO_OUT_MASK (arg_info)
                  = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));
                INFO_LOCAL_MASK (arg_info)
                  = DFMgenMaskClear (FUNDEF_DFM_BASE (INFO_FUNDEF (arg_info)));

                INFO_BELOWWITH (arg_info) = TRUE;
                INFO_WITHOPMODE (arg_info) = WITHOPMODE_DATAFLOW;

                WITH2_WITHID (arg_node) = TRAVdo (WITH2_WITHID (arg_node), arg_info);
                WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
                WITH2_SEGS (arg_node) = TRAVdo (WITH2_SEGS (arg_node), arg_info);
                /* Traverse withops again to collect data-flow information. */
                WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

                INFO_WITHOPMODE (arg_info) = WITHOPMODE_DECIDE;
                INFO_BELOWWITH (arg_info) = FALSE;
            }
        } else {
            /* Traverse code to find nested withloops which we may want
             * to parallellize */
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        }
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

    switch (INFO_WITHOPMODE (arg_info)) {
    case WITHOPMODE_DECIDE:
        if (global.no_fold_parallel) {
            /*
             * We decided not to parallelize fold-with-loops.
             * Therefore, we stop traversal of with-ops, reset the flags and delete
             * a potential conditions derived from a previous with-op.
             */
            INFO_MAYPAR (arg_info) = FALSE;
            INFO_ISWORTH (arg_info) = FALSE;
            if (INFO_CONDITION (arg_info) != NULL) {
                INFO_CONDITION (arg_info) = FREEdoFreeTree (INFO_CONDITION (arg_info));
            }
        } else {
            if (FOLD_NEXT (arg_node) != NULL) {
                INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
                FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
            } else {
                if (INFO_MAYPAR (arg_info) && !INFO_ISWORTH (arg_info)) {
                    /*
                     * This with-loop only consists of fold operations. As long as we do
                     * not have a proper condition on fold-with-loops, we parallelize
                     * always.
                     */
                    INFO_ISWORTH (arg_info) = TRUE;
                }
            }
        }
        break;

    case WITHOPMODE_DATAFLOW:
        if (FOLD_NEUTRAL (arg_node) != NULL) {
            FOLD_NEUTRAL (arg_node) = TRAVdo (FOLD_NEUTRAL (arg_node), arg_info);
        }
        if (FOLD_NEXT (arg_node) != NULL) {
            INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
            FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
        }
        break;

    default:
        DBUG_ASSERT (FALSE, "Illegal withop-mode");
    }

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
    int size;
    bool size_static;
    node *arg1, *arg2;

    DBUG_ENTER ("SPMDIgenarray");

    switch (INFO_WITHOPMODE (arg_info)) {
    case WITHOPMODE_DECIDE:
        size_static = TUshapeKnown (IDS_NTYPE (INFO_LETIDS (arg_info)));

        if (size_static) {
            size = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LETIDS (arg_info))));
            if (size >= global.min_parallel_size) {
                /*
                 * We statically know the size of the result array and its beyond the
                 * threshold. We parallelize unconditionally and eliminate a condition
                 * created before.
                 */
                INFO_ISWORTH (arg_info) = TRUE;
                if (INFO_CONDITION (arg_info) != NULL) {
                    INFO_CONDITION (arg_info)
                      = FREEdoFreeTree (INFO_CONDITION (arg_info));
                }
            } else {
                /*
                 * We statically know the size of the result array and its *not* beyond
                 * the threshold.
                 */
                if (INFO_ISWORTH (arg_info)) {
                    /*
                     * We previously considered the with-loop to be worth parallelization.
                     * We stick to this initial decision.
                     */
                } else {
                    /*
                     * We now know that the with-loop is not worth parallelization.
                     * So, we eliminate a potential condition.
                     */
                    if (INFO_CONDITION (arg_info) != NULL) {
                        INFO_CONDITION (arg_info)
                          = FREEdoFreeTree (INFO_CONDITION (arg_info));
                    }
                }
            }
        } else {
            /*
             * We do not know the size statically.
             * Nevertheless, we do not construct a condition here. We first continue
             * the with-op traversal because we may find a modarray with-loop, which
             * allows us to build a simpler condition.
             */
        }

        if (GENARRAY_NEXT (arg_node) != NULL) {
            INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }

        if (!size_static && (INFO_CONDITION (arg_info) == NULL)) {
            /*
             * We would like to give this with-loop a try and obviously there have been
             * no modarray with-ops. So we must build a genarray parallelization
             * criterion.
             */

            if (NODE_TYPE (GENARRAY_SHAPE (arg_node)) == N_id) {
                arg1 = TBmakeId (ID_AVIS (GENARRAY_SHAPE (arg_node)));
            } else {
                arg1 = DUPdoDupNode (GENARRAY_SHAPE (arg_node));
            }

            if (NODE_TYPE (GENARRAY_DEFAULT (arg_node)) == N_id) {
                arg2 = TBmakeId (ID_AVIS (GENARRAY_DEFAULT (arg_node)));
            } else {
                arg2 = DUPdoDupNode (GENARRAY_DEFAULT (arg_node));
            }

            INFO_CONDITION (arg_info) = TCmakePrf2 (F_run_mt_genarray, arg1, arg2);
        }
        break;

    case WITHOPMODE_DATAFLOW:
        if (GENARRAY_SHAPE (arg_node) != NULL) {
            GENARRAY_SHAPE (arg_node) = TRAVdo (GENARRAY_SHAPE (arg_node), arg_info);
        }
        if (GENARRAY_DEFAULT (arg_node) != NULL) {
            GENARRAY_DEFAULT (arg_node) = TRAVdo (GENARRAY_DEFAULT (arg_node), arg_info);
        }
        if (GENARRAY_NEXT (arg_node) != NULL) {
            INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
            GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
        }
        break;

    default:
        DBUG_ASSERT (FALSE, "Illegal withop-mode");
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
    node *arg;
    int size;

    DBUG_ENTER ("SPMDImodarray");

    switch (INFO_WITHOPMODE (arg_info)) {
    case WITHOPMODE_DECIDE:
        if (TUshapeKnown (IDS_NTYPE (INFO_LETIDS (arg_info)))) {
            size = SHgetUnrLen (TYgetShape (IDS_NTYPE (INFO_LETIDS (arg_info))));
            if (size >= global.min_parallel_size) {
                /*
                 * We statically know the size of the result array and its beyond the
                 * threshold. We parallelize unconditionally and eliminate a condition
                 * created before.
                 */
                INFO_ISWORTH (arg_info) = TRUE;
                if (INFO_CONDITION (arg_info) != NULL) {
                    INFO_CONDITION (arg_info)
                      = FREEdoFreeTree (INFO_CONDITION (arg_info));
                }
            } else {
                /*
                 * We statically know the size of the result array and its *not* beyond
                 * the threshold.
                 */
                if (INFO_ISWORTH (arg_info)) {
                    /*
                     * We previously considered the with-loop to be worth parallelization.
                     * We stick to this initial decision.
                     */
                } else {
                    /*
                     * We now know that the with-loop is not worth parallelization.
                     * So, we eliminate a potential condition.
                     */
                    if (INFO_CONDITION (arg_info) != NULL) {
                        INFO_CONDITION (arg_info)
                          = FREEdoFreeTree (INFO_CONDITION (arg_info));
                    }
                }
            }
        } else {
            /*
             * We do not know the size statically. So, we give it a try and generate
             * a condition if no condition exists so far. Otherwise, we stick to the
             * existing condition.
             */
            if (INFO_CONDITION (arg_info) == NULL) {
                arg = TBmakeId (ID_AVIS (MODARRAY_ARRAY (arg_node)));
                INFO_CONDITION (arg_info) = TCmakePrf1 (F_run_mt_modarray, arg);
            }
        }
        break;

    case WITHOPMODE_DATAFLOW:
        if (MODARRAY_ARRAY (arg_node) != NULL) {
            MODARRAY_ARRAY (arg_node) = TRAVdo (MODARRAY_ARRAY (arg_node), arg_info);
        }
        break;

    default:
        DBUG_ASSERT (FALSE, "Illegal withop-mode");
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#endif
