/** <!--********************************************************************-->
 *
 * @defgroup sfwo Set fundef WAS_OPTIMIZED
 *
 * @brief  This phase sets FUNDEF_WASOPTIMIZED flags on all fundefs.
 *
 * @ingroup sfwo
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file setfundefwasoptimized.c
 *
 * Prefix: SFWO
 *
 *****************************************************************************/
#include "setfundefwasoptimized.h"

/*
 * Other includes go here
 */

#define DBUG_PREFIX "SFWO"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "globals.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SFWOdoSetFundefWasOptimized( node *arg_node)
 *
 *****************************************************************************/

node *
SFWOdoSetFundefWasOptimized (node *syntax_tree)
{

    DBUG_ENTER ();

    DBUG_PRINT ("Setting FUNDEF_WASOPTIMIZED flags starts");
    DBUG_ASSERT (N_module == NODE_TYPE (syntax_tree),
                 "SFWOdoSetFundefWasOptimizedModule needs N_module node");

    TRAVpush (TR_sfwo);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("Setting FUNDEF_WASOPTIMIZED flags ends");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *SFWOfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverses into fundef chain
 *
 *****************************************************************************/

node *
SFWOmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    MODULE_FUNS (arg_node) = TRAVopt(MODULE_FUNS (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SFWOfundef(node *arg_node, info *arg_info)
 *
 * @brief Sets FUNDEF_WASOPTIMIZED flag
 *
 *****************************************************************************/

node *
SFWOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    FUNDEF_WASOPTIMIZED (arg_node) = TRUE;

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt(FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

#undef DBUG_PREFIX
