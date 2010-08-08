/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup mss move sync statements to increase distance
 *
 * WARNING! Can only move 1 sync stament per function for now
 *
 * @ingroup fp
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file move_spawn_sync.c
 *
 * Prefix: MSS
 *
 *****************************************************************************/
#include "move_sync_statement.h"

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
    node *fundef;
    node *prevassign;
    node *ids;
    bool idsfound;
    node *sync;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_IDS(n) ((n)->ids)
#define INFO_IDSFOUND(n) ((n)->idsfound)
#define INFO_SYNC(n) ((n)->sync)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_IDSFOUND (result) = FALSE;
    INFO_SYNC (result) = NULL;

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
 * @fn node *MSSdoMoveSyncStatement( node *argnode)
 *
 *****************************************************************************/

node *
MSSdoMoveSyncStatement (node *argnode)
{
    info *info;
    DBUG_ENTER ("MSSdoMoveSyncStatement");
    DBUG_PRINT ("MSS", ("Moving sync statements..."));

    info = MakeInfo ();

    TRAVpush (TR_mss);
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
 * @name Static helper funcions
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static bool AssignIsSync( node *assign)
 *
 * @brief Returns whether or not the assignment node is a sync statement
 *
 * @param node an N_assign node
 *
 * @return is assign node a sync statement
 *
 *****************************************************************************/
static bool
AssignIsSync (node *assign)
{
    bool result;
    node *instr;

    DBUG_ENTER ("AssignIsSync");

    DBUG_ASSERT (NODE_TYPE (assign) == N_assign, "Node must be an N_assign node");

    instr = ASSIGN_INSTR (assign);

    result = NODE_TYPE (instr) == N_let && NODE_TYPE (LET_EXPR (instr)) == N_prf
             && PRF_PRF (LET_EXPR (instr)) == F_sync;

    DBUG_RETURN (result);
}

/** <!--********************************************************************-->
 *
 * @fn static bool IdsOccurInNode( node *ids,  node *node)
 *
 * @brief Check to see if ids occur as N_id in node
 *
 * @param ids N_ids node
 * @param node
 * @param arg_info
 *
 * @return void
 *
 *****************************************************************************/
static bool
IdsOccurInNode (node *ids, node *node, info *arg_info)
{
    DBUG_ENTER ("IdsOccurInNode");

    DBUG_ASSERT (NODE_TYPE (ids) == N_ids, "Ids must be an N_ids node");
    DBUG_PRINT ("MSS", ("Starting traversal for ids."));

    INFO_IDS (arg_info) = ids;
    INFO_IDSFOUND (arg_info) = FALSE;

    TRAVopt (node, arg_info);

    INFO_IDS (arg_info) = NULL;

    if (!INFO_IDSFOUND (arg_info)) {
        DBUG_PRINT ("MSS", ("Node can be moved down"));
    }

    DBUG_RETURN (INFO_IDSFOUND (arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn static bool CanNodeMoveDown( node *ids,  node *node)
 *
 * @brief Check to see if ids occur as N_id in node
 *
 * @param ids N_ids node
 * @param node
 * @param arg_info
 *
 * @return void
 *
 *****************************************************************************/
static bool
CanNodeMoveDown (node *current, node *next, info *arg_info)
{
    bool result;
    node *ids;

    DBUG_ENTER ("CanNodeMoveDown");

    DBUG_ASSERT (NODE_TYPE (current) == N_assign,
                 "Current node must be an N_assign node");
    DBUG_ASSERT (NODE_TYPE (next) == N_assign, "Current node must be an N_assign node");
    DBUG_PRINT ("MSS", ("Checking to see if node can move down"));

    ids = LET_IDS (ASSIGN_INSTR (current));

    result = !IdsOccurInNode (ids, next, arg_info);

    DBUG_RETURN (result);
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
 * @fn node *MSSfundef(node *arg_node, info *arg_info)
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
MSSfundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("MSSfundef");

    INFO_FUNDEF (arg_info) = arg_node;

    DBUG_PRINT ("MSS", ("traversing body of (%s) %s",
                        (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                        FUNDEF_NAME (arg_node)));

    FUNDEF_BODY (arg_node) = TRAVopt (FUNDEF_BODY (arg_node), arg_info);

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);

    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSSassign(node *arg_node, info *arg_info)
 *
 * @brief Do something...
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
MSSassign (node *arg_node, info *arg_info)
{
    node *nextnode;

    DBUG_ENTER ("MSSassign");
    DBUG_PRINT ("MSS", ("Traversing Assign node"));

    if (INFO_IDS (arg_info) != NULL) {
        // Only traverse into instr if looking for ids
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        nextnode = ASSIGN_NEXT (arg_node);

        // remove sync from tree if it is the next node
        if (nextnode != NULL && AssignIsSync (nextnode)) {
            INFO_SYNC (arg_info) = nextnode;
            ASSIGN_NEXT (arg_node) = ASSIGN_NEXT (nextnode);
            nextnode = ASSIGN_NEXT (arg_node);
        }

        // if a sync cannot go further down, insert it here
        if (INFO_SYNC (arg_info) != NULL
            && !CanNodeMoveDown (INFO_SYNC (arg_info), nextnode, arg_info)) {
            DBUG_PRINT ("MSS", ("Inserting Sync"));
            ASSIGN_NEXT (INFO_SYNC (arg_info)) = ASSIGN_NEXT (arg_node);
            ASSIGN_NEXT (arg_node) = INFO_SYNC (arg_info);
            INFO_SYNC (arg_info) = NULL;
        }

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSSid(node *arg_node, info *arg_info)
 *
 * @brief Do something...
 *
 * @param arg_node
 * @param arg_info
 *
 * @return
 *
 *****************************************************************************/
node *
MSSid (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ENTER ("MSSid");

    ids = INFO_IDS (arg_info);

    while (ids != NULL) {
        if (IDS_AVIS (ids) == ID_AVIS (arg_node)) {
            INFO_IDSFOUND (arg_info) = TRUE;
        }

        ids = IDS_NEXT (ids);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Add Sync -->
 *****************************************************************************/
