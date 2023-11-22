/*****************************************************************************
 *
 * file:   create_mtst_funs.c
 *
 * prefix: MTSTF
 *
 * description:
 *
 *   This file implements a program traversal that creates MT-variants of all
 *   functions executed in parallel by multiple threads.
 *
 *   The traversal uses the following rules:
 *     - Start traversing at any exported or provided fundef.
 *     - Classify the current context as ST.
 *     - When we encounter a function application:
 *        - If it is not marked, mark it with the current marker.
 *           - Traverse into the function recursively.
 *        - If it is marked with our current marker, skip.
 *        - If it is marked with the other marker, create a copy of the
 *          fundef and mark that one with our marker. Replace application with
 *          application to this function. Traverse into the new function.
 *     - When we encounter a with-loop tagged for parallelisation by the cost
 *       model:
 *        - If the current marker is ST, set the current marker to MT and
 *          enter the with-loop. Reset marker to ST upon return.
 *        - If the current marker is MT, untag the with-loop and leave it
 *          to sequential execution (more precisely replicated parallel
 *          execution due to an outer parallelisation context).
 *     - When we encounter a conditional introduced by the MT cost model (i.e.
 *       (parallel or sequntial execution of a with-loop depending on data
 *       not available before runtime) or CUDA cost model
 *        - If the current marker is ST, we continue traversing into both
 *          branches.
 *        - If the current marker is MT, we eliminate the conditional and
 *          the parallel/CUDA branch and continue with the sequential branch.
 *
 *    This algorithm repeats until all functions are marked and thus a fixed
 *    point is reached. Tagged functions are put in the MT subnamespace.
 *
 *****************************************************************************/

#include "create_mtst_funs.h"
#include "create_mtst_funs_module.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "DupTree.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"
#include "globals.h"
#include "shape.h"
#include "new_types.h"
#include "type_utils.h"

/**
 * INFO structure
 */

struct INFO {
    bool mtcontext :1;
    bool onspine :1;
    node *vardecs;
    node *companions;
    node *spmdassigns;
    node *spmdcondition;
};

/**
 * INFO macros
 */

#define INFO_MTCONTEXT(n) ((n)->mtcontext)
#define INFO_ONSPINE(n) ((n)->onspine)
#define INFO_VARDECS(n) ((n)->vardecs)
#define INFO_COMPANIONS(n) ((n)->companions)
#define INFO_SPMDASSIGNS(n) ((n)->spmdassigns)
#define INFO_SPMDCONDITION(n) ((n)->spmdcondition)

/**
 * INFO functions
 */

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_MTCONTEXT (result) = FALSE;
    INFO_ONSPINE (result) = FALSE;
    INFO_VARDECS (result) = NULL;
    INFO_COMPANIONS (result) = NULL;
    INFO_SPMDASSIGNS (result) = NULL;
    INFO_SPMDCONDITION (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/******************************************************************************
 *
 * @fn bool IsSpmdConditional( node *arg_node)
 *
 *  @brief checks for an argument node of type N_cond whether it is one of
 *    conditionals introduced by the cost model.
 *
 *  @param arg_node of type N_cond
 *
 *  @return predicate value
 *
 *****************************************************************************/

static bool
IsSpmdConditional (node *arg_node)
{
    bool res;
    node *prf;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_cond,
                 "IsSpmdConditional() applied to wrong node type.");

    res = FALSE;

    if (NODE_TYPE (COND_COND (arg_node)) == N_prf) {
        prf = COND_COND (arg_node);
        if ((PRF_PRF (prf) == F_run_mt_genarray) || (PRF_PRF (prf) == F_run_mt_modarray)
            || (PRF_PRF (prf) == F_run_mt_fold)) {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn bool IsCudaConditional( node *arg_node)
 *
 *  @brief checks for an argument node of type N_cond whether it is one of
 *    conditionals introduced by the cuda cost model.
 *
 *  @param arg_node of type N_cond
 *
 *  @return predicate value
 *
 *****************************************************************************/

static bool
IsCudaConditional (node *arg_node)
{
    bool res;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_cond,
                 "IsCudaConditional() applied to wrong node type.");

    res = FALSE;

    /* check if condition variable has cuda cost model name prefix */
    if (NODE_TYPE (COND_COND (arg_node)) == N_id
        && STReqn (ID_NAME (COND_COND (arg_node)), "_cucm", 5)) {
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @fn node *MTSTFdoCreateMtStFunsProg( node *syntax_tree)
 *
 *  @brief initiates MTSTF traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTSTFdoCreateMtStFunsProg (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (MODULE_FILETYPE (syntax_tree) == FT_prog,
                 "MTSTFdoCreateMtStFunsProg() not applicable to modules/classes");

    info = MakeInfo ();

    TRAVpush (TR_mtstf);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * @fn static node *MakeCompanion( node *fundef)
 *
 *  @brief Duplicates the given function and turns the duplicated into MT.
 *
 *  @param fundef
 *
 *  @return fundef
 *
 *****************************************************************************/

static node *
MakeCompanion (node *fundef)
{
    node *companion;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "MakeCompanion() called with non N_fundef argument node");

    DBUG_ASSERT (FUNDEF_ISMTFUN (fundef) || FUNDEF_ISSTFUN (fundef),
                 "Function to be duplicated into companion is neither ST nor MT.");

    companion = DUPdoDupNode (fundef);

    FUNDEF_COMPANION (fundef) = companion;
    FUNDEF_COMPANION (companion) = fundef;

    FUNDEF_ISMTFUN (companion) = !FUNDEF_ISMTFUN (fundef);
    FUNDEF_ISSTFUN (companion) = !FUNDEF_ISSTFUN (fundef);

    DBUG_RETURN (companion);
}

/******************************************************************************
 *
 * @fn node *HandleApFold( node *arg_node, info *arg_info)
 *
 *  @brief Joint traversal function for N_ap and N_fold nodes.
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

static node *
HandleApFold (node *callee, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISMTFUN (callee) || FUNDEF_ISSTFUN (callee)) {
        /*
         * This function has already been processed before.
         */
        if ((FUNDEF_ISMTFUN (callee) && INFO_MTCONTEXT (arg_info))
            || (FUNDEF_ISSTFUN (callee) && !INFO_MTCONTEXT (arg_info))) {
            /*
             * The called function is already in the right mode.
             * Hence, we do nothing.
             */
        } else {
            /*
             * The called function is in the wrong mode.
             */
            if (FUNDEF_COMPANION (callee) == NULL) {
                /*
                 * There is no companion yet, so we must create one,
                 * exchange the callee with its companion, traverse
                 * the new companion to recursively bring it into the right mode,
                 * and eventually append it to the list of companions.
                 */

                callee = TRAVdo (MakeCompanion (callee), arg_info);

                FUNDEF_NEXT (callee) = INFO_COMPANIONS (arg_info);
                INFO_COMPANIONS (arg_info) = callee;
            } else {
                /*
                 * The companion already exists. So, we simply exchange the called
                 * function.
                 */
                callee = FUNDEF_COMPANION (callee);
            }
        }
    } else {
        /*
         * This function has not yet been processed at all.
         * We turn it into the right mode and continue traversal in the callee,
         * unless it is an external function that cannot be replicated for either
         * mode.
         */

        if (!FUNDEF_ISEXTERN (callee)) {
            FUNDEF_ISMTFUN (callee) = INFO_MTCONTEXT (arg_info);
            FUNDEF_ISSTFUN (callee) = !INFO_MTCONTEXT (arg_info);

            callee = TRAVdo (callee, arg_info);
        }
    }

    DBUG_RETURN (callee);
}

/******************************************************************************
 *
 * @fn node *MTSTFmodule( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_module node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_ONSPINE (arg_info) = TRUE;

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFfundef( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_fundef node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_ONSPINE (arg_info)) {

        if (FUNDEF_ISMAIN (arg_node)) {
            FUNDEF_ISSTFUN (arg_node) = TRUE;

            INFO_MTCONTEXT (arg_info) = FALSE;
            INFO_ONSPINE (arg_info) = FALSE;
            arg_node = TRAVdo (arg_node, arg_info);
            INFO_ONSPINE (arg_info) = TRUE;
        }

        if (FUNDEF_NEXT (arg_node) != NULL) {
            FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
        } else {
            /*
             * We are at the end of the spine, so we append the companions
             * collected in the info structure if they exist. If they do exist,
             * we also traverse into them. This has no effect during the top-down
             * traversal as the companions have been fully processed yet, but we
             * need to clean up properly throughout *all* functions during the
             * bottom-up traversal. Likewise, we need to set the appropriate name
             * space for *all* functions.
             */
            if (INFO_COMPANIONS (arg_info) != NULL) {
                FUNDEF_NEXT (arg_node) = INFO_COMPANIONS (arg_info);
                INFO_COMPANIONS (arg_info) = NULL;
                FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
            }
        }

        /*
         * On the traversal back up the tree, put all functions marked MT into
         * the MT namespace and all functions marked ST into the ST name space.
         */
        if (FUNDEF_ISMTFUN (arg_node)) {
            namespace_t *old_namespace = FUNDEF_NS (arg_node);
            FUNDEF_NS (arg_node) = NSgetMTNamespace (old_namespace);
            old_namespace = NSfreeNamespace (old_namespace);
        }

        if (FUNDEF_ISSTFUN (arg_node)) {
            namespace_t *old_namespace = FUNDEF_NS (arg_node);
            FUNDEF_NS (arg_node) = NSgetSTNamespace (old_namespace);
            old_namespace = NSfreeNamespace (old_namespace);
        }

        /*
         * We clear all companion attributes as they are only meaningful and legal
         * within the current phase.
         */
        FUNDEF_COMPANION (arg_node) = NULL;
    } else {
        /*
         * We are *not* at the spine of the fundef chain.
         */
        DBUG_ASSERT (FUNDEF_ISSTFUN (arg_node) || FUNDEF_ISMTFUN (arg_node),
                     "Encountered off-spine fundef that is neither MT nor ST.");

        INFO_MTCONTEXT (arg_info) = FUNDEF_ISMTFUN (arg_node);

        if (FUNDEF_BODY (arg_node) != NULL) {
            node *vardecs;

            vardecs = INFO_VARDECS (arg_info);
            INFO_VARDECS (arg_info) = NULL;

            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

            FUNDEF_VARDECS (arg_node)
              = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDECS (arg_node));
            INFO_VARDECS (arg_info) = vardecs;
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFcond( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_cond node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFcond (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (IsSpmdConditional (arg_node) || IsCudaConditional (arg_node)) {
        if (INFO_MTCONTEXT (arg_info)) {
            /*
             * We are already in an MT context and this is a special conditional
             * introduced by the cost model to postpone the parallelisation decision
             * until runtime. We can now decide that we do *not* want to parallelise
             * the associated with-loop in the current context.
             */

            COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

            INFO_SPMDASSIGNS (arg_info) = BLOCK_ASSIGNS (COND_ELSE (arg_node));
            /*
             * We store the assignment chain of the else-case (sequential)
             * for subsequent integration into the surrounding assignment chain.
             */

            BLOCK_ASSIGNS (COND_ELSE (arg_node)) = NULL;
            /*
             * We must restore a correct conditional node for later de-allocation.
             */
        } else {
            /*
             * The conditional will remain, so we now lift the condition out into a
             * separate assignment.
             */
            node *new_avis;

            COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
            COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);

            new_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_bool),
                                                             SHmakeShape (0)));

            INFO_VARDECS (arg_info) = TBmakeVardec (new_avis, INFO_VARDECS (arg_info));
            INFO_SPMDCONDITION (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (new_avis, NULL),
                                         COND_COND (arg_node)),
                              NULL);
            COND_COND (arg_node) = TBmakeId (new_avis);
        }
    } else {
        /*
         * Normal conditionals are just traversed as usual.
         */
        COND_THEN (arg_node) = TRAVdo (COND_THEN (arg_node), arg_info);
        COND_ELSE (arg_node) = TRAVdo (COND_ELSE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFwith2( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_with2 node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFwith2 (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_MTCONTEXT (arg_info)) {
        if (WITH2_PARALLELIZE (arg_node)) {
            WITH2_PARALLELIZE (arg_node) = FALSE;
        }
        WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
        WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
    } else {
        /*
         * We are not yet in a parallel context. Hence, we traverse the SPMD
         * region in MT context and the optional sequential alternative of a
         * conditional SPMD block in sequential mode. We need not to traverse
         * the condition of a conditional SPMD block because for the time being
         * it may not contain applications of defined functions.
         */

        if (WITH2_PARALLELIZE (arg_node)) {
            INFO_MTCONTEXT (arg_info) = TRUE;
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
            WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
            INFO_MTCONTEXT (arg_info) = FALSE;
        } else {
            WITH2_CODE (arg_node) = TRAVdo (WITH2_CODE (arg_node), arg_info);
            WITH2_WITHOP (arg_node) = TRAVdo (WITH2_WITHOP (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFassign( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_assign node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_SPMDASSIGNS (arg_info) != NULL) {
        /*
         * During traversal of ASSIGN_STMT we have found a special conditional
         * introduced by the cost model that needs to be eliminated now.
         */
        node *further_assigns;

        further_assigns = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = NULL;
        arg_node = FREEdoFreeTree (arg_node);

        arg_node = TCappendAssign (INFO_SPMDASSIGNS (arg_info), further_assigns);
        INFO_SPMDASSIGNS (arg_info) = NULL;

        arg_node = TRAVdo (arg_node, arg_info);
    } else {
        if (INFO_SPMDCONDITION (arg_info) != NULL) {
            /*
             * During traversal of ASSIGN_STMT we have found a SPMD conditional
             * whose condition is now being flattened.
             */
            node *preassign;

            preassign = INFO_SPMDCONDITION (arg_info);
            INFO_SPMDCONDITION (arg_info) = NULL;
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
            ASSIGN_NEXT (preassign) = arg_node;
            arg_node = preassign;
        } else {
            ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFap( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_ap node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AP_FUNDEF (arg_node) = HandleApFold (AP_FUNDEF (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFfold( node *arg_node, info *arg_info)
 *
 *  @brief MTSTF traversal function for N_fold node
 *
 *  @param arg_node
 *  @param arg_info
 *
 *  @return arg_node
 *
 *****************************************************************************/

node *
MTSTFfold (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FOLD_FUNDEF (arg_node) = HandleApFold (FOLD_FUNDEF (arg_node), arg_info);

    FOLD_NEXT (arg_node) = TRAVopt (FOLD_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *MTSTFdoCreateMtStFuns( node *syntax_tree)
 *
 *  @brief initiates MTSTF traversal
 *
 *  @param syntax_tree
 *
 *  @return syntax_tree
 *
 *****************************************************************************/

node *
MTSTFdoCreateMtStFuns (node *syntax_tree)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_module, "Illegal argument node!");

    if (MODULE_FILETYPE (syntax_tree) == FT_prog) {
        syntax_tree = MTSTFdoCreateMtStFunsProg (syntax_tree);
    } else {
        syntax_tree = MTSTFMODdoCreateMtStFunsModule (syntax_tree);
    }

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
