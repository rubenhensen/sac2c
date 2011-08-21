/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup rmo Remove unused vardecs
 *
 * To prevent superfluous vardecs hanging around in the AST, this traversal
 * quickly eliminates all vardecs no IDS exists for.
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file remove_vardecs.c
 *
 * Prefix: RMV
 *
 *****************************************************************************/
#include "remove_vardecs.h"

#define DBUG_PREFIX "RMV"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "free.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/
struct INFO {
    bool onefundef;
    enum { TM_init, TM_delete } travmode;
};

#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_TRAVMODE(n) ((n)->travmode)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = FALSE;
    INFO_TRAVMODE (result) = TM_init;

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
 * @fn node *RMVdoRemoveVardecsOneFundef( node *fundef)
 *
 *****************************************************************************/
node *
RMVdoRemoveVardecsOneFundef (node *fundef)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "RMVdoRemoveVardecsOneFundef called on non N_fundef node");

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_rmv);
    fundef = TRAVdo (fundef, info);
    TRAVpop ();

    info = FreeInfo (info);

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
 * @fn node *RMVfundef(node *arg_node, info *arg_info)
 *
 * @brief Performs a traversal of the fundef chain entering the body
 *
 *****************************************************************************/
node *
RMVfundef (node *arg_node, info *arg_info)
{
    bool old_onefundef;
    DBUG_ENTER ();

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    old_onefundef = INFO_ONEFUNDEF (arg_info);
    INFO_ONEFUNDEF (arg_info) = FALSE;
    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    INFO_ONEFUNDEF (arg_info) = old_onefundef;

    if (!INFO_ONEFUNDEF (arg_info)) {
        FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMVblock( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMVblock (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (BLOCK_ASSIGNS (arg_node) != NULL) {
        if (BLOCK_VARDECS (arg_node) != NULL) {
            INFO_TRAVMODE (arg_info) = TM_init;
            BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);

            BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);

            INFO_TRAVMODE (arg_info) = TM_delete;
            BLOCK_VARDECS (arg_node) = TRAVdo (BLOCK_VARDECS (arg_node), arg_info);
        } else {
            BLOCK_ASSIGNS (arg_node) = TRAVopt (BLOCK_ASSIGNS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMVvardec( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMVvardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    VARDEC_NEXT (arg_node) = TRAVopt (VARDEC_NEXT (arg_node), arg_info);

    switch (INFO_TRAVMODE (arg_info)) {
    case TM_init:
        AVIS_ISDEAD (VARDEC_AVIS (arg_node)) = TRUE;
        DBUG_PRINT ("marking %s as dead!", VARDEC_NAME (arg_node));
        break;

    case TM_delete:
        if (AVIS_ISDEAD (VARDEC_AVIS (arg_node))) {
            DBUG_PRINT ("deleting %s !", VARDEC_NAME (arg_node));
            arg_node = FREEdoFreeNode (arg_node);
        }
        break;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RMVids( node *arg_node, info *arg_info)
 *
 *****************************************************************************/
node *
RMVids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    AVIS_ISDEAD (IDS_AVIS (arg_node)) = FALSE;
    DBUG_PRINT ("marking %s as alive!", IDS_NAME (arg_node));

    IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Remove Vardecs -->
 *****************************************************************************/

#undef DBUG_PREFIX
