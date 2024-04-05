/******************************************************************************
 *
 * @brief Unused Argument Removal
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
 * @brief Unmodified Return Removal
 *
 * We additionally annotate N_ret types whose corresponding return value is one
 * of the formal arguments. Because we are in SSA form, in that case the value
 * of this return will be exactly the same as that argument. A prominent example
 * of this is the identity function:
 *
 * int id (int a) {
 *     return a;
 * }
 *
 * By annotating the N_ret as unchanged, calling sites of this function may
 * replace any uses of the return value by the actual argument, potentially
 * allowing for better optimisation. This optimisation is especially important
 * in the context of records, where a record given to a function might be only
 * partially modified before being returned again.
 *
 * struct Body
 * move (struct Body body) {
 *     body.pos += body.vel;
 *     return body;
 * }
 *
 * Is expanded into:
 *
 * double[3], double[3], double
 * move (double[3] pos, double[3] vel, double mass) {
 *     pos += vel;
 *     return (pos, vel, mass);
 * }
 *
 * Here we see that only pos is modified, whereas vel and mass remain unchanged.
 * By marking those two return types as unmodified, calling sites of this
 * function can potentially apply further optimisations on those two arguments.
 *
 ******************************************************************************/
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"

#define DBUG_PREFIX "UAA"
#include "debug.h"

#include "unused_argument_annotate.h"

struct INFO {
    node *fundef;
};

#define INFO_FUNDEF(n) ((n)->fundef)

static info *
MakeInfo (void)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = (info *)MEMmalloc (sizeof (info));

    INFO_FUNDEF (arg_info) = NULL;

    DBUG_RETURN (arg_info);
}

static info *
FreeInfo (info *arg_info)
{
    DBUG_ENTER ();

    arg_info = MEMfree (arg_info);

    DBUG_RETURN (arg_info);
}

/******************************************************************************
 *
 * @fn bool UAAcanHaveUnusedArguments (node *fundef)
 *
 * @returns Whether this N_fundef is allowed to have unused arguments or
 * unmodified return types in its signature.
 *
 ******************************************************************************/
bool
UAAcanHaveUnusedArguments (node *fundef)
{
    DBUG_ENTER ();
    DBUG_RETURN (!FUNDEF_WASUSED (fundef)
              && !FUNDEF_ISSTICKY (fundef)
              && !FUNDEF_ISEXTERN (fundef)
              && !FUNDEF_ISPROVIDED (fundef)
              && !FUNDEF_ISLACFUN (fundef));
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
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "called with non-fundef node");

    arg_info = MakeInfo ();

    TRAVpush (TR_uaa);
    arg_node = TRAVdo (arg_node, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAAfundef (node *arg_node, info *arg_info)
 *
 * @brief Set IsUsedInBody flag for the arguments and return types of global,
 * non-wrapper, functions.
 *
 ******************************************************************************/
node *
UAAfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (UAAcanHaveUnusedArguments (arg_node)) {
        DBUG_PRINT ("----- Annotating unused arguments of %s -----",
                    FUNDEF_NAME (arg_node));
        // Reset IsUsedInBody flags
        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_RETS (arg_node) = TRAVopt (FUNDEF_RETS (arg_node), arg_info);

        INFO_FUNDEF (arg_info) = arg_node;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);
        INFO_FUNDEF (arg_info) = NULL;
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

/******************************************************************************
 *
 * @fn node *UAAret (node *arg_node, info *arg_info)
 *
 * @brief Reset ArgIndex and IsUsedInBody flags for all return types.
 *
 ******************************************************************************/
node *
UAAret (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    RET_ARGINDEX (arg_node) = -1;
    RET_ISUSEDINBODY (arg_node) = TRUE;
    RET_NEXT (arg_node) = TRAVopt (RET_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAAreturn (node *arg_node, info *arg_info)
 *
 * @brief Check all returned identifiers. If an identifier is an argument, mark
 * the return type at the same index as not IsUsedInBody. Additionally we set
 * the ArgIndex of this return type. This always works because we are in SSA
 * form.
 *
 ******************************************************************************/
node *
UAAreturn (node *arg_node, info *arg_info)
{
    node *formal_rets, *actual_rets, *decl;
    size_t num_args, actual_index;

    DBUG_ENTER ();

    num_args = TCcountArgs (FUNDEF_ARGS (INFO_FUNDEF (arg_info)));
    formal_rets = FUNDEF_RETS (INFO_FUNDEF (arg_info));
    actual_rets = RETURN_EXPRS (arg_node);

    while (formal_rets != NULL) {
        decl = ID_DECL (EXPRS_EXPR (actual_rets));
        if (NODE_TYPE (decl) == N_arg) {
            actual_index = num_args - TCcountArgs (decl);
            DBUG_PRINT ("Return value %s is equal to argument %s (index %lu)",
                        ID_NAME (EXPRS_EXPR (actual_rets)),
                        ARG_NAME (decl),
                        actual_index);

            ARG_ISUSEDINBODY (decl) = TRUE;
            RET_ARGINDEX (formal_rets) = (int)actual_index;

            DBUG_ASSERT (ARG_ISUNIQUE (decl) == RET_ISUNIQUE (formal_rets),
                         "Unique argument mismatch");
            // Unique arguments, such as StdIO, should never be marked as unused
            if (!ARG_ISUNIQUE (decl)) {
                RET_ISUSEDINBODY (formal_rets) = FALSE;
            }
        }

        formal_rets = RET_NEXT (formal_rets);
        actual_rets = EXPRS_NEXT (actual_rets);
    }

    DBUG_RETURN (arg_node);
}
