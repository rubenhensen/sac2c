/*
 *
 * $Log$
 * Revision 1.2  2004/09/02 15:27:01  khf
 * DDEPENDwithop removed, additional traverse in NWITHOP and NPART added
 *
 * Revision 1.1  2004/08/26 15:06:26  khf
 * Initial revision
 *
 *
 *
 *
 */

/**
 *
 * @file detectdependencies.c
 *
 * In this traversal a dependency between two withloops
 * is identified.
 * This traversal starts with one withloop and seaches for
 * direct or indirect references onto another WL. References
 * to that WL until current Withloop are stored in
 *    NWITH_REFERENCES_FUSIONABLE( arg_node).
 *
 * Ex.:
 *   A = with(iv)
 *        ([0] < iv <= [6])
 *         {res = ...;
 *         }: res
 *        genarray([6]);
 *   b = ...;
 *   c = ...(A)...;
 *   B = with(iv)
 *        ([0] < iv <= [6])
 *         {res = ...(c)...;
 *         }: res
 *        genarray([6]);
 *
 *   References to A until B are : A, c
 *   and this leads to dependency between B and A since
 *   Ncode of B contains a reference on c
 *
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
#include "detectdependencies.h"

/**
 * INFO structure
 */
struct INFO {
    bool wldependent;
    nodelist *references_fusionable;
};

/* usage of arg_info: */
#define INFO_DDEPEND_WLDEPENDENT(n) (n->wldependent)
#define INFO_DDEPEND_REFERENCES_FUSIONABLE(n) (n->references_fusionable)

/**
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    INFO_DDEPEND_WLDEPENDENT (result) = FALSE;
    INFO_DDEPEND_REFERENCES_FUSIONABLE (result) = NULL;

    result = Malloc (sizeof (info));

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_DDEPEND_REFERENCES_FUSIONABLE (info) = NULL;
    info = Free (info);

    DBUG_RETURN (info);
}

/** <!--********************************************************************-->
 *
 * @fn bool CheckDependency( node *checkid, nodelist *nl);
 *
 *   @brief checks whether checkid is contained in LHS of the assignments
 *          stored in nl.
 *
 *   @param  node *checkid :  N_id
 *           nodelist *nl  :  contains assignments which depends (indirect)
 *                            on fusionable withloop
 *   @return bool          :  returns TRUE iff checkid is dependent
 ******************************************************************************/
static bool
CheckDependency (node *checkid, nodelist *nl)
{
    nodelist *nl_tmp;
    bool is_dependent = FALSE;

    DBUG_ENTER ("CheckDependency");

    nl_tmp = nl;

    while (nl_tmp != NULL) {
        if (NODELIST_NODE (nl_tmp) == AVIS_SSAASSIGN (ID_AVIS (checkid))) {
            is_dependent = TRUE;
            break;
        }
        nl_tmp = NODELIST_NEXT (nl_tmp);
    }

    DBUG_RETURN (is_dependent);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDassign(node *arg_node, info *arg_info)
 *
 *   @brief  We are inside N_code of a withloop and we check
 *           if the current withloop depends on the fusionable withloop.
 *           If TRUE, the withloop is dependent and no more traversal is
 *           necessary.
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
DDEPENDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DDEPENDassign");

    ASSIGN_INSTR (arg_node) = Trav (ASSIGN_INSTR (arg_node), arg_info);
    if (INFO_DDEPEND_WLDEPENDENT (arg_info))
        DBUG_RETURN (arg_node);

    if (ASSIGN_NEXT (arg_node) != NULL) {
        ASSIGN_NEXT (arg_node) = Trav (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDid(node *arg_node, info *arg_info)
 *
 *   @brief  Checks if this Id is contained in
 *             INFO_DDEPEND_REFERENCES_FUSIONABLE( arg_info).
 *           If it is contained the current assigment is (indirect)
 *           dependent on the fusionable withloop.
 *
 *   @param  node *arg_node:  N_id
 *           info *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
node *
DDEPENDid (node *arg_node, info *arg_info)
{
    bool is_dependent;

    DBUG_ENTER ("DDEPENDid");

    is_dependent
      = CheckDependency (arg_node, INFO_DDEPEND_REFERENCES_FUSIONABLE (arg_info));
    INFO_DDEPEND_WLDEPENDENT (arg_info)
      = (INFO_DDEPEND_WLDEPENDENT (arg_info) || is_dependent);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDwith(node *arg_node, info *arg_info)
 *
 *   @brief to find dependent assignments we had to traverse
 *          into N_CODE and N_WITHOP.
 *
 *   @param  node *arg_node:  N_Nwith
 *           info *arg_info:  N_info
 *   @return node *        :  N_Nwith
 ******************************************************************************/

node *
DDEPENDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("DDEPENDwith");

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
 * @fn node *DetectDependencies( node *arg_node, info *arg_info)
 *
 *   @brief  Starting point for traversal DetectDependencies.
 *
 *   @param  node *arg_node:  N_with node
 *   @param  info *arg_info:
 *   @return node *        :  N_with node
 ******************************************************************************/

node *
DetectDependencies (node *arg_node)
{
    funtab *tmp_tab;
    info *arg_info;

    DBUG_ENTER ("DetectDependencies");

    DBUG_ASSERT ((NODE_TYPE (arg_node) == N_Nwith),
                 "DetectDependencies not started with N_Nwith node");

    DBUG_ASSERT ((NWITH_REFERENCES_FUSIONABLE (arg_node) != NULL),
                 "no references on fusionable withloop found");

    DBUG_PRINT ("DDEPEND", ("starting DetectDependencies"));

    arg_info = MakeInfo ();

    tmp_tab = act_tab;
    act_tab = ddepend_tab;

    INFO_DDEPEND_REFERENCES_FUSIONABLE (arg_info)
      = NWITH_REFERENCES_FUSIONABLE (arg_node);

    /*
     * traverse into N_PARTs, N_CODEs and N_WITHOPs to detect possible
     * dependencies from current withloop on the fusionable.
     * The result is stored in NWITH_DEPENDENT( arg_node)
     */
    INFO_DDEPEND_WLDEPENDENT (arg_info) = FALSE;

    DBUG_ASSERT ((NWITH_PART (arg_node) != NULL),
                 "NWITH_PARTS is >= 1 although no PART is available!");
    NWITH_PART (arg_node) = Trav (NWITH_PART (arg_node), arg_info);

    if (NWITH_CODE (arg_node) != NULL) {
        NWITH_CODE (arg_node) = Trav (NWITH_CODE (arg_node), arg_info);
    }

    DBUG_ASSERT ((NWITH_WITHOP (arg_node) != NULL), "N_NWITHOP is missing!");
    NWITH_WITHOP (arg_node) = Trav (NWITH_WITHOP (arg_node), arg_info);

    NWITH_DEPENDENT (arg_node) = INFO_DDEPEND_WLDEPENDENT (arg_info);

    arg_info = FreeInfo (arg_info);
    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}
