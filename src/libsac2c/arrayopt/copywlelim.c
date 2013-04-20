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
    node *pavis;
    node *withid;
    bool valid;
    dfmask_t *dfm;
};

/* The left hand side of the N_let for the consumer-WL */
#define INFO_LHS(n) (n->lhs)
#define INFO_FUNDEF(n) (n->fundef)
/* The function currently being traversed. This is here to ease debugging */
#define INFO_PAVIS(n) (n->pavis)
/* This is the selection-vector inside our with-loop */
#define INFO_WITHID(n) (n->withid)
/* Do we (still) have a valid case of cwle? */
#define INFO_VALID(n) (n->valid)
/* This saves all visited identifiers, so we only replace loops with known
   values */
#define INFO_DFM(n) (n->dfm)

static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_PAVIS (result) = NULL;
    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    INFO_LHS (info) = NULL;
    INFO_PAVIS (info) = NULL;
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
 * @fn static node *ivMatchCase1( node *withid,
 *                                node *cexpr)
 *
 * @brief: Attempt to match IV in _sel_VxA_( IV, srcwl)
 *         directly against WITHID_VEC withid_avis.
 *         We also attempt to match against:
 *               offset = _vect2offset( shape(target), IV);
 *               _idx_sel_( offset, srcwl);
 *
 * @params: withid is the N_withid of the WL we are trying to
 *          show is a copy-WL.
 *          cexprs is the WL result element. We determine if it
 *          was derived from the above _sel_VxA_ operation.
 *
 * @return: srcwl as N_avis of srcwl of _sel_VxA_( IV, srcwl),
 *          if found, else NULL.
 *
 *****************************************************************************/
static node *
ivMatchCase1 (node *withid, node *cexpr)
{
    node *srcwl = NULL;
    node *z = NULL;
    node *withid_son = NULL;
    node *withid_avis;
    node *offset = NULL;
    node *shp = NULL;
    node *id = NULL;
    node *iv = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    pattern *pat5;

    DBUG_ENTER ();
    withid_avis = IDS_AVIS (WITHID_VEC (withid));
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMparam (1, PMAgetNode (&withid_son)),
                  PMvar (1, PMAgetAvis (&srcwl), 0));

    pat5 = PMparam (1, PMAhasAvis (&withid_avis));

    if (PMmatchFlatSkipExtremaAndGuards (pat1, cexpr)
        && PMmatchFlatSkipExtremaAndGuards (pat5, withid_son)) {
        /* withid_son may be guarded withid. */
        z = srcwl;
        DBUG_PRINT ("Case 1: body matches _sel_VxA_(, iv, pwl)");
    }

    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAgetNode (&srcwl), 0));

    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&iv), 0));

    if ((NULL == z) && (PMmatchFlatSkipGuards (pat2, cexpr))
        && (PMmatchFlatSkipExtremaAndGuards (pat3, offset))
        && (IVUTivMatchesWithid (iv, withid))) {
        z = ID_AVIS (srcwl);
        DBUG_PRINT ("Case 2: body matches _idx_sel( offset, pwl) with pwl=%s",
                    AVIS_NAME (z));
    }

    pat4 = PMprf (1, PMAisPrf (F_idxs2offset), 3, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&id), 0), PMskip (0));

    if ((NULL == z) && (PMmatchFlatSkipExtremaAndGuards (pat2, cexpr))
        && (PMmatchFlatSkipExtremaAndGuards (pat4, offset))) {
        DBUG_ASSERT (FALSE, "Case 3: coding time for matching WITHID_IDS to ids");
        z = ID_AVIS (srcwl);
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
    pat5 = PMfree (pat5);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn static node *ivMatchCase4( node *withid,
 *                                node *cexpr)
 *
 * @brief: Attempt to match [i,j] in _sel_VxA_( [i,j], srcwl)
 *         against WL_IDS, [i,j]
 *
 *         We have to be careful of stuff like:
 *
 *              _sel_VxA_( [i], srcwl)
 *
 *         in which the full index vector is not used,
 *         and its converse:
 *
 *              _sel_VxA_( [i,notme, k], srcwl)
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
 * @params: withid is the N_withid of the WL we are trying to
 *          show is a copy-WL.
 *
 *          cexprs is the WL result element. We determine if it
 *          was derived from the above _sel_VxA_ operation.
 *
 * @return: srcwl as PRF_ARG2 of _sel_VxA_( IV, srcwl),
 *          if found, else NULL.
 *
 *****************************************************************************/
static node *
ivMatchCase4 (node *withid, node *cexpr)
{
    node *srcwl = NULL;
    node *withid_avis;
    node *withids;
    node *narray;
    node *narrayels;
    pattern *pat2;
    pattern *pat3;
    bool z = TRUE;

    DBUG_ENTER ();

    pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMarray (1, PMAgetNode (&narray), 0),
                  PMvar (1, PMAgetAvis (&srcwl), 0));
    pat3 = PMparam (1, PMAhasAvis (&withid_avis));

    withids = WITHID_IDS (withid);

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
            DBUG_PRINT ("Case 4: body matches _sel_VxA_( withid, &srcwl)");
        } else {
            srcwl = NULL;
        }
    }
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (srcwl);
}

/** <!--********************************************************************-->
 *
 * @fn node *CWLEfindCopyPartitionSrcWl( node *withid,  node *cexpr)
 *
 * @brief: Determine if this partition is a copy partition.
 *         If it is, return the N_avis of the source WL, else NULL.
 *
 * @params: withid: The N_withid of the putative copy partition
 *          we are examining.
 *
 *          cexpr: The result N_id of the current copy partition.
 *
 * @result: N_avis of the source WL, if found, else NULL.
 *
 *****************************************************************************/
node *
CWLEfindCopyPartitionSrcWl (node *withid, node *cexpr)
{
    node *srcwl = NULL;
    DBUG_ENTER ();

    srcwl = ivMatchCase1 (withid, cexpr);
    srcwl = (NULL != srcwl) ? srcwl : ivMatchCase4 (withid, cexpr);

    DBUG_RETURN (srcwl);
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
    INFO_FUNDEF (arg_info) = oldfundef;

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

    DBUG_PRINT ("Setting DFM for %s", AVIS_NAME (IDS_AVIS (arg_node)));
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

    DBUG_ENTER ();

    DBUG_PRINT ("Looking at WL %s", AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))));
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
                    AVIS_NAME (INFO_PAVIS (arg_info)));

        if (IVUTisShapesMatch (INFO_PAVIS (arg_info), IDS_AVIS (INFO_LHS (arg_info)),
                               GENERATOR_BOUND2 (
                                 PART_GENERATOR (WITH_PART (arg_node))))) {
            DBUG_PRINT ("All ok. replacing LHS(%s) WL by %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (INFO_PAVIS (arg_info)));
            global.optcounters.cwle_wl++;

            /*
             * 1. free the consumer; we do not need it anymore.
             * 2. return a brandnew N_id, built from the producer.
             */
            arg_node = FREEdoFreeTree (arg_node);
            arg_node = TBmakeId (INFO_PAVIS (arg_info));
        } else {
            DBUG_PRINT ("Shape mismatch: Unable to replace LHS(%s) WL by RHS(%s)",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (INFO_PAVIS (arg_info)));
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
 *
 *   1. At first we do check for a case of cwle in this withloop.
 *
 *   2. Traverse into all succeeding code-blocks, checking if they allow for
 *      a cwle.
 *
 *   3. If we had a look into all code blocks, we do mark the WITHID in our
 *      DFM, so it is available in nested withloops, and traverse into just
 *      these.
 *
 *****************************************************************************/
node *
CWLEcode (node *arg_node, info *arg_info)
{
    node *cexpr;
    node *srcwl;
    char *lhs;
    info *subinfo;

    DBUG_ENTER ();

    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("prev nodes and wl signal ok");
        cexpr = EXPRS_EXPR (CODE_CEXPRS (arg_node));
        srcwl = CWLEfindCopyPartitionSrcWl (INFO_WITHID (arg_info), cexpr);
        if (NULL == srcwl) {
            lhs = AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info)));
            DBUG_PRINT ("body of %s does not match _sel_VxA_( withid, &srcwl)", lhs);
            INFO_VALID (arg_info) = FALSE;
        }
    } else {
        DBUG_PRINT ("previous nodes signal NOT ok");
    }

    /*
     * if we have found some avis that meets the requirements, then lets check if
     * it is the same that we have found before. If we do not have found anything
     * before, assign it to INFO_PAVIS.
     * At this point we also check the DataFlowMask, to see if our source array
     * was defined _before_ this wl.
     */
    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("checking if target is legitimate and known");

        if ((NULL == INFO_PAVIS (arg_info) || srcwl == INFO_PAVIS (arg_info))
            && DFMtestMaskEntry (INFO_DFM (arg_info), NULL, srcwl)) {
            DBUG_PRINT ("srcwl is valid. saving");

            INFO_PAVIS (arg_info) = srcwl;
        } else {
            DBUG_PRINT ("srcwl is NOT valid. skipping wl");

            INFO_VALID (arg_info) = FALSE;
            INFO_PAVIS (arg_info) = NULL;
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
    INFO_FUNDEF (subinfo) = INFO_FUNDEF (arg_info);
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
