/*
 *
 * $Log$
 * Revision 1.2  2004/09/02 15:27:01  khf
 * TDEPENDwithop removed, additional traverse in NWITHOP and NPART added
 *
 * Revision 1.1  2004/08/26 15:06:30  khf
 * Initial revision
 *
 *
 *
 *
 */

/**
 *
 * @file tagdependencies.c
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

#define NEW_INFO

#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "new_types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "DupTree.h"
#include "Error.h"
#include "globals.h"
#include "dbug.h"
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
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    INFO_TDEPEND_INSIDEWL (result) = FALSE;
    INFO_TDEPEND_FUSIONABLE_WL (result) = NULL;

    result = Malloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    info = Free (info);

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
    DBUG_ENTER ("TDEPENDassign");

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

        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);

        if (ASSIGN_NEXT (arg_node) != NULL) {
            ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
        }
    } else if (NODE_TYPE (ASSIGN_RHS (arg_node)) == N_Nwith) {
        /*
         * We are not inside a WL but we traverse in one, so we had to
         * set INFO_TDEPEND_INSIDEWL( arg_info) TRUE and traverse into
         * instruction
         */
        INFO_TDEPEND_INSIDEWL (arg_info) = TRUE;
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    } else {
        /*
         * We are not inside a WL and we traverse in none, so we only had to
         * traverse in the instruction.
         */
        ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
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

    DBUG_ENTER ("TDEPENDid");

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
            assignn = Trav (assignn, arg_info);
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
 *   @param  node *arg_node:  N_Nwith
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
TDEPENDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TDEPENDwith");

    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL), "no Part is available!");
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), "N_NWITHOP is missing!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *TagDependencies( node *arg_node, info *arg_info)
 *
 *   @brief  Starting point for traversal TagDependencies.
 *
 *   @param  node *arg_node:  N_with node
 *   @param  info *arg_info:
 *   @return node *        :  N_with node
 ******************************************************************************/

node *
TagDependencies (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("TagDependencies");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith),
                 "TagDependencies not started with N_Nwith node");

    DBUG_ASSERT ((NWITH_FUSIONABLE_WL (arg_node) != NULL),
                 "no fusionable withloop found");

    DBUG_PRINT ("TDEPEND", ("starting TagDependencies"));

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = tdepend_tab;

    INFO_TDEPEND_FUSIONABLE_WL (arg_info) = NWITH_FUSIONABLE_WL (arg_node);

    /*
     * traverse N_PARTs,N_CODEs and N_WITHOP to find all assignments, the current withloop
     * depends on. This assigments had to be tagged.
     */
    INFO_TDEPEND_INSIDEWL (arg_info) = TRUE;

    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL),
                 "NWITH_PARTS is >= 1 although no PART is available!");
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), "N_NWITHOP is missing!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    arg_info = FreeInfo (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
