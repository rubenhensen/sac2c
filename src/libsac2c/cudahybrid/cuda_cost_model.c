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
#include "lac2fun.h"
#include "SSATransform.h"

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
    node *hostwl;
    /* assignment of the condition variable */
    node *preassigns;
    /* whether the current function will need to be converted to ssa again*/
    bool needsssa;
};

/**
 * A template entry in the template info structure
 */
#define INFO_FUNDEF(n) (n->fundef)
#define INFO_LETIDS(n) (n->letids)
#define INFO_ISWORTH(n) (n->isworth)
#define INFO_CONDITION(n) (n->condition)
#define INFO_HOSTWL(n) (n->hostwl)
#define INFO_PREASSIGNS(n) (n->preassigns)
#define INFO_NEEDSSSA(n) (n->needsssa)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_LETIDS (result) = NULL;
    INFO_ISWORTH (result) = FALSE;
    INFO_CONDITION (result) = NULL;
    INFO_HOSTWL (result) = NULL;
    INFO_PREASSIGNS (result) = NULL;
    INFO_NEEDSSSA (result) = FALSE;

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
 * @fn node *ApplySizeCriterion(ntype *array_type)
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
 *
 * @fn node *CreateConditionVariable(node *arg_node, info *arg_info)
 *
 * @brief Creates the condition variable if a withloop is worth cudarizing.
 *
 *   @param arg_info  info structure
 *   @return            FALSE if shape is not big enough, TRUE otherwise
 *
 *****************************************************************************/
static void
CreateConditionVariable (info *arg_info)
{
    node *new_avis, *new_rhs, *new_assign;
    static int counter = 0;

    DBUG_ENTER ();

    /* flag set by helper function above */
    if (INFO_ISWORTH (arg_info) && (INFO_CONDITION (arg_info) == NULL)) {
        /* create avis of new condition variable */
        new_avis = TBmakeAvis (TRAVtmpVarName (""),
                               TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0)));
        /* add new avis to variable declarations */
        FUNDEF_VARDECS (INFO_FUNDEF (arg_info))
          = TBmakeVardec (new_avis, FUNDEF_VARDECS (INFO_FUNDEF (arg_info)));

        new_rhs = TBmakePrf (F_is_cuda_thread, TBmakeExprs (TBmakeNum (counter), NULL));
        counter++;

        /* create assignment of new condition variable */
        new_assign = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL), new_rhs), NULL);
        INFO_PREASSIGNS (arg_info) = new_assign;
        AVIS_SSAASSIGN (new_avis) = new_assign; // SSA property

        INFO_CONDITION (arg_info) = new_avis;
    }

    DBUG_RETURN ();
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
        INFO_NEEDSSSA (arg_info) = FALSE;
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        if (INFO_NEEDSSSA (arg_info)) {
            arg_node = L2FdoLac2Fun (arg_node);
        }
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    node *cond_stmt, *cond_assign, *assign_next, *cuda_branch, *host_branch;

    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_CONDITION (arg_info) != NULL) {
        DBUG_PRINT ("Introducing conditional");
        assign_next = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;

        /* Create blocks for the cuda and host branches.
         Probably will need to mark each branch in the future... */
        cuda_branch = TBmakeBlock (arg_node, NULL);
        host_branch
          = TBmakeBlock (TBmakeAssign (TBmakeLet (DUPdoDupNode (INFO_LETIDS (arg_info)),
                                                  INFO_HOSTWL (arg_info)),
                                       NULL),
                         NULL);

        cond_stmt
          = TBmakeCond (TBmakeId (INFO_CONDITION (arg_info)), cuda_branch, host_branch);

        cond_assign = TBmakeAssign (cond_stmt, assign_next);

        arg_node = TCappendAssign (INFO_PREASSIGNS (arg_info), cond_assign);

        INFO_CONDITION (arg_info) = NULL;
        INFO_PREASSIGNS (arg_info) = NULL;
        INFO_NEEDSSSA (arg_info) = TRUE;

        assign_next = TRAVopt (assign_next, arg_info);

    } else {
        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

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
    DBUG_ENTER ();

    if (WITH_CUDARIZABLE (arg_node)) {
        DBUG_PRINT ("Found cudarizable with-loop.");

        INFO_ISWORTH (arg_info) = FALSE;
        INFO_CONDITION (arg_info) = NULL;

        WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

        /*
         * if there is a condition in the info structure, then we are going to
         * create two versions of this with-loop. We create a duplicate to run on
         * the host here.
         */
        if (INFO_CONDITION (arg_info) != NULL) {
            INFO_HOSTWL (arg_info) = DUPdoDupNode (arg_node);
            /* unmark duplicate for cudarization */
            WITH_CUDARIZABLE (INFO_HOSTWL (arg_info)) = FALSE;
        }
        WITH_CUDARIZABLE (arg_node) = INFO_ISWORTH (arg_info);

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
    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {

        INFO_ISWORTH (arg_info) = ApplySizeCriterion (IDS_NTYPE (INFO_LETIDS (arg_info)));

        CreateConditionVariable (arg_info);
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
    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {

        INFO_ISWORTH (arg_info) = ApplySizeCriterion (IDS_NTYPE (INFO_LETIDS (arg_info)));

        CreateConditionVariable (arg_info);
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

    DBUG_ENTER ();

    if (INFO_LETIDS (arg_info) != NULL) {

        /* Fold loops marked as cudarizable have size criteria applied implicitly
          later on */
        INFO_ISWORTH (arg_info) = TRUE;

        /* I could not make fold with-loops work heterogeneously, so these will be
         run on the first CUDA card only. */
        // CreateConditionVariable(arg_info);
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
