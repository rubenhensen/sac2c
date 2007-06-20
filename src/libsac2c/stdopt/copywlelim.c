/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cwle Copy with-loop elimination
 *
 * This traversal takes care of with-loops that do nothing more
 * than copying some array. These are replaced by A = B, like in
 *
 * B = with { (.<=iv<=.) : A[iv]; } : genarray( shape(A), n );
 * will be transformed to
 * B = A;
 *
 * @ingroup opt
 *
 * @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file copywlelim.c
 *
 * Prefix: CWLE
 *
 *****************************************************************************/
#include "copywlelim.h"

#include "globals.h"
#include "dbug.h"
#include "str.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "DupTree.h"
#include "free.h"
#include "compare_tree.h"
#include "DataFlowMask.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *lhs;
    node *rhsavis;
    node *withid;
    bool valid;
    dfmask_t *dfm;
};

/* The left hand side of the N_let, the array we are copying into */
#define INFO_LHS(n) (n->lhs)
/* The right hand side of the N_let, or the array we copy from, respectively */
#define INFO_RHSAVIS(n) (n->rhsavis)
/* This is the selection-vector inside our with-loop */
#define INFO_WITHID(n) (n->withid)
/* Do we (still) have a valid case of cwle? */
#define INFO_VALID(n) (n->valid)
/* This saves all visited identifiers, so we only replace loops with known
   values */
#define INFO_DFM(n) (n->dfm)

static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = MEMmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_RHSAVIS (result) = NULL;
    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_LHS (info) = NULL;
    INFO_RHSAVIS (info) = NULL;
    INFO_WITHID (info) = NULL;

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
 * @fn node *CWLEdoTemplateTraversal( node *syntax_tree)
 *
 *****************************************************************************/
node *
CWLEdoTemplateTraversal (node *syntax_tree)
{
    info *info;

    DBUG_ENTER ("CWLEdoTemplateTraversal");

    info = MakeInfo ();

    DBUG_PRINT ("CWLE", ("Starting template traversal."));

    TRAVpush (TR_cwle);
    syntax_tree = TRAVdo (syntax_tree, info);
    TRAVpop ();

    DBUG_PRINT ("CWLE", ("Template traversal complete."));

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
 *
 * @fn node *CWLEfundef(node *arg_node, info *arg_info)
 *
 * @brief Sets up a DFM and traverses into the function-body.
 *
 * Here we do set up a DataFlowMask, which we do need for checking if the
 *   array that we potentially copy from is already defined before the
 *   respective with-loop.
 *
 * Afterwards we just traverse into the function args (so they can be
 * marked in our DFM) and body.
 *
 *****************************************************************************/

node *
CWLEfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t *dfmask_base = NULL;

    DBUG_ENTER ("CWLEfundef");

    if (NULL != FUNDEF_BODY (arg_node)) {
        dfmask_base = DFMgenMaskBase (FUNDEF_ARGS (arg_node),
                                      BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
        INFO_DFM (arg_info) = DFMgenMaskClear (dfmask_base);

        if (NULL != FUNDEF_ARGS (arg_node)) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFM (arg_info) = DFMremoveMask (INFO_DFM (arg_info));
        DFMremoveMaskBase (dfmask_base);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEarg( node *arg_node, info *arg_info)
 *
 * @brief for setting the bitmask in our DFM.
 *
 *****************************************************************************/
node *
CWLEarg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEarg");

    if (NULL != ARG_NEXT (arg_node)) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    DFMsetMaskEntrySet (INFO_DFM (arg_info), NULL, ARG_AVIS (arg_node));

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLElet(node *arg_node, info *arg_info)
 *
 * @brief checks for the shape of its LHS and passes it on
 *
 * This function checks for the shape of the identifier that is placed
 *   on its left hand side. We need this information to check if the array
 *   that we copy from is the same size as the array that we would like
 *   to copy into.
 *
 *****************************************************************************/

node *
CWLElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLElet");
    DBUG_PRINT ("CWLE", ("Calling CWLElet"));

    INFO_VALID (arg_info) = TRUE;

    if (NULL != LET_IDS (arg_node)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    /*
     * reset to false, because the handling of this case is over, independent
     * of its success.
     */

    INFO_VALID (arg_info) = FALSE;

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEassign( node *arg_node, info *arg_info)
 *
 * @brief ensures a top-down traversal
 *
 *****************************************************************************/
node *
CWLEassign (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEassign");

    ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

    if (NULL != ASSIGN_NEXT (arg_node)) {
        ASSIGN_NEXT (arg_node) = TRAVdo (ASSIGN_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEids( node *arg_node, info *arg_info)
 *
 * @brief sets the bitmask for its bitmask in info_dfm.
 *
 *****************************************************************************/
node *
CWLEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEids");

    DFMsetMaskEntrySet (INFO_DFM (arg_info), NULL, IDS_AVIS (arg_node));

    if (NULL != IDS_NEXT (arg_node)) {
        IDS_NEXT (arg_node) = TRAVdo (IDS_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEwith(node *arg_node, info *arg_info)
 *
 * @brief traverses the codes and replaces itself with a N_id on success.
 *
 * Here we traverse into the codeblocks for checking for nested wls and to
 *   have a look for some cwle-action.
 *   If the checks in the codes succeed, we compare the found array we
 *   apparently copy from with the array we would like to create; if these
 *   fit (in shape) we may replace ourselves with a N_id-node of the array
 *   found in the codes.
 *
 *****************************************************************************/

node *
CWLEwith (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEwith");

    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /*
     * if our codes indicate that we are still on the run we have to check
     *   multiple things:
     *   1. is this a one-generator-loop?
     *   2. do we look at a modarray/genarray-loop?
     *   3. are the shapes of INFO_LHS and INFO_RHS equal?
     */

    if (INFO_VALID (arg_info) && NULL == WITHOP_NEXT (WITH_WITHOP (arg_node))
        && (N_genarray == WITH_TYPE (arg_node) || N_modarray == WITH_TYPE (arg_node))) {

        DBUG_PRINT ("CWLE", ("codes signal valid. comparing shapes"));

        if (NULL != AVIS_SHAPE (INFO_RHSAVIS (arg_info))
            && NULL != AVIS_SHAPE (IDS_AVIS (INFO_LHS (arg_info)))
            && CMPT_EQ
                 == CMPTdoCompareTree (AVIS_SHAPE (IDS_AVIS (INFO_LHS (arg_info))),
                                       AVIS_SHAPE (INFO_RHSAVIS (arg_info)))) {
            DBUG_PRINT ("CWLE", ("everything ok. replacing wl with %s",
                                 AVIS_NAME (INFO_RHSAVIS (arg_info))));

            /*
             * 1. free the with-loop, we do not need it anymore.
             * 2. return a brandnew N_id, build from our rhsavis.
             */
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = TBmakeId (INFO_RHSAVIS (arg_info));
        }
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEcode(node *arg_node, info *arg_info)
 *
 * @brief checks for a valid case of cwle in this wl and in nested with loops.
 *
 * several tasks are done in the N_code nodes:
 *   1. At first we do check for a case of cwle in this withloop.
 *   2. Traverse into all succeeding code-blocks, checking if they allow for
 *      a cwle.
 *   3. If we had a look into all code blocks, we do mark the WITHID in our
 *      DFM, so it is available in nested withloops, and traverse into just
 *      these.
 *
 *****************************************************************************/

node *
CWLEcode (node *arg_node, info *arg_info)
{
    node *let_expr;
    node *cexpr_avis;
    node *target = NULL;
    info *subinfo;

    DBUG_ENTER ("CWLEcode");

    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("CWLE", ("prev nodes and wl signal ok"));

        /*
         * we have to check for:
         * code->cexprs->expr-(id)>avis->ssaassign->n_assign->n_let->expr->prf
         *   existing?
         * prf of type F_sel_VxA?
         * prf->args->expr-(id)>avis == with_iv?
         * target = prf->args->next->expr-(id)>avis
         */
        cexpr_avis = ID_AVIS (EXPRS_EXPR (CODE_CEXPRS (arg_node)));

        DBUG_PRINT ("CWLE", ("foobar!"));

        if ((NULL != AVIS_SSAASSIGN (cexpr_avis))
            && (N_let == NODE_TYPE (ASSIGN_INSTR (AVIS_SSAASSIGN (cexpr_avis))))
            && (N_prf
                == NODE_TYPE (LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (cexpr_avis)))))) {
            DBUG_PRINT ("CWLE", ("first checks hold (ssaassign points to prf)"));

            let_expr = LET_EXPR (ASSIGN_INSTR (AVIS_SSAASSIGN (cexpr_avis)));

            if ((F_sel_VxA == PRF_PRF (let_expr))
                && (IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)))
                    == ID_AVIS (PRF_ARG1 (let_expr)))) {
                DBUG_PRINT ("CWLE", ("second checks hold (prf selects at iv)"));

                target = ID_AVIS (PRF_ARG2 (let_expr));
            }
        }
    } else {
        DBUG_PRINT ("CWLE", ("previous nodes signal NO ok"));
    }

    if (NULL != target) {
        DBUG_PRINT ("CWLE", ("found a target."));
    } else {
        INFO_VALID (arg_info) = FALSE;
    }

    /*
     * if we have found some avis that meets the requirements, then lets check if
     * it is the same that we have found before. If we do not have found anything
     * before, assign it to INFO_RHSAVIS.
     * At this point we also check the DataFlowMask, to see if our source array
     * was defined _before_ this wl.
     */
    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("CWLE", ("checking if target is legitimate and known"));

        if ((NULL == INFO_RHSAVIS (arg_info) || target == INFO_RHSAVIS (arg_info))
            && DFMtestMaskEntry (INFO_DFM (arg_info), NULL, target)) {
            DBUG_PRINT ("CWLE", ("target is valid. saving"));

            INFO_RHSAVIS (arg_info) = target;
        } else {
            DBUG_PRINT ("CWLE", ("target is NOT valid. skipping wl"));

            INFO_VALID (arg_info) = FALSE;
            INFO_RHSAVIS (arg_info) = NULL;
        }
    }

    /*
     * if we got another code, traverse it; if we are at the end of all codes,
     * mark the withid. This ensures that the withid is available inside all
     * wls which may be nested inside and copy from our withid.
     */
    if (NULL != CODE_NEXT (arg_node)) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    } else {
        INFO_WITHID (arg_info) = TRAVdo (INFO_WITHID (arg_info), arg_info);
    }

    /*
     * create a new info-structure for traversing the code-block, traverse, and
     * release the info-structure. we obviously need to pass it the dfmask, so
     * we use that from the local info structure.
     * We need the seperate structure so we do not mess with the current wl.
     */
    subinfo = MakeInfo ();
    INFO_DFM (subinfo) = INFO_DFM (arg_info);
    CODE_CBLOCK (arg_node) = TRAVdo (CODE_CBLOCK (arg_node), subinfo);
    subinfo = FreeInfo (subinfo);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Traversal functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Traversal template -->
 *****************************************************************************/
