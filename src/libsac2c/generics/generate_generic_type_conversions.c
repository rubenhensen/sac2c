/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup ggtc Generate Generic Type Conversions
 *
 * Generates generic conversion functions:
 *   - to_XXX / from_XXX for classtypes
 *   - wrapXXX / unwrapXXX for user defined types
 *
 * @ingroup ggtc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file generate_generic_type_conversions.c
 *
 * Prefix: GGTC
 *
 *****************************************************************************/
#include "generate_generic_type_conversions.h"

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
    node *providedsymbols;
};

/**
 * A template entry in the template info structure
 */
#define INFO_PROVIDEDSYMBOLS(n) ((n)->providedsymbols)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_PROVIDEDSYMBOLS (result) = NULL;

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
 * @fn node *GGTCdoGenerateGenericTypeConversions( node *syntax_tree)
 *
 *****************************************************************************/
node *
GGTCdoGenerateGenericTypeConversions (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("GGTCdoGenerateGenericTypeConversions");

    info = MakeInfo ();

    TRAVpush (TR_ggtc);
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
 * @fn node *GGTCmodule( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCmodule");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *GGTCtypedef( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
GGTCtypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("GGTCtypedef");

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = TRAVdo (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal -->
 *****************************************************************************/
