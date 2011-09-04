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
 * This traversal eliminates with-loops that do nothing more
 * than copy some array, in toto.
 *
 * The code ensures that, in _sel_VxA_( IV, A),
 * that IV matches the iv in the WL generator. I.e., there are
 * no offsets, nor transpositions, etc, happening, merely a
 * straight element-by-element copy.
 *
 * WLs matching the above criteria are replaced by A = B, e.g.:
 *
 * B = with {
 *       (.<=iv<=.) : A[iv];
 *     } : genarray( shape(A), n );
 *
 * will be transformed to:
 *
 * B = A;
 *
 * There are several cases, depending various factors such as
 * SAACYC vs. CYC, producer-WL as LACFUN parameter, etc.
 *
 * Implementation issues:
 * -----------------------
 *
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
 * NB: There should a better way to do this than using DFMs, but I'm
 *     busy...
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

#define DBUG_PREFIX "CWLE"
#include "debug.h"

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
#include "indexvectorutils.h"

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *lhs;
    node *fundef;
    node *pwlavis;
    node *withid;
    bool valid;
    dfmask_t *dfm;
};

/* The left hand side of the N_let, the array we are copying into */
#define INFO_LHS(n) (n->lhs)
/* The right hand side of the N_let, or the array we copy from, respectively */
#define INFO_FUNDEF(n) (n->fundef)
/* The function currently being traversed. This is here to ease debugging */
#define INFO_PWLAVIS(n) (n->pwlavis)
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

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PWLAVIS (result) = NULL;
    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_LHS (info) = NULL;
    INFO_PWLAVIS (info) = NULL;
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
 * @fn node *CWLEdoCopyWithLoopElimination( node *arg_node)
 *
 *****************************************************************************/
node *
CWLEdoCopyWithLoopElimination (node *arg_node)
{
    info *info;

    DBUG_ENTER ();

    info = MakeInfo ();
    DBUG_ASSERT (NODE_TYPE (arg_node) == N_fundef, "Called on non-N_fundef node");

    DBUG_PRINT ("Copy with-loop elimination traversal started.");

    TRAVpush (TR_cwle);
    arg_node = TRAVdo (arg_node, info);
    TRAVpop ();

    DBUG_PRINT ("Copy with-loop elimination traversal ended.");

    info = FreeInfo (info);

    DBUG_RETURN (arg_node);
}

/** <!--********************************************************************-->
 * @}  <!-- Entry functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn static node *ivMatchCase1( node *arg_node, info *arg_info,
 *                                node *cexpr)
 *
 * @brief: Attempt to match IV in _sel_VxA_( IV, target)
 *         directly against WITHID_VEC withid_avis.
 *
 * @params: cexprs is the WL result element. We determine if it
 *          was derived from the above _sel_VxA_ operation.
 *
 * @return: target as PRF_ARG2 of _sel_VxA_( IV, target),
 *          if found, else NULL.
 *
 *****************************************************************************/
static node *
ivMatchCase1 (node *arg_node, info *arg_info, node *cexpr)
{
    node *target = NULL;
    node *withid_avis;
    node *offset = NULL;
    node *shp = NULL;
    node *ids = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;

    DBUG_ENTER ();

    withid_avis = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMparam (1, PMAhasAvis (&withid_avis)),
                  PMvar (1, PMAgetAvis (&target), 0));

    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAgetAvis (&target), 0));

    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAhasAvis (&withid_avis), 0));

    pat4 = PMprf (1, PMAisPrf (F_idxs2offset), 3, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&ids), 0), PMskip (0));

    if (PMmatchFlatSkipGuards (pat1, cexpr)) {
        DBUG_PRINT ("Case 1: body matches _sel_VxA_(, iv, pwl)");
    }

    if (NULL == target) {
        if ((PMmatchFlatSkipGuards (pat2, cexpr))
            && (PMmatchFlatSkipGuards (pat3, offset))) {
            DBUG_PRINT ("Case 2: body matches _idx_sel( offset, pwl) with iv=%s",
                        AVIS_NAME (target));
        }
    }

    if (NULL == target) {
        if ((PMmatchFlatSkipGuards (pat2, cexpr))
            && (PMmatchFlatSkipGuards (pat4, offset))) {
            DBUG_PRINT ("Case 3: coding time for idxs2offset");
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (target);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ivMatchCase4( node *arg_node, info *arg_info,
 *                                node *cexpr)
 *
 * @brief: Attempt to match [i,j] in _sel_VxA_( [i,j], target)
 *         against WL_IDS, [i,j]
 *
 *         We have to be careful of stuff like:
 *
 *              _sel_VxA_( [i], target)
 *
 *         in which the full index vector is not used,
 *         and its converse:
 *
 *              _sel_VxA_( [i,notme, k], target)
 *
 *         In the case where we have guards present on the index
 *         vectors, we can safely ignore them, because the
 *         WL that we are copying will have identical guards
 *         on its WITH_ID index vector, which are guaranteed
 *         to be identical to ours. Hence, we ignore both
 *         guards and extrema on this search. As an example from
 *         histlp.sac, we have this pattern, sort of:
 *
 *           z = with { ...
 *             ( [0] <= iv < [lim]) {
 *              iv'  = _noteminval( iv, [0]);
 *              iv'' = _notemaxval( iv', [lim]);
 *              iv''', p0 = _val_lt_val_VxV_( iv'', [lim]);
 *              el = _sel_VxA_( iv''', producerWL);
 *              el' = _afterguard_( el, p0);
 *             } : el';
 *
 *        We also have to handle the scalar i equivalent of this,
 *        which is slightly more complex:
 *
 *           z = with { ...
 *             ( [0] <= iv=[i] < [lim]) {
 *              i'  = _noteminval( i, 0);
 *              i'' = _notemaxval( i', lim);
 *              i''', p0 = _val_lt_val_SxS_( i'', lim);
 *              iv' = [ i'''];
 *              el = _sel_VxA_( iv', producerWL);
 *              el' = _afterguard_( el, p0);
 *             } : el';
 *
 *
 * @params: cexprs is the WL result element. We determine if it
 *          was derived from the above _sel_VxA_ operation.
 *
 * @return: target as PRF_ARG2 of _sel_VxA_( IV, target),
 *          if found, else NULL.
 *
 *****************************************************************************/
static node *
ivMatchCase4 (node *arg_node, info *arg_info, node *cexpr)
{
    node *target = NULL;
    node *withid_avis;
    node *withids;
    node *narray;
    node *narrayels;
    pattern *pat2;
    pattern *pat3;
    char *lhs;
    bool z = TRUE;

    DBUG_ENTER ();

    pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMarray (1, PMAgetNode (&narray), 0),
                  PMvar (1, PMAgetAvis (&target), 0));
    pat3 = PMparam (1, PMAhasAvis (&withid_avis));

    withids = WITHID_IDS (INFO_WITHID (arg_info));

    lhs = AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))); /* ddd aid */
    DBUG_ASSERT ((N_prf != NODE_TYPE (cexpr)) || (F_idx_sel != PRF_PRF (cexpr)),
                 "Start coding, Mr doivecyc4!");
    if (PMmatchFlatSkipExtremaAndGuards (pat2, cexpr)) {
        /* Match all elements. If we exhaust elements on either side, no match */
        narrayels = ARRAY_AELEMS (narray);
        while (z && (NULL != withids) && (NULL != narrayels)) {
            withid_avis = IDS_AVIS (withids);
            z = PMmatchFlatSkipExtremaAndGuards (pat3, EXPRS_EXPR (narrayels));
            withids = IDS_NEXT (withids);
            narrayels = EXPRS_NEXT (narrayels);
        }
        /* If we didn't exhaust both sides, no match */
        z = z && (NULL == withids) && (NULL == narrayels);

        if (z) {
            DBUG_PRINT ("Case 4: body matches _sel_VxA_( withid, &target)");
        } else {
            target = NULL;
        }
    }
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (target);
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

node *
CWLEfundef (node *arg_node, info *arg_info)
{
    node *oldfundef;
    dfmask_base_t *dfmask_base = NULL;

    DBUG_ENTER ();

    oldfundef = INFO_FUNDEF (arg_info);
    INFO_FUNDEF (arg_info) = arg_node;
    if (NULL != FUNDEF_BODY (arg_node)) {
        DBUG_PRINT ("traversing body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));

        dfmask_base = DFMgenMaskBase (FUNDEF_ARGS (arg_node),
                                      BLOCK_VARDECS (FUNDEF_BODY (arg_node)));
        INFO_DFM (arg_info) = DFMgenMaskClear (dfmask_base);

        FUNDEF_ARGS (arg_node) = TRAVopt (FUNDEF_ARGS (arg_node), arg_info);
        FUNDEF_BODY (arg_node) = TRAVdo (FUNDEF_BODY (arg_node), arg_info);

        INFO_DFM (arg_info) = DFMremoveMask (INFO_DFM (arg_info));
        DFMremoveMaskBase (dfmask_base);
        DBUG_PRINT ("leaving body of (%s) %s",
                    (FUNDEF_ISWRAPPERFUN (arg_node) ? "wrapper" : "fundef"),
                    FUNDEF_NAME (arg_node));
    }

    FUNDEF_LOCALFUNS (arg_node) = TRAVopt (FUNDEF_LOCALFUNS (arg_node), arg_info);
    FUNDEF_NEXT (arg_node) = TRAVopt (FUNDEF_NEXT (arg_node), arg_info);

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
    DBUG_ENTER ();

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

node *
CWLElet (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();
#ifdef VERBOSE
    DBUG_PRINT ("Looking at %s", AVIS_NAME (IDS_AVIS (LET_IDS (arg_node))));
#endif // VERBOSE

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
    INFO_LHS (arg_info) = NULL;

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
    DBUG_ENTER ();

    ASSIGN_STMT (arg_node) = TRAVdo (ASSIGN_STMT (arg_node), arg_info);
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
node *
CWLEids (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

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
 *   fit (in shape) we may replace ourselves with a N_id node of the array
 *   found in the codes.
 *
 *****************************************************************************/

node *
CWLEwith (node *arg_node, info *arg_info)
{
    pattern *pat;
    node *pwlwith = NULL;
    node *pwlid;

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
    INFO_WITHID (arg_info) = WITH_WITHID (arg_node);
    WITH_CODE (arg_node) = TRAVdo (WITH_CODE (arg_node), arg_info);

    /*
     * if our codes indicate that we are still on the run we have to check
     *   multiple things:
     *   1. is this a one-generator-loop?
     *   2. do we look at a modarray/genarray-loop?
     *   3. do the shapes of INFO_LHS and INFO_RHS match?
     */

    if ((INFO_VALID (arg_info)) && (NULL == WITHOP_NEXT (WITH_WITHOP (arg_node)))
        && (NULL == PART_NEXT (WITH_PART (arg_node)))
        && (N_genarray == WITH_TYPE (arg_node) || N_modarray == WITH_TYPE (arg_node))) {

        DBUG_PRINT ("Codes OK. Comparing shapes of LHS(%s), RHS(%s)",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                    AVIS_NAME (INFO_PWLAVIS (arg_info)));

        pat = PMwith (1, PMAgetNode (&pwlwith), 0);
        pwlid = TBmakeId (INFO_PWLAVIS (arg_info));
        PMmatchFlatWith (pat, pwlid);
        pwlid = FREEdoFreeNode (pwlid);

        if (IVUTisWLShapesMatch (INFO_PWLAVIS (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                                 arg_node, pwlwith)) {
            DBUG_PRINT ("All ok. replacing LHS(%s) WL by %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (INFO_PWLAVIS (arg_info)));
            global.optcounters.cwle_wl++;

            /*
             * 1. free the cwl; we do not need it anymore.
             * 2. return a brandnew N_id, built from the pwl.
             */
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = TBmakeId (INFO_PWLAVIS (arg_info));
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
    node *cexpr;
    node *target;
    char *lhs;
    info *subinfo;

    DBUG_ENTER ();

    if (INFO_VALID (arg_info)) {

        DBUG_PRINT ("prev nodes and wl signal ok");
        cexpr = EXPRS_EXPR (CODE_CEXPRS (arg_node));
        target = ivMatchCase1 (arg_node, arg_info, cexpr);
        target = (NULL != target) ? target : ivMatchCase4 (arg_node, arg_info, cexpr);
        lhs = AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)));
        if (NULL == target) {
            DBUG_PRINT ("body of %s does not match _sel_VxA_( withid, &target)", lhs);
            INFO_VALID (arg_info) = FALSE;
        }
    } else {
        DBUG_PRINT ("previous nodes signal NOT ok");
    }

    /*
     * if we have found some avis that meets the requirements, then lets check if
     * it is the same that we have found before. If we do not have found anything
     * before, assign it to INFO_PWLAVIS.
     * At this point we also check the DataFlowMask, to see if our source array
     * was defined _before_ this wl.
     */
    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("checking if target is legitimate and known");

        if ((NULL == INFO_PWLAVIS (arg_info) || target == INFO_PWLAVIS (arg_info))
            && DFMtestMaskEntry (INFO_DFM (arg_info), NULL, target)) {
            DBUG_PRINT ("target is valid. saving");

            INFO_PWLAVIS (arg_info) = target;
        } else {
            DBUG_PRINT ("target is NOT valid. skipping wl");

            INFO_VALID (arg_info) = FALSE;
            INFO_PWLAVIS (arg_info) = NULL;
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

#undef DBUG_PREFIX
