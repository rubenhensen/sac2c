/*
 * $Id:
 */

/** <!--********************************************************************-->
 *
 * @defgroup syn add sync statements to match spawns
 *
 *   Add sync statements to match spawns
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file add_sync.c
 *
 * Prefix: SYN
 *
 *****************************************************************************/
#include "add_sync.h"

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
    bool onefundef;
    node *fundef;
    node *newassign;
    node *lhs;
};

#define INFO_ONEFUNDEF(n) ((n)->onefundef)
#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_LHS(n) ((n)->lhs)
#define INFO_NEWASSIGN(n) ((n)->newassign)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_ONEFUNDEF (result) = TRUE;
    INFO_FUNDEF (result) = NULL;
    INFO_LHS (result) = NULL;
    INFO_NEWASSIGN (result) = NULL;

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
 * @fn node *SYNdoAddSync( node *argnode)
 *
 *****************************************************************************/

node *
SYNdoAddSync (node *argnode)
{
    info *info;
    DBUG_ENTER ("SYNdoAddSync");
    DBUG_PRINT ("SYN", ("Adding sync statements..."));

    info = MakeInfo ();

    INFO_ONEFUNDEF (info) = TRUE;

    TRAVpush (TR_syn);
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
 * @fn node *SYNap(node *arg_node, info *arg_info)
 *
 * @brief Traverses into ap nodes to find out which are spawned.
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
SYNap (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SYNap");
    DBUG_PRINT ("SYN", ("Traversing Ap node..."));

    if (AP_ISSPAWNED (arg_node)) {
        DBUG_PRINT ("SYN", (" - Found a spawn"));
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Add Sync -->
 *****************************************************************************/
