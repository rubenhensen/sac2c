/**
 *
 * @file resolvedependencies.c
 *
 * This traversal is an special travesal for WithloopFusion and
 * modifies a Ncode of a Withloop before fusion.
 *
 * In this traversal a special dependency inside an Ncode of one Withloop
 * to another one (fusionable Withloop) is identified and resolved.
 * The special dependency consist of the primitive function F_sel_VxA
 * referencing an LHS identifier of the other Withloop and its selection array
 * contains only the index-vector of the current Withloop.
 * That selection can be replaced by the corresponding cexprs of the suitable
 * Ncode of the other Withloop.
 * This traversal starts with an assignment chain (instr of NCODE)
 *
 * Ex.:
 *      A = with(iv)
 *           ([0] < iv <= [6])
 *            {res_a = ...;
 *            }: res_a
 *           genarray([6]);
 *          ...
 *      B = with(iv)
 *           ([0] < iv <= [6])
 *            {res_b = ...A[iv]...;
 *            }: res_b
 *          genarray([6]);
 *
 *   Ncode of WL B is transformed into:
 *      res_b = ...res_a...;
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
#include "resolvedependencies.h"

/**
 * INFO structure
 */
struct INFO {
    node *fusionable_wl;
    node *withid;
    node *assign;
    node *cexprs;
    bool dependent;
    bool resolved;
};

/* usage of arg_info: */
#define INFO_RDEPEND_FUSIONABLE_WL(n) (n->fusionable_wl)
#define INFO_RDEPEND_WITHID(n) (n->withid)
#define INFO_RDEPEND_ASSIGN(n) (n->assign)
#define INFO_RDEPEND_CEXPRS(n) (n->cexprs)
#define INFO_RDEPEND_DEPENDENT(n) (n->dependent)
#define INFO_RDEPEND_RESOLVED(n) (n->resolved)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_RDEPEND_FUSIONABLE_WL (result) = NULL;
    INFO_RDEPEND_WITHID (result) = NULL;
    INFO_RDEPEND_ASSIGN (result) = NULL;
    INFO_RDEPEND_CEXPRS (result) = NULL;
    INFO_RDEPEND_DEPENDENT (result) = FALSE;
    INFO_RDEPEND_RESOLVED (result) = FALSE;

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
 * @fn node *SelId(node *arg_node, info *arg_info)
 *
 *   @brief  Checks if avis of this Id points to same avis of
 *             INFO_RDEPEND_FUSIONABLE( arg_info).
 *           Iff TRUE the Id depends direct on the fusionable withloop.
 *
 *   @param  node *arg_node:  N_id
 *           info *arg_info:  N_info
 *   @return node *        :  N_id
 ******************************************************************************/
static node *
SelId (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (AVIS_SSAASSIGN (ID_AVIS (arg_node)) == INFO_RDEPEND_FUSIONABLE_WL (arg_info)) {
        DBUG_PRINT ("found direct dependency");
        INFO_RDEPEND_DEPENDENT (arg_info) = TRUE;
    } else {
        INFO_RDEPEND_DEPENDENT (arg_info) = FALSE;
    }

    DBUG_RETURN (arg_node);
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
    node *sel, *cexprs, *ids_tmp;

    DBUG_ENTER ();

    DBUG_PRINT ("consider following assignment:");
    DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, INFO_RDEPEND_ASSIGN (arg_info)));

    /* first check first argument to detect direct dependency */
    PRF_ARG2 (arg_node) = SelId (PRF_ARG2 (arg_node), arg_info);

    /*
     * if there is a direct dependency to fusionable WL,
     * secure if the second argument only contains the non-transformed
     * index-vector
     *  -> resolveable dependency
     */
    if (INFO_RDEPEND_DEPENDENT (arg_info)) {
        sel = PRF_ARG1 (arg_node);

        if ((NODE_TYPE (sel) == N_id)
            && (ID_AVIS (sel)
                == IDS_AVIS (WITHID_VEC (INFO_RDEPEND_WITHID (arg_info))))) {

            ids_tmp = ASSIGN_LHS (INFO_RDEPEND_FUSIONABLE_WL (arg_info));
            cexprs = INFO_RDEPEND_CEXPRS (arg_info);
            while (ids_tmp != NULL) {
                if (IDS_AVIS (ids_tmp) == ID_AVIS (PRF_ARG2 (arg_node)))
                    break;
                ids_tmp = IDS_NEXT (ids_tmp);
                cexprs = EXPRS_NEXT (cexprs);
            }
            DBUG_ASSERT ((ids_tmp != NULL && cexprs != NULL),
                         "no suitable identifier found!");

            arg_node = FREEdoFreeNode (arg_node);
            arg_node = DUPdoDupNode (EXPRS_EXPR (cexprs));

            INFO_RDEPEND_RESOLVED (arg_info) = TRUE;
        } else {
            DBUG_UNREACHABLE ("found unresolveable selection!");
        }
    }

    INFO_RDEPEND_DEPENDENT (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RDEPENDassign(node *arg_node, info *arg_info)
 *
 *   @brief
 *
 *   @param  node *arg_node:  N_assign
 *           info *arg_info:  N_info
 *   @return node *        :  N_assign
 ******************************************************************************/

node *
RDEPENDassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_RDEPEND_ASSIGN (arg_info) = arg_node;

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);

    if (INFO_RDEPEND_RESOLVED (arg_info)) {
        DBUG_PRINT ("selection is resolved:");
        DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, INFO_RDEPEND_ASSIGN (arg_info)));
        INFO_RDEPEND_RESOLVED (arg_info) = FALSE;
    }

    ASSIGN_NEXT (arg_node) = TRAVopt(ASSIGN_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *RDEPENDprf(node *arg_node, info *arg_info)
 *
 *   @brief  calls special function if this prf is F_sel_VxA.
 *
 *   @param  node *arg_node:  N_prf
 *           info *arg_info:  N_info
 *   @return node *        :  N_prf
 ******************************************************************************/
node *
RDEPENDprf (node *arg_node, info *arg_info)
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
 * @fn node *RDEPENDdoResolveDependencies(node *assigns, node *cexprs,
 *                                        node *withid, node *fusionable_wl)
 *
 *   @brief  Starting point for traversal ResolveDependencies.
 *
 *   @param  node *assigns        :  N_assign chain
 *           node *cexprs         :  N_exprs
 *           node *withid         :  N_withid
 *           node *fusionable_wl  :  N_assign node which belongs to
 *                                   fusionable With-Loop
 *   @return node *               :  N_assign chain
 ******************************************************************************/

node *
RDEPENDdoResolveDependencies (node *assigns, node *cexprs, node *withid,
                              node *fusionable_wl)
{
    info *arg_info;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (assigns) == N_assign,
                 "ResolveDependencies not started with N_assign node");

    DBUG_ASSERT (cexprs != NULL, "no cexprs found");

    DBUG_ASSERT (withid != NULL, "no withid found");

    DBUG_ASSERT (fusionable_wl != NULL, "no fusionable withloop found");

    DBUG_PRINT ("starting resolving dependencies");

    arg_info = MakeInfo ();

    INFO_RDEPEND_FUSIONABLE_WL (arg_info) = fusionable_wl;
    INFO_RDEPEND_WITHID (arg_info) = withid;
    INFO_RDEPEND_CEXPRS (arg_info) = cexprs;

    TRAVpush (TR_rdepend);
    assigns = TRAVdo (assigns, arg_info);
    TRAVpop ();

    DBUG_PRINT ("resolving dependencies complete");

    arg_info = FreeInfo (arg_info);

    DBUG_RETURN (assigns);
}

#undef DBUG_PREFIX
