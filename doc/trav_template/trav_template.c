/** <!--********************************************************************-->
 *
 * @defgroup temp Traversal template
 *
 * <pre>
 * Property                                | should be | y/n |  who  |  when
 * =============================================================================
 * can be called on N_module               |   -----   |     |       |
 * can be called on N_fundef               |   -----   |     |       |
 * expects LaC funs                        |   -----   |     |       |
 * follows N_ap to LaC funs                |   -----   |     |       |
 * =============================================================================
 * deals with GLF properly                 |    yes    |     |       |
 * =============================================================================
 * is aware of potential SAA annotations   |    yes    |     |       |
 * utilises SAA annotations                |   -----   |     |       |
 * =============================================================================
 * tolerates flattened N_array             |    yes    |     |       |
 * tolerates flattened Generators          |    yes    |     |       |
 * tolerates flattened operation parts     |    yes    |     |       |
 * tolerates different generator variables
 *           in individual WL partitions   |    yes    |     |       |
 * =============================================================================
 * tolerates multi-operator WLs            |    yes    |     |       |
 * =============================================================================
 * </pre>
 *
 * Module description goes here.
 *
 * For an example, take a look at src/refcount/explicitcopy.c
 *
 * @ingroup tt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file trav_template.c
 *
 * Prefix: TEMP
 *
 *****************************************************************************/
#include "trav_template.h"

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
#define INFO_TEMP(n) ((n) > temp)

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
 * @fn node *TEMPdoTemplateTraversal( node *syntax_tree)
 *
 *****************************************************************************/
node *
TEMPdoTemplateTraversal (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("TEMPdoTemplateTraversal");

    info = MakeInfo ();

    DBUG_PRINT ("TEMP", ("Starting template traversal."));

    TRAVpush (TR_temp);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("TEMP", ("Template traversal complete."));

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
 *
 * @fn node *DummyStaticHelper(node *arg_node)
 *
 * @brief A dummy static helper functions used only in your traversal
 *
 *****************************************************************************/
static node *
DummyStaticHelper (node *arg_node)
{
    DBUG_ENTER ("DummyStaticHelper");

    DBUG_RETURN (arg_node);
}

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
 * @fn node *TEMPfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
TEMPfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TEMPfundef");

    FUNDEF_NEXT (arg_node) = TRAVopt(FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
