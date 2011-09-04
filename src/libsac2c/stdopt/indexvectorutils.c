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

/** <!--********************************************************************-->
 * @}  <!-- Static helper functions -->
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @fn node *IVUTshapeFromShapeArray( node *iv)
 *
 * @brief:  If index vector iv originated as an N_array
 *          formed from shape vector selections, find
 *          that shape vector.
 *
 * Case 1: if avisshape is an N_array of the form:
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
 *         then z is shap; else iv if an N_array, else ID_AVIS( iv).
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
 *          shap; else iv.
 *
 *****************************************************************************/
static node *
IVUTshapevectorFromShapeArraySel (node *iv)
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
        z = iv;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _sel_()", nmin);
    }

    DBUG_RETURN (z);
}

static node *
IVUTshapevectorFromShapeArrayIdxsel (node *iv)
{
    bool b;
    pattern *patarray;
    pattern *pat1;
    pattern *pat2;
    node *narray = NULL;
    constant *con = NULL;
    node *z = NULL;
    node *shapid;
    char *nmin;
    int n;

    DBUG_ENTER ();

    patarray = PMarray (1, PMAgetNode (&narray), 0);
    pat1 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAgetNode (&shapid), 0));
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMconst (1, PMAisVal (&con)),
                  PMvar (1, PMAisNode (&shapid), 0));
    b = (PMmatchFlatSkipExtremaAndGuards (patarray, iv) && (NULL != narray)
         && (NULL != ARRAY_AELEMS (narray))
         && (NULL != EXPRS_EXPR (ARRAY_AELEMS (narray))));
    if (b) {
        narray = ARRAY_AELEMS (narray);
        n = 0;
        con = COmakeConstantFromInt (n);
        if (PMmatchFlatSkipExtremaAndGuards (pat1, EXPRS_EXPR (narray))) {
            con = COfreeConstant (con);
            while (b && (NULL != narray)) {
                con = COmakeConstantFromInt (n);
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
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s is shape(%s)", nmin, AVIS_NAME (ID_AVIS (z)));
    } else {
        z = iv;
        DBUG_PRINT ("Case 2: AVIS_SHAPE %s not derived from _sel_()", nmin);
    }

    DBUG_RETURN (z);
}

node *
IVUTshapevectorFromShapeArray (node *iv)
{
    node *z;

    DBUG_ENTER ();

    z = IVUTshapevectorFromShapeArraySel (iv);
    if (iv == z) {
        z = IVUTshapevectorFromShapeArrayIdxsel (iv);
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
    sv = IVUTshapevectorFromShapeArray (iv);
    PMmatchFlatSkipExtremaAndGuards (pat, sv);

    pat = PMfree (pat);

    DBUG_RETURN (ARR);
}

/** <!--********************************************************************-->
 *
 * @fn bool IVUTisWLShapesMatch()
 *
 * @brief Predicate for matching two WL N_avis nodes, a producer-WL (pwl)
 *        and a consumer-WL (cwl); cwlwith is the N_with of the cwl.
 *        We assume that the caller has already ensured that
 *        pwl contains only one N_part,
 *        that it is a modarray or genarray,
 *        and that the generated elements are scalars.
 *
 * Case 1: Comparison of the WL result shapes.
 *         This works if they are both AKS.
 *
 * Case 2: Comparison of AVIS_SHAPE nodes.
 *         This works under circumstances in SAACYC.
 *
 * Case 3: Attempt to find shape( pwl) as a function of GENERATOR_BOUND2( cwl).
 *
 * Case 4: pwl comes into a function as a parameter, with:
 *
 *         shp1 = AVIS_SHAPE( pwl);
 *
 *       and within the function, another array is built with shp1.
 *
 *       For now, we see if one ( e.g., pwl) of the AVIS_SHAPE nodes
 *       is an N_array of the above form, and if so, if M
 *       is the same array as, e.g., cwl.
 *
 * @return: TRUE if the array shapes can be shown to match.
 *
 *****************************************************************************/
bool
IVUTisWLShapesMatch (node *pwl, node *cwl, node *cwlwith, node *pwlwith)
{
    node *M1;
    node *M2;
    ntype *pwltype;
    ntype *cwltype;
    node *bcwl;
    node *mcwl;
    bool z = FALSE;

    DBUG_ENTER ();

    DBUG_PRINT ("checking shape match for pwl=%s and cwl=%s", AVIS_NAME (pwl),
                AVIS_NAME (cwl));

    /* Case 1: See if AKS and result shapes match */
    pwltype = AVIS_TYPE (pwl);
    cwltype = AVIS_TYPE (cwl);
    z = TUshapeKnown (pwltype) && TUshapeKnown (cwltype) && TUeqShapes (pwltype, cwltype);

    /* Case 3: Attempt to find producerWL from cwl generator shape.
     */

    /* We can ignore GENERATOR_BOUND2 because it must be zero */
    if (!z) {
        bcwl = GENERATOR_BOUND2 (PART_GENERATOR (WITH_PART (cwlwith)));
        mcwl = IVUTarrayFromIv (bcwl);
        z = ((NULL != mcwl) && (ID_AVIS (mcwl) == pwl));
    }

    /* Case 2: See if AVIS_SHAPEs match */
    if ((!z) && (NULL != AVIS_SHAPE (pwl)) && (NULL != AVIS_SHAPE (cwl))) {
        z = (CMPT_EQ == CMPTdoCompareTree (AVIS_SHAPE (pwl), AVIS_SHAPE (cwl)));

        /* Case 4 */
        M1 = IVUTshapevectorFromShapeArray (AVIS_SHAPE (pwl));
        M2 = IVUTshapevectorFromShapeArray (AVIS_SHAPE (cwl));
        z = ((NULL != M1) && (NULL != M2)
             && ((M1 == M2)
                 || ((N_id == NODE_TYPE (M1)) && (N_id == NODE_TYPE (M2))
                     && (ID_AVIS (M1) == ID_AVIS (M2)))));
    }

    if (z) {
        DBUG_PRINT ("shapes match for %s and %s", AVIS_NAME (pwl), AVIS_NAME (cwl));
    } else {
        DBUG_PRINT ("shapes do not match for %s and %s", AVIS_NAME (pwl),
                    AVIS_NAME (cwl));
    }

    DBUG_RETURN (z);
}

#undef DBUG_PREFIX
