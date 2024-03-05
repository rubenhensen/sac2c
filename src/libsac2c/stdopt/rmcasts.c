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

#define DBUG_PREFIX "OPT"
#include "debug.h"

#include "types.h"
#include "memory.h"
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
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *lhs;
};

/**
 * A template entry in the template info structure
 */
#define INFO_LHS(n) ((n)->lhs)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_LHS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *RClet( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RClet (node *arg_node, info *arg_info)
{
    node *lhs;

    DBUG_ENTER ();

    lhs = INFO_LHS (arg_info);
    INFO_LHS (arg_info) = LET_IDS (arg_node);

    arg_node = TRAVcont (arg_node, arg_info);

    INFO_LHS (arg_info) = lhs;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RCcast( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCcast (node *arg_node, info *arg_info)
{
    node *expr, *type;

    DBUG_ENTER ();

    expr = TRAVdo (CAST_EXPR (arg_node), arg_info);

    DBUG_ASSERT (INFO_LHS (arg_info) != NULL, "I lost my left hand side");
    DBUG_ASSERT (IDS_NEXT (INFO_LHS (arg_info)) == NULL, "too much left hand side");

    type = TBmakeType (TYeliminateUser (AVIS_TYPE (IDS_AVIS (INFO_LHS (arg_info)))));
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 * @fn node *RCstructelem( node *arg_node, info *arg_info )
 *
 ******************************************************************************/

node *
RCstructelem (node *arg_node, info *arg_info)
{
    ntype *type, *new_type;

    DBUG_ENTER ();

    type = STRUCTELEM_TYPE (arg_node);
    if (TUisArrayOfUser (type)) {
        new_type = TYeliminateUser (type);
        type = TYfreeType (type);
        STRUCTELEM_TYPE (arg_node) = new_type;
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

    DBUG_ENTER ();

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
    info *info;

    DBUG_ENTER ();

    DBUG_PRINT ("starting remove casts traversal");

    info = MakeInfo ();

    TRAVpush (TR_rc);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    syntax_tree = UWTdoUpdateWrapperType (syntax_tree);

    /**
     * FIXME: is this really needed here?
     *        shouldn't that go to UWT????
     */
    syntax_tree = EATdoEliminateAlphaTypes (syntax_tree);
    syntax_tree = EBTdoEliminateBottomTypes (syntax_tree);

    DBUG_RETURN (syntax_tree);
}

#undef DBUG_PREFIX
