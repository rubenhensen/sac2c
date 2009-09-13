/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup
 *
 * A very simple traversal to locate and tag the main function as a
 * thread function.
 *
 * This traversal should not be needed once sl can switch from C mode to sl
 * mode
 *
 * This traversal must come before:
 * tag preparation
 * consistently rename identifiers
 *
 * @ingroup
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file tag_main_fun_thread.c
 *
 * Prefix: TMFT
 *
 *****************************************************************************/
#include "tag_main_fun_thread.h"

/*
 * Other includes go here
 */
#include "dbug.h"
#include "traverse.h"
#include "tree_basic.h"
#include "memory.h"
#include "str.h"

/*
 * Number of main functions expected
 * wrapper
 * actual
 */

#define EXPECTED 2

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    int found;
};

#define INFO_FOUND(n) (n->found)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FOUND (result) = 0;

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
 * @fn node *TMFTdoTagMainFunThread( node *syntax_tree)
 *
 *****************************************************************************/
node *
TMFTdoTagMainFunThread (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("TMFTdoTagMainFunThread");

    info = MakeInfo ();

    DBUG_PRINT ("TMFT", ("Starting Tag main function thread function."));

    TRAVpush (TR_tmft);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_ASSERTF ((INFO_FOUND (info) == EXPECTED),
                  ("Did not find correct number of main functions, found %d, "
                   "expected %d",
                   INFO_FOUND (info), EXPECTED));

    DBUG_PRINT ("TMFT", ("Finished Tag main function thread function."));

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
 * @fn node *TMFTfundef(node *arg_node, info *arg_info)
 *
 * @brief If this is the main function tag as a thread function.
 *
 *****************************************************************************/
node *
TMFTfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TMFTfundef");

    arg_node = TRAVcont (arg_node, arg_info);

    if (STReq (FUNDEF_NAME (arg_node), "main")) {
        FUNDEF_ISTHREADFUN (arg_node) = TRUE;
        FUNDEF_ISFUNTHREADFUN (arg_node) = TRUE;
        INFO_FOUND (arg_info) = INFO_FOUND (arg_info) + 1;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
