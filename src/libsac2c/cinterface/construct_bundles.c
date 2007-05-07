/* $Id$ */

/** <!--********************************************************************-->
 *
 * @defgroup cbl Construct Bundles template
 *
 * Module description goes here.
 *
 * @ingroup cbl
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file construct_bundles.c
 *
 * Prefix: CBL
 *
 *****************************************************************************/

#include "construct_bundles.h"

#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *temp;
};

/**
 * A template entry in the template info structure
 */
#define INFO_TEMP(n) (n->temp)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_TEMP (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = MEMfree (info);

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
 * @fn node *CBLdoConstructBundles( node *syntax_tree)
 *
 *****************************************************************************/
node *
CBLdoConstructBundles (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CBLdoConstructBundles");

    info = MakeInfo ();

    TRAVpush (TR_cbl);
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

#if 0
/** <!--********************************************************************-->
 *
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static 
node *DummyStaticHelper(node *arg_node)
{
  DBUG_ENTER( "DummyStaticHelper");

  DBUG_RETURN( arg_node);
}
#endif

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
 * @fn node *CBLfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverses the fundef chain and adds all wrappers to a matching
 *        function bundle.
 *
 *****************************************************************************/
node *
CBLfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CBLfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CBLmodule(node *arg_node, info *arg_info)
 *
 * @brief Traverses only the fundefs chain, as all others cannot contain
 *        wrappers.
 *
 *****************************************************************************/
node *
CBLmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CBLmodule");

    if (MODULE_FUNS (arg_node) != NULL) {
        MODULE_FUNS (arg_node) = TRAVdo (MODULE_FUNS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal Construct Bundles -->
 *****************************************************************************/
