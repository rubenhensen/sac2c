/*
 * $Id: Index Vector Extrema Cleanup.c 16070 2009-05-12 04:11:07Z rbe $
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivexc Index Vector Extrema Cleanup
 *
 * @brief  This phase resets AVIS_MINVAL, AVIS_MAXVAL,
 *         and WL_COUNTING_WL fields.
 *
 * @ingroup ivexc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file ivexcleanup.c
 *
 * Prefix: IVEXC
 *
 *****************************************************************************/
#include "ivexcleanup.h"

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
 * @fn node *IVEXCdoIndexVectorExtremaCleanup( node *syntax_tree)
 *
 *****************************************************************************/
node *
IVEXCdoIndexVectorExtremaCleanup (node *syntax_tree)
{

    DBUG_ENTER ("IVEXCdoIndexVectorExtremaCleanup");

    DBUG_PRINT ("SCC", ("Index vector extrema cleanup strip traversal starts."));

    TRAVpush (TR_ivexc);
    syntax_tree = TRAVdo (syntax_tree, NULL);
    TRAVpop ();

    DBUG_PRINT ("TEMP", ("Index vector extrema cleanup traversal complete."));

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
 * @fn node *IVEXCwith(node *arg_node, info *arg_info)
 *
 * @brief  Resets N_with fields no longer needed by SWLF.
 *
 *****************************************************************************/
node *
IVEXCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXCwith");

    WITH_ISEXTREMAINSERTED (arg_node) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVEXCavis(node *arg_node, info *arg_info)
 *
 * @brief Clears AVIS_MINVAL, AVIS_MAXVAL references
 *
 *****************************************************************************/
node *
IVEXCavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IVEXCavis");

    AVIS_MINVAL (arg_node) = NULL;
    AVIS_MAXVAL (arg_node) = NULL;
    AVIS_COUNTING_WL (arg_node) = NULL;
    AVIS_WL_NEEDCOUNT (arg_node) = 0;

    DBUG_RETURN (arg_node);
}
