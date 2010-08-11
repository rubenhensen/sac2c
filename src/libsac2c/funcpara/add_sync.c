/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup syn add sync statements to match spawns
 *
 *   Add sync statements to match spawns.
 *
 *   Sync statements are added directly after the line with the spawn.
 *   A spawned function then returns a temporary value which is used inside
 *   the sync statement to create a data dependency.
 *
 *   The transformation looks like this:
 *
 *     x = spawn f();
 *
 *   becomes
 *
 *     sync id;
 *     ...
 *     id = spawn f();
 *     x = _sync_(id);
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
#include "shape.h"
#include "ctinfo.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *fundef;
    node *newassign;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_NEWASSIGN(n) ((n)->newassign)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static void ErrorOnSpawnInExport( node *fundef, node *let)
 *
 * @brief Give an error when a spawn is found inside an exported function.
 *
 * @param fundef
 * @param let
 *
 *****************************************************************************/
static void
ErrorOnSpawnInExport (node *fundef, node *let)
{
    DBUG_ENTER ("ErrorOnSpawnInExport");

    CTIerrorLine (NODE_LINE (let), "Spawn found in exported function %s",
                  FUNDEF_NAME (fundef));
    CTIerrorContinued ("Not allowed, create a wrapper function to resolve this");

    DBUG_VOID_RETURN;
}

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
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
 * @fn node *SYNfundef(node *arg_node, info *arg_info)
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
SYNfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SYNfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("SYN", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SYNassign(node *arg_node, info *arg_info)
 *
 * @brief Traverse into instructions and add new assignment node if needed
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
SYNassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("SYNassign");
    DBUG_PRINT ("SYN", ("Traversing Assign node"));

    INFO_NEWASSIGN (arg_info) = NULL;

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (INFO_NEWASSIGN (arg_info) != NULL) {
        // insert a new assignment node after the current one
        ASSIGN_NEXT (INFO_NEWASSIGN (arg_info)) = ASSIGN_NEXT (arg_node);
        ASSIGN_NEXT (arg_node) = INFO_NEWASSIGN (arg_info);

        INFO_NEWASSIGN (arg_info) = NULL;
    }

    // todo: could skip next node if it is the newassign node
    ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *SYNlet(node *arg_node, info *arg_info)
 *
 * @brief Look for let nodes containing ap nodes that are spawned.
 *        If so, create the matching sync statement and saved it in info node
 *
 * @param arg_node
 * @param arg_info
 *
 * @return arg_node
 *
 *****************************************************************************/
node *
SYNlet (node *arg_node, info *arg_info)
{
    node *sync;
    node *avis;

    DBUG_ENTER ("SYNlet");

    DBUG_PRINT ("SYN", ("Traversing Let node"));

    if (NODE_TYPE (LET_EXPR (arg_node)) == N_ap && AP_ISSPAWNED (LET_EXPR (arg_node))) {

        DBUG_PRINT ("SYN", ("- found spawned ap"));

        if (FUNDEF_ISEXPORTED (INFO_FUNDEF (arg_info))) {
            ErrorOnSpawnInExport (INFO_FUNDEF (arg_info), arg_node);
        }

        // Create an avis node to hold temporary spawn value
        avis = TBmakeAvis (TRAVtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_sync), SHmakeShape (0)));

        // Create a new vardec node
        FUNDEF_VARDEC (INFO_FUNDEF (arg_info))
          = TBmakeVardec (avis, FUNDEF_VARDEC (INFO_FUNDEF (arg_info)));

        // Create the assign node containing sync
        sync = TBmakeAssign (TBmakeLet (LET_IDS (arg_node),
                                        TBmakePrf (F_sync,
                                                   TBmakeExprs (TBmakeId (avis), NULL))),
                             NULL);

        INFO_NEWASSIGN (arg_info) = sync;

        // change ids to use newly created temp spawn value
        LET_IDS (arg_node) = TBmakeIds (avis, NULL);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Add Sync -->
 *****************************************************************************/
