/******************************************************************************
 *
 * This traversal finds arguments that whose values are never used in a
 * function's body. Consider the following example:
 *
 * fst (int x, int y) {
 *   return x;
 * }
 *
 * This function has an argument y that is never used in the body. By traversing
 * all identifiers of the body, this traversal is able to find such arguments.
 * The IsUsedInBody flag is set to FALSE for these unused arguments (y).
 *
 * We do not want to do this annotation for every function. Firstly, this
 * annotation should only be done for user-defined functions. Therefore
 * conditional and loop functions should be excluded. Additionally, since module
 * trees are exported before the optimisation cycle, it is currently not
 * possible to do this annotation for modules. Future research is required to
 * investigate whether it is possible to move the module tree export to after
 * the optimisation cycle.
 *
 * It has already been investigated whether it is instead possible to add an
 * additional cycle before the module export for this optimisation, however this
 * is also not possible because applications are dispatched to prototypes for
 * locals instead, which do not have a body and will thus only have unused
 * arguments.
 *
 ******************************************************************************/
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "UAA"
#include "debug.h"

#include "unused_argument_annotate.h"

/******************************************************************************
 *
 * @fn bool CanHaveUnusedArguments (node *fundef)
 *
 * @returns Whether this N_fundef can have unused arguments in its signature.
 *
 ******************************************************************************/
static bool
CanHaveUnusedArguments (node *fundef)
{
    DBUG_ENTER ();
    DBUG_RETURN (!FUNDEF_WASUSED (fundef)
              && !FUNDEF_ISSTICKY (fundef)
              && !FUNDEF_ISPROVIDED (fundef)
              && !FUNDEF_ISLACFUN (fundef)
              && FUNDEF_ARGS (fundef) != NULL);
}

/******************************************************************************
 *
 * @fn node *UAAdoUnusedArgumentAnnotate (node *arg_node)
 *
 * @brief Modify the IsUsedInBody flag function arguments in the AST.
 *
 ******************************************************************************/
node *
UAAdoUnusedArgumentAnnotate (node *arg_node)
{
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "called with non-fundef node");

    TRAVpush (TR_uaa);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAAfundef (node *arg_node, info *arg_info)
 *
 * @brief Set IsUsedInBody flag for all global, non-wrapper, function arguments.
 * If the function has no arguments, there is no need to check it.
 *
 ******************************************************************************/
node *
UAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (CanHaveUnusedArguments (arg_node)) {
        DBUG_PRINT ("----- annotating unused arguments of %s -----",
                    FUNDEF_NAME (arg_node));
        FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAAarg (node *arg_node, info *arg_info)
 *
 * @brief Reset IsUsedInBody flag to false for all arguments.
 *
 ******************************************************************************/
node *
UAAarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    ARG_ISUSEDINBODY (arg_node) = FALSE;
    ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAAid (node *arg_node, info *arg_info)
 *
 * @brief If the declaration of an avis points to an argument, set the
 * IsUsedInBody flag to true for that argument.
 *
 ******************************************************************************/
node *
UAAid (node *arg_node, info *arg_info)
{
    node *decl;

    DBUG_ENTER ();

    decl = AVIS_DECL (ID_AVIS (arg_node));
    if (NODE_TYPE (decl) == N_arg) {
        ARG_ISUSEDINBODY (decl) = TRUE;
    }

    DBUG_RETURN (arg_node);
}
