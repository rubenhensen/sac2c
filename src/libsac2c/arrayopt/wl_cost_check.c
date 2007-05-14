/*
 * $Id: wl_cost_check.c dpa $
 */

/** <!--********************************************************************-->
 *
 * @defgroup wlcc With Loop Cost Check
 *
 * @brief Computes the cost of With Loops
 *
 * Definition of Cost function of With Loops
 *   SAC Function's Structure
 *     fundef         ::= assign*
 *     assign         ::= with-loop | sel | user-defined-fun | other
 *     with-loop      ::= with-part+
 *     with-part      ::= assign+
 *
 *     cost(fundef)    = sum of all assigns in the function
 *     cost(with-loop) = max cost(with-part) for all with-part in with-loop
 *     cost(sel)       = 1
 *     cost(other)     = 0
 *     cost(with-part) = sum of all assigns in the with-part
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
#include "node_basic.h"
#include "print.h"
#include "dbug.h"
#include "traverse.h"
#include "memory.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    node *with;
    bool do_check;
    int code_cost;
};

/**
 * Macro definitions for INFO structure
 */
#define INFO_WITH(n) ((n)->with)
#define INFO_DO_CHECK(n) ((n)->do_check)
#define INFO_CODE_COST(n) ((n)->code_cost)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_WITH (result) = NULL;
    INFO_DO_CHECK (result) = FALSE;
    INFO_CODE_COST (result) = 0;

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
 * @brief global entry  point of With Loop Cost Check
 *
 * @param with Fundef Node to do Cost Check
 *
 * @return checked node
 *
 *****************************************************************************/
node *
WLCCdoWLCostCheck (node *fundef)
{
    info *arg_info;

    DBUG_ENTER ("WLCCdoWLCostCheck");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "WLCCdoWLCostCheck called for non-fundef node");

    arg_info = MakeInfo ();

    TRAVpush (TR_wlcc);
    fundef = TRAVdo (fundef, arg_info);
    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (fundef);
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
    node *outer_with;

    DBUG_ENTER ("WLCCwith");

    if (INFO_DO_CHECK (arg_info)) {
        INFO_CODE_COST (arg_info) += WITH_COST (arg_node);
    } else {
        outer_with = INFO_WITH (arg_info);
        INFO_WITH (arg_info) = arg_node;

        arg_node = TRAVcont (arg_node, arg_info);

        /* Now all inner With Loops have their cost. */
        INFO_DO_CHECK (arg_info) = TRUE;
        INFO_WITH (arg_info) = outer_with;
        WITH_COST (arg_node) = 0; /* Initialize cost value */
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_DO_CHECK (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLCCcode( node *arg_node, info *arg_info)
 *
 * @brief applies WLCC on a with code
 *
 *****************************************************************************/
node *
WLCCcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("WLCCcode");

    if (!INFO_DO_CHECK (arg_info)) {
        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        INFO_CODE_COST (arg_info) = 0;
        if (CODE_CBLOCK (arg_node) != NULL) {
            CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);
        }

        if (INFO_CODE_COST (arg_info) > WITH_COST (INFO_WITH (arg_info))) {
            WITH_COST (INFO_WITH (arg_info)) = INFO_CODE_COST (arg_info);
        }

        if (CODE_NEXT (arg_node) != NULL) {
            CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
        }
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
        INFO_CODE_COST (arg_info) += 1;
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

    /* TODO: call cost check on user defined functions */
    INFO_CODE_COST (arg_info) += 2;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With Loop Cost Check -->
 *****************************************************************************/
