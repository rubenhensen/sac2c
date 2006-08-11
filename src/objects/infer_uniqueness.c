/* $Id$ */

#include "infer_uniqueness.h"

#include "ctinfo.h"
#include "dbug.h"
#include "internal_lib.h"
#include "new_types.h"
#include "traverse.h"
#include "tree_basic.h"
#include "type_utils.h"
#include "user_types.h"

/*
 * This traversal infers the uniqueness properties of all variables by
 * tracing their ancestry along the tree, marking IS_UNIQUE in AVIS nodes
 * as it goes.
 */

/*
 * INFO structure
 */
struct INFO {
    node *lhs;
};

/*
 * INFO macros
 */
#define INFO_LHS(n) ((n)->lhs)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/*
 * Traversal start function
 */
node *
IUdoInferUniqueness (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IUdoInferUniqueness");

    info = MakeInfo ();

    TRAVpush (TR_iu);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);
    DBUG_RETURN (syntax_tree);
}

/*
 * Traversal functions
 */

node *
IUap (node *arg_node, info *arg_info)
{
    node *arg, *lhs;

    DBUG_ENTER ("IUap");

    if (AP_ARGS (arg_node) != NULL) {
        AP_ARGS (arg_node) = TRAVdo (AP_ARGS (arg_node), arg_info);
    }

    /*
     * Loop through the LHS return values and the function's arguments
     * one-by-one, marking the LHS as unique if the function argument
     * is unique.
     */
    lhs = INFO_LHS (arg_info);
    arg = AP_ARGS (arg_node);

    while (arg != NULL) {

        arg = EXPRS_NEXT (arg);
        lhs = IDS_NEXT (lhs);
    }

    DBUG_RETURN (arg_node);
}

node *
IUassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUassign");

    DBUG_RETURN (arg_node);
}

node *
IUfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUfundef");

    DBUG_RETURN (arg_node);
}

node *
IUlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUlet");

    if (LET_IDS (arg_node) != NULL) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);
    }

    INFO_LHS (arg_info) = LET_IDS (arg_node);

    if (LET_EXPR (arg_node) != NULL) {
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    INFO_LHS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}
