/******************************************************************************
 *
 * This traversal removes unused arguments from function signatures.
 * Consider the following example:
 *
 * fst (int x, <int y>) {
 *     return x;
 * }
 *
 * Here, stdopt/unused_argument_annotate has already marked argument y of fst as
 * being not IsUsedInBody. At this point in the compiler, we can simply remove
 * this argument from the function definition.
 *
 * fst (int x) {
 *     return x;
 * }
 *
 * One problem is that this might produce two function with the exact same
 * signature, since functions are already dispatched at this point this is not a
 * problem internally. We DO need to ensure that the generated C functions do
 * not have the same name. Therefore, this traversal happens after precompile/
 * renameidentifiers. This ensures that generated function names are based on
 * the original signatures, including the unused argument.
 *
 * fst__i__i (int x) {
 *     return x;
 * }
 *
 * To do so we simply traverse the arguments of functions that are allowed to
 * have dummy arguments. Then for each argument in the N_arg chain, if that
 * argument is marked as not being IsUsedInBody, we remove that argument from
 * the chain.
 *
 * Note that after this traversal the number of formal arguments, and the number
 * of actual values provided to function applications, will no longer line up.
 * Because of this, it is important that precompile/dummy_definition_removal is
 * run at the same time.
 *
 ******************************************************************************/
#include "free.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "unused_argument_annotate.h"

#define DBUG_PREFIX "UAR"
#include "debug.h"

#include "unused_function_argument_removal.h"

/******************************************************************************
 *
 * @fn node *UFARdoUnusedFunctionArgumentRemoval (node *arg_node)
 *
 * @brief Removes dummies in function signatures from the AST.
 *
 ******************************************************************************/
node *
UFARdoUnusedFunctionArgumentRemoval (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    TRAVpush (TR_ufar);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UFARfundef (node *arg_node, info *arg_info)
 *
 * @brief Check each function body for arguments that can be removed.
 * If the function has no arguments, there is no need to check it.
 *
 ******************************************************************************/
node *
UFARfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (UAAcanHaveUnusedArguments (arg_node)) {
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UFARarg (node *arg_node, info *arg_info)
 *
 * @brief Remove function arguments from back to front. If an argument is marked
 * as not in use, remove it from the function definition and fix the pointers.
 * Arguments in applications of these functions will have already been removed
 * in the optimisation phase.
 *
 ******************************************************************************/
node *
UFARarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    if (!ARG_ISUSEDINBODY (arg_node)) {
        DBUG_PRINT ("Removing formal parameter %s from function defition",
                    ARG_NAME (arg_node));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}
