/******************************************************************************
 *
 * This traversal removes dummy identifiers from applications in function
 * bodies. Consider the following example:
 *
 * fst (int x, <int y>) {
 *     return x;
 * }
 *
 * main () {
 *     dummy = _UAR_dummy_type_ (int);
 *     return fst (1, dummy);
 * }
 *
 * Here, stdopt/unused_argument_annotate has already marked argument y of fst as
 * being not IsUsedInBody. At this point in the compiler, we can simply remove
 * the dummy value from the function application. As the number of function
 * arguments, and the number of applied values in applications, no longer needs
 * to line up.
 *
 * main () {
 *     return fst (1);
 * }
 *
 * To do so we first traverse all assignments of a function. When we encounter
 * an N_exprs chain in an application, we check the arguments and remove any
 * arguments that are marked as not in used. We then also remove the variable
 * declaration and assignment corresponding to this identifier.
 *
 * Note that this traversal can only be run in the precompile phase, because it
 * must happend at the same time as precompile/unused_function_argument_removal.
 * This is because the number of formal arguments, and the number of actual
 * values provided in an application, must always line up.
 *
 ******************************************************************************/
#include "free.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "unused_argument_annotate.h"

#define DBUG_PREFIX "DDR"
#include "debug.h"

#include "dummy_definition_removal.h"

/******************************************************************************
 *
 * @struct info
 *
 * @param INFO_IDAVIS The avis of the current identifier.
 * @param INFO_FORMALARGS The formal arguments of the current N_ap.
 *
 ******************************************************************************/
struct INFO {
    node *ids_avis;
    node *formal_args;
};

#define INFO_IDAVIS(n) (n->ids_avis)
#define INFO_FORMALARGS(n) (n->formal_args)

static info *
MakeInfo (void)
{
    info *res;

    DBUG_ENTER ();

    res = (info *)MEMmalloc (sizeof (info));

    INFO_IDAVIS (res) = NULL;
    INFO_FORMALARGS (res) = NULL;

    DBUG_RETURN (res);
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
 * @fn node *DDRdoDummyDefinitionRemoval (node *arg_node)
 *
 * @brief Removes dummies in applications from the AST.
 *
 ******************************************************************************/
node *
DDRdoDummyDefinitionRemoval (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (arg_node) == N_module,
                 "called with non-module node");

    info = MakeInfo ();

    TRAVpush (TR_ddr);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRfundef (node *arg_node, info *arg_info)
 *
 * @brief Remove dummies from all function bodies.
 *
 ******************************************************************************/
node *
DDRfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_BODY (arg_node) != NULL) {
        DBUG_PRINT ("----- dummy definition removal in %s -----",
                    FUNDEF_NAME (arg_node));
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
    }

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRblock (node *arg_node, info *arg_info)
 *
 * @brief First remove dummies from applications, then remove dummy assignments
 * and dummy declarations.
 *
 ******************************************************************************/
node *
DDRblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
    BLOCK_VARDECS (arg_node) = TRAVopt (BLOCK_VARDECS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRvardec (node *arg_node, info *arg_info)
 *
 * @brief Remove variable declarations of dummy arguments.
 *
 ******************************************************************************/
node *
DDRvardec (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    avis = VARDEC_AVIS (arg_node);

    if (AVIS_ISDUMMY (avis)) {
        DBUG_PRINT ("removing dummy declaration %s", AVIS_NAME (avis));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRassign (node *arg_node, info *arg_info)
 *
 * @brief Remove assignments of dummy arguments.
 *
 ******************************************************************************/
node *
DDRassign (node *arg_node, info *arg_info)
{
    node *avis;

    DBUG_ENTER ();

    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    INFO_IDAVIS (arg_info) = NULL;
    ASSIGN_STMT (arg_node) = TRAVopt (ASSIGN_STMT (arg_node), arg_info);
    avis = INFO_IDAVIS (arg_info);

    if (avis != NULL && AVIS_ISDUMMY (avis)) {
        DBUG_PRINT ("removing dummy assignment to %s", AVIS_NAME (avis));
        arg_node = FREEdoFreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRap (node *arg_node, info *arg_info)
 *
 * @brief Remove dummy arguments from function applications.
 *
 ******************************************************************************/
node *
DDRap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    // Nothing to do if the applied function cannot have unused arguments
    if (UAAcanHaveUnusedArguments (AP_FUNDEF (arg_node))) {
        INFO_FORMALARGS (arg_info) = FUNDEF_ARGS (AP_FUNDEF (arg_node));
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
        INFO_FORMALARGS (arg_info) = NULL;
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRexprs (node *arg_node, info *arg_info)
 *
 * @brief Remove dummy arguments from function applications.
 *
 ******************************************************************************/
node *
DDRexprs (node *arg_node, info *arg_info)
{
    node *formal;

    DBUG_ENTER ();

    if (INFO_FORMALARGS (arg_info) != NULL) {
        formal = INFO_FORMALARGS (arg_info);
        INFO_FORMALARGS (arg_info) = ARG_NEXT (INFO_FORMALARGS (arg_info));
        EXPRS_NEXT (arg_node) = TRAVopt (EXPRS_NEXT (arg_node), arg_info);

        if (!ARG_ISUSEDINBODY (formal)) {
            DBUG_PRINT ("removing actual argument %s",
                        ID_NAME (EXPRS_EXPR (arg_node)));
            arg_node = FREEdoFreeNode (arg_node);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @fn node *DDRids (node *arg_node, info *arg_info)
 *
 * @brief Store the N_ids AVIS in the info.
 *
 ******************************************************************************/
node *
DDRids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_IDAVIS (arg_info) = IDS_AVIS (arg_node);

    DBUG_RETURN (arg_node);
}
