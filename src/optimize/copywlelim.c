/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cwle Copy with-loop elimination
 *
 * This traversal takes care of with-loops that do nothing more
 * than copying some array. These are replaced by A = B, like in
 *
 * B = with { (.<=iv<=.) : A[iv]; } : genarray( shape(A), n );
 * will be transformed to
 * B = A;
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file copywlelim.c
 *
 * Prefix: CWLE
 *
 *****************************************************************************/
#include "copywlelim.h"

#include "globals.h"
#include "dbug.h"
#include "internal_lib.h"
#include "traverse.h"
#include "tree_basic.h"

/* these are not needed yet, maybe we will need them some day.
#include "print.h"
#include "tree_compound.h"
#include "new_types.h"
*/

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

    result = ILIBmalloc (sizeof (info));

    INFO_TEMP (result) = NULL;

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
 * @fn node *CWLEdoTemplateTraversal( node *syntax_tree)
 *
 *****************************************************************************/
node *
CWLEdoTemplateTraversal (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CWLEdoTemplateTraversal");

    info = MakeInfo ();

    DBUG_PRINT ("CWLE", ("Starting template traversal."));

    TRAVpush (TR_cwle);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("CWLE", ("Template traversal complete."));

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
 * @fn node *CWLEfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
CWLEfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEfundef");

    DBUG_PRINT ("CWLE", ("Calling CWLEfundef"));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEwith(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain without entering the body
 *
 *****************************************************************************/
node *
CWLEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEwith");

    DBUG_PRINT ("CWLE", ("Calling CWLEwith"));

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
