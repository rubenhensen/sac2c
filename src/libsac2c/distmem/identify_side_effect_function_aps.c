/*
 * Identifies function applications that have side effects for the distributed memory
 * backend. When the distributed memory backend is used such function applications may
 * only be executed by the master node. Return values of such a function application need
 * to be sent to the other nodes though.
 *
 * A function application is considered to have side effects if the
 * called function is marked as having side effects and the calling
 * function does not have side effects itself.
 *
 * Important: This needs to be run after dmisef.
 */

#include "identify_side_effect_function_aps.h"

#include "traverse.h"
#include "memory.h"
#include "tree_basic.h"

#define DBUG_PREFIX "DMISEFA"
#include "debug.h"

/*
 * INFO structure
 */
struct INFO {
    /* Indicates whether we are currently inside the body of a function with side effects.
     */
    bool traversing_fun_with_side_effects;
};

/*
 * INFO macros
 */
#define INFO_TRAVERSING_FUN_WITH_SIDE_EFFECTS(n) ((n)->traversing_fun_with_side_effects)

/*
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TRAVERSING_FUN_WITH_SIDE_EFFECTS (result) = FALSE;

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

/* Function definition */
node *
DMISEFAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_TRAVERSING_FUN_WITH_SIDE_EFFECTS (arg_info)
      = FUNDEF_DISTMEMHASSIDEEFFECTS (arg_node);

    /* Traverse the body if this is not an external function. */
    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    /* Continue to the next function if there is any. */
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/* Function application */
node *
DMISEFAap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_DISTMEMHASSIDEEFFECTS (AP_FUNDEF (arg_node))
        && !INFO_TRAVERSING_FUN_WITH_SIDE_EFFECTS (arg_info)) {
        /* The function application is considered to have side effects
         * if the called function has side effects but the calling
         * function does not have side effects. */
        AP_DISTMEMHASSIDEEFFECTS (arg_node) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/*
 * Traversal start function
 */
node *
DMISEFAdoIdentifySideEffectFunctionApplications (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ();

    TRAVpush (TR_dmisefa);

    info = MakeInfo ();
    syntax_tree = TRAVdo (syntax_tree, info);
    info = FreeInfo (info);

    TRAVpop ();

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
