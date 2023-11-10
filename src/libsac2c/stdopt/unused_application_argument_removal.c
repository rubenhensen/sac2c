/******************************************************************************
 *
 * This traversal optimises function applications by replacing values of unused
 * arguments by temporary dummies. Consider the following example.
 *
 * fst (int x, <int y>) {
 *   return x;
 * }
 *
 * main () {
 *   w = very_slow ();
 *   return fst (1, w);
 * }
 *
 * Here, stdopt/unused_argument_annotate has already marked argument y of fst as
 * being not IsUsedInBody. In main, we replace w in the application of fst by a
 * dummy so that stdopt/dead_code_removal is able to remove the application of
 * very_slow().
 *
 * main () {
 *   dummy = _dummy_type_ (int);
 *   return fst (1, dummy);
 * }
 *
 * It is required to run this optimisation in the optimisation cycle, in
 * combination with stdopt/unused_argument_annotate and stdopt/dead_code_removal.
 * This is because it is possible that arguments only become unused after
 * previous optimisation cycles.
 *
 * Simply removing w is not possible. We need to ensure that the expected code
 * structure is not violated. Namely this means that we should adhere to the
 * following rules:
 *   - The number of values in an application is equal to the number of
 *     arguments of that function.
 *   - Applications contain only identifiers (the code is flattened).
 *   - Every identifier has an AVIS.
 *   - Every AVIS has a declaration and an assignment.
 *
 * Additionally we need to ensure that the type checker is happy; the dummy
 * should be of the correct type. For this reason a primitive function
 * _dummy_type_(type) has been added, which ensures that this dummy is of the
 * same type as the unused argument.
 *
 * Doing this ensures that no modifications to existing traversers is required.
 * The only exception is memory/referencecounting. Since these dummies are
 * always removed by precompile/dummy_definition_removal in the precompile
 * phase, no reference counting should by applied to cases where IsUsedInBody is
 * FALSE.
 *
 * This is implemented by first traversing the applications of the function
 * body. This creates a list of formal parameters that were marked as not
 * IsUsedInBody, and a list of corresponding actual arguments given in the
 * application. In the fundef traversal we then use this list to replace these
 * actual arguments by new dummy variables, with corresponding variable
 * declarations and assignments. To ensure this dummy has the correct type, the
 * type from the formal argument is copied.
 *
 ******************************************************************************/
#include "DupTree.h"
#include "memory.h"
#include "new_types.h"
#include "str.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"

#define DBUG_PREFIX "UAR"
#include "debug.h"

#include "unused_application_argument_removal.h"

/******************************************************************************
 *
 * @struct info
 *
 * @param INFO_UNUSEDARGS Formal arguments in the function signature that are
 *        not used by the current function application.
 * @param INFO_UNUSEDEXPRS Actual arguments that are not used by the current
 *        function application.
 *
 ******************************************************************************/
struct INFO {
    node *unused_args;
    node *unused_exprs;
};

#define INFO_UNUSEDARGS(n) (n->unused_args)
#define INFO_UNUSEDEXPRS(n) (n->unused_exprs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_UNUSEDARGS (result) = NULL;
    INFO_UNUSEDEXPRS (result) = NULL;

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
 * @fn static node *AddNewDummyToFundef (node *fundef, node *arg)
 *
 * @brief Adds the declaration and assignment for a new dummy variable to the
 * function body. Returns the AVIS of the newly created dummy.
 *
 ******************************************************************************/
static node *
AddNewDummyToFundef (node *fundef, ntype *type)
{
    char *name;
    node *avis, *res;

    DBUG_ENTER ();

    name = TRAVtmpVarName ("dummy");
    DBUG_PRINT ("creating a new dummy %s", name);

    // Create dummy AVIS
    avis = TBmakeAvis (STRcpy (name), TYcopyType (type));
    AVIS_DECLTYPE (avis) = TYcopyType (type);
    AVIS_ISDUMMY (avis) = TRUE;

    // Create dummy declaration
    FUNDEF_VARDECS (fundef) = TBmakeVardec (avis, FUNDEF_VARDECS (fundef));
    AVIS_DECL (avis) = FUNDEF_VARDECS (fundef);

    // Create dummy assignment
    res = TCmakePrf1 (F_dummy_type, TBmakeType (TYcopyType (type)));
    res = TBmakeLet (TBmakeIds (avis, NULL), res);

    // Prepend the generated assignment to the function's assignments
    FUNDEF_ASSIGNS (fundef) = TBmakeAssign (res, FUNDEF_ASSIGNS (fundef));
    AVIS_SSAASSIGN (avis) = FUNDEF_ASSIGNS (fundef);

    DBUG_RETURN (avis);
}

/******************************************************************************
 *
 * @fn node *UAARdoUnusedApplicationArgumentRemoval (node *arg_node)
 *
 * @brief Replace unused arguments by dummies in the AST.
 *
 ******************************************************************************/
node *
UAARdoUnusedApplicationArgumentRemoval (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef,
                 "called with non-fundef node");

    info = MakeInfo ();

    TRAVpush (TR_uaar);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAARfundef (node *arg_node, info *arg_info)
 *
 * @brief Check each function body for applications that can be optimised.
 * If an application of a function with unused arguments was found during the
 * traversal of the body we have generated an N_args chain in INFO_UNUSEDARGS
 * containing duplicates of the formal parameters (N_arg nodes). Additionally,
 * INFO_UNUSEDEXPRS contains an N_exprs chain pointing to the shared N_id nodes.
 *
 ******************************************************************************/
node *
UAARfundef (node *arg_node, info *arg_info)
{
    ntype *type;
    node *avis;

    DBUG_ENTER ();

    DBUG_PRINT ("----- removing unused application arguments of %s -----",
                FUNDEF_NAME (arg_node));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    while (INFO_UNUSEDARGS (arg_info) != NULL) {
        type = ARG_NTYPE (INFO_UNUSEDARGS (arg_info));
        avis = AddNewDummyToFundef(arg_node, type);

        DBUG_PRINT ("replacing actual argument %s by a dummy %s",
                    ID_NAME (EXPRS_EXPR (INFO_UNUSEDEXPRS (arg_info))),
                    AVIS_NAME (avis));

        ID_AVIS (EXPRS_EXPR (INFO_UNUSEDEXPRS (arg_info))) = avis;

        // Ensure the referenced actual argument is not freed
        EXPRS_EXPR (INFO_UNUSEDEXPRS (arg_info)) = NULL;
        // Free the created N_arg and N_exprs chains
        INFO_UNUSEDARGS (arg_info) = FREEdoFreeNode (INFO_UNUSEDARGS (arg_info));
        INFO_UNUSEDEXPRS (arg_info) = FREEdoFreeNode (INFO_UNUSEDEXPRS (arg_info));
    }

    DBUG_ASSERT (INFO_UNUSEDARGS (arg_info) == NULL,
                 "remaining unused formal parameters");
    DBUG_ASSERT (INFO_UNUSEDEXPRS (arg_info) == NULL,
                 "remaining unused actual arguments");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *UAARap (node *arg_node, info *arg_info)
 *
 * @brief Optimise applications of global, non-wrapper, functions. This is done
 * by iterating over all pairs of function arguments and application expressions
 * in order. If a function argument is marked as in use or has been removed in a
 * previous optimisation cycle, nothing happens. Otherwise, the expression is
 * removed from the application, but is kept in the function definition (as it
 * must only be removed at a later stage).
 *
 ******************************************************************************/
node *
UAARap (node *arg_node, info *arg_info)
{
    node *formal, *actual, *avis;

    DBUG_ENTER ();

    formal = FUNDEF_ARGS (AP_FUNDEF (arg_node));
    actual = AP_ARGS (arg_node);

    while (formal != NULL) {
        avis = ID_AVIS (EXPRS_EXPR (actual));

        if (!ARG_ISUSEDINBODY (formal) && !AVIS_ISDUMMY (avis)) {
            DBUG_PRINT ("formal parameter %s is marked as not in use, "
                        "replace actual argument %s by a dummy",
                        ARG_NAME (formal), AVIS_NAME (avis));

            INFO_UNUSEDARGS (arg_info) = TCappendArgs (
                INFO_UNUSEDARGS (arg_info),
                DUPdoDupNode (formal));
            INFO_UNUSEDEXPRS (arg_info) = TCappendExprs (
                INFO_UNUSEDEXPRS (arg_info),
                TBmakeExprs (EXPRS_EXPR (actual), NULL));
        }

        formal = ARG_NEXT (formal);
        actual = EXPRS_NEXT (actual);
    }

    DBUG_ASSERT (formal == NULL, "remaining formal parameters");
    DBUG_ASSERT (actual == NULL, "remaining actual arguments");

    DBUG_RETURN (arg_node);
}
