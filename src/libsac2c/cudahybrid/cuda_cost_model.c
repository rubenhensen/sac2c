/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cucm CUDA Cost Model
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |  y  |  msd  | 15/2/2012
 * can be called on N_fundef               |   -----   |  n  |  msd  | 15/2/2012
 * expects LaC funs                        |   -----   |  y  |  msd  | 15/2/2012
 * follows N_ap to LaC funs                |   -----   |  n  |  msd  | 15/2/2012
 * =============================================================================
 * deals with GLF properly                 |    yes    |     |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |     |       |
 * utilises SAA annotations                |   -----   |  n  |  msd  | 15/2/2012
 * =============================================================================
 * tolerates flattened N_array             |    yes    |     |       |
 * tolerates flattened Generators          |    yes    |     |       |
 * tolerates flattened operation parts     |    yes    |     |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |     |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |  n  |  msd  | 15/2/2012
 * =============================================================================
 * </pre>
 *
 * @ingroup cudahybrid
 *
 * Prefix: CUCM
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file cuda_cost_model.c
 *
 * Implements a simple cost model for CUDA withloops, similar to what is done
 * for the multithread backend.
 * Withloops marked as cudarizable are unmarked should they fail any of the cost
 * criteria. At the moment, the only criteria is whether the iteration space
 * meets a minimum size requirement, which obviously can only be done here for
 * when iteration size is know statically.
 * Withloops that meet this criteria are then duplicated and a conditional is
 * formed, similar to what is done for the multithread cost model. The idea is
 * to keep a cudarized and uncudarized version of each withloop, as the
 * transformations each will incur are quite different.
 *
 *
 *****************************************************************************/
#include "cuda_cost_model.h"

#define DBUG_PREFIX "CUCM"
#include "debug.h"

#include "globals.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"
#include "memory.h"
#include "create_cond_fun.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    /* pointer to the current fundef, required for ssa duplication */
    node *fundef;
    /* ids of current withloop, although more than one probably breaks this */
    node *letids;
    /* is current withloop worth cudarization? */
    bool isworth;
    /* id of the condition variable */
    node *condition;
    /* uncudarized duplicate of a withloop */
    node *host;
    /* new conditional functions */
    node *lacfuns;
    /* assignment of the condition variable */
    node *preassigns;
    /* avis of the host branch withloop */
    node *elseavis;
};

/**
 * A template entry in the template info structure
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_ISWORTH(n) (n->isworth)
#define INFO_CONDITION(n) (n->condition)
#define INFO_HOST(n) (n->host)
#define INFO_LACFUNS(n) (n->lacfuns)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_ELSEAVIS(n) (n->elseavis)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_ISWORTH (result) = FALSE;
    INFO_CONDITION (result) = NULL;
    INFO_HOST (result) = NULL;
    INFO_LACFUNS (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_ELSEAVIS (result) = NULL;

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
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *CUCMdoCUDACostModel( node *syntax_tree)
 *
 *****************************************************************************/
node *
CUCMdoCUDACostModel (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();

    TRAVpush (TR_cucm);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ApplySizeCriterion(node *arg_node, info *arg_info)
 *
 * @brief Checks whether an array has the minimum shape size to be run on CUDA.
 *
 *   @param array_size  type of the array
 *   @return            FALSE if shape is not big enough, TRUE otherwise
 *
 *****************************************************************************/
static bool
ApplySizeCriterion (ntype *array_type)
{
    bool size_static, result;
    int size;

    DBUG_ENTER ();

    size_static = TUshapeKnown (array_type);

    if (size_static) {

        DBUG_PRINT ("Found with-loop with static shape.");

        size = SHgetUnrLen (TYgetShape (array_type));

        /* minimum size taken from optimal CUDA thread and block counts*/
        if (size >= global.optimal_threads) {
            DBUG_PRINT ("With-loop big enough, allowing cudarization.");
            result = TRUE;
        } else {
            DBUG_PRINT ("With-loop not big enough, uncudarizing.");
            result = FALSE;
        }
    } else {
        DBUG_PRINT ("Found with-loop without static shape.");
        result = TRUE;
    }

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CUCMfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain, skipping sticky functions.
 *        Current function node is saved in info structure.
 *
 *****************************************************************************/
node *
CUCMfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* During the main traversal, we only look at non-prelude functions */
    if (!FUNDEF_ISSTICKY (arg_node) && FUNDEF_BODY (arg_node) != NULL) {
        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMmodule(node *arg_node, info *arg_info)
 *
 * @brief Appends newly created conditional functions to current module.
 *
 *****************************************************************************/
node *
CUCMmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt (MODULE_FUNS (arg_node), arg_info);

    /* Append all newly created lac functions to the module */
    if (INFO_LACFUNS (arg_info) != NULL) {
        MODULE_FUNS (arg_node)
          = TCappendFundef (MODULE_FUNS (arg_node), INFO_LACFUNS (arg_info));
        INFO_LACFUNS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMassign(node *arg_node, info *arg_info)
 *
 * @brief For each withloop, create the conditional function with data from
 *        info structure and replace assignment with application of new
 *        function. If the condition requires some pre-assignment statements,
 *        these are inserted before the withloop let.
 *
 *****************************************************************************/
node *
CUCMassign (node *arg_node, info *arg_info)
{
    node *host_assign, *cuda_assign, *assign_next, *preassign;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_CONDITION (arg_info) != NULL) {
        DBUG_PRINT ("Introducing conditional");

        host_assign = TBmakeAssign (INFO_HOST (arg_info), NULL);
        cuda_assign = TBmakeAssign (ASSIGN_STMT (arg_node), NULL);

        /* Probably will need to mark each branch in the future... */
        // BLOCK_ISCUDABRANCH( cuda_assign) = TRUE;
        // BLOCK_ISHOSTBRANCH( host_assign) = TRUE;

        assign_next = ASSIGN_NEXT (arg_node);
        /* Am I using this function right? Not much documentation... */
        arg_node
          = CCFdoCreateCondFun (INFO_FUNDEF (arg_info), cuda_assign, host_assign,
                                INFO_CONDITION (arg_info),
                                IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node))),
                                IDS_AVIS (LET_IDS (ASSIGN_STMT (arg_node))),
                                INFO_ELSEAVIS (arg_info), &INFO_LACFUNS (arg_info));
        ASSIGN_NEXT (arg_node) = assign_next;

        /*
         * if condition variable needs prior assignments,
         * insert them before new conditional
         */
        if (INFO_PREASSIGNS (arg_info) != NULL) {
            /* find end of assignments */
            for (preassign = INFO_PREASSIGNS (arg_info); ASSIGN_NEXT (preassign) != NULL;
                 preassign = ASSIGN_NEXT (preassign))
                ;
            ASSIGN_NEXT (preassign) = arg_node;
            arg_node = INFO_PREASSIGNS (arg_info);
        }

        INFO_CONDITION (arg_info) = NULL;
        INFO_HOST (arg_info) = NULL;
        INFO_PREASSIGNS (arg_info) = NULL;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMlet(node *arg_node, info *arg_info)
 *
 * @brief Saves let ids, just in case a with-loop comes up.
 *
 *****************************************************************************/
node *
CUCMlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LETIDS (arg_info) = LET_IDS (arg_node);

    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    INFO_LETIDS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMwith(node *arg_node, info *arg_info)
 *
 * @brief Looks at cudarizable with-loops. Depending on the operation, these
 *        will be classified as worth cudarization or not. If they end up being
 *        worth it, the with loop is duplicated with the duplicate marked as
 *        not cudarizable. Otherwise, we clear the cudarizable flag.
 *
 *****************************************************************************/
node *
CUCMwith (node *arg_node, info *arg_info)
{
    node *host_with, *dup_ids;

    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        DBUG_PRINT ("Found cudarizable with-loop.");

        INFO_ISWORTH (arg_info) = FALSE;
        INFO_CONDITION (arg_info) = NULL;

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        if (INFO_ISWORTH (arg_info)) {
            host_with = DUPdoDupTreeSsa (arg_node, INFO_FUNDEF (arg_info));
            /* unmark with-loop and each partition */
            WITH_CUDARIZABLE (host_with) = FALSE;
            WITH_PART (host_with) = TRAVopt (WITH_PART (host_with), arg_info);

            dup_ids = DUPdoDupTreeSsa (INFO_LETIDS (arg_info), INFO_FUNDEF (arg_info));
            INFO_ELSEAVIS (arg_info) = IDS_AVIS (dup_ids);
            INFO_HOST (arg_info) = TBmakeLet (dup_ids, host_with);
        } else {
            WITH_CUDARIZABLE (arg_node) = FALSE;
            WITH_PART (arg_node) = TRAVopt (WITH_PART (arg_node), arg_info);
        }

    } else {
        DBUG_PRINT ("Found non-cudarizable with-loop.");
        /* Traverse code to find nested withloops which we may want
         * to parallelize */
        /* Only applies to outer fold loops when cuda parallel folding off (?) */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMpart(node *arg_node, info *arg_info)
 *
 * @brief Clears the thread block shape property of non-cudarizable withloops'
 *        partitions.
 *
 *****************************************************************************/
node *
CUCMpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FREEdoFreeTree (PART_THREADBLOCKSHAPE (arg_node));
    PART_THREADBLOCKSHAPE (arg_node) = NULL;

    PART_NEXT (arg_node) = TRAVopt (PART_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMgenarray(node *arg_node, info *arg_info)
 *
 * @brief Run cost model on genarray withloops
 *
 *****************************************************************************/
node *
CUCMgenarray (node *arg_node, info *arg_info)
{
    node *new_avis, *new_rhs;

    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {

        INFO_ISWORTH (arg_info) = ApplySizeCriterion (IDS_NTYPE (INFO_LETIDS (arg_info)));

        /* flag set by helper function above */
        if (INFO_ISWORTH (arg_info) && (INFO_CONDITION (arg_info) == NULL)) {
            /* create avis of new condition variable */
            new_avis
              = TBmakeAvis (TRAVtmpVarName ("cucm"),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
            /* add new avis to variable declarations */
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            /*********
             * TODO: *
             *********
             * implement a dynamic cudarization criterion, using just TRUE for now
             **/
            new_rhs = TBmakeBool (TRUE);

            /* create assignment of new condition variable */
            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), new_rhs), NULL);
            INFO_CONDITION (arg_info) = new_avis;
        }
    }

    /* check next operation, although this probably breaks stuff... */
    if (GENARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        GENARRAY_NEXT (arg_node) = TRAVdo (GENARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMmodarray(node *arg_node, info *arg_info)
 *
 * @brief Run cost model on modarray withloops
 *
 *****************************************************************************/
node *
CUCMmodarray (node *arg_node, info *arg_info)
{
    node *new_avis, *new_rhs;

    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {

        INFO_ISWORTH (arg_info) = ApplySizeCriterion (IDS_NTYPE (INFO_LETIDS (arg_info)));

        /* flag set by helper function above */
        if (INFO_ISWORTH (arg_info) && (INFO_CONDITION (arg_info) == NULL)) {
            /* create avis of new condition variable */
            new_avis
              = TBmakeAvis (TRAVtmpVarName ("cucm"),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
            /* add new avis to variable declarations */
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            /*********
             * TODO: *
             *********
             * implement a dynamic cudarization criterion, using just TRUE for now
             **/
            new_rhs = TBmakeBool (TRUE);

            /* create assignment of new condition variable */
            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), new_rhs), NULL);
            INFO_CONDITION (arg_info) = new_avis;
        }
    }

    /* check next operation, although this probably breaks stuff... */
    if (MODARRAY_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        MODARRAY_NEXT (arg_node) = TRAVdo (MODARRAY_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CUCMfold(node *arg_node, info *arg_info)
 *
 * @brief Run cost model on fold withloops
 *
 *****************************************************************************/
node *
CUCMfold (node *arg_node, info *arg_info)
{
    node *new_avis, *new_rhs;

    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {

        /* Fold loops marked as cudarizable have size criteria applied implicitly
          later on */
        INFO_ISWORTH (arg_info) = TRUE;

        /* flag set by helper function above */
        if (INFO_ISWORTH (arg_info) && (INFO_CONDITION (arg_info) == NULL)) {
            /* create avis of new condition variable */
            new_avis
              = TBmakeAvis (TRAVtmpVarName ("cucm"),
                            TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
            /* add new avis to variable declarations */
            FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
              = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

            /*********
             * TODO: *
             *********
             * implement a dynamic cudarization criterion, using just TRUE for now
             **/
            new_rhs = TBmakeBool (TRUE);

            /* create assignment of new condition variable */
            INFO_PREASSIGNS (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), new_rhs), NULL);
            INFO_CONDITION (arg_info) = new_avis;
        }
    }

    /* check next operation, although this probably breaks stuff... */
    if (FOLD_NEXT (arg_node) != NULL) {
        INFO_LETIDS (arg_info) = IDS_NEXT (INFO_LETIDS (arg_info));
        FOLD_NEXT (arg_node) = TRAVdo (FOLD_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- CUDA Cost Model -->
 *****************************************************************************/
