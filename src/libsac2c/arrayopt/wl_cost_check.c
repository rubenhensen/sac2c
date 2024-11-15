/** <!--********************************************************************-->
 *
 * @defgroup wlcc With Loop Cost Estimate
 *
 * @brief Computes a crude estimate of the per-element-computation
 *        cost of each with-loop partition.
 *
 *        Basically, we want to identify the set of producerWLs
 *        that are worthy of being folded into more than one
 *        consumerWLs. A typical example of this is:
 *
 *           tod( genarray(iota(N)))
 *
 *        where the per-element computation cost is small.
 *
 *        There is no problem in making this function more
 *        precise, should you feel the need.
 *
 * The cost is associated with each N_code, as the cost
 * may vary greatly from partition to partition.
 *
 * Definition of Cost function of with-loop partition:
 *   SAC Function's Structure
 *
 *     cost(primitive fn):                  1
 *        e.g., F_add_SxV,  F_tod_S_
 *     cost( indexing):                     1
 *
 *     cost( N_ap):                        infinite
 *     cost( N_with):                      infinite
 *     cost( guards)                       0
 *     cost( extrema)                      0
 *     cost( other):                       infinite
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

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

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
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_WITH (result) = NULL;
    INFO_DO_CHECK (result) = FALSE;
    INFO_CODE_COST (result) = 0;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef, "Called with non-fundef node");

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

    DBUG_ENTER ();

    if (INFO_DO_CHECK (arg_info)) {
        INFO_CODE_COST (arg_info) += 2;
    } else {
        outer_with = INFO_WITH (arg_info);
        INFO_WITH (arg_info) = arg_node;

        arg_node = TRAVcont (arg_node, arg_info);

        /* Now all inner With Loops have their cost. */

        INFO_DO_CHECK (arg_info) = TRUE;
#ifdef FIXME                      // wrong
        WITH_COST (arg_node) = 0; /* Initialize cost value */
#endif                            // FIXME
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);
        INFO_DO_CHECK (arg_info) = FALSE;
        INFO_WITH (arg_info) = outer_with;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *WLCCpart( node *arg_node, info *arg_info)
 *
 * @brief applies WLCC to an N_with
 *
 *****************************************************************************/
node *
WLCCpart (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

    CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

    if (!INFO_DO_CHECK (arg_info)) {
        arg_node = TRAVcont (arg_node, arg_info);
    } else {
        INFO_CODE_COST (arg_info) = 0;
        CODE_CBLOCK (arg_node) = TRAVopt (CODE_CBLOCK (arg_node), arg_info);

        /* FIXME wrong
        if( INFO_CODE_COST( arg_info) > WITH_COST( INFO_WITH( arg_info))) {
          WITH_COST( INFO_WITH( arg_info)) = INFO_CODE_COST( arg_info);
        }
        */

        CODE_NEXT (arg_node) = TRAVopt (CODE_NEXT (arg_node), arg_info);
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
    DBUG_ENTER ();

    if ((PRF_PRF (arg_node) == F_sel_VxA) || (PRF_PRF (arg_node) == F_idx_sel)) {
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
    DBUG_ENTER ();

    INFO_CODE_COST (arg_info) += 2;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- With Loop Cost Check -->
 *****************************************************************************/

#undef DBUG_PREFIX
