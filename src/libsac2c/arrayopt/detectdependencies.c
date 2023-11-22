/**
 *
 * @file detectdependencies.c
 *
 * This traversal is an special travesal for WithloopFusion and
 * identifies dependencies between two withloops.
 *
 * It starts with one withloop and seaches for
 * direct or indirect references onto another WL stored at
 * INFO_DDPEND_FUSIONABLE_WL( arg_info). Collected references
 * until current WL are stored at INFO_DDPEND_REFERENCES_FUSIONABLE( arg_info).
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
 * Additonal resolveable dependencies are identified.
 * This are direct references on LHS identifier of considered Withloop
 * selecting Elmenents by untransformed index-vector of current WL.
 *  Ex.: val = ...A[iv]...;
 * These dependencies can be resolved in an later step of WithloopFusion.
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
#include "print.h"
#include "detectdependencies.h"

/**
 * INFO structure
 */
struct INFO {
    node *fusionable_wl;
    nodelist *references_fusionable;
    node *withid;
    bool wldependent;
    bool chk_direct_depend;
    bool resolv_depend;
};

/* usage of arg_info: */
#define INFO_DDEPEND_FUSIONABLE_WL(n) (n->fusionable_wl)
#define INFO_DDEPEND_REFERENCES_FUSIONABLE(n) (n->references_fusionable)
#define INFO_DDEPEND_WITHID(n) (n->withid)
#define INFO_DDEPEND_WLDEPENDENT(n) (n->wldependent)
#define INFO_DDEPEND_CHECK_DIRECT_DEPENDENCY(n) (n->chk_direct_depend)
#define INFO_DDEPEND_RESOLVEABLE_DEPENDENCIES(n) (n->resolv_depend)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_DDEPEND_FUSIONABLE_WL (result) = NULL;
    INFO_DDEPEND_REFERENCES_FUSIONABLE (result) = NULL;
    INFO_DDEPEND_WITHID (result) = NULL;
    INFO_DDEPEND_WLDEPENDENT (result) = FALSE;
    INFO_DDEPEND_CHECK_DIRECT_DEPENDENCY (result) = FALSE;
    INFO_DDEPEND_RESOLVEABLE_DEPENDENCIES (result) = FALSE;

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

    DBUG_ENTER ();

    nl_tmp = nl;

    while (nl_tmp != NULL) {
        if (NODELIST_NODE (nl_tmp) == AVIS_SSAASSIGN (ID_AVIS (checkid))) {
            is_dependent = TRUE;
            DBUG_PRINT ("Dependency found for %s\n", AVIS_NAME (ID_AVIS (checkid)));
            break;
        }
        nl_tmp = NODELIST_NEXT (nl_tmp);
    }

    DBUG_RETURN (is_dependent);
}

/** <!--********************************************************************-->
 *
 * @fn node *CheckPrfSel(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_prf
 *           info *arg_info:  N_info
 *   @return node *        :  N_prf
 ******************************************************************************/
static node *
CheckPrfSel (node *arg_node, info *arg_info)
{
    node *sel;

    DBUG_ENTER ();

    DBUG_PRINT ("consider following selection:");
    DBUG_EXECUTE (PRTdoPrintNode (arg_node));

    /* only if WL is independent so far */
    if (!INFO_DDEPEND_WLDEPENDENT (arg_info)) {

        /* first traverse into first argument to detect direct dependency */
        INFO_DDEPEND_CHECK_DIRECT_DEPENDENCY (arg_info) = TRUE;
        PRF_ARG2 (arg_node) = TRAVdo (PRF_ARG2 (arg_node), arg_info);
        INFO_DDEPEND_CHECK_DIRECT_DEPENDENCY (arg_info) = FALSE;

        /*
         * if there is a direct dependency to fusionable WL,
         * check if the second argument only contains the non-transformed
         * index-vector
         *  -> resolveable dependency
         */
        if (INFO_DDEPEND_WLDEPENDENT (arg_info)) {
            sel = PRF_ARG1 (arg_node);

            if ((NODE_TYPE (sel) == N_id)
                && (ID_AVIS (sel)
                    == IDS_AVIS (WITHID_VEC (INFO_DDEPEND_WITHID (arg_info))))) {

                DBUG_PRINT ("selection is resolveable");

                INFO_DDEPEND_WLDEPENDENT (arg_info) = FALSE;
                INFO_DDEPEND_RESOLVEABLE_DEPENDENCIES (arg_info) = TRUE;
            }
        } else {
            /* check for indirect dependencies */
            PRF_ARGS (arg_node) = TRAVopt(PRF_ARGS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
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
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
    if (INFO_DDEPEND_WLDEPENDENT (arg_info)) {
        DBUG_RETURN (arg_node);
    }

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDprf(node *arg_node, info *arg_info)
 *
 *   @brief  calls special function if this prf is F_sel_VxA.
 *
 *   @param  node *arg_node:  N_prf
 *           info *arg_info:  N_info
 *   @return node *        :  N_prf
 ******************************************************************************/
node *
DDEPENDprf (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    switch (PRF_PRF (arg_node)) {
    case F_sel_VxA:
    case F_idx_sel:
        arg_node = CheckPrfSel (arg_node, arg_info);
        break;

    default:
        PRF_ARGS (arg_node) = TRAVopt(PRF_ARGS (arg_node), arg_info);
    }
    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDid(node *arg_node, info *arg_info)
 *
 *   @brief  Checks if this Id references (direct/indirect) the other
 *           considered Withloop.
 *
 *   @param  node *arg_node:  N_id
 *           info *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
node *
DDEPENDid (node *arg_node, info *arg_info)
{
    bool is_dependent;

    DBUG_ENTER ();

    if (INFO_DDEPEND_CHECK_DIRECT_DEPENDENCY (arg_info)) {
        if (AVIS_SSAASSIGN (ID_AVIS (arg_node))
            == INFO_DDEPEND_FUSIONABLE_WL (arg_info)) {
            DBUG_PRINT ("found direct dependency for %s", AVIS_NAME (ID_AVIS (arg_node)));
            is_dependent = TRUE;
        } else
            is_dependent = FALSE;
    } else {
        /* check for direct and indirect dependencies */
        is_dependent
          = CheckDependency (arg_node, INFO_DDEPEND_REFERENCES_FUSIONABLE (arg_info));
    }

    INFO_DDEPEND_WLDEPENDENT (arg_info)
      = (INFO_DDEPEND_WLDEPENDENT (arg_info) || is_dependent);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDwith(node *arg_node, info *arg_info)
 *
 *   @brief to find dependent assignments we had to traverse
 *          into N_PARTS, N_CODE and N_WITHOP.
 *
 *   @param  node *arg_node:  N_with
 *           info *arg_info:  N_info
 *   @return node *        :  N_with
 ******************************************************************************/

node *
DDEPENDwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_DDEPEND_WITHID (arg_info) = WITH_WITHID (arg_node);

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
     * Traverse into withops
     */
    WITH_WITHOP (arg_node) = TRAVdo (WITH_WITHOP (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DDEPENDcode(node *arg_node, info *arg_info)
 *
 *   @brief traverse only cblock and cexprs
 *
 *   @param  node *arg_node:  N_code
 *           info *arg_info:  info
 *   @return node *        :  N_code
 ******************************************************************************/

node *
DDEPENDcode (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    CODE_HASRESOLVEABLEDEPENDENCIES (arg_node) = FALSE;

    /*
     * Traverse into cblock
     */
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), arg_info);

    /*
     * Traverse into cexprs
     */
    CODE_CEXPRS (arg_node) = TRAVdo (CODE_CEXPRS (arg_node), arg_info);

    /*
     * if code contains at most resolveable dependencies
     *   -> mark code for later modification while fusioning
     */
    if (INFO_DDEPEND_RESOLVEABLE_DEPENDENCIES (arg_info)
        && !INFO_DDEPEND_WLDEPENDENT (arg_info)) {
        DBUG_PRINT ("code contains resolveable dependencies");
        CODE_HASRESOLVEABLEDEPENDENCIES (arg_node) = TRUE;
    }

    CODE_NEXT (arg_node) = TRAVopt(CODE_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *DEPENDdoDetectDependencies( node *with, node *fusionable_wl,
 *                                       nodelist references_fwl)
 *
 *   @brief  Starting point for traversal DetectDependencies.
 *
 *   @param  node *with               :  N_with node
 *           node *fusionable_wl      :  N_assign node which belongs to
 *                                       fusionable With-Loop
 *           nodelist *references_fwl :  list of direct and indirect references
 *                                       on fusionable_wl
 *   @return node *                   :  N_with node
 ******************************************************************************/

node *
DDEPENDdoDetectDependencies (node *with, node *fusionable_wl, nodelist *references_fwl)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (with) == N_with,
                 "DEPENDdoDetectDependencies not started with N_with node");

    DBUG_ASSERT (fusionable_wl != NULL, "no fusionable withloop found");

    DBUG_ASSERT (references_fwl != NULL, "no references on fusionable withloop found");

    DBUG_PRINT ("starting detection of dependencies");

    arg_info = MakeInfo ();

    INFO_DDEPEND_FUSIONABLE_WL (arg_info) = fusionable_wl;
    INFO_DDEPEND_REFERENCES_FUSIONABLE (arg_info) = references_fwl;

    /*
     * traverse into N_WITH to detect possible
     * dependencies from current withloop on the fusionable.
     * The result is stored in WITH_DEPENDENT( arg_node)
     */

    TRAVpush (TR_ddepend);
    with = TRAVdo (with, arg_info);
    TRAVpop ();

    WITH_ISDEPENDENT (with) = INFO_DDEPEND_WLDEPENDENT (arg_info);

    DBUG_PRINT ("detection of dependencies complete");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (with);
}

#undef DBUG_PREFIX
