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
#include "internal_lib.h"
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

    result = ILIBmalloc (sizeof (info));

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

    info = ILIBfree (info);

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
 * Afterwards we just traverse into the function-body.
 *
 *****************************************************************************/

node *
CWLEfundef (node *arg_node, info *arg_info)
{
    dfmask_base_t *dfmask_base = NULL;

    DBUG_ENTER ("CWLEfundef");
    DBUG_PRINT ("CWLE", ("Calling CWLEfundef"));

    /*
     * no body - no cwle
     */

    if (NULL != FUNDEF_BODY (arg_node)) {

        /*
         * create our empty, unused DataFlowMask and fill it with nothing.
         */

        dfmask_base = DFMgenMaskBase (FUNDEF_ARGS (arg_node),
                                      BLOCK_VARDEC (FUNDEF_BODY (arg_node)));
        INFO_DFM (arg_info) = DFMgenMaskClear (dfmask_base);

        /*
         * traverse into the function arguments (for checking with the DFM) and
         * the function body.
         */

        if (NULL != FUNDEF_ARGS (arg_node)) {
            FUNDEF_ARGS (arg_node) = TRAVdo (FUNDEF_ARGS (arg_node), arg_info);
        }

        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        /*
         * clean our dfm up again
         */

        INFO_DFM (arg_info) = DFMremoveMask (INFO_DFM (arg_info));
        DFMremoveMaskBase (dfmask_base);
    }

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEargs( node *arg_node, info *arg_info)
 *
 * @brief for setting the bitmask in our DFM.
 *
 *****************************************************************************/
node *
CWLEargs (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("CWLEargs");

    /*
     * do not forget to traverse into the next arg
     */

    if (NULL != ARG_NEXT (arg_node)) {
        ARG_NEXT (arg_node) = TRAVdo (ARG_NEXT (arg_node), arg_info);
    }

    /*
     * set the bitmask for this avis
     */

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

    /*
     * first traverse into the ids, for saving it in the DFM.
     * if we do not have a ids, we should not care about cwle either.
     */

    if (NULL != LET_IDS (arg_node)) {
        LET_IDS (arg_node) = TRAVdo (LET_IDS (arg_node), arg_info);

        /*
         * save the ids of our let and traverse into the epression, hoping
         * that it is a with-loop.
         */

        INFO_LHS (arg_info) = LET_IDS (arg_node);
        LET_EXPR (arg_node) = TRAVdo (LET_EXPR (arg_node), arg_info);
    }

    /*
     * we are through, everything is false now.
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

    /* set the bitmask for our avis */

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
    DBUG_PRINT ("CWLE", ("Calling CWLEwith"));

    /*
     * save the index-vector of our with-loop
     */

    DBUG_PRINT ("CWLE", ("traversing codes"));

    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /*
     * if our codes indicate that we are still on the run we have to check
     *   multiple things:
     *   1. is this a one-generator-loop?
     *   2. Do we look at a modarray/genarray-loop?
     */

    if (INFO_VALID (arg_info) && NULL == WITHOP_NEXT (WITH_WITHOP (arg_node))
        && (N_genarray == WITH_TYPE (arg_node) || N_modarray == WITH_TYPE (arg_node))) {

        /*
         * we may now compare the shapes and check our DataFlowMask.
         */

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
 * @brief checks for nested with-loops and for a valid cwle-case in this wl.
 *
 * several tasks are done in the N_code-nodes:
 *   1. At first we traverse into the code-block. This ensures that even if
 *      we are no more at a valid cwle-case for this with loop we can find
 *      nested cwle-cases.
 *   2. traverse into the other codes, so all nested wls are found.
 *   3. start look for a cwle-case in this code and compare it with the
 *      information from the other codes.
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
    DBUG_PRINT ("CWLE", ("Calling CWLEcode"));

    /*
     * if we still have a valid cwle-case, check if we do agree with that.
     * if, then save the source-array, if not, set valid to false.
     */

    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("CWLE", ("prev nodes and wl signal ok"));

        /*
         * we have to check for:
         * code->cexprs->expr-(id)>avis->ssaassign->n_assign->n_let->expr->prf
         *   existing?
         * prf of type F_sel?
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

            if ((F_sel == PRF_PRF (let_expr))
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
     * if we have found some avis, that meets the requirements, then
     * lets check if it is the same that we have found before. If we
     * do not have found anything before, assign it to INFO_RHSAVIS.
     *
     * At this point we also check the DataFlowMask, to see if our
     * source array was defined _before_ this wl.
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
     * after checking this wl, we may traverse into the next node.
     * if there is none present, we may mark the withid, so that it is
     * present in the inner with loops.
     */

    if (NULL != CODE_NEXT (arg_node)) {
        CODE_NEXT (arg_node) = TRAVdo (CODE_NEXT (arg_node), arg_info);
    } else {
        INFO_WITHID (arg_info) = TRAVdo (INFO_WITHID (arg_info), arg_info);
    }

    /*
     * create a new info-structure for traversing the code-block,
     * traverse, and release the info-structure.
     * we obviously need to pass it the dfmask, so we use that from
     * the local info structure.
     *
     * this needs to be done in order to keep the info-structures
     * seperate in the case of nested with-loops.
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
