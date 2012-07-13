/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @file cost_model.c
 *
 *  @brief
 *
 *  In this first step towards generating multithreaded code, we essentially
 *  aim at categorising with-loops into three kinds: those that will be
 *  parallelised, those that will not be parallelised and those that may either
 *  run in parallel or sequentially. All with-loops to be parallelised are
 *  marked with the flag MT. All with-loops for which the parallelisation
 *  decision must be postponed until runtime are embedded within a conditional
 *  that has a predicate depending on the with-loop operator and both an
 *  MT-tagged and a non-tagged version of the with-loop in each branch.
 *
 *  The cost model is rather simple as it does not take the actual code into
 *  account, but rather the size of the iteration space. This is deduced
 *  from the operators. The traversal explicitly takes multi-operator
 *  with-loops into account such that it looks for the most suitable operator
 *  to base the decision on. Those are in descending order modarray, genarray
 *  and fold.
 *
 *****************************************************************************/

#include "cost_model.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "globals.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "memory.h"

/**
 * INFO structure
 */

struct INFO {
    node *fundef;
    node *letids;
    bool maypar;
    bool isworth;
    node *condition;
    node *sequential;
    node *vardecs;
    node *topmostblock;
};

/**
 * INFO macros
 */

#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_ISWORTH(n) (n->isworth)
#define INFO_MAYPAR(n) (n->maypar)
#define INFO_CONDITION(n) (n->condition)
#define INFO_SEQUENTIAL(n) (n->sequential)
#define INFO_VARDECS(n) (n->vardecs)
#define INFO_TOPMOSTBLOCK(n) (n->topmostblock)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_ISWORTH (result) = FALSE;
    INFO_MAYPAR (result) = FALSE;
    INFO_CONDITION (result) = NULL;
    INFO_SEQUENTIAL (result) = NULL;
    INFO_VARDECS (result) = NULL;
    INFO_TOPMOSTBLOCK (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn MTCMdoRunCostModel
 *
 *  @brief initiates the cost model traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTCMdoRunCostModel (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    info = MakeInfo ();

    TRAVpush (TR_mtcm);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTCMmodule( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *MTCMblock(node *arg_node, info *arg_info)
 *
 ******************************************************************************/

node *
MTCMblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TOPMOSTBLOCK (arg_info) == NULL) {
        INFO_TOPMOSTBLOCK (arg_info) = arg_node;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    if (INFO_TOPMOSTBLOCK (arg_info) == arg_node) {
        if (INFO_VARDECS (arg_info) != NULL) {
            BLOCK_VARDECS (arg_node)
              = TCappendVardec (BLOCK_VARDECS (arg_node), INFO_VARDECS (arg_info));

            INFO_VARDECS (arg_info) = NULL;
        }
        INFO_TOPMOSTBLOCK (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTCMfundef( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
 *   node *MTCMassign( node *arg_node, info *arg_info)
 *
 * description:
 *   Generates a SPMD-region for each first level with-loop.
 *   Then in SPMD_IN/OUT/LOCAL the in/out/local-vars of the
 *   SPMD-region are stored.
 *
 ******************************************************************************/

node *
MTCMassign (node *arg_node, info *arg_info)
{
    node *parblock, *seqblock;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_CONDITION (arg_info) != NULL) {
        parblock = TBmakeBlock (TBmakeAssign (ASSIGN_STMT (arg_node), NULL), NULL);
        BLOCK_ISMTPARALLELBRANCH (parblock) = TRUE;

        seqblock = TBmakeBlock (TBmakeAssign (INFO_SEQUENTIAL (arg_info), NULL), NULL);
        BLOCK_ISMTSEQUENTIALBRANCH (seqblock) = TRUE;

        ASSIGN_STMT (arg_node)
          = TBmakeCond (INFO_CONDITION (arg_info), parblock, seqblock);

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
 * @fn node *MTCMlet( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTCMwith2( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_MAYPAR (arg_info) = TRUE;
    INFO_ISWORTH (arg_info) = FALSE;
    INFO_CONDITION (arg_info) = NULL;

    WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);

    if (INFO_MAYPAR (arg_info)) {
        if (INFO_ISWORTH (arg_info)) {
            WITH2_PARALLELIZE (arg_node) = TRUE;
        } else {
            if (INFO_CONDITION (arg_info) != NULL) {
                INFO_SEQUENTIAL (arg_info)
                  = TBmakeLet (DUPdoDupTree (INFO_LETIDS (arg_info)),
                               DUPdoDupTree (arg_node));

                WITH2_PARALLELIZE (arg_node) = TRUE;
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
 * @fn node *MTCMwith( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTCMfold( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
 * @fn node *MTCMgenarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMgenarray (node *arg_node, info *arg_info)
{
    int size;
    bool size_static;

    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {
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

            INFO_CONDITION (arg_info)
              = TCmakePrf2 (F_run_mt_genarray, DUPdoDupNode (GENARRAY_MEM (arg_node)),
                            TBmakeNum (global.min_parallel_size));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MTCMmodarray( node *arg_node, info *arg_info)
 *
 *****************************************************************************/

node *
MTCMmodarray (node *arg_node, info *arg_info)
{
    int size;

    DBUG_ENTER ();

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
             * We statically know the size of the result array and it is *not* beyond
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
            INFO_CONDITION (arg_info)
              = TCmakePrf2 (F_run_mt_modarray, DUPdoDupNode (MODARRAY_MEM (arg_node)),
                            TBmakeNum (global.min_parallel_size));
        }
    }

    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
