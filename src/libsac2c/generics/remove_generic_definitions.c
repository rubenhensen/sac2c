/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup rgd remove generic defintions
 *
 * Removes all generic function definitions from the syntax tree.
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

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "traverse.h"
#include "tree_basic.h"
#include "map_fun_trav.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!-- ****************************************************************** -->
 * @brief Transforms function into a zombie if it was a generic function.
 *
 * @param fundef a function
 * @param arg_info not used
 *
 * @return the possibly transformed function
 ******************************************************************************/
static node *
RemoveGenericFun (node *fundef, info *arg_info)
{
    DBUG_ENTER ();

    if (FUNDEF_ISGENERIC (fundef)) {
        fundef = FREEdoFreeNode (fundef);
    }

    DBUG_RETURN (fundef);
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

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
    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (syntax_tree) = N_module,
                 "RGDdoRemoveGenericDefinitions expects a module node as argument!");

    MODULE_FUNS (syntax_tree)
      = MFTdoMapFunTrav (MODULE_FUNS (syntax_tree), NULL, RemoveGenericFun);

    DBUG_RETURN (syntax_tree);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!--  Remove Generic Definitions -->
 *****************************************************************************/

#undef DBUG_PREFIX
