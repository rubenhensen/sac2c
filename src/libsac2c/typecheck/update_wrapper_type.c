/*
 * $Log$
 * Revision 1.1  2005/09/08 11:06:07  sbs
 * Initial revision
 *
 *
 *
 */

#include "update_wrapper_type.h"

#include "dbug.h"
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

    DBUG_ENTER ("UWTfundef");

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

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/**
 * traversal start function
 */
node *
UWTdoUpdateWrapperType (node *arg_node)
{

    DBUG_ENTER ("UWTdoUpdateWrapperType");

    TRAVpush (TR_uwt);

    arg_node = TRAVdo (arg_node, NULL);

    TRAVpop ();

    DBUG_RETURN (arg_node);
}
