/*
 * $Id$
 */

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
#include "dbug.h"
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
 * @fn node *SFWOdoSetFundefWasOptimizedModule( node *arg_node)
 *
 *****************************************************************************/
node *
SFWOdoSetFundefWasOptimizedModule (node *arg_node)
{

    DBUG_ENTER ("SFWOdoSetFundefWasOptimizedModule");

    DBUG_PRINT ("SFWO", ("Setting FUNDEF_WASOPTIMIZED flags starts"));
    DBUG_ASSERT (N_module == NODE_TYPE (arg_node),
                 ("SFWOdoSetFundefWasOptimizedModule needs N_module node"));

    TRAVpush (TR_sfwo);
    arg_node = TRAVdo (arg_node, NULL);
    TRAVpop ();

    DBUG_PRINT ("SFWO", ("Setting FUNDEF_WASOPTIMIZED flags ends"));

    DBUG_RETURN (arg_node);
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
 * @brief Sets FUNDEF_WASOPTIMIZED flag
 *
 *****************************************************************************/
node *
SFWOfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SFWOfundef");

    FUNDEF_WASOPTIMIZED (arg_node) = TRUE;

    DBUG_RETURN (arg_node);
}
