/** <!--********************************************************************-->
 *
 * @defgroup Minimize the number of host<->device transfers in a
 *           sequential block of instructions.
 *
 *   This modules tries to eliminate <host2device>/<device2host> instructions
 *   in a sequential block of code. Two difference cases expose the opportunities
 *   for elimination:
 *
 *   1) a_host = device2host( b_dev);
 *      ...
 *      ...
 *      a_dev = host2device( a_host);
 *
 *      The second memory transfer, i.e. a_dev = host2device( a_host)
 *      can be eliminated. Any reference to a_dev after it will be
 *      replaced by b_dev.
 *
 *
 *
 *   2) b_dev = host2device( a_host);
 *      ...
 *      ...
 *      c_dev = host2device( a_host);
 *
 *      The second memory transfer, i.e. c_dev = host2device( a_host)
 *      can be eliminated. Any reference to c_dev after it will be
 *      replaced by b_dev.
 *
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file minimize_block_transfers.c
 *
 * Prefix: MBTRAN2
 *
 *****************************************************************************/
#include "minimize_block_transfers.h"

#include <stdlib.h>
#include "new_types.h"
#include "tree_compound.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "LookUpTable.h"
#include "memory.h"
#include "dbug.h"
#include "deadcoderemoval.h"
#include "cuda_utils.h"
#include "SSACSE.h"
#include "DupTree.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
};

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

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
 * @fn node *MBTRAN2doMinimizeBlockTransfers( node *syntax_tree)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRAN2doMinimizeBlockTransfers (node *syntax_tree)
{
    info *info;
    DBUG_ENTER ("MBTRAN2doMinimizeBlockTransfers");

    info = MakeInfo ();

    TRAVpush (TR_mbtran2);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    info = FreeInfo (info);

    syntax_tree = CSEdoCommonSubexpressionEliminationModule (syntax_tree);

    /* We rely on Dead Code Removal to remove the
     * unused <host2device>/<device2host> */
    syntax_tree = DCRdoDeadCodeRemovalModule (syntax_tree);

    DBUG_RETURN (syntax_tree);
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
 * @fn node *MBTRAN2prf( node *arg_node, info *arg_info)
 *
 * @brief
 *
 *****************************************************************************/
node *
MBTRAN2prf (node *arg_node, info *arg_info)
{
    node *ssaassign;

    DBUG_ENTER ("MBTRAN2prf");

    switch (PRF_PRF (arg_node)) {
    case F_host2device:
        ssaassign = AVIS_SSAASSIGN (ID_AVIS (PRF_ARG1 (arg_node)));
        if (ISDEVICE2HOST (ssaassign)) {
            node *dev_id = PRF_ARG1 (ASSIGN_RHS (ssaassign));
            node *dev_avis = ID_AVIS (dev_id);
            arg_node = FREEdoFreeNode (arg_node);
            arg_node = TBmakeId (dev_avis);
        }
        break;
    default:
        PRF_ARGS (arg_node) = TRAVopt (PRF_ARGS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
