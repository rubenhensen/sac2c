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
#include "constants.h"
#include "flattengenerators.h"
#include "new_typecheck.h"
#include "indexvectorutils.h"

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn constant *IVUToffset2Constant(...)
 *
 * @brief: arg_node is the offset PRF_ARG1 of an _idx_sel(, offset, mat)
 *         or _idx_modarray_AxSxS( mat, offset, val).
 *
 *         mat is the array being indexed.
 *
 *         Case 1: arg_node is a constant:
 *
 *         Case 2: arg_node is a _vect2offset( shp, iv),
 *         on constants.
 *
 *         Case 3: arg_node is an _idxs2offset( shp, i0, i1,...);
 *         on constants.
 *
 * @return: The offset converted back to its
 *          IV form, as a vector constant, if known, or NULL.
 *
 *****************************************************************************/
constant *
IVUToffset2Constant (node *arg_node, node *mat)
{
    constant *z = NULL;
    constant *shp = NULL;
    constant *iv = NULL;
    pattern *pat1;
    pattern *pat2;
    node *elems = NULL;
    shape *shpmat;
    int offset;
    int i;
    int len;
    int el;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMconst (1, PMAgetVal (&shp)),
                  PMconst (1, PMAgetVal (&iv), 0));

    pat2 = PMprf (1, PMAisPrf (F_idxs2offset), 1, PMconst (1, PMAgetVal (&shp)), 1,
                  PMskip (0));

    if ((N_id == NODE_TYPE (arg_node)) && (TYisAKS (AVIS_TYPE (ID_AVIS (mat))))
        && (TYisAKV (AVIS_TYPE (ID_AVIS (arg_node))))) {
        z = COaST2Constant (arg_node);

        offset = COconst2Int (z);
        shpmat = TYgetShape (AVIS_TYPE (ID_AVIS (mat)));
        len = SHgetDim (shpmat);
        /* ( shape(mat) represent offset */
        for (i = len - 1; i >= 0; i--) {
            el = offset % SHgetExtent (shpmat, i);
            offset = offset - el; /* Not really needed */
            offset = offset / SHgetExtent (shpmat, i);
            elems = TCappendExprs (TBmakeExprs (TBmakeNum (el), NULL), elems);
        }

        if (NULL != elems) {
            elems = TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (0)),
                                 SHcreateShape (1, len), elems);
            z = COaST2Constant (elems);
            elems = FREEdoFreeTree (elems);
        }
    }

    if ((NULL == z) && (PMmatchFlat (pat1, arg_node))) {
        z = COvect2offset (shp, iv, NULL);
        shp = COfreeConstant (shp);
        iv = COfreeConstant (iv);
    }

    /* z = IVUTidxs2offset( arg_node); */
    if ((NULL == z) && (PMmatchFlat (pat2, arg_node))) {
        shp = COfreeConstant (shp);
        DBUG_UNREACHABLE ("start coding...");
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
node *
IVUTarrayFromProxySel (node *iv)
{
    bool b;
    pattern *patarray;
    pattern *pat1;
    pattern *pat2;
    node *narray = NULL;
    constant *con = NULL;
    constant *c;
    node *mat = NULL;
    node *z = NULL;
    int n;
#ifndef DBUG_OFF
    const char *nmin;
#endif

    DBUG_ENTER ();

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAgetNode (&mat), 0));
    pat2 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAisNode (&mat), 0));
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

#ifndef DBUG_OFF
    nmin = ((NULL != iv) && (N_id == NODE_TYPE (iv))) ? AVIS_NAME (ID_AVIS (iv))
                                                      : "( N_array)";
#endif
    if (b) {
        z = mat;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s is shape(%s)", nmin,
                    AVIS_NAME (ID_AVIS (mat)));
    } else {
        z = NULL;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _sel_()", nmin);
    }

    DBUG_RETURN (z);
}

node *
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
    node *mat;
#ifndef DBUG_OFF
    const char *nmin;
#endif
    node *offset = NULL;
    int n;

    DBUG_ENTER ();

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAgetNode (&mat), 0));
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
                  PMvar (1, PMAisNode (&mat), 0));
    b = (PMmatchFlatSkipExtremaAndGuards (patarray, iv) && (NULL != narray)
         && (NULL != ARRAY_AELEMS (narray))
         && (NULL != EXPRS_EXPR (ARRAY_AELEMS (narray))));
    if (b) {
        narray = ARRAY_AELEMS (narray);
        n = 0;
        if (PMmatchFlatSkipExtremaAndGuards (pat1, EXPRS_EXPR (narray))) {
            con = IVUToffset2Constant (offset, mat);
            if ((NULL != con) && COisZero (con, TRUE)) {
                con = COfreeConstant (con);
                while (b && (NULL != narray)) {
                    if (PMmatchFlatSkipExtremaAndGuards (pat2, EXPRS_EXPR (narray))) {
                        con = IVUToffset2Constant (offset, mat);
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

#ifndef DBUG_OFF
    nmin = ((NULL != iv) && (N_id == NODE_TYPE (iv))) ? AVIS_NAME (ID_AVIS (iv))
                                                      : "( N_array)";
#endif

    if (b) {
        z = mat;
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
 * @fn node *IVUTmatFromIv( node *iv)
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
IVUTmatFromIv (node *iv)
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
    bool z = FALSE;
    pattern *patp;
    pattern *patc;
    pattern *patm1;
    pattern *patm2;
    node *shpp = NULL;
    node *shpc = NULL;

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
        mcwl = IVUTmatFromIv (cavisshape);
        z = ((NULL != mcwl) && (ID_AVIS (mcwl) == pavis));
    }

    /* Case 2: See if AVIS_SHAPEs match */
    if ((!z) && (NULL != AVIS_SHAPE (pavis)) && (NULL != AVIS_SHAPE (cavis))) {
        patp = PMany (1, PMAgetNode (&shpp), 0);
        patc = PMany (1, PMAgetNode (&shpc), 0);
        patm1 = PMany (1, PMAisNode (&shpp), 0);
        patm2 = PMany (1, PMAisNode (&shpc), 0);
        z = ((PMmatchFlatSkipExtremaAndGuards (patp, AVIS_SHAPE (pavis)))
             && (PMmatchFlatSkipExtremaAndGuards (patc, AVIS_SHAPE (cavis)))
             && (CMPT_EQ == CMPTdoCompareTree (shpp, shpc)));

        patp = PMfree (patp);
        patc = PMfree (patc);
        patm1 = PMfree (patm1);
        patm2 = PMfree (patm2);

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
 * @return: the avis node for the new N_array, or NULL, if arg_node is NULL.
 *
 *****************************************************************************/
static node *
CreateIvArray (node *arg_node, node **vardecs, node **preassigns)
{
    node *avis = NULL;
    node *ids;
    node *assgn;
    node *nlet;
    size_t len;
    node *z;

    DBUG_ENTER ();

    if (NULL != arg_node) {
        len = TCcountExprs (arg_node);
        avis = TBmakeAvis (TRAVtmpVar (),
                           TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, len)));
        *vardecs = TBmakeVardec (avis, *vardecs);

        ids = TBmakeIds (avis, NULL);
        assgn = TBmakeAssign (TBmakeLet (ids,
                                         TBmakeArray (TYmakeAKS (TYmakeSimpleType (T_int),
                                                                 SHcreateShape (0)),
                                                      SHcreateShape (1, len),
                                                      DUPdoDupTree (arg_node))),
                              NULL);
        *preassigns = TCappendAssign (*preassigns, assgn);
        AVIS_SSAASSIGN (avis) = assgn;
        nlet = ASSIGN_STMT (assgn);
        z = IVEXPgenerateNarrayExtrema (nlet, vardecs, preassigns);
        LET_EXPR (nlet) = z;
    }

    DBUG_RETURN (avis);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUToffset2Vect(...)
 *
 * @brief  Produce a flattened N_array iv for arg_node, an N_prf that is
 *         a _sel_VxA_() or _idx_sel().
 *         E.g., in the following:
 *
 *            iv = [ i, j, k];
 *            z = _sel_VxA_( iv, producerWL);
 *
 *         ( iv may have extrema attached to it). Return iv.
 *
 *         Alternately, we have
 *
 *            poffset = idxs2offset( shape( producerWL), i, j, k...);
 *            z = _idx_sel_( poffset, producerWL);
 *
 *         There are several cases:
 *
 *         1. We are doing _sel_VxA_( iv, ProducerWL), where iv is an N_array.
 *            Return iv's avis.
 *
 *         2. We are doing idx_sel_( offset, ProducerWL).
 *            Find the idxs2offset( pwl, i, j, k...),
 *            build a flattened N_array from [ i, j, k], and
 *            return its avis.
 *
 *            If we find that [i,j,k] match the ConsumerWL's WITHID_IDS,
 *            return the WITHID_VEC.
 *
 *         3. We can not find the idxs2offset, but the
 *            producerWL generates a vector result.
 *            Return an N_array containing iv if the PWL is a vector.
 *            If not, and offset is constant, and PWL is AKD,
 *            construct a flattened N_array from the offset.
 *
 *         4. arg_node is a vect2offset( shape( producerWL), iv);
 *            Return a flattened N_array if iv is an N_array.
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
 *
 * @param: arg_node - an N_prf node.
 * @param: vardecs - pointer to pointer of N_vardecs, in case we have to
 *                   create new variables.
 * @param: preassigns - pointer to pointer of N_assigns, in case we have to
 *                   create new variables.
 * @param: cwlpart - Optional consumerWL N_part, or NULL
 * @param: pwlpart - Optional producerWL N_part, or NULL
 *
 * @return: the desired avis node, pointing to a vector N_array.
 *          If we can't find one, we return NULL.
 *
 *          We have to return an extant N_avis, when possible,
 *          because we require extrema or an AKV value.
 *
 *****************************************************************************/
node *
IVUToffset2Vect (node *arg_node, node **vardecs, node **preassigns, node *cwlpart,
                 node *pwlpart)
{
    node *z = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    pattern *pat5;
    node *shp = NULL;
    node *iv = NULL;
    node *iv2 = NULL;
    node *iv0 = NULL;
    constant *ivc = NULL;
    node *narr = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *fcon = NULL;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_noteintersect), 2, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAgetNode (&arg2), 0), 0);

    pat2 = PMprf (1, PMAisPrf (F_idxs2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&iv0), 0), 0);

    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&iv), 0));

    pat4 = PMany (1, PMAgetNode (&iv2), 0);

    pat5 = PMarray (1, PMAgetNode (&narr), 1, PMskip (0));

    arg1 = PRF_ARG1 (arg_node);
    PMmatchFlat (pat1, arg1); /* Skip any noteintersect */

    /* If we have an N_array, we are done. Return its LHS */
    if (PMmatchFlat (pat5, arg1)) {
        z = ID_AVIS (arg1);
    }

    if ((NULL == z) && /* G((G( vect2offset))) case */
        (PMmatchFlat (pat3, arg1))) {
        if (PMmatchFlatSkipExtremaAndGuards (pat5, iv)) {
            /* Skipping extrema & guards dictated by
             * CF unit test xxx
             */
            z = ID_AVIS (iv);
        } else { /* May be withid_vec. If so, get withid_ids */
            if ((NULL != cwlpart)
                && (IVUTisIvMatchesWithid (iv, WITHID_VEC (PART_WITHID (cwlpart)),
                                           WITHID_IDS (PART_WITHID (cwlpart))))) {
                z = TCconvertIds2Exprs (WITHID_IDS (PART_WITHID (cwlpart)));
                z = CreateIvArray (z, vardecs, preassigns);
            }
        }
    }

#ifdef DEADCODE
    int dim = -1;
    node *expr;
    if ((NULL == z)
        && (PMmatchFlatSkipExtremaAndGuards (pat2, arg1))) { /* _idx_sel() case */
        z = CreateIvArray (iv0, vardecs, preassigns);
    }

    /* We have not have _idxs2offset any more, due to opts.
     * Case 1: If the dim of the PWL is 1, then the offset is PRF_ARG1.
     * Case 2: If the offset is constant, we can compute the IV.
     *
     * For future work: If PWL is AKD, then we could compute
     * IV symbolically, perhaps via a sacprelude offset2Vect.
     *
     */
    if (NULL != cwlpart) {
        dim = TCcountIds (WITHID_IDS (PART_WITHID (cwlpart)));
    }

    if ((NULL == z) && /* Case 1 */
        ((NULL != cwlpart) && (1 == dim))) {
        expr = TBmakeExprs (TBmakeId (ID_AVIS (arg1)), NULL);
        z = CreateIvArray (expr, vardecs, preassigns);
        expr = FREEdoFreeTree (expr);
    }

#endif /*DEADCODE*/

    /* Constant iv */
    if ((NULL == z) && /* Case 2 */
        (TYisAKS (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))))
        && (TYisAKV (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)))))) {
        ivc = IVUToffset2Constant (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node));
        DBUG_ASSERT (NULL != ivc, "failed to convert offset to constant");
        z = COconstant2AST (ivc);
        DBUG_ASSERT (N_array == NODE_TYPE (z), "Confusion3");
        ivc = COfreeConstant (ivc);
        // Flatten the elements of the resulting N_array
        fcon = FLATGflattenExprsChain (ARRAY_AELEMS (z), vardecs, preassigns, NULL);
        FREEdoFreeTree (ARRAY_AELEMS (z));
        ARRAY_AELEMS (z) = fcon;
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
    pat5 = PMfree (pat5);

    if ((NULL != z) && (N_avis != NODE_TYPE (z))) {
        DBUG_ASSERT (N_array == NODE_TYPE (z), "Expected N_array");
        z = FLATGexpression2Avis (DUPdoDupTree (z), vardecs, preassigns, NULL);
    }

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUToffset2IV(...)
 *
 * @brief arg_node is an iv in _sel_VxA_( iv, PWL) or
 *                       an offset in _idx_sel_( offset, PWL).
 *
 *        Try to map arg_node back to an IV.
 *        For iv, this is simple. For offset, it is a bit harder.
 *
 * @return: the IV, if we can find it, or NULL, otherwise.
 *
 *****************************************************************************/
node *
IVUToffset2IV (node *arg_node)
{
    node *z = NULL;
    pattern *pat;
    pattern *pat2;
    node *bndarr = NULL;
    prf ivprf = F_unknown;
    node *arg1 = NULL;
    node *arg2 = NULL;

    DBUG_ENTER ();

    pat = PMarray (1, PMAgetNode (&bndarr), 1, PMskip (0));
    pat2 = PMprf (1, PMAgetPrf (&ivprf), 3, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAgetNode (&arg2), 0), PMskip (0));

    /* Skip the F_noteintersect, if it is present */
    if ((PMmatchFlatSkipGuards (pat2, arg_node) && (F_noteintersect == ivprf))) {
        arg_node = arg1;
    }

    // If scalar arg_node, look for an IV */
    if (0 == TYgetDim (AVIS_TYPE (ID_AVIS (arg_node)))) { /* _idx_sel() case */
        if (PMmatchFlatSkipGuards (pat2, arg_node)) {
            if ((F_idxs2offset == ivprf) || (F_vect2offset == ivprf)) {
                DBUG_PRINT ("Found F_idxsoffset. Looking at %s",
                            AVIS_NAME (ID_AVIS (arg2)));
                arg_node = arg2;
            }
        }
    }

    if (0 != TYgetDim (AVIS_TYPE (ID_AVIS (arg_node)))) {
        z = arg_node;
    }

    pat = PMfree (pat);
    pat2 = PMfree (pat2);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUTfindIvWithid(...)
 *
 * @brief arg_node is an iv in _sel_VxA_( iv, PWL) or
 *                       offset in _idx_sel_( offset, PWL).
 *
 *        Try to map arg_node back to the producerWL WITHID_VEC.
 *        If this is successful, then iv and the result
 *        are identical, in the sense that there is no
 *        index offset, nor are axes elided, duplicated, or
 *        interchanged.
 *
 * @return: the N_avis node for the WITHID_VEC, if we can find it.
 *          Otherwise, NULL.
 *
 *****************************************************************************/
node *
IVUTfindIvWithid (node *arg_node, node *cwlpart)
{
    node *z = NULL;
    node *withidvec;
    node *withidids;

    DBUG_ENTER ();

    arg_node = IVUToffset2IV (arg_node);
    if (NULL != arg_node) {
        withidvec = WITHID_VEC (PART_WITHID (cwlpart));
        withidids = WITHID_IDS (PART_WITHID (cwlpart));
        if (IVUTisIvMatchesWithid (arg_node, withidvec, withidids)) {
            z = IDS_AVIS (withidvec);
        }
    }

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
 *        NB. Obsolescent now. Replaced by IVUToffset2IV.
 *
 * @return: the N_id of the PRF_ARG2 node, or NULL if we could not
 *          find it.
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

    /* Must skip extrema, too: UTReshape.sac */
    PMmatchFlatSkipExtremaAndGuards (pat, arg_node);
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

    PMmatchFlatSkipGuards (pat, arg_node);
    pat = PMfree (pat);

    DBUG_RETURN (iv);
}

/** <!--********************************************************************-->
 *
 * @fn bool IVUToffsetMatchesOffset( node *offset1, node *offset2)
 *
 * @brief offset1, offset2 are PRF_ARG1 of an _idx_sel() or
 *                             PRG_ARG2 of an _idx_modarray....()
 *
 * @return: Return TRUE if the offsets can be shown to be
 *          identical.
 *
 *****************************************************************************/
bool
IVUToffsetMatchesOffset (node *offset1, node *offset2)
{
    bool z;
    pattern *pat1;
    pattern *pat2;
    node *offset;
    node *iv1;
    node *iv2;

    DBUG_ENTER ();

    /* Case 1  direct match */
    pat1 = PMany (1, PMAgetNode (&offset));
    pat2 = PMany (1, PMAisNode (&offset));

    z = (PMmatchFlatSkipGuards (pat1, offset1))
        && (PMmatchFlatSkipGuards (pat2, offset2));

    if (!z) {
        /* Case 2: match through vect2offsets */
        iv1 = IVUTfindOffset2Iv (offset1);
        iv2 = IVUTfindOffset2Iv (offset2);
        z = (NULL != iv1) && (NULL != iv2) && (PMmatchFlatSkipGuards (pat1, iv1))
            && (PMmatchFlatSkipGuards (pat2, iv2));
    }

    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn bool IVUTisIvMatchesWithid( node *iv, node *withidvec, node *withidids);
 *
 * @brief See if iv is either WITHID_VEC or WITHID_IDS
 *        We skip guards and extrema in this search.
 *
 * @param iv is an N_id node.
 * @param withidvec is an N_ids WITHID_VEC node.
 * @param withidids is a WITHID_IDS chain
 *
 * @return: TRUE if iv matches WITHID_VEC or WITHIDS_IDS
 *
 *****************************************************************************/
bool
IVUTisIvMatchesWithid (node *iv, node *withidvec, node *withidids)
{
    bool z = FALSE;
    pattern *pat;
    node *iv2 = NULL;
    node *aelems;
    node *ids;

    DBUG_ENTER ();

    pat = PMany (1, PMAgetNode (&iv2));

    iv = IVUToffset2IV (iv); // Deal with idx_sel( offset, PWL)
    if (PMmatchFlatSkipExtremaAndGuards (pat, iv)) {

        switch (NODE_TYPE (iv2)) {

        default:
            break;

        case N_id:
            z = (NULL != withidvec) && (IDS_AVIS (withidvec) == ID_AVIS (iv2));
            break;

        case N_array:
            /* We have to skip over extrema and guards when comparing these items */
            aelems = ARRAY_AELEMS (iv2);
            ids = withidids;
            z = (NULL != ids);
            while ((NULL != aelems) && (NULL != ids) && (NULL != EXPRS_EXPR (aelems))
                   && (TRUE == z)
                   && (PMmatchFlatSkipExtremaAndGuards (pat, EXPRS_EXPR (aelems)))
                   && (N_id == NODE_TYPE (iv2))) {
                z = z && (IDS_AVIS (ids) == ID_AVIS (iv2));
                aelems = EXPRS_NEXT (aelems);
                ids = IDS_NEXT (ids);
            }
            z = z && (NULL == ids) && (NULL == aelems);
            break;
        }
    }
    pat = PMfree (pat);

    DBUG_RETURN (z);
}

/** <!--********************************************************************-->
 *
 * @fn node *IVUTindex2Array(...)
 *
 * @brief arg_node is offset in _idx_sel( offset, X) or
 *        an index vector iv in _sel_VxA_( iv, X);
 *
 *        Skip the _vect2offset_ and guards, and look for an N_array from its
 *        PRG_ARG2.
 *
 *        If we end up with a WITHID_VEC, return the corresponding WITHID_IDS.
 *
 * @return: An N_array node that represents the iv/offset, or NULL.
 *
 *****************************************************************************/
node *
IVUTindex2Array (node *arg_node)
{
    node *iv = NULL;
    node *shp;
    node *arr = NULL;
    node *avis;
    pattern *patv;

    DBUG_ENTER ();

    patv = PMprf (1, PMAisPrf (F_vect2offset), 2, PMany (1, PMAgetNode (&shp), 0),
                  PMany (1, PMAgetNode (&iv), 0));
    if (PMmatchFlatSkipGuards (patv, arg_node)) {
        if (N_array != NODE_TYPE (iv)) {
            avis = ID_AVIS (iv);
            DBUG_PRINT ("We found %s", AVIS_NAME (avis));
            // iv could be an N_prf, a WITHID, an N_ap or a function parameter
            // If it's a WITHID_VEC, we'll return the corresponding WITHID_IDS.
            if ((NULL != AVIS_NPART (avis))
                && (avis == IDS_AVIS (WITHID_VEC (PART_WITHID (AVIS_NPART (avis)))))) {
                arr = WITHID_IDS (PART_WITHID (AVIS_NPART (avis)));
            }
        }
    }
    patv = PMfree (patv);

    DBUG_RETURN (arr);
}

#undef DBUG_PREFIX
