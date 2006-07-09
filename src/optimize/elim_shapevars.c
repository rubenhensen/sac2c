/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup esv Eliminate Shape Variables
 *
 * This traversal removes all expressions for dim and shape from AVIS nodes.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file elim_shapevars.c
 *
 * Prefix: ESV
 *
 *****************************************************************************/
#include "elim_shapevars.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "dbug.h"
#include "internal_lib.h"
#include "DupTree.h"
#include "new_types.h"
#include "type_utils.h"
#include "shape.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
};

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = ILIBmalloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = ILIBfree (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 * @}  <!-- INFO structure -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *ESVdoEliminateShapeVariables( node *syntax_tree)
 *
 *****************************************************************************/
node *
ESVdoEliminateShapeVariables (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("ESVdoEliminateShapeVariables");

    info = MakeInfo ();

    TRAVpush (TR_esv);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

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
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *ESVavis( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
ESVavis (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ESVavis");

    if (AVIS_DIM (arg_node) != NULL) {
        AVIS_DIM (arg_node) = FREEdoFreeNode (AVIS_DIM (arg_node));
    }

    if (AVIS_SHAPE (arg_node) != NULL) {
        AVIS_SHAPE (arg_node) = FREEdoFreeNode (AVIS_SHAPE (arg_node));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Insert Shape Variables -->
 *****************************************************************************/
