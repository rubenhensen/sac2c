/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup ctz introduce a comparison to zero in compare statements
 *
 *   This module searches for comparisons and transforms them to a comparison
 *   with zero. This allows for further optimization of complex comparisons.
 *
 *     a + c > b + c => a + c - b + c > 0
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file comparison_to_zero.c
 *
 * Prefix: CZC
 *
 *****************************************************************************/
#include "comparison_to_zero.h"

#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"
#include "ctinfo.h"
#include "pattern_match.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool onefundef;
};

#define INFO_ONEFUNDEF(n) ((n)->onefundef)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;

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
 * @fn node *CTZdoComparisonToZero( node *argnode)
 *
 *****************************************************************************/
node *
CTZdoComparisonToZero (node *argnode)
{
    info *info;
    DBUG_ENTER ("CTZdoComparisonToZero");

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_ctz);
    argnode = TRAVdo (argnode, info);
    TRAVpop ();

    info = FreeInfo (info);

    DBUG_RETURN (argnode);
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
 * @fn node *CTZblock(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CTZblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZblock");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZassign(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CTZassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZassign");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZlet(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CTZlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZlet");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZprf(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CTZprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZprf");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CTZfundef(node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
CTZfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CTZfundef");

    DBUG_PRINT ("CTZ", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Conditional Zero Comparison -->
 *****************************************************************************/
