/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup rgd remove generic defintions
 *
 * Removes all generic function definitions from the syntax tree
 *
 * @ingroup rgd
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file remove_generic_definitions.c
 *
 * Prefix: RGD
 *
 *****************************************************************************/
#include "remove_generic_definitions.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name Entry functions
 * @{
 *
 *****************************************************************************/
/** <!--********************************************************************-->
 *
 * @fn node *RGDdoRemoveGenericDefinitions( node *syntax_tree)
 *
 *****************************************************************************/
node *
RGDdoRemoveGenericDefinitions (node *syntax_tree)
{
    DBUG_ENTER ("RGDdoRemoveGenericDefinitions");

    DBUG_ASSERT ((NODE_TYPE (syntax_tree) = N_module),
                 "RGDdoRemoveGenericDefinitions expects a module node as argument!");

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!--  Remove Generic Definitions -->
 *****************************************************************************/
