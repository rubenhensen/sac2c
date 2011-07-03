/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup mss move sync statements to increase distance from spawn
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file move_sync_statement.c
 *
 * Prefix: MSS
 *
 *****************************************************************************/
#include "move_sync_statement.h"

#define DBUG_PREFIX "MSS"
#include "debug.h"

#include "tree_basic.h"
#include "memory.h"
#include "traverse.h"
#include "pattern_match.h"
#include "move_assigns.h"
#include <limits.h> /* MAX_INT */
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

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

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
 * @fn node *MSSdoMoveSyncStatement( node *syntax_tree)
 *
 *****************************************************************************/

node *
MSSdoMoveSyncStatement (node *syntax_tree)
{
    info *info;
    pattern *pat;
    pattern *stop_pat;
    DBUG_ENTER ();
    DBUG_PRINT ("Moving sync statements...");

    info = MakeInfo ();

    TRAVpush (TR_mss);

    pat = PMprf (1, PMAisPrf (F_sync), 0);
    stop_pat = PMfalse (0, 0);

    syntax_tree = MAdoMoveAssigns (syntax_tree, pat, FALSE, INT_MAX, stop_pat);

    TRAVpop ();

    pat = PMfree (pat);
    stop_pat = PMfree (stop_pat);

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
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Move Sync Statement -->
 *****************************************************************************/

#undef DBUG_PREFIX
