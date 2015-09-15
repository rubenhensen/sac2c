/*
 * Identifies functions that have side effects for the distributed memory backend.
 * When the distributed memory backend is used such functions
 * may only be executed by the master node. Return values of such a function need
 * to be sent to the other nodes though.
 *
 * A function is considered to have side effects if it has
 * at least one unique leaf argument. A unique leaf argument
 * is a unique argument that is not passed on to another function.
 *
 * Important: This analysis needs to be run while the syntax tree is still in SSA form.
 *
 * Potential problems: - A function may modify a unique argument itself AND still pass it
 * on to another function. The current implementation assumes that it is "either or".
 *                     - A unique argument may be assigned to another variable which is
 * then passed on to another function. For now we can only detect if a unique argument is
 * passed on directly.
 */

#include "identify_side_effect_functions.h"

#include "traverse.h"
#include "memory.h"
#include "tree_basic.h"
#include "new_types.h"

#define DBUG_PREFIX "DMISEF"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    /* We traverse the arguments of a function twice.
     * In the first iteration we initialize DistMemIsUniqueLeaf of all unique arguments to
     * TRUE. In the following we set DistMemIsUniqueLeaf to FALSE for all unique arguments
     * that are passed on to another function. Finally, in the second iteration we check
     * which arguments are still markes as unique leaf arguments. This flag is to know
     * whether we are currently in the first or second iteration. */
    bool is_first_traversal;
    /* Indicates whether at least one unique argument was found while traversing a
     * function's arguments for the first time. */
    bool has_unique_arg;
    /* Indicates whether at least one unique leaf argument was found while traversing a
     * function's arguments for the second time. */
    bool has_unique_leaf_arg;
    /* Indicates whether we are currently traversing the arguments of a function
     * application. We need to know this because N_Expr nodes also occur in other places.
     */
    bool traversing_ap_args;
};

/*
 * INFO macros
 */
#define INFO_IS_FIRST_TRAVERSAL(n) ((n)->is_first_traversal)
#define INFO_HAS_UNIQUE_ARG(n) ((n)->has_unique_arg)
#define INFO_HAS_UNIQUE_LEAF_ARG(n) ((n)->has_unique_leaf_arg)
#define INFO_TRAVERSING_AP_ARGS(n) ((n)->traversing_ap_args)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_IS_FIRST_TRAVERSAL (result) = FALSE;
    INFO_HAS_UNIQUE_ARG (result) = FALSE;
    INFO_HAS_UNIQUE_LEAF_ARG (result) = FALSE;
    INFO_TRAVERSING_AP_ARGS (result) = FALSE;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/*
 * Traversal functions
 */

/* Argument of function definition */
node *
DMISEFarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /* We are in the first traversal of a function's arguments.
     * Initialize DistMemIsUniqueLeaf to TRUE for unique arguments. */
    if (INFO_IS_FIRST_TRAVERSAL (arg_info)) {
        if (ARG_ISUNIQUE (arg_node)) {
            /* The argument is unique. For now, assume that is a unique leaf argument. */
            ARG_DISTMEMISUNIQUELEAF (arg_node) = TRUE;
            /* In any case there is at least one unique argument. */
            INFO_HAS_UNIQUE_ARG (arg_info) = TRUE;
        }
        /* Examine the remaining arguments if there are any. */
        ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);
        /* We are in the second traversal of a function's arguments.
         * Check if there is at least one unique leaf argument remaining. */
    } else {
        if (ARG_DISTMEMISUNIQUELEAF (arg_node)) {
            /* There is at least one unique leaf argument.
             * So we know that this function has side effects.
             * We do not need to examine the remaining arguments. */
            INFO_HAS_UNIQUE_LEAF_ARG (arg_info) = TRUE;
        } else {
            /* Examine the remaining arguments if there are any. */
            ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/* Function definition */
node *
DMISEFfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_HAS_UNIQUE_ARG (arg_info) = FALSE;
    INFO_IS_FIRST_TRAVERSAL (arg_info) = TRUE;

    /* Traverse the arguments for the first time if there are any.
     * This is to initialize the unique arguments. */
    FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);

    /* Check if the function has at least one unique argument.
     * Otherwise we know that the function does not have side effects and we are done. */
    if (INFO_HAS_UNIQUE_ARG (arg_info)) {
        /* Traverse the body if this is not an external function.
         * This is to find out whether the unique arguments are passed on to another
         * function. */
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        INFO_HAS_UNIQUE_LEAF_ARG (arg_info) = FALSE;
        INFO_IS_FIRST_TRAVERSAL (arg_info) = FALSE;

        /* Traverse the arguments for the second time.
         * This is to find if there are any unique leaf arguments. */
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);

        if (INFO_HAS_UNIQUE_LEAF_ARG (arg_info)) {
            FUNDEF_DISTMEMHASSIDEEFFECTS (arg_node) = TRUE;
        }

        /* Traverse the returns if there is any.
         * This is to mark them as broadcasted. */
        FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);
    }

    /* Continue to the next function if there is any. */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/* Function application */
node *
DMISEFap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TRAVERSING_AP_ARGS (arg_info) = TRUE;

    /* Traverse the arguments of the function application. */
    AP_ARGS (arg_node) = TRAVopt (AP_ARGS (arg_node), arg_info);

    INFO_TRAVERSING_AP_ARGS (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/* Expression (may occur in argument of function application) */
node *
DMISEFexprs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (INFO_TRAVERSING_AP_ARGS (arg_info)) {
        /* We are traversing the arguments of a function application. */
        if (NODE_TYPE (EXPRS_EXPR (arg_node)) == N_id) {
            /* The argument is a variable. */
            node *id = EXPRS_EXPR (arg_node);
            node *avis = ID_AVIS (id);
            if (NODE_TYPE (AVIS_DECL (avis)) == N_arg) {
                /* The variable is a function argument. */
                node *arg = AVIS_DECL (avis);
                /* This variable is not a unique leaf argument
                 * since it is passed on to another function. */
                ARG_DISTMEMISUNIQUELEAF (arg) = FALSE;
            }
        }

        /* Continue with the remaining arguments if there are any. */
        EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
DMISEFdoIdentifySideEffectFunctions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_dmisef);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
