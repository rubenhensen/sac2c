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

#include "dbug.h"
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
};

#define INFO_COUNT(n) ((n)->count)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_COUNT (result) = 0;

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
 * @fn node *CSSdoCountSpawnSync( node *argnode)
 *
 *****************************************************************************/

node *
CSSdoCountSpawnSync (node *argnode)
{
    info *info;
    DBUG_ENTER ("CSSdoCountSpawnSync");
    DBUG_PRINT ("CSS", ("Counting spawn and sync nodes"));

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
    DBUG_ENTER ("CSSfundef");

    DBUG_PRINT ("CSS", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    if (FUNDEF_CONTAINSSPAWN (arg_node)) {
        INFO_COUNT (arg_info) = 0;
        FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

        DBUG_PRINT ("CSS", ("Num Found: %i", INFO_COUNT (arg_info)));
        FUNDEF_NUMSPAWNSYNC (arg_node) = INFO_COUNT (arg_info);
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ("CSSap");

    if (AP_ISSPAWNED (arg_node)) {
        INFO_COUNT (arg_info)++;
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
    DBUG_ENTER ("CSSprf");

    if (PRF_PRF (arg_node) == F_sync) {
        INFO_COUNT (arg_info)++;
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Tag Ap Nodes -->
 *****************************************************************************/