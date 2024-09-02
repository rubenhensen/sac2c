/*****************************************************************************
 *
 * file:   create_mtst_funs.c
 *
 * prefix: MTSTF
 *
 * description:
 *
 *   In the MT backend, we implement a flat level of parallelism. That means
 *   whenever we encounter a with-loop that can be executed in parallel while
 *   we *are* already executing in parallel, we execute that "inner" with-loop
 *   sequentially.
 *   To avoid runtime checks as much as possible, we trace the control flow
 *   keeping track whether we are still single-threaded (ST) or already inside
 *   a with-loop that is executed in parallel (MT).
 *   In case functions are colled from both contexts or whenever we cannot
 *   statically decide in which context we are, this can, at worst, lead to
 *   a duplication of the code.
 *
 *   For example:
 *
 *     int[*] id (int[*] a)
 *     {  return a; }
 *   
 *     int[.] par (int[.] a)
 *     {  return with {   // parallel!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *     }
 *   
 *     int[.,.] nested_par (int[.,.] a)
 *     {  return with {   // parallel!
 *                (. <= [i] <= .) : par (a[i]);
 *               } : modarray (a);
 *     }
 *   
 *     int main ()
 *     {
 *         a = id (iota (100));
 *         b = par (a);
 *         c = reshape ([10,10], a);
 *         d = nested_par (c);
 *         ...
 *     }
 *
 *   ==>
 *
 *     int[*] id#ST (int[*] a)
 *     {  return a; }
 *
 *     int[.] par#ST (int[.] a)
 *     {  return with {   // parallel!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *     }
 *
 *     int[.] par#MT (int[.] a)
 *     {  return with {   // sequential!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *     }
 *
 *     int[.,.] nested_par#ST (int[.,.] a)
 *     {  return with {   // parallel!
 *                (. <= [i] <= .) : par#MT (a[i]);
 *               } : modarray (a);
 *     }
 *
 *     int main#ST ()
 *     {
 *         a = id#ST (iota#ST (100));
 *         b = par#ST (a);
 *         c = reshape#ST ([10,10], a);
 *         d = nested_par#ST (c);
 *         ...
 *     }
 *
 *   Here, we can see how the function "par" gets duplicated,
 *   how the function calls are adjusted, and how the with-loop
 *   in the par#MT version gets turned into a sequential as this
 *   version is called from a parallel context only!
 *
 *   To make matters a bit more "interesting", we can encounter 
 *   not only sequential and parallel with-loops but also 
 *   potentially-parallel ones, whenever we cannot statically decide
 *   whether it is worth-while or even possible to run that 
 *   with-loop in parallel. In fact, the 'par' example from above,
 *   if left unspecialised, would be such a case. Here, we would
 *   face roughly the following code when entering this traversal:
 *
 *     int[.] par (int[.] a)
 *     {  if (run_mt_genarray (b_mem, 250)) {
 *            b = with {   // parallel!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *        } else {
 *            b = with {   // sequential!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *        }
 *        return b;
 *     }
 *
 *   Again, we need to create an MT and an ST version, but they would
 *   look like this:
 *
 *     int[.] par#MT (int[.] a)
 *     {  
 *        b = with {   // sequential!
 *            (. <= iv <= .) : a[iv] + 1;
 *           } : modarray (a);
 *        return b;
 *     }
 *
 *     int[.] par#ST (int[.] a)
 *     {
 *         pred = run_mt_genarray (b_mem, 250);
 *         if (pred) {
 *            b = with {   // parallel!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *        } else {
 *            b = with {   // sequential!
 *                (. <= iv <= .) : a[iv] + 1;
 *               } : modarray (a);
 *        }
 *        return b;
 *     }
 *
 *   Finally, we may encounter functions that are not called at all.
 *   Although these are dead code and could have been eliminated by 
 *   DFR, the user can turn this optimisation off or DFR can be 
 *   buggy (see issues #2432 and #2433 for examples).
 *   While this is not a problem for functions that have parallel
 *   or sequential with-loops, it is a problem for potentially
 *   parallel ones. For these, we need to make sure we lift the
 *   predicate as in the 'par#ST' example from above. (see issue 
 *   #2432).
 *
 *   The traversal is implemented as follows:
 *   Conceptually, we have three different traversals:
 *     1) we skip through the fundef until we find "main".
 *        This is implemented by a simple loop in MTSTFmodule.
 *     2) we follow the callgraph through MTSTFap nodes starting
 *        at MTSTFfundef of main.
 *        At the start, we set the context to ST
 *        (INFO_MTCONTEXT == false).
 *        When we encounter a function (MTSTFfundef), we do:
 *        - If it is not marked, mark it with the current marker.
 *           - Traverse into the function recursively.
 *        - If it is marked with our current marker, return.
 *        - If it is marked with the other marker, check whether 
 *          FUNDEF_COMPANION already exists; if not, create a copy of the
 *          fundef and mark that one with our marker. Put the new fundef 
 *          into INFO_COMPANIONS, make sure both functions have
 *          FUNDEF_COMPANION apropriately set, and replace the old
 *          application with an application to the companion function.
 *          Traverse into the companion if it was freshly built.
 *        When we encounter a with-loop (MTSTFwith2) tagged for
 *        parallelisation by the cost model:
 *        - If the current marker is ST, set the current marker to MT and
 *          enter the with-loop. Reset marker to ST upon return.
 *        - If the current marker is MT, untag the with-loop and leave it
 *          to sequential execution (more precisely replicated parallel
 *          execution due to an outer parallelisation context).
 *        When we encounter a conditional (MTSTFcond) introduced by the MT
 *        cost model (i.e. parallel or sequntial execution of a with-loop
 *        depending on data not available before runtime) or CUDA cost model
 *        - If the current marker is ST, we continue traversing into both
 *          branches and lift the conditional out.
 *        - If the current marker is MT, we eliminate the conditional and
 *          the parallel/CUDA branch and continue with the sequential branch.
 *     3) we traverse through the fundef chain looking for functions that 
 *        have not yet been marked; These functions are deleted. At the end
 *        the chain, append the INFO_COMPANIONS. Again, we implement this 
 *        simple traversal in MTSTFmodule.
 *
 *****************************************************************************/

#include "create_mtst_funs.h"
#include "create_mtst_funs_module.h"

#define DBUG_PREFIX "MTSTF"
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
    node *vardecs;
    node *companions;
    node *spmdassigns;
    node *spmdcondition;
};

/**
 * INFO macros
 */

#define INFO_MTCONTEXT(n) ((n)->mtcontext)
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
 * @fn void SetMtNamespace (node * fundef)
 *
 *****************************************************************************/

static void
SetMtNamespace (node * fundef)
{
    namespace_t *old_namespace;

    DBUG_ENTER ();
    old_namespace = FUNDEF_NS (fundef);
    FUNDEF_NS (fundef) = NSgetMTNamespace (old_namespace);
    old_namespace = NSfreeNamespace (old_namespace);
    DBUG_RETURN ();
}

/******************************************************************************
 *
 * @fn void SetStNamespace (node * fundef)
 *
 *****************************************************************************/

static void
SetStNamespace (node * fundef)
{
    namespace_t *old_namespace;

    DBUG_ENTER ();
    old_namespace = FUNDEF_NS (fundef);
    FUNDEF_NS (fundef) = NSgetSTNamespace (old_namespace);
    old_namespace = NSfreeNamespace (old_namespace);
    DBUG_RETURN ();
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
    node * fundefs;
    node * last;
    DBUG_ENTER ();

    fundefs = MODULE_FUNS (arg_node);

    DBUG_PRINT ("Searching for main...");

    while ((fundefs != NULL) && !FUNDEF_ISMAIN (fundefs)) {
        fundefs = FUNDEF_NEXT (fundefs);
    }
    DBUG_ASSERT (fundefs != NULL, "main function missing!");

    DBUG_PRINT ("Traversing call-graph starting at main...");
    DBUG_PRINT ("  setting context to ST");
    INFO_MTCONTEXT (arg_info) = FALSE;
    fundefs = TRAVdo (fundefs, arg_info);

    DBUG_PRINT ("Searching for untagged functions...");
    fundefs = MODULE_FUNS (arg_node);
    last = NULL;
    while (fundefs != NULL) {
        DBUG_PRINT ("  traversing function '%s'", FUNDEF_NAME (fundefs));
        if (!FUNDEF_ISSTFUN (fundefs) && !FUNDEF_ISMTFUN (fundefs)) {
            DBUG_PRINT ("  deleting dead function");
            fundefs = FREEdoFreeNode (fundefs); // zombiefies fundef only!
        }
        FUNDEF_COMPANION (fundefs) = NULL;
        last = fundefs;
        fundefs = FUNDEF_NEXT (fundefs);
    }
    DBUG_PRINT ("appending companions");
    FUNDEF_NEXT (last) = INFO_COMPANIONS (arg_info);
    MODULE_FUNS (arg_node) = FREEremoveAllZombies (MODULE_FUNS (arg_node));

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
    node *vardecs;
    DBUG_ENTER ();

    DBUG_PRINT ("  traversing function '%s'", FUNDEF_NAME (arg_node));

    vardecs = INFO_VARDECS (arg_info);
    INFO_VARDECS (arg_info) = NULL;

    if (!FUNDEF_ISSTFUN (arg_node) && !FUNDEF_ISMTFUN (arg_node)) {
        DBUG_PRINT ("  not marked yet");
        if (INFO_MTCONTEXT (arg_info)) {
            DBUG_PRINT ("  setting to MT");
            FUNDEF_ISMTFUN (arg_node) = TRUE;
            SetMtNamespace (arg_node);
        } else {
            DBUG_PRINT ("  setting to ST");
            FUNDEF_ISSTFUN (arg_node) = TRUE;
            SetStNamespace (arg_node);
        }
        DBUG_PRINT ("  traversing body");
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    } else if ((INFO_MTCONTEXT (arg_info) && FUNDEF_ISSTFUN (arg_node))
               || (!INFO_MTCONTEXT (arg_info) && FUNDEF_ISMTFUN (arg_node))) {
        DBUG_PRINT ("  wrongly tagged fundef found");
        if (FUNDEF_COMPANION (arg_node) != NULL) {
            DBUG_PRINT ("  existing companion found");
            arg_node = FUNDEF_COMPANION (arg_node);
        } else {
            DBUG_PRINT ("  creating new companion");
            arg_node = MakeCompanion (arg_node);
            if (INFO_MTCONTEXT (arg_info)) {
                DBUG_PRINT ("  setting to MT");
                SetMtNamespace (arg_node);
            } else {
                DBUG_PRINT ("  setting to ST");
                SetStNamespace (arg_node);
            }
            DBUG_PRINT ("  traversing body");
            FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        }
    } else {
        DBUG_PRINT ("  matching tag found; nothing to do!");
    }

    FUNDEF_VARDECS (arg_node)
        = TCappendVardec (INFO_VARDECS (arg_info), FUNDEF_VARDECS (arg_node));
    INFO_VARDECS (arg_info) = vardecs;

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
	DBUG_PRINT ("dynamic concurrency check found");
        if (INFO_MTCONTEXT (arg_info)) {
            /*
             * We are already in an MT context and this is a special conditional
             * introduced by the cost model to postpone the parallelisation decision
             * until runtime. We can now decide that we do *not* want to parallelise
             * the associated with-loop in the current context.
             */
            DBUG_PRINT ("within MT => delete concurrency check");

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

            DBUG_PRINT ("lifting concurrency check");
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

    AP_FUNDEF (arg_node) = TRAVdo (AP_FUNDEF (arg_node), arg_info);

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

    FOLD_FUNDEF (arg_node) = TRAVdo (FOLD_FUNDEF (arg_node), arg_info);

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
