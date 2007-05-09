/* $Id$ */

/** <!--********************************************************************-->
 *
 * @defgroup cwh Create wrapper header
 *
 * Creates a wrapper header file from a given syntax tree.
 *
 * @ingroup cwh
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file creater_wrapper_header.c
 *
 * Prefix: CWH
 *
 *****************************************************************************/
#include "create_wrapper_header.h"

/*
 * Other includes go here
 */
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
 * @fn node *CWHdoCreateWrapperHeader( node *syntax_tree)
 *
 *****************************************************************************/
node *
CWHdoCreateWrapperHeader (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CWHdoCreateWrapperHeader");

    info = MakeInfo ();

    TRAVpush (TR_cwh);
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
 * @fn node *CWHfunbundle(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CWHfunbundle (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWHfunbundle");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWHfundef(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CWHfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWHfundef");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWHmodule(node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
CWHmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWHmodule");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Create Wrapper Headers -->
 *****************************************************************************/
