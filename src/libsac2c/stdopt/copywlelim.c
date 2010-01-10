/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup cwle Copy with-loop elimination
 *
 *  Overview:
 *  ---------
 *
 * This traversal takes care of with-loops that do nothing more
 * than copying some array. These are replaced by A = B, like in
 *
 * B = with {
 *       (.<=iv<=.) : A[iv];
 *     } : genarray( shape(A), n );
 *
 * will be transformed to
 *
 * B = A;
 *
 *
 * Implementational issue:
 * -----------------------
 * When matching for the pattern A[iv] (in CWLEcode), we need to make sure
 * that "A" has been defined BEFORE this WL-body rather than within it.
 * Otherwise we wrongly transform a WL of the following (pathological :-) kind:
 *
 * B = with {
 *       (.<=iv<=.) {
 *         A = genarray( shp, 21);
 *       } : A[iv];
 *     } : genarray( shp, n);
 *
 * To detect these cases, we use the DFMs and mark all LHS defined BEFORE
 * entering the WL as such.
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
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    bool onefundef;
    node *lhs;
    node *rhsavis;
    node *withid;
    bool valid;
    dfmask_t *dfm;
};

#define INFO_ONEFUNDEF(n) (n->onefundef)
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

    INFO_ONEFUNDEF (result) = FALSE;
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
    DBUG_ASSERT (NODE_TYPE (syntax_tree) == N_fundef, "CWLE called on nonN_fundef node");

    INFO_ONEFUNDEF (info) = TRUE;

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
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static node *arrayFromShapeSel( node *avisshape)
 *
 * @brief: if avisshape is an N_array of the form:
 *         s0 = idx_shape_sel( 0, M);
 *         s1 = idx_shape_sel( 1, M);
 *         s2 = idx_shape_sel( 2, M);
 *         avisshape = [ s0, s1, s2];
 *
 *         then, z = M;
 *
 * @return: If avisshape is such an N_array, the result is M;
 *          else NULL.
 *
 *****************************************************************************/
static node *
arrayFromShapeSel (node *avisshape)
{
    bool b = TRUE;
    pattern *patarray;
    pattern *patshapesel1;
    pattern *patshapesel2;
    node *narray = NULL;
    constant *con = NULL;
    node *M = NULL;
    int n = 0;

    DBUG_ENTER ("arrayFromShapeSel");

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    patshapesel1 = PMprf (1, PMAisPrf (F_idx_shape_sel), 2, PMconst (1, PMAisVal (&con)),
                          PMvar (1, PMAgetNode (&M), 0));
    patshapesel2 = PMprf (1, PMAisPrf (F_idx_shape_sel), 2, PMconst (1, PMAisVal (&con)),
                          PMvar (1, PMAisNode (&M), 0));
    if (PMmatchFlatSkipExtrema (patarray, avisshape)) {
        narray = ARRAY_AELEMS (narray);
        con = COmakeConstantFromInt (0);
        if (PMmatchFlatSkipExtrema (patshapesel1, EXPRS_EXPR (narray))) {
            COfreeConstant (con);
            while (b && (NULL != narray)) {
                con = COmakeConstantFromInt (n);
                n++;
                b = b && PMmatchFlatSkipExtrema (patshapesel2, EXPRS_EXPR (narray));
                COfreeConstant (con);
                narray = EXPRS_NEXT (narray);
            }
            M = b ? M : NULL;
        }
    }
    PMfree (patarray);
    PMfree (patshapesel1);
    PMfree (patshapesel2);

    if (b && (NULL != M)) {
        DBUG_PRINT ("CWLE", ("AVIS_SHAPE %s is %s", AVIS_NAME (ID_AVIS (avisshape)),
                             AVIS_NAME (ID_AVIS (M))));
    } else {
        DBUG_PRINT ("CWLE",
                    ("AVIS_SHAPE %s is unknown", AVIS_NAME (ID_AVIS (avisshape))));
    }

    DBUG_RETURN (M);
}

/** <!--********************************************************************-->
 *
 * @fn static bool isAvisShapesMatch( node *arg1, node *arg2)
 *
 * @brief Predicate for matching two N_avis AVIS_SHAPE nodes.
 *
 *        We start by doing a simple tree compare. If that fails,
 *        we do a more sophisticated comparison, to handle shapes
 *        that are represented as N_arrays, perhaps as created
 *        by WLBSC or IVESPLIT.
 *
 *        We may have arg1:
 *
 *         s0 = idx_shape_sel( 0, M);
 *         s1 = idx_shape_sel( 1, M);
 *         s2 = idx_shape_sel( 2, M);
 *         arg1 = [ s0, s1, s2];
 *
 *       and have arg2 (or worse, as an N_arg, where we know nothing):
 *
 *         arg2 = shape_( M);
 *
 *       For now, we see if one ( e.g., arg1) of the AVIS_SHAPE nodes
 *       is an N_array of the above form, and if so, if M
 *       is the same array as, e.g., arg2.
 *
 * @return: TRUE if the array shapes can be shown to match.
 *
 *****************************************************************************/

static bool
isAvisShapesMatch (node *arg1, node *arg2)
{
    node *M1;
    node *M2;
    ntype *arg1type;
    ntype *arg2type;
    bool z = FALSE;

    DBUG_ENTER ("isAvisShapesMatch");

    DBUG_PRINT ("rbe", ("checking shape match for %s and %s", AVIS_NAME (arg1),
                        AVIS_NAME (arg2)));

    /* Case 1: AKS and result shapes match */
    arg1type = AVIS_TYPE (arg1);
    arg2type = AVIS_TYPE (arg2);
    z = TUshapeKnown (arg1type) && TUshapeKnown (arg2type)
        && TUeqShapes (arg1type, arg2type);

    /* Case 2: AVIS_SHAPEs match */
    if ((!z) && (NULL != AVIS_SHAPE (arg1)) && (NULL != AVIS_SHAPE (arg2))) {
        z = (CMPT_EQ == CMPTdoCompareTree (AVIS_SHAPE (arg1), AVIS_SHAPE (arg2)));

        /*  Case 3 :Primogenitor of one shape matches other, or
         *  both primogenitors match */
        M1 = arrayFromShapeSel (AVIS_SHAPE (arg1));
        M2 = arrayFromShapeSel (AVIS_SHAPE (arg2));

        z = z || ((NULL != M1) && (ID_AVIS (M1) == arg2))
            || ((NULL != M2) && (ID_AVIS (M2) == arg1)) || ((NULL != M1) && (M1 == M2));
    }

#ifdef CRUD
    /* I think the following cases may only be relevant for non-SAA
     * environments.
     */

    if (!z) {
        /* Case 4: genarray of genarray, with scalar default cell */
        if (TUisScalar (AVIS_TYPE (ID_AVIS (GENARRAY_DEFAULT (arg_node))))) {

            /* Case 5: genarray of modarray, with scalar default cell */

            /* Case 6: modarray of genarray */

            /* Case 7: modarray of modarray */
        }
#endif // CRUD

        if (z) {
            DBUG_PRINT ("rbe", ("shapes match for %s and %s", AVIS_NAME (arg1),
                                AVIS_NAME (arg2)));
        } else {
            DBUG_PRINT ("rbe", ("shapes do not match for %s and %s", AVIS_NAME (arg1),
                                AVIS_NAME (arg2)));
        }

        DBUG_RETURN (z);
    }

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

    node *CWLEfundef (node * arg_node, info * arg_info)
    {
        bool old_onefundef;
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

        old_onefundef = INFO_ONEFUNDEF (arg_info);
        INFO_ONEFUNDEF (arg_info) = FALSE;
        FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
        INFO_ONEFUNDEF (arg_info) = old_onefundef;

        if (!INFO_ONEFUNDEF (arg_info)) {
            FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);
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
    node *CWLEarg (node * arg_node, info * arg_info)
    {
        DBUG_ENTER ("CWLEarg");

        ARG_NEXT (arg_node) = TRAVopt (ARG_NEXT (arg_node), arg_info);

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

    node *CWLElet (node * arg_node, info * arg_info)
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
    node *CWLEassign (node * arg_node, info * arg_info)
    {
        DBUG_ENTER ("CWLEassign");

        ASSIGN_INSTR (arg_node) = TRAVdo (ASSIGN_INSTR (arg_node), arg_info);

        ASSIGN_NEXT (arg_node) = TRAVopt (ASSIGN_NEXT (arg_node), arg_info);

        DBUG_RETURN (arg_node);
    }

    /** <!--********************************************************************-->
     *
     * @fn node *CWLEids( node *arg_node, info *arg_info)
     *
     * @brief sets the bitmask for its bitmask in info_dfm.
     *
     *****************************************************************************/
    node *CWLEids (node * arg_node, info * arg_info)
    {
        DBUG_ENTER ("CWLEids");

        DFMsetMaskEntrySet (INFO_DFM (arg_info), NULL, IDS_AVIS (arg_node));

        IDS_NEXT (arg_node) = TRAVopt (IDS_NEXT (arg_node), arg_info);

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

    node *CWLEwith (node * arg_node, info * arg_info)
    {
        DBUG_ENTER ("CWLEwith");

        INFO_WITHID (arg_info) = WITH_WITHID (arg_node);
        WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

        /*
         * if our codes indicate that we are still on the run we have to check
         *   multiple things:
         *   1. is this a one-generator-loop?
         *   2. do we look at a modarray/genarray-loop?
         *   3. do the shapes of INFO_LHS and INFO_RHS match?
         */

        if (INFO_VALID (arg_info) && NULL == WITHOP_NEXT (WITH_WITHOP (arg_node))
            && (N_genarray == WITH_TYPE (arg_node)
                || N_modarray == WITH_TYPE (arg_node))) {

            DBUG_PRINT ("CWLE", ("Codes OK. Comparing shapes of LHS(%s), RHS(%s)",
                                 AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                                 AVIS_NAME (INFO_RHSAVIS (arg_info))));

            if (isAvisShapesMatch (INFO_RHSAVIS (arg_info),
                                   IDS_AVIS (INFO_LHS (arg_info)))) {
                DBUG_PRINT ("CWLE", ("All ok. replacing LHS(%s) WL by %s",
                                     AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                                     AVIS_NAME (INFO_RHSAVIS (arg_info))));
                global.optcounters.cwle_wl++;

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

    node *CWLEcode (node * arg_node, info * arg_info)
    {
        node *cexpr;
        node *withid_avis;
        pattern *pat;
        node *target = NULL;
        info *subinfo;

        DBUG_ENTER ("CWLEcode");

        if (INFO_VALID (arg_info)) {
            DBUG_PRINT ("CWLE", ("prev nodes and wl signal ok"));

            cexpr = EXPRS_EXPR (CODE_CEXPRS (arg_node));
            withid_avis = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));

            pat
              = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMparam (1, PMAhasAvis (&withid_avis)),
                       PMvar (1, PMAgetAvis (&target), 0));

            if (PMmatchFlatSkipExtrema (pat, cexpr)) {
                DBUG_PRINT ("CWLE", ("body matches _sel_VxA_( withid, &target)"));
            } else {
                INFO_VALID (arg_info) = FALSE;
            }

            pat = PMfree (pat);

        } else {
            DBUG_PRINT ("CWLE", ("previous nodes signal NOT ok"));
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
