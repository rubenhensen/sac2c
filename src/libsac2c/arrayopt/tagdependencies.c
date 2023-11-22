/**
 *
 * @file tagdependencies.c
 *
 * This traversal is an special travesal for WithloopFusion.
 *
 * In this traversal all assignments outside current withloop
 * which are referenced inside are tagged.
 *
 * Ex.:
 *   a = ...;
 *   b = ...;
 *   c = ...;
 *   A = with(iv)
 *        ([0] < iv <= [6])
 *         {res = ...(c,a)...;
 *         }: res
 *        genarray([6]);
 *
 *   tagged assigns are: a,c
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "new_types.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "free.h"
#include "DupTree.h"
#include "globals.h"

#define DBUG_PREFIX "WLFS"
#include "debug.h"

#include "traverse.h"
#include "constants.h"
#include "tagdependencies.h"

/**
 * INFO structure
 */
struct INFO {
    bool insidewl;
    node *fusionable_wl;
};

/* usage of arg_info: */
#define INFO_TDEPEND_INSIDEWL(n) (n->insidewl)
#define INFO_TDEPEND_FUSIONABLE_WL(n) (n->fusionable_wl)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_TDEPEND_INSIDEWL (result) = FALSE;
    INFO_TDEPEND_FUSIONABLE_WL (result) = NULL;

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
 *
 * @fn node *TDEPENDassign(node *arg_node, info *arg_info)
 *
 *   @brief store actual assign node in arg_info and traverse instruction
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
TDEPENDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Finds all assignments the current withloop depends on. This assignments
     * had to be tagged. Also already visited assignments had to be tagged,
     * to avoid reiterations. To avoid confusions the assignments are tagged
     * with the pointer on the current fusionable withloop.
     */

    ASSIGN_TAG (arg_node) = INFO_TDEPEND_FUSIONABLE_WL (arg_info);

    if (INFO_TDEPEND_INSIDEWL (arg_info)) {
        /*
         * We are inside a WL and have to traverse the instructions and
         * the next assignment
         */

        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

        ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);
    } else if (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_with) {
        /*
         * We are not inside a WL but we traverse in one, so we had to
         * set INFO_TDEPEND_INSIDEWL( arg_info) TRUE and traverse into
         * instruction
         */
        INFO_TDEPEND_INSIDEWL (arg_info) = TRUE;
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    } else {
        /*
         * We are not inside a WL and we traverse in none, so we only had to
         * traverse in the instruction.
         */
        ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TDEPENDid(node *arg_node, info *arg_info)
 *
 *   @brief  Checks if this Id is contained in
 *             INFO_DDEPEND_REFERENCES_FUSIONABLE( arg_info).
 *           If it is contained the current assigment is (indirect)
 *           dependent from the fusionable withloop.
 *
 *   @param  node *arg_node:  N_id
 *           info *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
node *
TDEPENDid (node *arg_node, info *arg_info)
{
    node *assignn;
    bool insidewl_tmp;

    DBUG_ENTER ();

    /*
     * Assignments which are tagged with the fusionable withloop
     * were already traversed. We only consider untagged assignments.
     */

    /* get the definition assignment via the AVIS_SSAASSIGN backreference */
    assignn = AVIS_SSAASSIGN (ID_AVIS (arg_node));

    /* is there a definition assignment? */
    if (assignn != NULL) {
        /* is the assignment already tagged */
        if (ASSIGN_TAG (assignn) != INFO_TDEPEND_FUSIONABLE_WL (arg_info)) {
            /* stack INFO_WLFS_INSIDEWL( arg_info) */
            insidewl_tmp = INFO_TDEPEND_INSIDEWL (arg_info);
            INFO_TDEPEND_INSIDEWL (arg_info) = FALSE;
            assignn = TRAVdo (assignn, arg_info);
            INFO_TDEPEND_INSIDEWL (arg_info) = insidewl_tmp;
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TDEPENDwith(node *arg_node, info *arg_info)
 *
 *   @brief to find dependent assignments we had to traverse
 *          into N_CODE and N_WITHOP.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
TDEPENDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    /*
     * Traverse into parts
     */
    DBUG_ASSERT (WITH_PART (arg_node) != NULL, "no Part is available!");
    WITH_PART (arg_node) = TRAVdo (WITH_PART (arg_node), arg_info);

    /*
     * Traverse into codes
     */
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /*
     * Traverse into withop
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TDEPENDdoTagDependencies( node *arg_node, node *fusionable_wl)
 *
 *   @brief  Starting point for traversal TagDependencies.
 *
 *   @param  node *arg_node      : N_with node
 *   @param  node *fusionable_wl :
 *   @return node *              : N_with node
 ******************************************************************************/

node *
TDEPENDdoTagDependencies (node *with, node *fusionable_wl)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with,
                 "TDEPENDdoTagDependencies not started with N_with node");

    DBUG_ASSERT (fusionable_wl != NULL, "no fusionable withloop found");

    DBUG_PRINT ("starting TDEPENDdoTagDependencies");

    arg_info = MakeInfo ();

    INFO_TDEPEND_FUSIONABLE_WL (arg_info) = fusionable_wl;
    INFO_TDEPEND_INSIDEWL (arg_info) = TRUE;

    TRAVpush (TR_tdepend);
    with = TRAVdo (with, arg_info);
    TRAVpop ();

    DBUG_PRINT ("tagging of dependencies complete");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (with);
}

#undef DBUG_PREFIX
