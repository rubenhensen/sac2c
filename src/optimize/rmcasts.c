/*
 * $Log$
 * Revision 1.7  2005/09/09 16:44:54  sbs
 * replaces casts by type_convs now.
 *
 * Revision 1.6  2005/09/08 11:05:14  sbs
 * Now, user defined types are eliminatred from the wrapper types too.
 *
 * Revision 1.5  2005/09/08 07:46:36  sbs
 * added RCtype
 *
 * Revision 1.4  2005/07/16 17:41:26  sbs
 * Now, all user types are resolved
 *
 * Revision 1.3  2004/11/26 14:36:47  mwe
 * SacDevCamp: compiles!
 *
 * Revision 1.2  2004/07/18 19:54:54  sah
 * switch to new INFO structure
 * PHASE I
 * (as well some code cleanup)
 *
 * Revision 1.1  2001/05/22 09:09:45  nmw
 * Initial revision
 *
 *
 */

/*****************************************************************************
 *
 * file:   rmcasts.c
 *
 * prefix: RM
 *
 * description:
 *
 *   This module removed all cast nodes from AST.
 *
 *****************************************************************************/

#include "dbug.h"
#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "rmcasts.h"
#include "update_wrapper_type.h"
#include "new2old.h"

/** <!--********************************************************************-->
 *
 * @fn node *RCcast( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCcast (node *arg_node, info *arg_info)
{
    node *expr, *type;

    DBUG_ENTER ("RCcast");

    expr = TRAVdo (CAST_EXPR (arg_node), arg_info);

    type = TBmakeType (TYeliminateUser (CAST_NTYPE (arg_node)));
    expr = TCmakePrf2 (F_type_conv, type, expr);

    CAST_EXPR (arg_node) = NULL;
    arg_node = FREEdoFreeTree (arg_node);

    DBUG_RETURN (expr);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCavis( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCavis (node *arg_node, info *arg_info)
{
    ntype *type, *new_type;

    DBUG_ENTER ("RCavis");

    type = AVIS_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        new_type = TYeliminateUser (type);
        type = TYfreeType (type);
        AVIS_TYPE (arg_node) = new_type;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCret( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCret (node *arg_node, info *arg_info)
{
    ntype *type, *new_type;

    DBUG_ENTER ("RCret");

    type = RET_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        new_type = TYeliminateUser (type);
        type = TYfreeType (type);
        RET_TYPE (arg_node) = new_type;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCtype( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCtype (node *arg_node, info *arg_info)
{
    ntype *type, *new_type;

    DBUG_ENTER ("RCtype");

    type = TYPE_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        new_type = TYeliminateUser (type);
        type = TYfreeType (type);
        TYPE_TYPE (arg_node) = new_type;
    }

    arg_node = TRAVcont (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node* RemoveCasts(node* syntax_tree)
 *
 * description:
 *   starts traversal of syntax tree to remove all casts.
 *
 ******************************************************************************/
node *
RCdoRemoveCasts (node *syntax_tree)
{

    DBUG_ENTER ("RemoveCasts");

    DBUG_PRINT ("OPT", ("starting remove casts traversal"));

    TRAVpush (TR_rc);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    syntax_tree = UWTdoUpdateWrapperType (syntax_tree);

    syntax_tree = NT2OTdoTransform (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
