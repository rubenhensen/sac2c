/*
 * $Id: wl_cost_check.c dpa $
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlcc With Loop Cost Check
 *
 * @brief With Loop Cost Check
 *
 * check if a WL is composed of trivial operations, which means
 * primitive functions but maximal one sel, no user defined
 * function.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file wl_cost_check.c
 *
 * Prefix: WLCC
 *
 *****************************************************************************/
#include "wl_cost_check.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "node_basic.h"
#include "print.h"
#include "dbug.h"
#include "traverse.h"
#include "str.h"
#include "memory.h"
#include "inferneedcounters.h"
#include "compare_tree.h"
#include "DupTree.h"
#include "free.h"
#include "LookUpTable.h"
#include "globals.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *with;
    int cost;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_WITH(n) ((n)->with)
#define INFO_COST(n) ((n)->cost)

static info *
MakeInfo (node *with)
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH (result) = with;
    INFO_COST (result) = 0;

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
 * @fn node *WLCCdoWLCostCheck( node *with)
 *
 * @brief global entry  point of With-Loop cost check
 *
 * @param with With-Node to do cost-check
 *
 * @return checked with node
 *
 *****************************************************************************/
node *
WLCCdoWLCostCheck (node *with)
{
    DBUG_ENTER ("WLCCdoWLCostCheck");

    DBUG_ASSERT ((NODE_TYPE (with) == N_with),
                 "WLCCdoWLCostCheck called for non-with node");

    info *arg_info = MakeInfo (with);

    TRAVpush (TR_wlcc);
    with = TRAVdo (with, arg_info);
    TRAVpop ();

    WITH_ISTRIVIAL (with) = INFO_COST (arg_info) > 1 ? FALSE : TRUE;

    DBUG_RETURN (with);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *WLCCwith( node *arg_node, info *arg_info)
 *
 * @brief applies WLCC on a with loop
 *
 *****************************************************************************/
node *
WLCCwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLCCwith");

    if (arg_node != INFO_WITH (arg_info)) {
        // This with-node is a nested with-node.
        INFO_COST (arg_info) += 2;
    } else {
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLCCprf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLCCprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLCCprf");

    if (PRF_PRF (arg_node) == F_sel) {
        INFO_COST (arg_info) += 1;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLCCap( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLCCap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLCCap");

    INFO_COST (arg_info) += 2;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With Loop Cost Check -->
 *****************************************************************************/
