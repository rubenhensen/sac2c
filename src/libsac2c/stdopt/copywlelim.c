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
 * The checks in function XXX ensure that, in _sel_VxA_( IV, A),
 * that IV matches the iv in the WL generator. I.e., there are
 * no offsets, nor transpositions, etc, happening, merely a
 * straight element-by-element copy.
 *
 * WLs matching the above criteria are replaced by A = B, e.g.:
 *
 * Case 1:
 *
 * B = with {
 *       (.<=iv<=.) : A[iv];
 *     } : genarray( shape(A), n );
 *
 * will be transformed to:
 *
 * B = A;
 *
 * Case 2: ( NOT IMPLEMENTED YET!)
 *
 * In the presence of guards, things get a bit messier, due to
 * the unrolling of guard expressions. We start with something
 * along these lines:
 *
 * ub = shape( A);  NB.   ish
 * lb = 0 * ub;
 *
 * B = with {
 *       (lb <=iv=[i]<= ub) {
 *       imin =    _noteminval( i, lb);
 *       iminmax = _notemaxval( imin, ub);
 *       i',   p0 = _non_neg_val_S( iminmax);
 *       i'',  p1 = _val_lt_val_SxS_( i', ub0);
 *       iv''' = [ i''];
 *       tmp = A[iv'''];
 *       tmp2 = _afterguard_( t, p0, p1);
 *       } : tmp2;
 *     } : genarray( shape(A), n );
 *
 *
 * This should be transformed into:
 *
 * ub = shape( A);  NB.   ish
 * ub0 = ub[0];
 * lb = 0 * ub;
 * lb0 = lb[0];
 * lb0, p0 = _non_neg_val_S_( lb0);
 * ub1, p1 = _val_lt_val_SxS_( ub0, ub0);   NB. PRF_ARG2 validity
 * B = _afterguard( A, p0, p1);
 *
 * Case 3: Like Case 2, but operating on IV, rather than on [i].
 *    NOT IMPLEMENTED YET.
 *
 * Case 4:
 *
 *  This is Case 2 when CF has been able to remove the guards,
 *  we start with:
 *
 * ub = shape( A);
 * lb = 0 * ub;
 * ub0 = ub[0];
 * lb0 = lb[0];
 *
 * B = with {
 *       (lb <=iv=[i]<= ub) {
 *       imin =    _noteminval( i, lb0);
 *       iminmax = _notemaxval( imin, ub0);
 *       iv''' = [ iminmax];
 *       tmp = A[iv'''];
 *       } : tmp;
 *     } : genarray( shape(A), n );
 *
 * This should be transformed into:
 *
 * B = A;
 *
 * Case 5: This is Case 4, but operating on IV, rathern than on [i].
 *    NOT IMPLEMENTED YET.
 *
 * Case 6: This is brought about by the redesign of prfunr to handle
 *         _shape_A_( mat). It previously generated an
 *         array of  _idx_shape_sel() ops:
 *
 *            s0 = idx_shape_sel( 0, M);
 *            s1 = idx_shape_sel( 1, M);
 *            s2 = idx_shape_sel( 2, M);
 *            avisshape = [ s0, s1, s2];
 *
 *         This approach led to performance problems because M
 *         would always be materialized, even if it was no longer
 *         used. The new prfunr code avoids this, and operates
 *         on the shape vector only, in the hope that SAA and
 *         friends will share that shape information:
 *
 *            shp = _shape_A_( M);
 *            s0 = _sel_VxA_( 0, shp);
 *            s1 = _sel_VxA_( 1, shp);
 *            s2 = _sel_VxA_( 2, shp);
 *            avisshape = [ s0, s1, s2];
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

/** <!--********************************************************************-->
 *
 * @name INFO structure
 * @{
 *
 *****************************************************************************/

struct INFO {
    node *lhs;
    node *fundef;
    node *rhsavis;
    node *withid;
    bool valid;
    dfmask_t *dfm;
};

/* The left hand side of the N_let, the array we are copying into */
#define INFO_LHS(n) (n->lhs)
/* The right hand side of the N_let, or the array we copy from, respectively */
#define INFO_FUNDEF(n) (n->fundef)
/* The function currently being traversed. This is here to ease debugging */
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

    DBUG_ENTER ();

    result = MEMmalloc (sizeof (info));

    INFO_VALID (result) = FALSE;
    INFO_LHS (result) = NULL;
    INFO_FUNDEF (result) = NULL;
    INFO_RHSAVIS (result) = NULL;
    INFO_WITHID (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

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
 * @fn static node *ShapeFromShape( node *avisshape)
 *
 * @brief:  If this shape originated as an N_array
 *          formed from shape vector selections, find
 *          that shape vector.
 *
 * Case 1: if avisshape is an N_array of the form:
 *
 *            shp = _shape_A_( M);
 *            v0 = [0];
 *            s0 = _sel_VxA_( v0, shp);
 *            v1 = [1];
 *            s1 = _sel_VxA_( v1, shp);
 *            v2 = [2];
 *            s2 = _sel_VxA_( v2, shp);
 *            avisshape = [ s0, s1, s2];
 *
 *         then z is shp;
 *
 *         The same behavior holds if the selections are of the form:
 *
 *            i0 = 0;
 *            s0 = idx_sel( i0, shp);
 *
 * @return: If avisshape is such an N_array, the result is shp;
 *          else avisshape.
 *
 *****************************************************************************/
static node *
ShapeFromShape (node *avisshape)
{
    bool b;
    pattern *patarray;
    pattern *pat1;
    pattern *pat2;
    node *narray = NULL;
    constant *con = NULL;
    constant *c;
    node *M = NULL;
    int n;
    char *nm1;
    char *nm2;

    DBUG_ENTER ();

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAgetNode (&M), 0));
    pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAisNode (&M), 0));
    b = (PMmatchFlatSkipExtrema (patarray, avisshape) && (NULL != narray)
         && (NULL != ARRAY_AELEMS (narray))
         && (NULL != EXPRS_EXPR (ARRAY_AELEMS (narray))));
    if (b) {
        narray = ARRAY_AELEMS (narray);
        n = 0;
        c = COmakeConstantFromInt (n);
        con = COcopyScalar2OneElementVector (c);
        c = COfreeConstant (c);
        /* We can safely skip guards here, because the array we
         * are ostensibly copying from will have similar guards on its
         * creation.
         */
        DBUG_ASSERT ((N_prf != NODE_TYPE (EXPRS_EXPR (narray)))
                       || (F_idx_sel != PRF_PRF (EXPRS_EXPR (narray))),
                     "Start coding, Mr doivecyc!");
        if (PMmatchFlatSkipGuards (pat1, EXPRS_EXPR (narray))) {
            con = COfreeConstant (con);
            while (b && (NULL != narray)) {
                c = COmakeConstantFromInt (n);
                con = COcopyScalar2OneElementVector (c);
                c = COfreeConstant (c);
                b = b && PMmatchFlatSkipGuards (pat2, EXPRS_EXPR (narray));
                con = COfreeConstant (con);
                narray = EXPRS_NEXT (narray);
                n++;
            }
        }
        con = (NULL != con) ? COfreeConstant (con) : NULL;
    }
    PMfree (patarray);
    PMfree (pat1);
    PMfree (pat2);

    M = b ? M : avisshape;

    nm1 = ((NULL != avisshape) && (N_id == NODE_TYPE (avisshape)))
            ? AVIS_NAME (ID_AVIS (avisshape))
            : "( N_array)";
    nm2
      = ((NULL != M) && (N_id == NODE_TYPE (M))) ? AVIS_NAME (ID_AVIS (M)) : "( N_array)";
    if (b) {
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s is shape(%s)", nm1, nm2);
    } else {
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _sel_()", nm1);
    }

    DBUG_RETURN (M);
}

/** <!--********************************************************************-->
 *
 * @fn static node *arrayFromShape( node *avisshape)
 *
 * @brief: Find the array that was the argument to the
 *         expressions that built shape vector avisshape.
 *
 * @param: avsishape is an N_id, hopefully pointing to an N_array,
 *         or an N_array.
 *
 * Case 1: if avisshape is an N_array of the form:
 *
 *            s0 = idx_shape_sel( 0, M);
 *            s1 = idx_shape_sel( 1, M);
 *            s2 = idx_shape_sel( 2, M);
 *            avisshape = [ s0, s1, s2];
 *
 *         then z is M;
 *
 * Case 2: if avisshape is an N_array of the form:
 *
 *            shp = _shape_A_( M);
 *            v0 = 0;
 *            s0 = _sel_VxA_( v0, shp);
 *            v1 = 0;
 *            s1 = _sel_VxA_( v1, shp);
 *            v2 = 0;
 *            s2 = _sel_VxA_( v2, shp);
 *            avisshape = [ s0, s1, s2];
 *
 *         then z is M;
 *
 * @return: If avisshape is such an N_array, the result is M;
 *          else NULL.
 *
 *****************************************************************************/
static node *
arrayFromShape (node *avisshape)
{
    bool b = TRUE;
    pattern *patarray;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *narray = NULL;
    constant *con = NULL;
    constant *c;
    node *M = NULL;
    node *shp = NULL;
    int n;
    char *nm1;
    char *nm2;

    DBUG_ENTER ();

    DBUG_ASSERT ((N_id == NODE_TYPE (avisshape)) || (N_array == NODE_TYPE (avisshape)),
                 "Expected N_id avisshape");

    /* Case 1*/
    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_idx_shape_sel), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAgetNode (&M), 0));
    pat2 = PMprf (1, PMAisPrf (F_idx_shape_sel), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAisNode (&M), 0));
    if (PMmatchFlatSkipExtrema (patarray, avisshape)) {
        narray = ARRAY_AELEMS (narray);
        n = 0;
        con = COmakeConstantFromInt (n);
        if ((NULL != narray) && (NULL != EXPRS_EXPR (narray))
            && (PMmatchFlatSkipExtrema (pat1, EXPRS_EXPR (narray)))) {
            COfreeConstant (con);
            while (b && (NULL != narray)) {
                con = COmakeConstantFromInt (n);
                n++;
                b = b && PMmatchFlatSkipExtrema (pat2, EXPRS_EXPR (narray));
                COfreeConstant (con);
                narray = EXPRS_NEXT (narray);
            }
            M = b ? M : NULL;
        }
    }
    PMfree (pat1);
    PMfree (pat2);

    nm1 = ((NULL != avisshape) && (N_id == NODE_TYPE (avisshape)))
            ? AVIS_NAME (ID_AVIS (avisshape))
            : "( N_array)";
    nm2
      = ((NULL != M) && (N_id == NODE_TYPE (M))) ? AVIS_NAME (ID_AVIS (M)) : "( N_array)";

    if (b) {
        DBUG_PRINT ("Case 1: AVIS_SHAPE %s is shape(%s)", nm1, nm2);
    } else {
        DBUG_PRINT ("Case 1: AVIS_SHAPE %s not derived from _idx_shape_sel_()", nm1);
    }

    if (NULL == M) { /* Case 2 */
        narray = NULL;
        con = NULL;
        /* The following patterns match the above, except for PMAisPrf */
        pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                      PMvar (1, PMAgetNode (&M), 0));
        pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                      PMvar (1, PMAisNode (&M), 0));
        pat3 = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&shp), 0));
        if ((PMmatchFlatSkipExtrema (patarray, avisshape)) && (NULL != narray)
            && (NULL != ARRAY_AELEMS (narray))
            && (NULL != EXPRS_EXPR (ARRAY_AELEMS (narray)))) {
            narray = ARRAY_AELEMS (narray);
            DBUG_ASSERT ((N_prf != NODE_TYPE (EXPRS_EXPR (narray)))
                           || (F_idx_sel != PRF_PRF (EXPRS_EXPR (narray))),
                         "Start coding, Mr doivecyc2!");
            n = 0;
            c = COmakeConstantFromInt (n);
            con = COcopyScalar2OneElementVector (c);
            c = COfreeConstant (c);
            if (PMmatchFlatSkipExtrema (pat1, EXPRS_EXPR (narray))) {
                con = COfreeConstant (con);
                while (b && (NULL != narray)) {
                    c = COmakeConstantFromInt (n);
                    con = COcopyScalar2OneElementVector (c);
                    c = COfreeConstant (c);
                    b = b && PMmatchFlatSkipExtrema (pat2, EXPRS_EXPR (narray));
                    con = COfreeConstant (con);
                    narray = EXPRS_NEXT (narray);
                    n++;
                }
                if (b && PMmatchFlatSkipExtrema (pat3, M)) {
                    M = shp;
                }
            }
        }

        con = (NULL != con) ? COfreeConstant (con) : NULL;
        PMfree (patarray);
        PMfree (pat1);
        PMfree (pat2);
        PMfree (pat3);

        nm1 = ((NULL != avisshape) && (N_id == NODE_TYPE (avisshape)))
                ? AVIS_NAME (ID_AVIS (avisshape))
                : "( N_array)";
        nm2 = ((NULL != M) && (N_id == NODE_TYPE (M))) ? AVIS_NAME (ID_AVIS (M))
                                                       : "( N_array)";

        if (b) {
            DBUG_PRINT ("Case 2: AVIS_SHAPE %s is shape(%s)", nm1, nm2);
        } else {
            DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _idx_shape_sel_()", nm1);
        }
    }

    DBUG_ASSERT ((NULL == M) || (N_id == NODE_TYPE (M)), "Result not N_id");

    DBUG_RETURN (M);
}

/** <!--********************************************************************-->
 *
 * @fn static bool isAvisShapesMatch( node *arg1, node *arg2)
 *
 * @brief Predicate for matching two N_avis AVIS_SHAPE nodes.
 *
 * Case 1:
 *       A tree compare of the two nodes.
 *
 * Case 2:
 *        A more sophisticated comparison, to handle shapes
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
 * Case 3: We have arg2 (or worse, as an N_arg, where we know nothing):
 *
 *         arg2 = shape_( M);
 *
 * Case 4: arg1 comes into a function as a parameter, with:
 *
 *         shp1 = AVIS_SHAPE( arg1);
 *
 *       and within the function, another array is built with shp1.
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

    DBUG_ENTER ();

    DBUG_PRINT ("checking shape match for %s and %s", AVIS_NAME (arg1), AVIS_NAME (arg2));

    /* Case 1: See if AKS and result shapes match */
    arg1type = AVIS_TYPE (arg1);
    arg2type = AVIS_TYPE (arg2);
    z = TUshapeKnown (arg1type) && TUshapeKnown (arg2type)
        && TUeqShapes (arg1type, arg2type);

    /* Case 2: See if AVIS_SHAPEs match */
    if ((!z) && (NULL != AVIS_SHAPE (arg1)) && (NULL != AVIS_SHAPE (arg2))) {
        z = (CMPT_EQ == CMPTdoCompareTree (AVIS_SHAPE (arg1), AVIS_SHAPE (arg2)));

        /*  Case 3 :Primogenitor of one shape matches other, or
         *  both primogenitors match */
        M1 = arrayFromShape (AVIS_SHAPE (arg1));
        M2 = arrayFromShape (AVIS_SHAPE (arg2));

        z = z || ((NULL != M1) && (ID_AVIS (M1) == arg2))
            || ((NULL != M2) && (ID_AVIS (M2) == arg1)) || ((NULL != M1) && (M1 == M2));

        if (!z) { /* This would be simpler if AVIS_SHAPE were always an N_id */
            M1 = ShapeFromShape (AVIS_SHAPE (arg1));
            M2 = ShapeFromShape (AVIS_SHAPE (arg2));
            z = ((NULL != M1) && (NULL != M2)
                 && ((M1 == M2)
                     || ((N_id == NODE_TYPE (M1)) && (N_id == NODE_TYPE (M2))
                         && (ID_AVIS (M1) == ID_AVIS (M2)))));
        }
    }

    if (z) {
        DBUG_PRINT ("shapes match for %s and %s", AVIS_NAME (arg1), AVIS_NAME (arg2));
    } else {
        DBUG_PRINT ("shapes do not match for %s and %s", AVIS_NAME (arg1),
                    AVIS_NAME (arg2));
    }

    DBUG_RETURN (z);
}

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
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;

    DBUG_ENTER ();

    withid_avis = IDS_AVIS (WITHID_VEC (INFO_WITHID (arg_info)));
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMparam (1, PMAhasAvis (&withid_avis)),
                  PMvar (1, PMAgetAvis (&target), 0));

    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAgetAvis (&target), 0));

    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAhasAvis (&withid_avis), 0));

    if (PMmatchFlat (pat1, cexpr)) {
        DBUG_PRINT ("Case 1: body matches _sel_VxA_(, iv, pwl)");
    }

    if (NULL == target) {
        if ((PMmatchFlat (pat2, cexpr)) && (PMmatchFlat (pat3, offset))) {
            DBUG_PRINT ("Case 1: body matches _idx_sel( offset, pwl) with iv=%s",
                        AVIS_NAME (target));
        }
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

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
    bool old_onefundef;
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

    if (INFO_VALID (arg_info) && NULL == WITHOP_NEXT (WITH_WITHOP (arg_node))
        && (N_genarray == WITH_TYPE (arg_node) || N_modarray == WITH_TYPE (arg_node))) {

        DBUG_PRINT ("Codes OK. Comparing shapes of LHS(%s), RHS(%s)",
                    AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                    AVIS_NAME (INFO_RHSAVIS (arg_info)));

        if (isAvisShapesMatch (INFO_RHSAVIS (arg_info), IDS_AVIS (INFO_LHS (arg_info)))) {
            DBUG_PRINT ("All ok. replacing LHS(%s) WL by %s",
                        AVIS_NAME (IDS_AVIS (INFO_LHS (arg_info))),
                        AVIS_NAME (INFO_RHSAVIS (arg_info)));
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
     * before, assign it to INFO_RHSAVIS.
     * At this point we also check the DataFlowMask, to see if our source array
     * was defined _before_ this wl.
     */
    if (INFO_VALID (arg_info)) {
        DBUG_PRINT ("checking if target is legitimate and known");

        if ((NULL == INFO_RHSAVIS (arg_info) || target == INFO_RHSAVIS (arg_info))
            && DFMtestMaskEntry (INFO_DFM (arg_info), NULL, target)) {
            DBUG_PRINT ("target is valid. saving");

            INFO_RHSAVIS (arg_info) = target;
        } else {
            DBUG_PRINT ("target is NOT valid. skipping wl");

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

#undef DBUG_PREFIX
