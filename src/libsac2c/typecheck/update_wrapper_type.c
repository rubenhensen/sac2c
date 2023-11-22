#include "update_wrapper_type.h"

#define DBUG_PREFIX "UWT"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "new_types.h"
#include "type_utils.h"

/**
 * traversal functions
 */

node *
UWTfundef (node *arg_node, info *arg_info)
{
    ntype *new_type, *type;
    node *fundef;

    DBUG_ENTER ();

    if (FUNDEF_ISWRAPPERFUN (arg_node)) {
        type = FUNDEF_WRAPPERTYPE (arg_node);
        if (TYisFun (type)) {
            new_type = TUrebuildWrapperTypeAlphaFix (type);
        } else {
            fundef = FUNDEF_IMPL (arg_node);
            FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
            new_type = TUmakeProductTypeFromRets (FUNDEF_RETS (fundef));
        }
        FUNDEF_WRAPPERTYPE (arg_node) = TYfreeType (type);
        FUNDEF_WRAPPERTYPE (arg_node) = new_type;
    }

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
UWTdoUpdateWrapperType (node *arg_node)
{

    DBUG_ENTER ();

    TRAVpush (TR_uwt);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
