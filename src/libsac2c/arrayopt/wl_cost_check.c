/*
 * $Id: wl_cost_check.c dpa $
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlcc With Loop Cost Check
 *
 * @brief With Loop Cost Check
 *
 * TODO: check if a WL is composed of trivial operation. that
 *       means a cheap with loop.
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH (result) = NULL;
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
 * @fn node *WLCCdoWLCostCheck( node *fundef)
 *
 * @brief global entry  point of With-Loop cost check
 *
 * @param fundef Fundef-Node to check cost
 *
 * @return checked fundef node
 *
 *****************************************************************************/
node *
WLCCdoWLCostCheck (node *fundef)
{
    DBUG_ENTER ("WLCCdoWLCostCheck");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "WLCCdoWLCostCheck called for non-fundef node");

    TRAVpush (TR_wlcc);
    fundef = TRAVdo (fundef, NULL);
    TRAVpop ();

    DBUG_RETURN (fundef);
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
 * @fn int estimate( node *assign)
 *
 * @brief estimate the cost of the assign node
 *
 *****************************************************************************/
static int
estimate (node *assign)
{
    int cost = 0;

    DBUG_ENTER ("estimate");

    // TODO

    DBUG_RETURN (cost);
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
 * @fn node *WLCCfundef(node *arg_node, info *arg_info)
 *
 * @brief applies WLCC to a given fundef.
 *
 *****************************************************************************/
node *
WLCCfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLCCfundef");

    if (FUNDEF_BODY (arg_node) != NULL) {

        DBUG_PRINT ("WLCC", ("With Loop Cost Check  %s begins", FUNDEF_NAME (arg_node)));

        arg_info = MakeInfo ();
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);
        arg_info = FreeInfo (arg_info);

        DBUG_PRINT ("WLCC",
                    ("With Loop Cost Check %s completes", FUNDEF_NAME (arg_node)));
    }

    DBUG_RETURN (arg_node);
}

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

    INFO_WITH (arg_info) = arg_node;
    WITH_ISTRIVIAL (arg_node) = FALSE;
    INFO_COST (arg_info) = 0;

    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    if (arg_node == INFO_WITH (arg_info) && INFO_COST (arg_info) <= 1) {

        WITH_ISTRIVIAL (arg_node) = TRUE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node WLCCassign( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
WLCCassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLCCassign");

    if (INFO_WITH (arg_info) != NULL) {
        INFO_COST (arg_info) += estimate (arg_node);
    }

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With Loop Cost Check -->
 *****************************************************************************/
