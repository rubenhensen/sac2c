/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup mss move sync statements to increase distance from spawn
 *
 *   Traversed the tree until a sync statement is found, then tries to move
 *   that node down the tree. This is done by looking at the next node and see
 *   if any declarations are needed in the next node. If not, move further
 *   down.
 *
 *   If a node cannot be moved further down, the obstructing node is tried to
 *   move as well. This pushes sync even further down.
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
    node *ids;
    bool idsfound;
    int moved;
};

#define INFO_FUNDEF(n) ((n)->fundef)
#define INFO_IDS(n) ((n)->ids)
#define INFO_IDSFOUND(n) ((n)->idsfound)
#define INFO_MOVED(n) ((n)->moved)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_FUNDEF (result) = NULL;
    INFO_IDS (result) = NULL;
    INFO_IDSFOUND (result) = FALSE;
    INFO_MOVED (result) = 0;

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
        DBUG_PRINT ("MSS", ("No matching ids found"));
    }

    DBUG_RETURN (INFO_IDSFOUND (arg_info));
}

/** <!--********************************************************************-->
 *
 * @fn static bool CanNodeMoveDown( noce *current, node *next, info *arg_info)
 *
 * @brief Check to see if current node can move accross next
 *
 *   Can only move N_let nodes for now.
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
    bool canmove;
    node *ids;

    DBUG_ENTER ("CanNodeMoveDown");

    DBUG_ASSERT (NODE_TYPE (current) == N_assign,
                 "Current node must be an N_assign node");
    DBUG_ASSERT (next == NULL || NODE_TYPE (next) == N_assign,
                 "Current node must be an N_assign node");
    DBUG_PRINT ("MSS", ("Checking to see if node can move down"));

    if (next == NULL || NODE_TYPE (ASSIGN_INSTR (current)) != N_let) {
        // TODO: allow non let nodes as well
        DBUG_PRINT ("MSS", ("Node is NULL or a non-let node, cannot move"));
        canmove = FALSE;
    } else {
        ids = LET_IDS (ASSIGN_INSTR (current));
        canmove = !IdsOccurInNode (ids, next, arg_info);
    }

    if (canmove) {
        DBUG_PRINT ("MSS", ("Node can move"));
    } else {
        DBUG_PRINT ("MSS", ("Node can _not_ move"));
    }

    DBUG_RETURN (canmove);
}

/** <!--********************************************************************-->
 *
 * @fn static bool MoveAssignDown( node *parent, info *arg_info)
 *
 * @brief Move the child of given parent node down. Try to move blocking nodes
 *        down as well
 *
 * @param parent an N_assign node
 * @param arg_info
 *
 * @return if node was moved or not
 *
 *****************************************************************************/
static bool
MoveAssignDown (node *parent, info *arg_info)
{
    node *new_parent;
    node *node;
    bool moved;

    DBUG_ENTER ("MoveAssignDown");
    DBUG_PRINT ("MSS", ("Moving assignment node down"));

    new_parent = parent;
    node = ASSIGN_NEXT (parent);
    moved = FALSE;

    if (node == NULL || ASSIGN_HASMOVED (node)) {
        DBUG_PRINT ("MSS", ("Node is NULL or has already been moved before"));
    } else {
        // see if node can be moved in the first place
        if (CanNodeMoveDown (node, ASSIGN_NEXT (node), arg_info)) {
            ASSIGN_NEXT (parent) = ASSIGN_NEXT (node);
            new_parent = ASSIGN_NEXT (node);
            moved = TRUE;

            while (new_parent != NULL
                   && CanNodeMoveDown (node, ASSIGN_NEXT (new_parent), arg_info)) {
                new_parent = ASSIGN_NEXT (new_parent);
            }

            // insert node at new position
            ASSIGN_NEXT (node) = ASSIGN_NEXT (new_parent);
            ASSIGN_NEXT (new_parent) = node;
        }

        // node cannot be moved further, so try to move blocking node as well
        if (MoveAssignDown (node, arg_info)) {
            // move again if blocking node was moved
            moved = MoveAssignDown (new_parent, arg_info) || moved;
        }

        ASSIGN_HASMOVED (node) = TRUE;
    }

    DBUG_RETURN (moved);
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
 * @brief Look for sync statements and try to move them down
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
    node *next_node;

    DBUG_ENTER ("MSSassign");

    if (INFO_IDS (arg_info) != NULL) {
        // Only traverse into instr if looking for ids
        ASSIGN_INSTR (arg_node) = TRAVopt (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        next_node = ASSIGN_NEXT (arg_node);

        // If next node is a sync statement, move it down
        if (next_node != NULL && AssignIsSync (next_node)) {
            MoveAssignDown (arg_node, arg_info);
        }

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *MSSid(node *arg_node, info *arg_info)
 *
 * @brief Look if current node occurs in IDS stored in info
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
 * @}  <!-- Move Sync Statement -->
 *****************************************************************************/
