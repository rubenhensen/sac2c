/*
 * $Id$
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
#include "tree_compound.h"
#include "traverse.h"
#include "free.h"
#include "new_types.h"
#include "type_utils.h"
#include "rmcasts.h"
#include "update_wrapper_type.h"
#include "elim_alpha_types.h"
#include "elim_bottom_types.h"

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
 * @fn node *RCarray( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCarray (node *arg_node, info *arg_info)
{
    ntype *type, *new_type;

    DBUG_ENTER ("RCarray");

    type = ARRAY_ELEMTYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        new_type = TYeliminateUser (type);
        type = TYfreeType (type);
        ARRAY_ELEMTYPE (arg_node) = new_type;
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
 * @fn node *RCobjdef( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCobjdef (node *arg_node, info *arg_info)
{
    ntype *type, *new_type;

    DBUG_ENTER ("RCobjdef");

    type = OBJDEF_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        new_type = TYeliminateUser (type);
        type = TYfreeType (type);
        OBJDEF_TYPE (arg_node) = new_type;
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

    /**
     * FIXME: is this really needed here?
     *        shouldn't that go to UWT????
     */
    syntax_tree = EATdoEliminateAlphaTypes (syntax_tree);
    syntax_tree = EBTdoEliminateBottomTypes (syntax_tree);

    DBUG_RETURN (syntax_tree);
}
