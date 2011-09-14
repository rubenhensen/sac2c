/*
 * $Id$
 */

/** <!--********************************************************************-->
 *
 * @defgroup ivut Index vector utility functions
 *
 *  Overview: These functions are intended to provide useful
 *            index-vector-manipulation services for
 *            shape vectors, index vectors, array generator bounds,
 *            and the like.
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file indexvectorutils.c
 *
 * Prefix: IVUT
 *
 *****************************************************************************/
#include "copywlelim.h"

#include "globals.h"

#define DBUG_PREFIX "IVUT"
#include "debug.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "free.h"
#include "pattern_match.h"
#include "constants.h"
#include "shape.h"
#include "type_utils.h"
#include "compare_tree.h"
#include "new_types.h"
#include "DupTree.h"
#include "ivexpropagation.h"

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn constant *IVUToffset2Constant(...)
 *
 * @brief: arg_node is the offset PRF_ARG1 of an _idx_sel()
 *         or _idx_modarray_AxSxS().
 *
 *         Case 1: arg_node is a constant:
 *
 *         Case 2: arg_node is a _vect2offset( shp, iv),
 *         on constants.
 *
 *         Case 3: arg_node is an _idxs2offset( shp, i0, i1,...);
 *         on constants.
 *
 *         In the latter two cases, CF could simplify them to case 1.
 *         However, doing that would remove any trace of the IV that
 *         generated the offset, which makes AWLF and other optimizations
 *         difficult. So, we compute the same result as would CF on
 *         those offset-generating functions, but avoid actually replacing
 *         the computation.
 *
 * @return: The offset as a constant, if known, or NULL.
 *
 *****************************************************************************/
constant *
IVUToffset2Constant (node *arg_node)
{
    constant *z = NULL;
    constant *shp = NULL;
    constant *iv = NULL;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMconst (1, PMAgetVal (&shp)),
                  PMconst (1, PMAgetVal (&iv), 0));

    pat2 = PMprf (1, PMAisPrf (F_vect2offset), 1, PMconst (1, PMAgetVal (&shp)), 1,
                  PMskip (0));

    if ((N_id == NODE_TYPE (arg_node)) && (TYisAKV (AVIS_TYPE (ID_AVIS (arg_node))))) {
        z = COaST2Constant (arg_node);
    }

    if ((NULL == z) && (PMmatchFlat (pat1, arg_node))) {
        z = COvect2offset (shp, iv);
        shp = COfreeConstant (shp);
        iv = COfreeConstant (iv);
    }

    if ((NULL == z) && (PMmatchFlat (pat2, arg_node))) {
        shp = COfreeConstant (shp);
        /* I have no idea how to call COxxx with a variable number of arguments... */
        /* z = IVUTidxs2offset( arg_node); */
        DBUG_ASSERT (FALSE, "start coding...");
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUTarrayFromProxySel( node *iv)
 * @fn node *IVUTarrayFromProxyIdxsel( node *iv)
 *
 * @brief:  If index vector iv originated as an N_array
 *          formed from shape vector selections, find
 *          that shape vector, as z.
 *
 * Case 1: if iv is an N_array of the form:
 *
 *            shap = _shape_A_( );
 *            v0 = [0];
 *            s0 = _sel_VxA_( v0, shap);
 *            v1 = [1];
 *            s1 = _sel_VxA_( v1, shap);
 *            v2 = [2];
 *            s2 = _sel_VxA_( v2, shap);
 *            iv = [ s0, s1, s2];
 *
 *         then z is shap, and we define iv as a proxy for shap.
 *         Otherwise, NULL.
 *
 * Case 2: Like Case 1, excepf for _idx_sel() instead of _sel_VxA_():
 *         E.g.,:
 *
 *            i0 = 0;
 *            s0 = idx_sel( i0, shap);
 *            i1 = 1;
 *            s0 = idx_sel( i1, shap);
 *            ...
 *
 * @comment: We can safely skip guards and extrema, because
 *           we of the restrictions on index vector range.
 *
 * @return: If iv originated as such an N_array, the result is the
 *          shap; else NULL.
 *
 *****************************************************************************/
static node *
IVUTarrayFromProxySel (node *iv)
{
    bool b;
    pattern *patarray;
    pattern *pat1;
    pattern *pat2;
    node *narray = NULL;
    constant *con = NULL;
    constant *c;
    node *shapid = NULL;
    node *z = NULL;
    int n;
    char *nmin;

    DBUG_ENTER ();

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAgetNode (&shapid), 0));
    pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAisNode (&shapid), 0));
    b = (PMmatchFlatSkipExtremaAndGuards (patarray, iv) && (NULL != narray)
         && (NULL != ARRAY_AELEMS (narray))
         && (NULL != EXPRS_EXPR (ARRAY_AELEMS (narray))));
    if (b) {
        narray = ARRAY_AELEMS (narray);
        n = 0;
        c = COmakeConstantFromInt (n);
        con = COcopyScalar2OneElementVector (c);
        c = COfreeConstant (c);
        if (PMmatchFlatSkipExtremaAndGuards (pat1, EXPRS_EXPR (narray))) {
            con = COfreeConstant (con);
            while (b && (NULL != narray)) {
                c = COmakeConstantFromInt (n);
                con = COcopyScalar2OneElementVector (c);
                c = COfreeConstant (c);
                b = b && PMmatchFlatSkipExtremaAndGuards (pat2, EXPRS_EXPR (narray));
                con = COfreeConstant (con);
                narray = EXPRS_NEXT (narray);
                n++;
            }
        } else {
            b = FALSE;
        }
        con = (NULL != con) ? COfreeConstant (con) : NULL;
    }
    PMfree (patarray);
    PMfree (pat1);
    PMfree (pat2);

    nmin = ((NULL != iv) && (N_id == NODE_TYPE (iv))) ? AVIS_NAME (ID_AVIS (iv))
                                                      : "( N_array)";
    if (b) {
        z = shapid;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s is shape(%s)", nmin,
                    AVIS_NAME (ID_AVIS (shapid)));
    } else {
        z = NULL;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _sel_()", nmin);
    }

    DBUG_RETURN (z);
}

static node *
IVUTarrayFromProxyIdxsel (node *iv)
{
    bool b;
    pattern *patarray;
    pattern *pat1;
    pattern *pat2;
    node *narray = NULL;
    constant *con = NULL;
    constant *ncon = NULL;
    node *z = NULL;
    node *shapid;
    char *nmin;
    node *offset = NULL;
    int n;

    DBUG_ENTER ();

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAgetNode (&shapid), 0));
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAisNode (&shapid), 0));
    b = (PMmatchFlatSkipExtremaAndGuards (patarray, iv) && (NULL != narray)
         && (NULL != ARRAY_AELEMS (narray))
         && (NULL != EXPRS_EXPR (ARRAY_AELEMS (narray))));
    if (b) {
        narray = ARRAY_AELEMS (narray);
        n = 0;
        if (PMmatchFlatSkipExtremaAndGuards (pat1, EXPRS_EXPR (narray))) {
            con = IVUToffset2Constant (offset);
            if ((NULL != con) && COisZero (con, TRUE)) {
                con = COfreeConstant (con);
                while (b && (NULL != narray)) {
                    if (PMmatchFlatSkipExtremaAndGuards (pat2, EXPRS_EXPR (narray))) {
                        con = IVUToffset2Constant (offset);
                        ncon = COmakeConstantFromInt (n);
                        b = COcompareConstants (con, ncon);
                        con = COfreeConstant (con);
                        ncon = COfreeConstant (ncon);
                        narray = EXPRS_NEXT (narray);
                        n++;
                    } else {
                        b = FALSE;
                    }
                }
            }
        } else {
            b = FALSE;
        }
        con = (NULL != con) ? COfreeConstant (con) : NULL;
    }
    PMfree (patarray);
    PMfree (pat1);
    PMfree (pat2);

    nmin = ((NULL != iv) && (N_id == NODE_TYPE (iv))) ? AVIS_NAME (ID_AVIS (iv))
                                                      : "( N_array)";
    if (b) {
        z = shapid;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s is shape(%s)", nmin, AVIS_NAME (ID_AVIS (z)));
    } else {
        z = NULL;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _sel_()", nmin);
    }

    DBUG_RETURN (z);
}

node *
IVUTarrayFromProxy (node *iv)
{
    node *z;

    DBUG_ENTER ();

    z = IVUTarrayFromProxySel (iv);
    if (NULL == z) {
        z = IVUTarrayFromProxyIdxsel (iv);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUTarrayFromIv( node *iv)
 *
 * @brief: Find the array that was the argument to the
 *         expressions that built index vector iv.
 *
 * @param: iv is an N_id, hopefully pointing to an N_array,
 *         or an N_array.
 *
 * Case 2: if iv is an N_array of the form:
 *
 *            sv = _shape_A_( ARR);
 *            v0 = 0;
 *            s0 = _sel_VxA_( v0, sv);
 *            v1 = 0;
 *            s1 = _sel_VxA_( v1, sv);
 *            v2 = 0;
 *            s2 = _sel_VxA_( v2, sv);
 *            iv = [ s0, s1, s2];
 *
 *         then z is ARR;
 *
 * @return: If iv is such an N_array, the result is ARR;
 *          else NULL.
 *
 *****************************************************************************/
node *
IVUTarrayFromIv (node *iv)
{
    pattern *pat;
    node *sv = NULL;
    node *ARR = NULL;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_shape_A), 1, PMvar (1, PMAgetNode (&ARR), 0));
    sv = IVUTarrayFromProxy (iv);
    PMmatchFlatSkipExtremaAndGuards (pat, sv);

    pat = PMfree (pat);

    DBUG_RETURN (ARR);
}

/** <!--********************************************************************-->
 *
 * @fn bool IVUTisShapesMatch()
 *
 * @brief Predicate for matching two N_avis nodes. a producer (pavis)
 *        and a consumer (cavis), with the help of a shape
 *        expression for the consumer, cavishape.
 *
 *        cavisshape may be any expression, thanks to the chaps who thought
 *        that unflattening GENERATOR_BOUND1/2 and GENARRAY_SHAPE, etc.,
 *        was a swell idea.
 *
 * Case 1: Comparison of the result shapes.
 *         This works if they are both AKS.
 *
 * Case 2: Comparison of AVIS_SHAPE nodes.
 *         This works under many circumstances in SAACYC,
 *         if they are both N_id nodes.
 *
 * Case 3: Attempt to find shape( pavis) as a function of cshape.
 *         ctshape will match AVIS_SHAPE( clet), but only under
 *         SAACYC. Otherwise, say in the case of a consumer-WL,
 *         we have to provide GENARRAY_SHAPE( cavis), or
 *         GENERATOR_BOUND2( cavis).
 *
 * Case 4: pavis comes into a function as a parameter, with:
 *
 *         shp1 = AVIS_SHAPE( pavis);
 *
 *       and within the function, another array is built with shp1.
 *
 *       For now, we see if one ( e.g., pavis) of the AVIS_SHAPE nodes
 *       is an N_array of the above form, and if so, if M
 *       is the same array as, e.g., cavis.
 *
 * Case 5: In the case where pavis is an N_id and cavis is an N_array,
 * ...
 *
 * @return: TRUE if the array shapes can be shown to match.
 *
 *****************************************************************************/
bool
IVUTisShapesMatch (node *pavis, node *cavis, node *cavisshape)
{
    node *csv;
    node *psv;
    ntype *ptype;
    ntype *ctype;
    node *mcwl;
    node *shp = NULL;
    bool z = FALSE;
    pattern *pat1;
    pattern *pat2;

    DBUG_ENTER ();

    DBUG_PRINT ("checking shape match for producer=%s and consumer=%s", AVIS_NAME (pavis),
                AVIS_NAME (cavis));

    /* Case 1: See if AKS and result shapes match */
    ptype = AVIS_TYPE (pavis);
    ctype = AVIS_TYPE (cavis);
    z = TUshapeKnown (ptype) && TUshapeKnown (ctype) && TUeqShapes (ptype, ctype);

    /* Case 3: Attempt to find producerWL from clet generator shape.
     */

    if (!z) {
        mcwl = IVUTarrayFromIv (cavisshape);
        z = ((NULL != mcwl) && (ID_AVIS (mcwl) == pavis));
    }

    /* Case 2: See if AVIS_SHAPEs match */
    if ((!z) && (NULL != AVIS_SHAPE (pavis)) && (NULL != AVIS_SHAPE (cavis))) {
        pat1 = PMvar (1, PMAgetNode (&shp), 0);
        pat2 = PMvar (1, PMAisNode (&shp), 0);
        z = (PMmatchFlatSkipExtremaAndGuards (pat1, AVIS_SHAPE (pavis))
             && PMmatchFlatSkipExtremaAndGuards (pat2, AVIS_SHAPE (cavis)));
        pat1 = PMfree (pat1);
        pat2 = PMfree (pat2);

        if (!z) {
            /* Case 4 */
            psv = IVUTarrayFromProxy (AVIS_SHAPE (pavis));
            if (NULL == psv) {
                psv = AVIS_SHAPE (pavis);
            }
            csv = IVUTarrayFromProxy (AVIS_SHAPE (cavis));
            if (NULL == csv) {
                csv = AVIS_SHAPE (cavis);
            }
            z = ((NULL != psv) && (NULL != csv)
                 && ((psv == csv)
                     || ((N_id == NODE_TYPE (psv)) && (N_id == NODE_TYPE (csv))
                         && (ID_AVIS (psv) == ID_AVIS (csv)))));
        }
    }

    /* Case 5: We have something like (bug871.sac):
     *
     *  fifty = 50;
     *  AVIS_SHAPE( cwl) = [ 50, siz];
     *  FIXME???
     *
     *
     */

    if (z) {
        DBUG_PRINT ("shapes match for producer=%s and consumer=%s", AVIS_NAME (pavis),
                    AVIS_NAME (cavis));
    } else {
        DBUG_PRINT ("shapes do not match for producer=%s and consumer=%s",
                    AVIS_NAME (pavis), AVIS_NAME (cavis));
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *CreateIvArray( node *arg_node, node **vardecs,
 *                           node **preassigns)
 *
 * @brief: Create an N_array from the index scalars in the N_exprs arg_node.
 *         Basically, we are temporarily recreating the
 *         vect2offset index vector argument, to mollify
 *         those optimizations that need to have an index vector,
 *         rather than an offset, for indexing analysis.
 *
 * @return: the avis node for the new N_array.
 *
 *****************************************************************************/
static node *
CreateIvArray (node *arg_node, node **vardecs, node **preassigns)
{
    node *avis;
    node *ids;
    node *assgn;
    node *nlet;
    int len;
    node *z;

    DBUG_ENTER ();

    len = TCcountExprs (arg_node);
    avis = TBmakeAvis (TRAVtmpVar (),
                       TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, len)));
    *vardecs = TBmakeVardec (avis, *vardecs);

    ids = TBmakeIds (avis, NULL);
    assgn
      = TBmakeAssign (TBmakeLet (ids, TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (0)),
                                                   SHcreateShape (1, len),
                                                   DUPdoDupTree (arg_node))),
                      NULL);
    *preassigns = TCappendAssign (*preassigns, assgn);
    AVIS_SSAASSIGN (avis) = assgn;
    nlet = ASSIGN_STMT (assgn);
    z = IVEXPgenerateNarrayExtrema (nlet, vardecs, preassigns);
    LET_EXPR (nlet) = z;

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUToffset2Iv(...)
 *
 * @brief  We are looking at PRF_ARG1 in the _sel_VxA_() or _idx_sel(),
 *         e.g., iv in the following:
 *
 *            iv = [ i, j, k];
 *            z = _sel_VxA_( iv, producerWL);
 *
 *         within the consumerWL, and iv may have extrema attached to it.
 *
 *         Alternately, we have
 *
 *            poffset = idxs2offset( shape( producerWL), i, j, k...);
 *            z = _idx_sel_( poffset, producerWL);
 *
 *         There are several cases:
 *
 *         1. We are doing _sel_VxA_( iv, ProducerWL):
 *            Return iv's avis.
 *
 *         2. We are doing idx_sel_( offset, ProducerWL).
 *            Find the idxs2offset( pwl, i, j, k...),
 *            build a flattened N_array from [ i, j, k], and
 *            return its avis.
 *
 *            If we find that [i,j,k] match the WITHID_IDS,
 *            return the WITHID_VEC.
 *
 *         3. We can not find the idxs2offset, but the
 *            producerWL generates a vector result.
 *            Return  xxx FIXME.
 *
 *         4. arg_node is a vect2offset( shape( producerWL), iv);
 *            Return iv.
 *
 *        This function is, admittedly, a kludge. It might make more sense to
 *        have IVE generate an intermediate primitive that preserves
 *        the iv. E.g.,:
 *
 *            iv = [ i, j, k];
 *            offset = vect2offset( shp, iv);
 *
 *        becomes:
 *
 *            offset = idxsiv2offset( shp, iv, i, j, k);
 *
 *        This would allow AWLF and WLF to access the index vector
 *        and its extrema without the sort of heuristic that
 *        this function represents.
 *
 *        Then, idxsiv2offset would be turned into a proper
 *        idxs2offset by some other, post-SAACYC traversal.
 *
 * @return: the desired avis node.
 *          If we can't find one, we return NULL.
 *
 *****************************************************************************/
node *
IVUToffset2Iv (node *arg_node, node **vardecs, node **preassigns, node *cwlpart)
{
    node *z = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    node *expr;
    node *shp = NULL;
    node *iv = NULL;
    node *iv0 = NULL;
    node *arg1 = NULL;

    DBUG_ENTER ();

    pat1 = PMprf (2, PMAisPrf (F_noteintersect), PMAgetNode (&arg1), 0);

    pat2 = PMprf (1, PMAisPrf (F_idxs2offset), 1, PMvar (1, PMAgetNode (&shp), 0), 1,
                  PMvar (1, PMAgetNode (&iv0), 0), 0);

    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&iv), 0));

    if (0 != TYgetDim (AVIS_TYPE (ID_AVIS (arg_node)))) { /* _sel_VxA_() case */
        z = ID_AVIS (arg_node);
    }

    if ((NULL == z) && /* skip any noteintersect */
        (PMmatchFlatSkipGuards (pat1, arg_node))) {
        arg_node = PRF_ARG1 (arg1);
    }

    if ((NULL == z) && /* vect2offset case */
        (PMmatchFlatSkipGuards (pat3, arg_node))) {
        z = ID_AVIS (iv);
    }

    if ((NULL == z) && (PMmatchFlatSkipGuards (pat2, arg_node))) { /* _idx_sel() case */
        z = CreateIvArray (iv0, vardecs, preassigns);
    }

    /* We have not have _idxs2offset any more, due to opts.
     * This is OK if PWL bounds are 1-D
     */
    if ((NULL == z) && (NULL != cwlpart)
        && (1 == TCcountIds (WITHID_IDS (PART_WITHID (cwlpart))))) {
        expr = TBmakeExprs (TBmakeId (ID_AVIS (arg_node)), NULL);
        z = CreateIvArray (expr, vardecs, preassigns);
        expr = FREEdoFreeTree (expr);
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_ASSERT (NULL != z, "Unable to rebuild iv from offset");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUTfindIvWith(...)
 *
 * @brief Try to map arg_node back to the producerWL WITHID_VEC.
 *
 * @return: the desired avis node.
 *
 *****************************************************************************/
node *
IVUTfindIvWith (node *arg_node, node *cwlpart)
{
    node *z = NULL;
    pattern *pat;
    pattern *pat2;
    node *bndarr = NULL;
    node *ivprf = NULL;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&bndarr), 1, PMskip (0));
    pat2 = PMvar (1, PMAgetNode (&ivprf), 1, PMskip (0));

    if (0 != TYgetDim (AVIS_TYPE (ID_AVIS (arg_node)))) { /* _sel_VxA_() case */
        z = ID_AVIS (arg_node);
    } else {
        /* Skip the F_noteintersect */
        if ((PMmatchFlatSkipGuards (pat2, arg_node) && (N_prf == NODE_TYPE (ivprf))
             && (F_noteintersect == PRF_PRF (ivprf)))) {
            arg_node = PRF_ARG1 (ivprf);
        }

        if (PMmatchFlatSkipGuards (pat2, arg_node)) { /* _idx_sel() case */
            if ((N_prf == NODE_TYPE (ivprf)) && (F_idxs2offset == PRF_PRF (ivprf))) {
                DBUG_PRINT ("look for pwlpart WITHIDS here");
                z = NULL; /* FIXME */
            } else {
                /* We have not have _idxs2offset any more, due to opts.
                 * This is OK if PWL bounds are 1-D
                 */
                if ((NULL != cwlpart)
                    && (1 == TCcountIds (WITHID_IDS (PART_WITHID (cwlpart))))) {
                    DBUG_PRINT ("confusion   look for pwlprat WITHIDS here");
                    z = NULL; /* FIXME */
                    z = NULL;
                }
            }
        }
    }

    /* Case 4 */
    if ((NULL == z) && (NULL != ivprf) && (F_vect2offset == PRF_PRF (ivprf))) {
        z = ID_AVIS (PRF_ARG2 (ivprf));
    }

    pat = PMfree (pat);
    pat2 = PMfree (pat2);

    DBUG_ASSERT (NULL != z, "Unable to locate iv from offset");

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUTfindOffset2Iv(...)
 *
 * @brief arg_node is offset in _idx_sel( offset, X).
 *        Try to find the vect2offset it came from, and return its
 *        PRF_ARG2.
 *
 * @return: the N_id of the PRF_ARG2 node.
 *
 *****************************************************************************/
node *
IVUTfindOffset2Iv (node *arg_node)
{
    node *shp = NULL;
    node *iv = NULL;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_vect2offset), 2, PMany (1, PMAgetNode (&shp), 0),
                 PMany (1, PMAgetNode (&iv), 0));

    PMmatchFlat (pat, arg_node);
    pat = PMfree (pat);

    DBUG_RETURN (iv);
}

/** <!--********************************************************************-->
 *
 * @fn constant *IVUTiV2Constant(...)
 *
 * @brief arg_node an N_id
 *
 * @return: If arg_node is constant, return its value.
 *
 *****************************************************************************/
constant *
IVUTiV2Constant (node *arg_node)
{
    constant *iv = NULL;
    pattern *pat;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&iv));

    PMmatchFlat (pat, arg_node);
    pat = PMfree (pat);

    DBUG_RETURN (iv);
}

#undef DBUG_PREFIX