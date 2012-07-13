/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup css count the number of spawns and syncs in function
 *
 * TODO: add the count itself to spawn or sync node
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file count_spawn_sync.c
 *
 * Prefix: CSS
 *
 *****************************************************************************/
#include "count_spawn_sync.h"

#define DBUG_PREFIX "CSS"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "memory.h"
#include "traverse.h"
#include "new_types.h"
#include "new_typecheck.h"
#include "constants.h"
#include "type_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    int count;
    node *let;
};

#define INFO_COUNT(n) ((n)->count)
#define INFO_LET(n) ((n)->let)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_COUNT (result) = 0;
    INFO_LET (result) = NULL;

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
 * @fn node *CSSdoCountSpawnSync( node *argnode)
 *
 *****************************************************************************/

node *
CSSdoCountSpawnSync (node *argnode)
{
    info *info;
    DBUG_ENTER ();
    DBUG_PRINT ("Counting spawn and sync nodes");

    info = MakeInfo ();

    TRAVpush (TR_css);
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
 * @name Traversal functions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *CSSfundef(node *arg_node, info *arg_info)
 *
 * @brief Traverses into fundef local LAC functions, then function
 *        bodies and finally function next pointers. When traversing
 *        into a body a pointer in the info struct is maintained to
 *        the inner fundef.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CSSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    DBUG_PRINT ("traversing body of (%s) %s",
                (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                FUNDEF_NAME (arg_node));

    if (FUNDEF_CONTAINSSPAWN (arg_node)) {
        INFO_COUNT (arg_info) = 0;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("Num Found: %i", INFO_COUNT (arg_info));
        FUNDEF_NUMSPAWNSYNC (arg_node) = INFO_COUNT (arg_info);
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSSlet(node *arg_node, info *arg_info)
 *
 * @brief ...
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CSSlet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_LET (arg_info) = arg_node;
    LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSSap(node *arg_node, info *arg_info)
 *
 * @brief ...
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CSSap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AP_ISSPAWNED (arg_node)) {
        DBUG_PRINT ("Spawn index: %d", INFO_COUNT (arg_info));
        // check: does inc and assign work okay at same time?
        LET_SPAWNSYNCINDEX (INFO_LET (arg_info)) = INFO_COUNT (arg_info)++;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CSSprf(node *arg_node, info *arg_info)
 *
 * @brief ...
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
CSSprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PRF_PRF (arg_node) == F_sync) {
        DBUG_PRINT ("Sync index:  %d", INFO_COUNT (arg_info));
        // check: does inc and assign work okay at same time?
        LET_SPAWNSYNCINDEX (INFO_LET (arg_info)) = INFO_COUNT (arg_info)++;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Tag Ap Nodes -->
 *****************************************************************************/

#undef DBUG_PREFIX
