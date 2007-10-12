/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup iutc Introduce User Trace Calls Traversal
 *
 * Module description goes here.
 *
 * @ingroup iutc
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file introduce_user_tracing_calls.c
 *
 * Prefix: IUTC
 *
 *****************************************************************************/
#include "introduce_user_tracing_calls.h"

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
    node *postassign;
    node *preassign;
};

/**
 * A template entry in the template info structure
 */
#define INFO_POSTASSIGN(n) (n->postassign)
#define INFO_PREASSIGN(n) (n->preassign)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_POSTASSIGN (result) = NULL;
    INFO_PREASSIGN (result) = NULL;

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
 * @fn node *IUTCdoIntroduceUserTraceCalls( node *syntax_tree)
 *
 *****************************************************************************/
node *
IUTCdoIntroduceUserTraceCalls (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("IUTCdoIntroduceUserTraceCalls");

    info = MakeInfo ();

    TRAVpush (TR_iutc);
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
 * @fn node *IUTCfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain.
 *
 *****************************************************************************/
node *
IUTCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUTCfundef");

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = TRAVdo (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTClet(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the let chain.
 *
 *****************************************************************************/
node *
IUTClet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUTClet");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCreturn(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the return chain.
 *
 *****************************************************************************/
node *
IUTCreturn (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUTCreturn");

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *IUTCassign(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the assign chain.
 *
 *****************************************************************************/
node *
IUTCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("IUTCassign");

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Introduce User Trace Calls -->
 *****************************************************************************/
