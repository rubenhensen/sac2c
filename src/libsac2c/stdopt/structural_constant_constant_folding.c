/** <!--********************************************************************-->
 *
 * @defgroup cf  Structural Constant Constant Folding
 *
 * This module implements Constant Folding on Structural Constants.
 *
 * By "Structural Constant", we mean anything represented by an N_array node.
 *
 * Description:
 *   The code here assumes that the compiler is running with -ecc.
 *   Therefore, it does not make any error checks that will have been
 *   made by various guards.
 *   E.g., the modarray code assumes that arg3 is always a scalar.
 *
 *  @ingroup opt
 *
 *  @{
 *
 *****************************************************************************/

/** <!--********************************************************************-->
 *
 * @file structural_constant_constant_folding.c
 *
 * Prefix: SCCF
 *
 *****************************************************************************/
#include "structural_constant_constant_folding.h"

#define DBUG_PREFIX "SCCF"
#include "debug.h"

#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
#include "tree_utils.h"
#include "type_utils.h"
#include "new_typecheck.h"
#include "globals.h"
#include "traverse.h"
#include "free.h"
#include "DupTree.h"
#include "constants.h"
#include "shape.h"
#include "ctinfo.h"
#include "compare_tree.h"
#include "namespaces.h"
#include "remove_vardecs.h"
#include "constant_folding_info.h"
#include "symbolic_constant_simplification.h"
#include "pattern_match.h"
#include "print.h"
#include "phase.h"
#include "indexvectorutils.h"
#include "ivextrema.h"
#include "flattengenerators.h"
#include "narray_utilities.h"
#include "with_loop_utilities.h"

/******************************************************************************
 *
 * function:
 *   node *StructOpSel(node *arg_node, info *arg_info)
 *
 * description:
 *   computes sel on array expressions with constant structural constant
 *   arguments, as follows:
 *
 *      z = _sel_VxA_ (constant iv, structconstant X)
 *   E.g.,
 *      a = [0,1,2];
 *      b = [3,4,5];
 *      c = [6,7,8];
 *      X   = [a,b,c,d];
 *
 * Case 1:  (shape(iv) == frame_dim(X)):
 *
 *      z = sel([2], X);
 *      --->
 *      z = c;
 *
 * Case 2: (shape(iv) > frame_dim(X)):
 *
 *      z = sel([2,1], X);
 *      --->
 *      tmpX   = c;
 *      tmpiv = [ 1];
 *      z = sel( tmpiv, tmpX);
 *
 * Case 3: (shape(iv) < frame_dim(X):
 *
 *    Illegal.
 *
 * Note:
 *   In order to make iotan.sac work properly, Case 1 has to
 *   work across extrema. E.g.:
 *
 *      x = [a,b,c,...];
 *      x' = _noteminval( x, ...);
 *      z = _sel_VxA_( [0], x');
 *
 * Note:
 *
 *   In order to make CF unit test bug964.sac work properly,
 *   Case 1 (and perhaps others) has to work across guards.
 *   In that case, we have _idx_sel_( constant, guard (structcon)).
 *   Re the note about Bug #1060 below, I think what happens is this:
 *
 *      x = [ v];
 *      x' = _non_neg_val_V_( x);
 *      z = _idx_sel_( 0, x'];
 *
 *   If we optimize across, e.g., v', p= _non_neg_val( v) guard, then
 *   we lose the AVIS_MIN on x'. I think this is the intent of Bug #1060,
 *   but saczilla remains off the air, months later...
 *
 *
 *****************************************************************************/

static node *
StructOpSelHelper (node *prfarg1, node *prfarg2, info *arg_info)
{

    node *result = NULL;
    int iv_len;
    int X_dim;
    constant *take_vec = NULL;
    constant *con1 = NULL;
    constant *con2 = NULL;
    constant *arg2fs = NULL;
    size_t offset;
    node *tmpivid = NULL;
    node *tmpivval = NULL;
    node *tmpivavis = NULL;
    node *tmpXid = NULL;
    node *arg2 = NULL;
    pattern *patconst;
    pattern *patarray;

    DBUG_ENTER ();

    /**
     *   Match for    _sel_VxA_( constant, N_array)
     *   and bind      con1    to constant
     *                 arg2    to N_array-node
     *                 arg2fs  to the frameshape of N_array
     */
    patconst = PMconst (1, PMAgetVal (&con1));
    patarray = PMarray (2, PMAgetNode (&arg2), PMAgetFS (&arg2fs), 1, PMskip (0));

    if ((PMmatchFlat (patconst, prfarg1)) &&
        /* Bug #525 must skip guards here. */

        /* Unfortunately, doing so introduces Bug #1060. */
        /* I'd go look up that bug, but saczilla remains off the air
         * this month, too. However, CF unit test funnyivecyc
         * suffers from failure to remove _idx_sel_( 0, WITHID_VEC),
         * because of the noteintersect in the WL. Hence, we try
         * introducing extrema-skipping here.
         *
         * Well, that makes SAACFprf_le_SxV.sac die, because
         * the sel() ends up giving us a direct reference to
         * the WITHID_IDS, sans extrema. This then makes the
         * le() CF removal fail. Poop.
         *
         * Look at the code in IdxselStructOpSel, after the return
         * from here. That looks to be intended to fix this
         * very problem.
         *
         */
        (PMmatchFlat (patarray, prfarg2))) { /* Get N_array */
        X_dim = SHgetExtent (COgetShape (arg2fs), 0);
        arg2fs = COfreeConstant (arg2fs);
        iv_len = SHgetUnrLen (COgetShape (con1));
        DBUG_ASSERT (iv_len >= X_dim, "shape(iv) <  dim(X)");
        take_vec = COmakeConstantFromInt (X_dim);

        // Select the N_array element we want w/iv prefix
        con2 = COtake (take_vec, con1, NULL);
        offset = Idx2OffsetArray (con2, arg2);
        con2 = COfreeConstant (con2);
        tmpXid = DUPdoDupNode (TCgetNthExprsExprOrNull (offset, ARRAY_AELEMS (arg2)));
        if (iv_len == X_dim) {
            // Case 1 : Exact selection: do the sel operation now.
            DBUG_PRINT ("Exact selection performed for %s = _sel_VxA_( %s, %s)",
                        AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))),
                        AVIS_NAME (ID_AVIS (prfarg1)), AVIS_NAME (ID_AVIS (prfarg2)));
            result = tmpXid;
        } else {
            // Case 2: Selection vector has more elements than frame_dim(X):
            // Perform partial selection on X now; build new selection for
            // run-time.
            DBUG_ASSERT (N_id == NODE_TYPE (tmpXid), "X element not N_id");
            con1 = COdrop (take_vec, con1, NULL); // iv suffix
            take_vec = COfreeConstant (take_vec);
            tmpivavis = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (prfarg1))),
                                    TYmakeAKS (TYmakeSimpleType (T_int),
                                               SHcreateShape (1, iv_len - X_dim)));
            tmpivval = COconstant2AST (con1);
            INFO_VARDECS (arg_info) = TBmakeVardec (tmpivavis, INFO_VARDECS (arg_info));
            tmpivid = TBmakeId (tmpivavis);
            INFO_PREASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (tmpivavis, NULL), tmpivval),
                              INFO_PREASSIGN (arg_info));

            AVIS_SSAASSIGN (tmpivavis) = INFO_PREASSIGN (arg_info);
            DBUG_PRINT ("sel(iv,X) replaced iv: old: %s; new: %s",
                        AVIS_NAME (ID_AVIS (prfarg1)), AVIS_NAME (tmpivavis));

            // Create new sel() operation  _sel_VxA_(tmpiv, tmpX);
            result = TCmakePrf2 (F_sel_VxA, tmpivid, tmpXid);
        }
        con1 = (NULL != con1) ? COfreeConstant (con1) : con1;
    }
    patconst = PMfree (patconst);
    patarray = PMfree (patarray);

    DBUG_RETURN (result);
}

static node *
StructOpSel (node *arg_node, info *arg_info)
{
    node *result = NULL;

    DBUG_ENTER ();

    result = StructOpSelHelper (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node), arg_info);

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *IdxselStructOpSel(node *arg_node, info *arg_info)
 *
 * description:
 *  Like StructOpSel(), but for _idx_sel().
 *
 *  We may have the following situation:
 *
 *     offset = _vect2offset( [1] [0]);
 *     z = _idx_sel_( offset, _notemaxval_( WITHID_VEC));
 *
 *  Even though the offset is constant, we have to preserve it
 *  until after saacyc for use in AWLF, CWLE, etc.
 *
 *  We can not make the PMmatchFlat below skip the extrema on
 *  WITHID_VEC, or we would end up at WITH_IDS, which have no
 *  extrema on them.
 *
 *  Effectively, we now rebuild the iv from the offset.
 *  Hence, the call to StructOpSelHelper...
 *
 *  If that fails, we look for a WITHID_IDS reference.
 *
 *****************************************************************************/
static node *
IdxselStructOpSel (node *arg_node, info *arg_info)
{

    node *result = NULL;
    node *iv = NULL;
    node *ivid = NULL;
    pattern *pat3;
    node *arg2 = NULL;
    int iv_len;
    int offset;
    constant *con1 = NULL;

    DBUG_ENTER ();

    iv = IVUToffset2Vect (arg_node, &INFO_VARDECS (arg_info), &INFO_PREASSIGN (arg_info),
                          NULL, NULL);
    if (NULL != iv) {
        ivid = TBmakeId (iv);
        result = StructOpSelHelper (ivid, PRF_ARG2 (arg_node), arg_info);
        ivid = FREEdoFreeNode (ivid);
    }

    if (NULL == result) {
        con1 = IVUToffset2Constant (PRF_ARG1 (arg_node), PRF_ARG2 (arg_node));
        if (NULL != con1) {
            iv_len = SHgetUnrLen (COgetShape (con1));
            pat3 = PMany (1, PMAgetNode (&arg2), 0);
            /* Code for matching WITHID_IDS:
             * The PM( pat1...) in StructOpSelHelper  must not skip the extrema
             * or we lose that information on the CF result.
             * This is due to the fact that WITH_IDS names are the same
             * across N_part nodes, and can't have extrema on them.
             *
             * We build a copy of the WITHID_IDS element, with extrema.
             */
            if (NULL != INFO_PART (arg_info)) {
                if ((PMmatchFlatSkipExtrema (pat3, PRF_ARG2 (arg_node)))
                    && (N_array == NODE_TYPE (arg2)) /* a WITHID_IDS? */
                    && (1 == iv_len)) {              /* Vector index */
                    offset = COconst2Int (con1);     /* Offset into ravel */
                    DBUG_ASSERT (offset >= 0, "offset cannot be < 0");
                    result = IVEXIwithidsKludge (offset, arg2, INFO_PART (arg_info),
                                                 &INFO_PREASSIGN (arg_info),
                                                 &INFO_VARDECS (arg_info));
                    if (NULL != result) {
                        DBUG_PRINT (
                          "Replaced _idx_sel( offset, WITHID_ID by WITHID_IDS=%s",
                          AVIS_NAME (result));
                        result = TBmakeId (result);
                    }
                }
            }
            pat3 = PMfree (pat3);
            con1 = COfreeConstant (con1);
        }
    }

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_reshape(node *arg_node, info *arg_info)
 *
 * description:
 *   Eliminates structural constant reshape expression
 *      _reshape_VxA_( shp, structcon)

 *   when it can determine that shp is a constant,
 *   and that ( times reduce shp) == times reduce shape(structcon)
 *
 *****************************************************************************/
node *
SCCFprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    shape *resshape;
    constant *con = NULL;
    node *structcon = NULL;
    pattern *pat;
    int timesrhoarg2;
    int prodarg1;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_reshape_VxA), 2, PMconst (1, PMAgetVal (&con)),
                 PMarray (1, PMAgetNode (&structcon), 1, PMskip (0)));

    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        if (0 == TYgetDim (ARRAY_ELEMTYPE (structcon))) {
            resshape = COconstant2Shape (con);
            prodarg1 = SHgetUnrLen (resshape);
            timesrhoarg2 = SHgetUnrLen (ARRAY_FRAMESHAPE (structcon));
            if (prodarg1 == timesrhoarg2) {
                /* If result is a scalar, return that. Else, create an N_array. */
                if (0 == SHgetDim (resshape)) {
                    res = DUPdoDupNode (TCgetNthExprsExprOrNull (0, ARRAY_AELEMS (structcon)));
                } else {
                    res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (structcon)), resshape,
                                       DUPdoDupTree (ARRAY_AELEMS (structcon)));
                }
                DBUG_PRINT ("SCCFprf_reshape performed _reshape_VxA_(%s, %s)",
                            AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
            }
            con = COfreeConstant (con);
        }
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_take_SxV(node *arg_node, info *arg_info)
 *
 * description:
 *
 *  Case 1: Idempotent take, e.g, _take_SxV_( k, iota( k));
 *          with non-constant k. Unfortunately, today, this does not
 *          handle:  _take_SxV_( (-k), iota( k));
 *
 *
 *  Case 2: Undertake on array expression with constant arg1,
 *          and arg2 an N_array.
 *
 *  Case 3: Like Case 1, except constant k.
 *
 *   [If both arguments are constant, CF was done by the typechecker.]
 *
 *****************************************************************************/
node *
SCCFprf_take_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *tail;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *arg2array = NULL;
    constant *con = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    node *el0 = NULL;

    int takecount;
    int dropcount;
    int argxrho;
    int resxrho;
    ntype *typ;

    DBUG_ENTER ();

    pat1 = PMprf (1, PMAisPrf (F_take_SxV), 2, PMvar (1, PMAgetNode (&arg1), 0),
                  PMvar (1, PMAgetNode (&arg2), 0));
    pat2 = PMarray (1, PMAgetNode (&arg2array), 0);
    pat3 = PMprf (1, PMAisPrf (F_take_SxV), 2, PMconst (1, PMAgetVal (&con)),
                  PMvar (1, PMAgetNode (&arg2), 0));
    pat4 = PMprf (1, PMAisPrf (F_take_SxV), 2, PMconst (1, PMAgetVal (&con)),
                  PMarray (1, PMAgetNode (&arg2array), 1, PMskip (0)));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        DBUG_PRINT ("checking _take_SxV_ (%s, %s)...", ID_NAME (arg1), ID_NAME (arg2));
        /* Case 1 */
        if ((NULL != AVIS_SHAPE (ID_AVIS (arg2)))
            && (PMmatchFlat (pat2, AVIS_SHAPE (ID_AVIS (arg2))))) {
            // match shape( arg2)[0] against arg1
            el0 = TCgetNthExprs (0, ARRAY_AELEMS (arg2array));
            if (TULSisValuesMatch (el0, PRF_ARG1 (arg_node))) {
                res = DUPdoDupNode (PRF_ARG2 (arg_node)); /* Idempotent take */
                DBUG_PRINT ("Idempotent take performed based on AVIS_SHAPE ");
            }
        }

        /* Case 2 */
        if ((NULL == res) && (PMmatchFlatSkipExtrema (pat4, arg_node))) {
            DBUG_PRINT ("  ...%s is constant", ID_NAME (arg1));
            argxrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2array));
            takecount = COconst2Int (con);
            resxrho = abs (takecount);
            DBUG_ASSERT (resxrho <= argxrho, "Attempted overtake");
            dropcount = (takecount >= 0) ? 0 : argxrho + takecount;
            DBUG_ASSERT (dropcount >= 0, "drop count cannot be < 0");
            tail = TCtakeDropExprs (resxrho, dropcount, ARRAY_AELEMS (arg2array));
            DBUG_PRINT ("Undertake performed ");
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2array)),
                               SHcreateShape (1, resxrho), tail);
        }

        /* Case 3 */
        typ = AVIS_TYPE (ID_AVIS (arg2));
        if ((res == NULL) && (NULL != con) && (TYisAKV (typ) || TYisAKS (typ))
            && (PMmatchFlatSkipExtrema (pat3, arg_node))) {
            argxrho = SHgetUnrLen (TYgetShape (typ));
            takecount = COconst2Int (con);
            resxrho = abs (takecount);
            if (argxrho == resxrho) {
                res = DUPdoDupNode (PRF_ARG2 (arg_node)); /* Idempotent take */
                DBUG_PRINT ("Idempotent constant take performed ");
            }
        }
    }

    con = (NULL != con) ? COfreeConstant (con) : NULL;
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_drop_SxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements drop for constants
 *   If both arguments are constant, CF was done by the typechecker.
 *   This handles the first two following cases:
 *
 *     1. arg1 is constant zero.
 *     2. arg1 is constant non-zero, and arg2 is an N_array.
 *     3. arg1 is constant, of value matching shape of arg2:
 *        Handled by saaconstant_folding.
 *
 *****************************************************************************/
node *
SCCFprf_drop_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *tail;
    node *arg2 = NULL;
    node *arg2array = NULL;
    constant *con = NULL;
    pattern *pat;
    pattern *pat2;
    int dropcount;
    int dc;
    int resxrho;
    int arg2xrho;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_drop_SxV), 2, PMconst (1, PMAgetVal (&con)),
                 PMvar (1, PMAgetNode (&arg2), 0));

    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        dc = COconst2Int (con);
        if (0 == dc) {
            res = DUPdoDupNode (PRF_ARG2 (arg_node)); /* Case 1 */
        } else {
            pat2 = PMarray (1, PMAgetNode (&arg2array), 0);

            /* We have to skip guards for matmulAKD.sac in the awlf unit tests */
            if (PMmatchFlatSkipExtremaAndGuards (pat2, arg2)) { /* Case 2 */
                dropcount = (dc < 0) ? 0 : dc;
                arg2xrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2array));
                resxrho = arg2xrho - abs (dc);
                if (resxrho < 0) {
                    CTIabort (LINE_TO_LOC (global.linenum),
                              "SCCFprf_drop_SxV tried overdrop of size %d on vector of shape %d",
                              resxrho, arg2xrho);
                }
                DBUG_ASSERT (dropcount >= 0, "drop count cannot be < 0");
                tail = TCtakeDropExprs (resxrho, dropcount, ARRAY_AELEMS (arg2array));
                DBUG_PRINT ("SCCFprf_drop performed ");
                res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2array)),
                                   SHcreateShape (1, resxrho), tail);
            }
            con = COfreeConstant (con);
            pat2 = PMfree (pat2);
        }
        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @function:
 *   node *ModarrayModarray_AxSxS(node *arg_node, info *arg_info)
 *
 * @param: arg_node is a _modarray_AxSxS_ N_prf.
 *
 * @result: If two consecutive modarray operations modify
 *      the same array element, then the first modarray
 *      can safely be removed.
 *
 *      This is done merely by marking the first modarray (b)
 *      for later removal. This is an easy to preserve the
 *      guards on the creation of b.
 *
 * @brief:
 *
 *      b = _modarray_AxSxS_(arr, offset, val1);
 *      z = _modarray_AxSxS_(b,   offset, val2);
 *
 *      This becomes, eventually:
 *
 *      z = _modarray_AxSxS_((arr, offset, val3);
 *
 *****************************************************************************/
static node *
ModarrayModarray_AxSxS (node *arg_node, info *arg_info)
{

    DBUG_ENTER ();

#ifdef BUG897
    // all very dead code now.

    node *arr = NULL;
    node *b = NULL;
    node *offset = NULL;
    node *prf = NULL;
    pattern *pat1;
    pattern *pat2;

    /* z = _modarray_AxSxS_( b, offset, val2)  */
    pat1 = PMprf (1, PMAisPrf (F_idx_modarray_AxSxS), 3, PMany (1, PMAgetNode (&b), 0),
                  PMany (1, PMAgetNode (&offset), 0), PMskip (0));

    /* b = _modarray_AxSxS_( arr, offset, val1)  */
    pat2 = PMprf (2, PMAgetNode (&prf), PMAisPrf (F_idx_modarray_AxSxS), 3,
                  PMany (1, PMAgetNode (&arr), 0), PMany (1, PMAisNode (&offset)),
                  PMskip (0));

    if ((PMmatchFlat (pat1, arg_node)) && (PMmatchFlat (pat2, b))) {
        DBUG_PRINT ("Marked _idx_modarray_AxSxS for $s for removal",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
        PRF_ISNOP (prf) = TRUE;
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

#endif // BUG897

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * @function:
 *   node *ModarrayModarray_AxVxS(node *arg_node, info *arg_info)
 *
 * @param: arg_node is a _modarray_AxVxS_ N_prf.
 *
 * @result: If two consecutive modarray operations modify
 *      the same array element, then the first modarray
 *      can safely be removed.
 *
 *      This is done merely by marking the first modarray
 *      for later removal.
 *
 * @brief:
 *
 *      b = modarray(arr, iv, val1);
 *      z = modarray(b,   iv, val2);
 *
 *      This becomes, eventually:
 *
 *      z = modarray(arr, iv, val3);
 *
 *****************************************************************************/
static node *
ModarrayModarray_AxVxS (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

#ifdef BUG897
    // all very dead code now.

    node *arr = NULL;
    node *b = NULL;
    node *iv = NULL;
    node *prf = NULL;
    pattern *pat1;
    pattern *pat2;

    /* z = _modarray_AxVxS_( b, iv, val2)  */
    pat1 = PMprf (1, PMAisPrf (F_modarray_AxVxS), 3, PMany (1, PMAgetNode (&b), 0),
                  PMany (1, PMAgetNode (&iv), 0), PMskip (0));

    /* b = _modarray_AxVxS_( arr, iv, val1)  */
    pat2
      = PMprf (2, PMAgetNode (&prf), PMAisPrf (F_modarray_AxVxS), 3,
               PMany (1, PMAgetNode (&arr), 0), PMany (1, PMAisNode (&iv)), PMskip (0));

    if ((PMmatchFlat (pat1, arg_node)) && (PMmatchFlat (pat2, b))) {
        DBUG_PRINT ("Marked _modarray_AxVxS for %s for removal",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))));
        PRF_ISNOP (prf) = TRUE;
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
#endif // BUG897

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function: node *ReplaceNarrayElementHelper( node *X, int offset, node *val,
 *                  info *arg_info)
 *
 * description: X is an N_array. Perform X[offset] = val;
 *
 *              This entails flattening each element of the N_array, so
 *              that we do not end up with a mongrel such as [ 666, val];
 *
 * result: Shiny, new N_array node.
 *
 *****************************************************************************/
static node *
ReplaceNarrayElementHelper (node *X, size_t offset, node *val, info *arg_info)
{
    node *z;
    node *exprs;

    DBUG_ENTER ();

    z = DUPdoDupNode (X);
    ARRAY_AELEMS (z) = FLATGflattenExprsChain (ARRAY_AELEMS (z), &INFO_VARDECS (arg_info),
                                               &INFO_PREASSIGN (arg_info), NULL);
    if (offset >= TCcountExprs (ARRAY_AELEMS (z))) {
        DBUG_PRINT ("index error performing indexed assign into %s",
                    AVIS_NAME (IDS_AVIS (LET_IDS (INFO_LET (arg_info)))));
        DBUG_UNREACHABLE ("Index error performing indexed assign into N_array");
    }

    exprs = TCgetNthExprs (offset, ARRAY_AELEMS (z));
    EXPRS_EXPR (exprs) = DUPdoDupNode (val);

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_idx_modarray_AxSxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *
 *     z = _modarray_AxSxS_(X, offset, val);
 *
 *   Also handles scalar X:
 *
 *     z = _modarray_AxSxS_(scalarX, anyoffset, val);
 *
 *   becomes val.
 *
 *   Case 1: Degenerate scalar PRF_ARG1 case.
 *
 *   Case 2: PRF_ARG1 is a vector. NB. The SCCFprf_modarray_AxVxS does
 *           a fancier job on this. FIXME - this could be extended to
 *           handle higher-rank PRF_ARG1.
 *
 *           PRF_ARG1 is a structural constant, and PRF_ARG2 is
 *           a constant.
 *
 *           We do the modarray here, returning a new structural constant.
 *
 *   Case 3: Remove the N_prf if SelModarray() has marked this N_prf as a no-op.
 *
 *
 *****************************************************************************/
node *
SCCFprf_idx_modarray_AxSxS (node *arg_node, info *arg_info)
{
    node *z = NULL;
    pattern *pat;
    node *X = NULL;
    constant *fsX = NULL;
    constant *coiv = NULL;
    node *val = NULL;
    int offset;

    DBUG_ENTER ();

    arg_node = ModarrayModarray_AxSxS (arg_node, arg_info);

    /*
     *  Case 1:  z = F_modarray_AxSxS( X, offset, val)
     *           where X and val are both scalars, becomes:
     *           z = val;
     */
    if ((TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node)))))
        && (TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG3 (arg_node)))))) {
        z = DUPdoDupNode (PRF_ARG3 (arg_node));
        DBUG_PRINT ("_modarray_AxVxS( %s, [], %s) replaced by %s",
                    AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                    AVIS_NAME (ID_AVIS (PRF_ARG3 (arg_node))),
                    AVIS_NAME (ID_AVIS (PRF_ARG3 (arg_node))));
    }

    /* Case 2 */

    if ((NULL == z)) {
        pat = PMprf (1, PMAisPrf (F_idx_modarray_AxSxS), 3,
                     PMarray (2, PMAgetNode (&X), PMAgetFS (&fsX), 1, PMskip (0)),
                     PMconst (1, PMAgetVal (&coiv)), PMvar (1, PMAgetNode (&val), 0));
        if ((PMmatchFlat (pat, arg_node)) && (1 == SHgetUnrLen (COgetShape (fsX)))
            && // PRF_ARG1 is vector
            (TUisScalar (AVIS_TYPE (ID_AVIS (val))))) {
            offset = COconst2Int (coiv);
            DBUG_ASSERT (offset >= 0, "offset cannot be < 0");
            z = ReplaceNarrayElementHelper (X, offset, PRF_ARG3 (arg_node), arg_info);
            DBUG_PRINT ("_idx_modarray_AxSxS (structcon, [..], val) eliminated");
        }
        pat = PMfree (pat);
    }

#ifdef FIXME
    /* need to clone code from next function down.
     * Also, we have to trace back iv to ensure it is an empty vector,
     * in Case 1, and
     */

#endif // FIXME

    /* Case 3 */
    if (TRUE == PRF_ISNOP (arg_node)) {
        z = DUPdoDupNode (PRF_ARG1 (arg_node));
        DBUG_PRINT ("PRF_ISNOP _modarray_AxSxS deleted");
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_modarray_AxVxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxS_(X, iv, val)
 *
 *   This also removes the N_prf if SelModarray() has
 *   marked this N_prf as a no-op.
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxS (node *arg_node, info *arg_info)
{
    node *z = NULL;

    node *X = NULL;
    node *val = NULL;
    constant *emptyVec;
    constant *coiv = NULL;
    constant *fsX = NULL;
    pattern *pat1 = NULL;
    pattern *pat2 = NULL;
    constant *offsetcon;
    int offset;

    DBUG_ENTER ();

    /*
     * if iv is an empty vector, we simply replace the entire
     * expression by val!
     * Well, not quite!!! This is only valid, iff
     *      shape( val) ==  shape(X)
     * If we do not know this, then the only thing we can do is
     * to replace the modarray by
     *      _type_conv_( type(X), val))
     * iff a is AKS! * cf bug246 !!!
     *
     *
     */

    emptyVec = COmakeConstant (T_int, SHcreateShape (1, 0), NULL);
    pat1 = PMprf (1, PMAisPrf (F_modarray_AxVxS), 3, PMvar (1, PMAgetNode (&X), 0),
                  PMconst (1, PMAisVal (&emptyVec)), PMvar (1, PMAgetNode (&val), 0));

    pat2 = PMprf (1, PMAisPrf (F_modarray_AxVxS), 3,
                  PMarray (2, PMAgetNode (&X), PMAgetFS (&fsX), 1, PMskip (0)),
                  PMconst (1, PMAgetVal (&coiv)), PMvar (1, PMAgetNode (&val), 0));

    /*
     *  Case 1:  z = F_modarray_AxVxS( X, [], val)
     *           where X and val are both scalars, becomes:
     *           z = val;
     */
    if (PMmatchFlat (pat1, arg_node) && (TUisScalar (AVIS_TYPE (ID_AVIS (X))))
        && (TUisScalar (AVIS_TYPE (ID_AVIS (val))))) {
        z = DUPdoDupNode (PRF_ARG3 (arg_node));
        DBUG_PRINT ("_modarray_AxVxS (X, [], scalar) eliminated");
    }

    /*
     * Case 2: F_modarray_AxVxS( X = [x0, x1,...xn], iv = [c0,...,cn], val)
     *         where val is scalar, and the shapes of X and iv match.
     */
    if (NULL == z) {
        val = NULL;
        X = NULL;
        if (PMmatchFlat (pat2, arg_node) && TUisScalar (AVIS_TYPE (ID_AVIS (val)))
            && (SHcompareShapes (COgetShape (fsX), COgetShape (coiv)))) {
            offsetcon = COvect2offset (fsX, coiv, NULL);
            offset = COconst2Int (offsetcon);
            DBUG_ASSERT (offset >= 0, "offset cannot be < 0");
            z = ReplaceNarrayElementHelper (X, offset, PRF_ARG3 (arg_node), arg_info);
            DBUG_PRINT ("_modarray_AxVxS (structcon, [..], val) eliminated");
        }
    }

    fsX = (fsX != NULL) ? COfreeConstant (fsX) : fsX;
    coiv = (coiv != NULL) ? COfreeConstant (coiv) : coiv;
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    emptyVec = COfreeConstant (emptyVec);

    /* other Cases */
    arg_node = ModarrayModarray_AxVxS (arg_node, arg_info);

    /* Case 3: Remove nop */
    if ((NULL == z) && (TRUE == PRF_ISNOP (arg_node))) {
        DBUG_PRINT ("PRF_ISNOP _modarray_AxVxS deleted");
        z = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_modarray_AxVxA(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxA_(X, iv, val)
 *   shape(iv) <= dim(X). This makes things a bit harder
 *   than the _modarray_AxVxS case.
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxA (node *arg_node, info *arg_info)
{
    node *res = NULL;

    node *X = NULL;
    node *val = NULL;
    node *exprs, *val_exprs;
    constant *emptyVec;
    constant *coiv = NULL;
    constant *fsX = NULL;
    constant *fsval = NULL;
    constant *ivlen = NULL;
    constant *fsX_tail = NULL;
    pattern *pat1 = NULL;
    pattern *pat2 = NULL;
    pattern *pat3 = NULL;
    pattern *pat4 = NULL;
    constant *offsetcon;
    int offset;

    DBUG_ENTER ();

    /**
     * match F_modarray_AxVxA( _, [], val)
     */
    emptyVec = COmakeConstant (T_int, SHcreateShape (1, 0), NULL);
    pat1 = PMprf (1, PMAisPrf (F_modarray_AxVxA), 3, PMvar (0, 0),
                  PMconst (1, PMAisVal (&emptyVec)), PMvar (1, PMAgetNode (&val), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        res = DUPdoDupNode (PRF_ARG3 (arg_node));
    } else {
        /**
         *   match F_modarray_AxVxA( X = [...], [c0,...,cn], val)
         */
        pat2 = PMprf (1, PMAisPrf (F_modarray_AxVxA), 3,
                      PMarray (2, PMAgetNode (&X), PMAgetFS (&fsX), 1, PMskip (0)),
                      PMconst (1, PMAgetVal (&coiv)), PMvar (1, PMAgetNode (&val), 0));
        if (PMmatchFlatSkipExtrema (pat2, arg_node)) {
            DBUG_ASSERT (COgetDim (fsX) == 1,
                         "illegal frameshape on first arg to modarray");
            /**
             * we distinguish 2 cases:
             * val == [...]    and
             * val == c
             */
            pat3 = PMarray (2, PMAgetNode (&val), PMAgetFS (&fsval), 1, PMskip (0));
            pat4 = PMconst (1, PMAgetVal (&coiv));

            if (PMmatchFlatSkipExtrema (pat3, val) && (COgetDim (coiv) == 1)
                && (COgetExtent (coiv, 0) <= COgetExtent (fsX, 0))) {
                /**
                 *  NB: The only way to get this version of modarray is by means
                 *  of WLUR!! Therefore, it is guaranteed that in fact shape(iv) <=
                 * dim(X)! However, this does not guarantee that shape(iv) <= shape(
                 * frameshape( X))! Example: foo( int[2,2] a)
                 *    {
                 *      X = [a,a];
                 *      ...
                 *    }
                 *  => frameshape( X) = [2]  but shape(X) = [2,2,2] !!
                 */
                DBUG_ASSERT (COgetDim (fsval) == 1,
                             "illegal frameshape on last arg to modarray");
                /**
                 * we have to distinguish 3 cases here:
                 * a) COgetExtent( coiv, 0) + COgetExtent( fsval, 0)
                 *     == COgetExtent( fsX, 0)) :
                 *     this is the trivial case. All we need to do is to replace
                 *     the correct elements in X by the elements of val
                 * b) COgetExtent( coiv, 0) + COgetExtent( fsval, 0)
                 *    < COgetExtent( fsX, 0)) :
                 *    i.e. "we know more more about the array X than needed".
                 *    Example:
                 *       X = [[[a,b],[c,d]], [[a,b],[c,d]]]
                 *       iv = [1]
                 *       val = [q,r]
                 *    Here we want to adjust the level of X by 2 PREASSIGNS:
                 *       A = [a,b]
                 *       B = [c,d]
                 *    and then replace with:
                 *       res = [ [A, B], [q,r] ]
                 * c) COgetExtent( coiv, 0) + COgetExtent( fsval, 0)
                 *    > COgetExtent( fsX, 0)) :
                 *    i.e., "we know more avout the value than needed".
                 *    Example:
                 *       X = [[a,b], [c,d]]
                 *       iv = [1]
                 *       val = [[e,f], [g,h]]
                 *    Here we want to adjust the level of val by 2 PREASSSIGNS:
                 *       A = [e,f]
                 *       B = [g,h]
                 *    and then replace with
                 *       res = [ [a,b], [A,B]]
                 *
                 * HOWEVER, currently we have implemented case a) only!
                 */
                if ((COgetDim (coiv) == 1)
                    && ((COgetExtent (coiv, 0) + COgetExtent (fsval, 0))
                        == COgetExtent (fsX, 0))) {
                    ivlen = COmakeConstantFromInt (COgetExtent (coiv, 0));
                    fsX_tail = COdrop (ivlen, fsX, NULL);
                    if (COcompareConstants (fsval, fsX_tail)) {
                        offsetcon = COvect2offset (fsX, coiv, NULL);
                        offset = COconst2Int (offsetcon);
                        res = DUPdoDupNode (X);
                        DBUG_ASSERT (offset >= 0, "offset cannot be < 0");
                        exprs = TCgetNthExprsOrNull (offset, ARRAY_AELEMS (res));
                        val_exprs = ARRAY_AELEMS (val);
                        while (val_exprs != NULL) {
                            EXPRS_EXPR (exprs) = FREEdoFreeNode (EXPRS_EXPR (exprs));
                            EXPRS_EXPR (exprs) = DUPdoDupNode (EXPRS_EXPR (val_exprs));
                            exprs = EXPRS_NEXT (exprs);
                            val_exprs = EXPRS_NEXT (val_exprs);
                        }
                    }
                    ivlen = COfreeConstant (ivlen);
                    fsX_tail = COfreeConstant (fsX_tail);
                }
            } else if (PMmatchFlatSkipExtrema (pat4, val) && (COgetDim (coiv) == 1)
                       && (COgetExtent (coiv, 0) == COgetExtent (fsX, 0))) {
                /*
                 * match F_modarray_AxVxA( X = [...], iv = [co,...,cn], val = V)
                 *
                 * we only do the simple case (case a above) where V fits neatly
                 * into X as an element.
                 */
                offsetcon = COvect2offset (fsX, coiv, NULL);
                offset = COconst2Int (offsetcon);
                DBUG_ASSERT (offset >= 0, "offset cannot be < 0");
                res = ReplaceNarrayElementHelper (X, offset, val, arg_info);
            }
        }
        if (fsX != NULL) {
            fsX = COfreeConstant (fsX);
            if (coiv != NULL) {
                coiv = COfreeConstant (coiv);
                if (fsval != NULL) {
                    fsval = COfreeConstant (fsval);
                }
            }
        }
    }
    pat1 = (pat1 != NULL) ? PMfree (pat1) : pat1;
    pat2 = (pat2 != NULL) ? PMfree (pat2) : pat2;
    pat3 = (pat3 != NULL) ? PMfree (pat3) : pat3;
    pat4 = (pat4 != NULL) ? PMfree (pat4) : pat4;
    emptyVec = COfreeConstant (emptyVec);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_cat_VxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements vector catenate
 *
 *
 *****************************************************************************/
node *
SCCFprf_cat_VxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    node *arg1aelems;
    node *arg2aelems;
    node *els;
    constant *fs1 = NULL;
    constant *fs2 = NULL;
    constant *frameshape;
    shape *frameshaperes;
    int arg1xrho;
    int arg2xrho;

    DBUG_ENTER ();

    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG1 (arg_node)),
                 "SCCFprf_cat_VxV arg1 not N_id");
    DBUG_ASSERT (N_id == NODE_TYPE (PRF_ARG2 (arg_node)),
                 "SCCFprf_cat_VxV arg2 not N_id");
    DBUG_PRINT ("SCCFprf_cat_VxV PRF_ARG1=%s, PRF_ARG2=%s",
                AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));

    /* This catches the empty-vector case when one argument is not an N_array */
    if (TUisEmptyVect (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node))))) {
        DBUG_PRINT ("SCCFprf_cat (1) removed []++var");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }
    if (TUisEmptyVect (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))) {
        DBUG_PRINT ("SCCFprf_cat (1) removed var++[]");
        res = DUPdoDupNode (PRF_ARG1 (arg_node));
    }

    if ((NULL == res)
        && PMO (
             PMOarrayConstructorGuards (&fs2, &arg2,
                                        PMOarrayConstructorGuards (&fs1, &arg1,
                                                                   PMOprf (F_cat_VxV,
                                                                           arg_node))))) {

        DBUG_ASSERT (1 == SHgetDim (ARRAY_FRAMESHAPE (arg1)),
                     "SCCFprf_cat expected vector arg1 frameshape");
        DBUG_ASSERT (1 == SHgetDim (ARRAY_FRAMESHAPE (arg2)),
                     "SCCFprf_cat expected vector arg2 frameshape");

        DBUG_ASSERT (TUeqShapes (ARRAY_ELEMTYPE (arg1), ARRAY_ELEMTYPE (arg2)),
                     "SCCFprf_cat args have different element types");

        /* Both arguments are constants or structure constants */
        arg1xrho = COconst2Int (fs1);
        arg2xrho = COconst2Int (fs2);
        frameshape = COadd (fs1, fs2, NULL);
        frameshaperes = COconstant2Shape (frameshape);
        frameshape = COfreeConstant (frameshape);
        fs1 = COfreeConstant (fs1);
        fs2 = COfreeConstant (fs2);

        /* Perform the actual element catenation */
        if (0 == arg1xrho) { /* []++arg2 */
            DBUG_PRINT ("SCCFprf_cat (2)removed []++var");
            res = DUPdoDupNode (arg2);
        } else if (0 == arg2xrho) { /* arg2++[] */
            DBUG_PRINT ("SCCFprf_cat (2) removed var++[]");
            res = DUPdoDupNode (arg1);
        } else { /* arg1++arg2 */
            arg1aelems = DUPdoDupTree (ARRAY_AELEMS (arg1));
            arg2aelems = DUPdoDupTree (ARRAY_AELEMS (arg2));
            els = TCappendExprs (arg1aelems, arg2aelems);

            DBUG_PRINT ("SCCFprf_cat performed const1++const2");
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg1)), frameshaperes, els);
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @function:
 *   node *SelModarray(node *arg_node)
 *
 * @param: arg_node is a _sel_ N_prf.
 *
 * @brief:
 *   tries a sel-modarray optimization for the following cases:
 *
 *   FIXME: This optimization should, by all rights be
 *   guarded by an array bounds on iv against M.
 *   See Bug #356.
 *
 *  Case 1. We have one of the following code patterns:
 *
 *    X = modarray_AxVxS_( M, iv, val);
 *    z = sel_VxA( iv, X);
 *
 *     or
 *
 *    X = modarray_AxVxA_( M, iv, val);
 *    z = sel_VxA( iv, X);
 *
 *     or
 *
 *    X = idx_modarray_AxVxS_( M, iv, val);
 *    z = idx_sel_VxA( iv, X);
 *
 *    All of these will turn into:
 *
 *    z = val;
 *
 * Case 4: M' = _modarray( M, iv, q);
 *         X  = _afterguard( M', p0, p1, ...);
 *         z  = _sel_VxA_( iv, X);
 *
 *     We map this into:
 *
 *         z  = _afterguard( q, p0, p1, ...);
 *
 * Case 5: This arises when compiling with -ecc or -check c and AWLF:
 *         Scalar index variant.
 *
 *     lim = ...;
 *     iv = [ i ];
 *     q = 42;
 *     M' = _modarray_AxVxS_( M, iv, q);
 *     M''  = _afterguard_( M' , p0...);
 *     i' , p1 = _val_lt_val_SxS_( i, lim);
 *     iv'' = [ i'];
 *     z' = _sel_VxA_( iv'', M'');
 *     z  = _afterguard_( z', p1);
 *
 *     We map this into:
 *
 *     z' = q;
 *     z  = _afterguard( z', p0, p1, ...);
 *
 *     FIXME: We don't get the afterguard just yet...
 *
 * Case 6: Index vector variant of Case 5:
 *
 *     lim = [...];
 *     iv = [ i ];
 *     q = 42;
 *     M' = _modarray_AxVxS_( M, iv, q);
 *     M''  = _afterguard_( M' , p10, p11, ...);
 *     iv',  p2 = _shape_matches_dim_VxA_( iv, M'');
 *     iv'' ,p1 = _val_lt_shape_VxA_( iv', M'');
 *     z' = _sel_VxA_( iv'', M'');
 *     z  = _afterguard_( z', p1, p2);
 *
 *  We have to show that iv'' == iv, and that M'' == M; then
 *  we can replace the _sel_VxA by:
 *
 *     z' = _afterguard( q, p10, p11,...);
 *     z  = _afterguard_( z', p1, p2);
 *
 *
 * @comments:
 *
 *      This opt is important for loop fusion
 *      and array contraction. WLF has similar properties, but
 *      this example may be clearer. Consider the example of vector-vector
 *      operations in:
 *
 *      Z = B + C * D;
 *
 *      This becomes, roughly:
 *
 *        for (i=0; i<shape(C); i++) {
 *          c      = C[i];
 *          d      = D[i];
 *          t1     = c * d;
 *          tmp1[i]= t1;
 *        }
 *        for (i=0; i<shape(B); i++) {
 *          t2     = tmp1[i];
 *          b      = B[i];
 *          t3     = b + t2;
 *          Z[i]   = t3;
 *        }
 *
 *      Loop fusion will (assuming B and C have same shape)  turn this into:
 *        for (i=0; i<shape(C); i++) {
 *          c      = C[i];
 *          d      = D[i];
 *          t1     = c * d;
 *          tmp1[i]= t1;             NB. This is the modarray
 *          t2     = tmp1[i];         NB. This is the sel()
 *          b      = B[i];
 *          t3     = b + t2;
 *          Z[i]   = t3;
 *        }
 *
 *      Next, this case of SelModArray will turn the code into:
 *
 *        for (i=0; i<shape(C); i++) {
 *          c      = C[i];
 *          d      = D[i];
 *          t1     = c * d;
 *          tmp1[i]= t1;             NB. This is the modarray
 *          t2     = t1;
 *          b      = B[i];
 *          t3     = b + t2;
 *          Z[i]   = t3;
 *        }
 *
 *      After this point, tmp is likely dead, so DCR will remove it
 *      sometime later. CVP will also clean things up a bit more,
 *      to produce:
 *
 *        for (i=0; i<shape(C); i++) {
 *          c      = C[i];
 *          d      = D[i];
 *          t1     = c * d;
 *          b      = B[i];
 *          t3     = b + t1;
 *          Z[i]   = t3;
 *        }
 *
 *
 *  Case 2. See below.
 *
 *  Case 3. Essentially the inverse of case (2): iv is not a constant:
 *      b = modarray(arr, iv, val5);
 *      c = modarray(b, constantiv, val3);
 *      x = sel(iv, c)
 *
 *      Since we do not know the value of iv, we must not make
 *      any assertions about its relationship to constantiv.
 *      Hence, this case must not be optimized.
 *
 * @result: If the selection can be folded, val from the modarray.
 *          Else, NULL.
 *
 *****************************************************************************/

static node *
SelModarray (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *iv = NULL;
    node *iv2 = NULL;
    node *iv3 = NULL;
    node *ivpp = NULL;
    node *X = NULL;
    node *Mprime = NULL;
    node *val = NULL;
    node *modar = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;
    pattern *pat4;
    pattern *pativ;
    pattern *pat5;
    pattern *pat6;
    pattern *pat7;

    DBUG_ENTER ();

    /* z = _sel_VxA_( iv'', X); */
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&ivpp), 0),
                  PMvar (1, PMAgetNode (&X), 0));

    /* Chase  iv'' to iv's RHS */
    pativ = PMany (1, PMAgetNode (&iv), 0);

    /* _X = modarray_AxVxS_( M, iv, val) */
    pat2 = PMprf (2, PMAgetNode (&modar), PMAisPrf (F_modarray_AxVxS), 3, PMvar (0, 0),
                  PMvar (1, PMAgetNode (&iv2), 0), PMvar (1, PMAgetNode (&val), 0));

    /* _X = modarray_AxSxA_( M, iv, val) */
    pat3 = PMprf (2, PMAgetNode (&modar), PMAisPrf (F_modarray_AxVxA), 3, PMvar (0, 0),
                  PMvar (1, PMAgetNode (&iv2), 0), PMvar (1, PMAgetNode (&val), 0));

    /* X' = _afterguard_( X, p0, p1, ... ); */
    pat4 = PMprf (2, PMAisPrf (F_afterguard), PMAgetNode (&Mprime), 1, PMskip (0));

    /* Chase iv' in _modarray(X, iv', val) back to iv */
    pat5 = PMany (1, PMAisNode (&iv), 0);
    /* Ditto for iv2 in _sel_VxA_( iv2, Mprime) */
    pat6 = PMany (1, PMAgetNode (&iv3), 0);
    pat7 = PMany (1, PMAgetNode (&iv), 0);

    /* Skip guards on iv and afterguard on X */
    if (PMmatchFlat (pat1, arg_node) && PMmatchFlatSkipGuards (pativ, ivpp)
        && (PMmatchFlatSkipGuards (pat2, X) || PMmatchFlatSkipGuards (pat3, X))
        && PMmatchFlatSkipGuards (pat5, iv2)) {

#ifdef BUG897
        PRF_ISNOP (modar) = TRUE;
#endif // BUG897
        res = DUPdoDupNode (PRF_ARG3 (modar));
        DBUG_PRINT ("replaced _sel_VxA_(iv, %s) of modarray by %s",
                    AVIS_NAME (ID_AVIS (X)), AVIS_NAME (ID_AVIS (PRF_ARG3 (modar))));
    } else {

        /* Case 4 */
        /*
         * Mprime will be the F_afterguard N_prf node.
         */
        if ((NULL != ivpp) && (PMmatchFlat (pativ, ivpp)) && (PMmatchFlat (pat4, X))
            && ((PMmatchFlat (pat2, PRF_ARG1 (Mprime))
                 || PMmatchFlat (pat3, PRF_ARG1 (Mprime))))
            && (PMmatchFlat (pat6, iv)) && (PMmatchFlat (pat7, iv2))
            && (PMmatchFlat (pat5, iv3))) {

#ifdef BUG897
            PRF_ISNOP (modar) = TRUE;
#endif // BUG897
            res = DUPdoDupNode (PRF_ARG3 (modar));
            DBUG_PRINT ("replaced _sel_VxA_(iv, %s) of modarray by guarded %s",
                        AVIS_NAME (ID_AVIS (X)), AVIS_NAME (ID_AVIS (PRF_ARG3 (modar))));
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);
    pat4 = PMfree (pat4);
    pativ = PMfree (pativ);
    pat5 = PMfree (pat5);
    pat6 = PMfree (pat6);
    pat7 = PMfree (pat7);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @function:
 *   node *IdxselModarray(node *arg_node)
 *
 * @param: arg_node is a _idx_sel_ N_prf.
 *
 * @brief: Like SelModarray, but works on _idx_sel/_idx_modarray pairs.
 *         See there for details.
 *
 *                  offset = vect2offset( shape(M), iv);
 *                  X = idx_modarray_AxSxS_( M, offset, val);
 *                  z = _idx_sel( offset, X);
 *
 *         becomes:
 *                  z = val;
 *

 *
 *
 * @result: If the selection can be folded, val from the modarray.
 *          Else, NULL.
 *
 *****************************************************************************/

static node *
IdxselModarray (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat1;
    pattern *pat2;
    node *X = NULL;
    node *offset1 = NULL;
    node *offset2 = NULL;
    node *val = NULL;
    node *modar = NULL;

    DBUG_ENTER ();

    /* z = _idx_sel( offset, X); */
    pat1 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset1), 0),
                  PMvar (1, PMAgetNode (&X), 0));

    /* _X = idx_modarray_AxSxS_( M, offset, val) */
    pat2
      = PMprf (2, PMAisPrf (F_idx_modarray_AxSxS), PMAgetNode (&modar), 3, PMvar (0, 0),
               PMvar (1, PMAgetNode (&offset2), 0), PMvar (1, PMAgetNode (&val), 0));

    /* Must skip afterguard on X */
    if ((PMmatchFlat (pat1, arg_node)) && (PMmatchFlatSkipGuards (pat2, X))
        && (IVUToffsetMatchesOffset (offset1, offset2))) {
#ifdef BUG897
        PRF_ISNOP (modar) = TRUE;
#endif // BUG897
        res = DUPdoDupNode (PRF_ARG3 (modar));
        DBUG_PRINT ("replaced _idx_sel(offset, %s) of modarray by %s",
                    AVIS_NAME (ID_AVIS (X)), AVIS_NAME (ID_AVIS (PRF_ARG3 (modar))));
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @function:
 *   node *SelModarrayCase2(node *arg_node, info *arg_info)
 *
 * @param: arg_node is a _sel_ N_prf.
 *
 * @brief: Case 2. ivc1, ivc2, ivc3c are constants:
 *
 *      ivc1 = [4];
 *      b = modarray(arr, ivc1, val5);
 *      ivc2 = [4];
 *      c = modarray(b, ivc2, val3);
 *      ivc3 = [4];
 *      d = modarray(c, ivc3, val2);
 *      z = sel(ivc3, d)
 *
 *      If ivc2 != ivc3, but ivc3 == ivc1, then
 *      this becomes:
 *
 *        z = val5;
 *
 *      In the above, if any of ivc1, ivc2, or ivc3 are not
 *      constants, then we must NOT perform the
 *      optimization, because we are unable to assert that
 *      those values are not equal to ivc3.
 *
 * @result: if the selection can be folded, the result is the
 *          val5 from the first modarray. Otherwise, NULL.
 *
 *****************************************************************************/
static node *
SelModarrayCase2 (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *ivc1 = NULL;
    constant *ivc2 = NULL;
    node *val = NULL;
    node *X = NULL;
    node *X2 = NULL;
    pattern *pat1;
    pattern *pat2;
    node *modar = NULL;

    DBUG_ENTER ();

    /* z = _sel_VxA_( ivc1, X); */
    pat1 = PMprf (2, PMAisPrf (F_sel_VxA), PMAgetNode (&modar), 2,
                  PMconst (1, PMAgetVal (&ivc1)), PMvar (1, PMAgetNode (&X), 0));

    /* X = _modarray_AxVxS_( M, ivc2, val)  */
    pat2 = PMprf (2, PMAisPrf (F_modarray_AxVxS), PMAgetNode (&modar), 3,
                  PMvar (1, PMAgetNode (&X2), 0), PMconst (1, PMAgetVal (&ivc2)),
                  PMvar (1, PMAgetNode (&val), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        while (PMmatchFlatSkipExtrema (pat2, X)) {
            if (COcompareConstants (ivc1, ivc2)) {
                break;
            } else { /* Chase the modarray chain */
                val = NULL;
                ivc2 = NULL;
                X = X2;
                X2 = NULL;
            }
        }
        if (NULL != val) {
            DBUG_PRINT ("Replaced _sel_VxA_(const, %s) by %s", AVIS_NAME (ID_AVIS (X)),
                        AVIS_NAME (ID_AVIS (val)));
            res = DUPdoDupNode (val);
#ifdef BUG897
            PRF_ISNOP (modar) = TRUE;
#endif // BUG897
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @function:
 *   node *IdxselModarrayCase2(node *arg_node, info *arg_info)
 *
 * @param: arg_node is an idx_sel N_prf.
 *
 * @brief: Case 2. ivc1, ivc2, ivc3c are constants:
 *
 *      ivc1 = [4];
 *      b = _idx_modarray_AxSxS(arr, ivc1, val5);
 *      ivc2 = [4];
 *      c = _idx_modarray_AxSxS(b, ivc2, val3);
 *      ivc3 = [4];
 *      d = _idx_modarray_AxSxS(c, ivc3, val2);
 *      z = _idx_sel(ivc3, d)
 *
 *      If ivc2 != ivc3, but ivc3 == ivc1, then
 *      this becomes:
 *
 *        z = val5;
 *
 *      In the above, if any of ivc1, ivc2, or ivc3 are not
 *      constants, then we must NOT perform the
 *      optimization, because we are unable to assert that
 *      those values are not equal to ivc3.
 *
 * @result: if the selection can be folded, the result is the
 *          val5 from the first modarray. Otherwise, NULL.
 *
 *****************************************************************************/
static node *
IdxselModarrayCase2 (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *ivc1 = NULL;
    constant *ivc2 = NULL;
    node *val = NULL;
    node *X = NULL;
    node *X2 = NULL;
    pattern *pat1;
    pattern *pat2;
    node *modar = NULL;

    DBUG_ENTER ();

    /* z = _idx_sel( ivc3, d); */
    pat1 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMconst (1, PMAgetVal (&ivc1)),
                  PMvar (1, PMAgetNode (&X), 0));

    /* X = _idx_modarray_AxSxS_( M, ivc2, val)  */
    pat2 = PMprf (2, PMAisPrf (F_idx_modarray_AxSxS), PMAgetNode (&modar), 3,
                  PMvar (1, PMAgetNode (&X2), 0), PMconst (1, PMAgetVal (&ivc2)),
                  PMvar (1, PMAgetNode (&val), 0));

    if (PMmatchFlatSkipExtrema (pat1, arg_node)) {
        while (PMmatchFlatSkipExtrema (pat2, X)) {
            if (COcompareConstants (ivc1, ivc2)) {
                break;
            } else { /* Chase the modarray chain */
                val = NULL;
                ivc2 = NULL;
                X = X2;
                X2 = NULL;
            }
        }
        if (NULL != val) {
            DBUG_PRINT ("Replaced _sel_VxA_(const, %s) by %s", AVIS_NAME (ID_AVIS (X)),
                        AVIS_NAME (ID_AVIS (val)));
            res = DUPdoDupNode (val);
#ifdef BUG897
            PRF_ISNOP (modar) = TRUE;
#endif // BUG897
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * SelEmptyScalar(node *arg_node, info *arg_info)
 *
 * description:
 *   Detects:                  z = sel([], scalar);
 *     and converts it into:   z = scalar;
 *
 *
 *****************************************************************************/
static node *
SelEmptyScalar (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1;
    node *arg2;

    DBUG_ENTER ();

    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    DBUG_ASSERT (N_id == NODE_TYPE (arg1), "arg1 not N_id");
    DBUG_ASSERT (N_id == NODE_TYPE (arg2), "arg2 not N_id");

    if (TUisScalar (AVIS_TYPE (ID_AVIS (arg2)))
        && TUisEmptyVect (AVIS_TYPE (ID_AVIS (arg1)))) {
        DBUG_PRINT ("Removed sel([], replaced by scalar=%s", AVIS_NAME (ID_AVIS (arg2)));
        res = DUPdoDupNode (arg2);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *SelArrayOfEqualElements( node *arg_node, info *arg_info)
 *
 * @brief Matches selections of the following form
 *
 *        z = sel( iv, [i, i, i, i, i]);
 *
 *        where the length of iv matches the length of the frameshape of
 *        the array.
 *
 *        NB. We do not make any check of iv's value. Our assumption
 *        here is that you will compile with "-check c" if you really
 *        care about checking for index error.
 *
 * @param arg_node N_prf node of sel
 * @param arg_info info structure
 *
 * @return if prf matches the above pattern it returns i
 *         else NULL
 *
 ******************************************************************************/
static node *
SelArrayOfEqualElements (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *iv = NULL;
    node *aelems = NULL;
    node *elem = NULL;
    constant *frameshape = NULL;
    pattern *pat1;
    pattern *pat2;
    pattern *pat3;

    DBUG_ENTER ();

    pat1
      = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
               PMarray (2, PMAgetNode (&aelems), PMAgetFS (&frameshape), 1, PMskip (0)));
    pat2 = PMany (1, PMAisNodeOrAvis (&elem), 0);
    pat3 = PMany (1, PMAgetNodeOrAvis (&elem), 0);

    if ((PMmatchFlat (pat1, arg_node))
        && (TUshapeKnown (AVIS_TYPE (ID_AVIS (iv))) && (0 != ARRAY_AELEMS (aelems))
            && (SHgetExtent (TYgetShape (AVIS_TYPE (ID_AVIS (iv))), 0)
                == COgetExtent (frameshape, 0)))) {
        if (NAUTisAllElemsSame (aelems)) {
            DBUG_PRINT ("Removed sel()");
            res = DUPdoDupTree (EXPRS_EXPR (ARRAY_AELEMS (aelems)));
        }
    }

    frameshape = (NULL != frameshape) ? COfreeConstant (frameshape) : NULL;
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);
    pat3 = PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *IdxselArrayOfEqualElements( node *arg_node, info *arg_info)
 *
 * @brief Matches selections of the following form
 *
 *        z = sel( iv, [i, i, i, i, i]);
 *
 *        where the length of iv matches the length of the frameshape of
 *        the array.
 *
 *        or
 *
 *        z =_idx_sel( offset, [ i, i, i, i, i]);
 *
 *
 * @param arg_node N_prf node of sel
 * @param arg_info info structure
 *
 * @return if prf matches the above pattern it returns i
 *         else NULL
 *
 ******************************************************************************/
static node *
IdxselArrayOfEqualElements (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *aelems = NULL;
    node *offset = NULL;
    constant *frameshape = NULL;
    pattern *pat1;
    pattern *pat3;
    node *iv = NULL;
    node *shp = NULL;

    DBUG_ENTER ();

    pat1
      = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&offset), 0),
               PMarray (2, PMAgetNode (&aelems), PMAgetFS (&frameshape), 1, PMskip (0)));

    /* Backtrack and find IV from offset to check that
     * shape(iv) == frameshape. See _sel_() case.
     */
    pat3 = PMprf (1, PMAisPrf (F_vect2offset), 2, PMvar (1, PMAgetNode (&shp), 0),
                  PMvar (1, PMAgetNode (&iv), 0));

    
    if ((PMmatchFlat (pat1, arg_node)) && (PMmatchFlat (pat3, offset))
        && (TUshapeKnown (AVIS_TYPE (ID_AVIS (iv)))) && (0 != ARRAY_AELEMS (aelems))
        && (SHgetExtent (TYgetShape (AVIS_TYPE (ID_AVIS (iv))), 0)
            == COgetExtent (frameshape, 0))) {
        if (NAUTisAllElemsSame (aelems)) {
            DBUG_PRINT ("replacing idx_sel (%s, %s)",
                        ID_NAME (PRF_ARG1 (arg_node)),
                        ID_NAME (PRF_ARG2 (arg_node))
                        );
            DBUG_EXECUTE (PRTdoPrintNodeFile (stderr, aelems););
            res = DUPdoDupTree (EXPRS_EXPR (ARRAY_AELEMS (aelems)));
        }
    }

    frameshape = (NULL != frameshape) ? COfreeConstant (frameshape) : NULL;
    pat1 = PMfree (pat1);
    pat3 = PMfree (pat3);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn node *SelProxyArray( node *arg_node, info *arg_info)
 *
 * @brief This matches the case where elements are selected from an
 *        array via a proxy array built just for this purpose.
 *
 *        vi = sel( [l1, ..., lm, i], A);
 *        P = [v1, ..., vn]
 *
 *        ...
 *
 *        x = sel( [iv], P);
 *
 *        where iv_x is the index of some withloop. We cannot replace P by A,
 *        as P might be just a subarray. However, we can replace the sel by
 *
 *        x = sel( [l1, ..., lm, iv], A);
 *
 *        Furthermore, we can filter out the prefix of all elements from
 *        axes that have an extent of 1. In these cases, the corresponding
 *        index will iterate only over one value and thus it does not matter
 *        whether we pick the l1, ..., lm or some constants. This is a common
 *        pattern when code gets specialised. So, if
 *
 *        P = [ [v1, ..., vn]];
 *
 *        ...
 *
 *        x = sel( {iv_1, iv_2], P);
 *
 *        we can ignore iv_1 as it has to be 0 anyway. We lose a check here
 *        but ecc should catch these cases for us.
 *        Finally, the proxy might cover more than one dimension.
 *
 *        You can funk this up far more by adding support for permutations,
 *        but lets not overdo this for now :)
 *
 * @param arg_node the sel prf
 * @param arg_info info structure
 *
 * @return retargeted sel if possible
 ******************************************************************************/
static void *IPS_FAILED = (void *)0x1;

static void *
IsProxySel (constant *idx, void *sels, void *xtemplate)
{
    node *index;

    DBUG_ENTER ();

    if (sels == NULL) {
        DBUG_ASSERT (TRUE, "ran out of selection operations!");

        sels = IPS_FAILED;
    } else if (sels != IPS_FAILED) {
        index = COconstant2AST (idx);

        DBUG_ASSERT (NODE_TYPE (index) == N_array, "index not array?!?");

        if (!PMO (
              PMOexprs (&ARRAY_AELEMS (index),
                        PMOpartExprs ((node *)xtemplate,
                                      PMOarray (NULL, NULL,
                                                PMOprf (F_sel_VxA,
                                                        EXPRS_EXPR ((node *)sels))))))) {
            sels = IPS_FAILED;
        } else {
            sels = EXPRS_NEXT ((node *)sels);
        }

        index = FREEdoFreeTree (index);
    }

    DBUG_RETURN (sels);
}

static node *
IsSingleSourceArray (node *aelems_P, prf selop, info *arg_info)
{
    node *xtemplate = NULL;
    pattern *pat_e1;
    pattern *pat_en;
    node *var_A = NULL;
    node *tmp;
    bool all_sels = TRUE;
    ;

    DBUG_ENTER ();
    /*
     * before we check that P is a proxy, we check whether it is defined
     * by sel operations on a single source array. This test is way
     * cheaper, so testing twice is worth it.
     *
     * The result is the N_exprs node for the first IV N_array.
     */
    DBUG_PRINT ("Found matching sel!");

    pat_e1 = PMprf (1, PMAisPrf (selop), 2,
                    PMarray (0, 1, PMskip (1, PMAgetNode (&xtemplate))),
                    PMvar (1, PMAgetNode (&var_A), 0));

    pat_en = PMprf (1, PMAisPrf (selop), 2, PMarray (0, 1, PMskip (0)),
                    PMvar (1, PMAisVar (&var_A), 0));
    tmp = aelems_P;
    all_sels = PMmatchFlat (pat_e1, EXPRS_EXPR (tmp));
    tmp = EXPRS_NEXT (tmp);

    while (all_sels && (tmp != NULL)) {
        all_sels = PMmatchFlat (pat_en, EXPRS_EXPR (tmp));
        tmp = EXPRS_NEXT (tmp);
    }

    pat_e1 = PMfree (pat_e1);
    pat_en = PMfree (pat_en);

    if (all_sels) {
        DBUG_PRINT ("Might have found a proxy=%s", AVIS_NAME (ID_AVIS (var_A)));
        INFO_PROXYARR (arg_info) = var_A;
    } else {
        DBUG_PRINT ("No proxy found.");
        xtemplate = NULL;
        INFO_PROXYARR (arg_info) = NULL;
    }

    DBUG_RETURN (xtemplate);
}

static node *
SelProxyArray (node *arg_node, info *arg_info)
{
    node *aelems_iv = NULL;
    node *aelems_P = NULL;
    node *arr_P = NULL;
    node *tmp;
    node *filter_iv;
    node *xtemplate = NULL;
    constant *fs_iv = NULL;
    constant *fs_P = NULL;
    shape *fs_P_shp;
    shape *iter_shp;
    node *iv_avis;
    int pos;
    size_t tlen, flen;
    pattern *pat;

    node *res = NULL;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_sel_VxA), 2,
                 PMarray (1, PMAgetFS (&fs_iv), 1, PMskip (1, PMAgetNode (&aelems_iv))),
                 PMarray (2, PMAgetFS (&fs_P), PMAgetNode (&arr_P), 1,
                          PMskip (1, PMAgetNode (&aelems_P))));

    if (PMmatchFlatSkipExtrema (pat, arg_node) && (aelems_P != NULL)) {
        xtemplate = IsSingleSourceArray (aelems_P, F_sel_VxA, arg_info);
        if (NULL != xtemplate) {

            /*
             * First of all, we filter out the prefix of indices that correspond
             * to a 1 extent dimension in P and the corresponding elements
             * from the frameshape of P to get the iteration space over A. This
             * caters for proxies of the form
             *
             * A = ...
             * P = [[sel(p_iv1, A), ..., [sel(p_ivn, A)]];
             * r = sel( iv, P);
             *
             * where the outer indices of iv have no correspondence in the p_ivx.
             * Note, however, that this transformation is still correct if the
             * outer dimension has a correspondence in P_iv, as it will be
             * identical for all selections.
             */
            filter_iv = DUPdoDupTree (aelems_iv);
            fs_P_shp = ARRAY_FRAMESHAPE (arr_P);

            pos = 0;
            while ((SHgetExtent (fs_P_shp, pos) == 1) && (filter_iv != NULL)) {
                filter_iv = FREEdoFreeNode (filter_iv);
                pos++;
            }

            DBUG_ASSERT (filter_iv != NULL, "weird selection encountered....");

            flen = TCcountExprs (filter_iv);
            iter_shp = SHdropFromShape (SHgetDim (fs_P_shp) - flen, fs_P_shp);
            tlen = TCcountExprs (xtemplate);

            /*
             * If by now we still have not managed to reduce the index used
             * for selections into the proxy such that it is shorter or
             * equally long as the selections used to construct the proxy,
             * we have to give up.
             */
            if (tlen >= flen) {
                /*
                 * now the final step:
                 *
                 * check whether all sels are of the form
                 *
                 * sel_VxA( [v1, ..., vn, c1, ..., cn], A)
                 *
                 */
                if (tlen == flen) {
                    xtemplate = NULL; /* no non-index part */
                } else {
                    xtemplate = DUPdoDupTree (xtemplate);
                    tmp = TCgetNthExprs (tlen - flen - 1, xtemplate);
                    EXPRS_NEXT (tmp) = FREEdoFreeTree (EXPRS_NEXT (tmp));
                }

                /*
                 * now we check whether all selections are
                 *
                 * xtemplate ++ some constants
                 */
                tmp = (node *)COcreateAllIndicesAndFold (iter_shp, IsProxySel, aelems_P,
                                                         xtemplate, FALSE);

                /*
                 * if that worked out, we can replace the selection by
                 *
                 * sel ( [v1, ..., vn, i1, ..., in]], A)
                 */
                if (tmp != IPS_FAILED) {
                    DBUG_PRINT_TAG ("CF_PROXY", "Replacing a proxy sel!");
                    iv_avis
                      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (1, tlen)));
                    INFO_VARDECS (arg_info)
                      = TBmakeVardec (iv_avis, INFO_VARDECS (arg_info));
                    INFO_PREASSIGN (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (iv_avis, NULL),
                                                 TCmakeIntVector (
                                                   TCappendExprs (xtemplate, filter_iv))),
                                      INFO_PREASSIGN (arg_info));
                    AVIS_SSAASSIGN (iv_avis) = INFO_PREASSIGN (arg_info);

                    res = TCmakePrf2 (F_sel_VxA, TBmakeId (iv_avis),
                                      DUPdoDupNode (INFO_PROXYARR (arg_info)));
                } else {
                    if (xtemplate != NULL) {
                        xtemplate = FREEdoFreeTree (xtemplate);
                    }
                    filter_iv = FREEdoFreeTree (filter_iv);
                }
            } else {
                filter_iv = FREEdoFreeTree (filter_iv);
            }

            iter_shp = SHfreeShape (iter_shp);
        }

        fs_iv = COfreeConstant (fs_iv);
        fs_P = COfreeConstant (fs_P);
    } else {
        if (fs_iv != NULL) {
            fs_iv = COfreeConstant (fs_iv);
            if (fs_P != NULL) {
                fs_P = COfreeConstant (fs_P);
            }
        }
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

static node *
IdxselProxyArray (node *arg_node, info *arg_info)
{
    node *aelems_iv = NULL;
    node *aelems_P = NULL;
    node *arr_P = NULL;
    node *tmp;
    node *filter_iv;
    node *xtemplate = NULL;
    constant *fs_P = NULL;
    shape *fs_P_shp;
    shape *iter_shp;
    node *iv_avis;
    int pos;
    size_t tlen, flen;
    pattern *pat;
    node *offset = NULL;

    node *res = NULL;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_idx_sel), 2, PMany (1, PMAgetNode (&offset), 0),
                 PMarray (2, PMAgetFS (&fs_P), PMAgetNode (&arr_P), 1,
                          PMskip (1, PMAgetNode (&aelems_P))));

    if (PMmatchFlatSkipExtrema (pat, arg_node) && (aelems_P != NULL)) {
        xtemplate = IsSingleSourceArray (aelems_P, F_sel_VxA, arg_info);
        if (NULL != xtemplate) {

            /* FIXME need aelems_iv */
            /*
             * First of all, we filter out the prefix of indices that correspond
             * to a 1 extent dimension in P and the corresponding elements
             * from the frameshape of P to get the iteration space over A. This
             * caters for proxies of the form
             *
             * A = ...
             * P = [[sel(p_iv1, A), ..., [sel(p_ivn, A)]];
             * r = sel( iv, P);
             *
             * where the outer indices of iv have no correspondence in the p_ivx.
             * Note, however, that this transformation is still correct if the
             * outer dimension has a correspondence in P_iv, as it will be
             * identical for all selections.
             */
            filter_iv = DUPdoDupTree (aelems_iv);
            fs_P_shp = ARRAY_FRAMESHAPE (arr_P);

            pos = 0;
            while ((SHgetExtent (fs_P_shp, pos) == 1) && (filter_iv != NULL)) {
                filter_iv = FREEdoFreeNode (filter_iv);
                pos++;
            }

            DBUG_ASSERT (filter_iv != NULL, "weird selection encountered....");

            flen = TCcountExprs (filter_iv);
            iter_shp = SHdropFromShape (SHgetDim (fs_P_shp) - flen, fs_P_shp);
            tlen = TCcountExprs (xtemplate);

            /*
             * If by now we still have not managed to reduce the index used
             * for selections into the proxy such that it is shorter or
             * equally long as the selections used to construct the proxy,
             * we have to give up.
             */
            if (tlen >= flen) {
                /*
                 * now the final step:
                 *
                 * check whether all sels are of the form
                 *
                 * sel_VxA( [v1, ..., vn, c1, ..., cn], A)
                 *
                 */
                if (tlen == flen) {
                    xtemplate = NULL; /* no non-index part */
                } else {
                    xtemplate = DUPdoDupTree (xtemplate);
                    tmp = TCgetNthExprs (tlen - flen - 1, xtemplate);
                    EXPRS_NEXT (tmp) = FREEdoFreeTree (EXPRS_NEXT (tmp));
                }

                /*
                 * now we check whether all selections are
                 *
                 * xtemplate ++ some constants
                 */
                tmp = (node *)COcreateAllIndicesAndFold (iter_shp, IsProxySel, aelems_P,
                                                         xtemplate, FALSE);
                /*
                 * if that worked out, we can replace the selection by
                 *
                 * sel ( [v1, ..., vn, i1, ..., in]], A)
                 */
                if (tmp != IPS_FAILED) {
                    DBUG_PRINT_TAG ("CF_PROXY", "Replacing a proxy sel!");
                    iv_avis
                      = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                              SHcreateShape (1, tlen)));
                    INFO_VARDECS (arg_info)
                      = TBmakeVardec (iv_avis, INFO_VARDECS (arg_info));
                    INFO_PREASSIGN (arg_info)
                      = TBmakeAssign (TBmakeLet (TBmakeIds (iv_avis, NULL),
                                                 TCmakeIntVector (
                                                   TCappendExprs (xtemplate, filter_iv))),
                                      INFO_PREASSIGN (arg_info));
                    AVIS_SSAASSIGN (iv_avis) = INFO_PREASSIGN (arg_info);

                    res = TCmakePrf2 (F_sel_VxA, TBmakeId (iv_avis),
                                      DUPdoDupNode (INFO_PROXYARR (arg_info)));
                } else {
                    if (xtemplate != NULL) {
                        xtemplate = FREEdoFreeTree (xtemplate);
                    }
                    filter_iv = FREEdoFreeTree (filter_iv);
                }
            } else {
                filter_iv = FREEdoFreeTree (filter_iv);
            }

            iter_shp = SHfreeShape (iter_shp);
        }

        fs_P = COfreeConstant (fs_P);
    } else {
        if (fs_P != NULL) {
            fs_P = COfreeConstant (fs_P);
        }
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_sel(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements sel(arg1, arg2) for structural constants
 *
 *
 *****************************************************************************/
node *
SCCFprf_sel (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();
    res = SelEmptyScalar (arg_node, arg_info);

    if (NULL == res) {
        res = SelModarray (arg_node, arg_info);
    }

    if (NULL == res) {
        res = SelModarrayCase2 (arg_node, arg_info);
    }

    if (NULL == res) {
        res = StructOpSel (arg_node, arg_info);
    }

    if (NULL == res) {
        res = SelArrayOfEqualElements (arg_node, arg_info);
    }

    if (NULL == res) {
        res = SelProxyArray (arg_node, arg_info);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * IdxselEmptyScalar(node *arg_node, info *arg_info)
 *
 * description:
 *   Detects:                  z = _idx_sel( 0, scalar);
 *     and converts it into:   z = scalar;
 *
 *****************************************************************************/
static node *
IdxselEmptyScalar (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat;
    constant *arg1c = NULL;
    node *iv = NULL;

    DBUG_ENTER ();

    pat = PMconst (1, PMAgetVal (&arg1c));
    iv = IVUTfindOffset2Iv (PRF_ARG1 (arg_node));
    if (NULL != iv) {
        arg1c = IVUTiV2Constant (iv);
    }
    if (NULL == arg1c) {
        PMmatchFlatSkipExtremaAndGuards (pat, PRF_ARG1 (arg_node));
    }
    if ((NULL != arg1c) && (TUisScalar (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node)))))
        && (COisZero (arg1c, TRUE))) {
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
        DBUG_PRINT ("Replaced _idx_sel( 0, Scalar) by %s", AVIS_NAME (ID_AVIS (res)));
    }
    arg1c = (NULL != arg1c) ? COfreeConstant (arg1c) : arg1c;
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *HandleCompositionWithGenarray( node *arg_node...)
 *
 * @brief Handler for compositions of indexing on genarray result:
 *        Currently supported:
 *          Q = genarray([shp], scalar);
 *          el = idx_sel( offset, Q);  OR
 *          el = _sel_VxA_( iv, Q);
 *
 * @param  arg_node: The putative N_prf for el
 *
 * @return If we can prove that all elements of Q are identical,
 *         we can replace the selection operation by el = scalar.
 *
 *         This code assumes that -check c will ensure that
 *         shp satisfies rank, shape, and value requirements
 *         ( index error checking) for the index operation.
 *
 ******************************************************************************/
static node *
HandleCompositionWithGenarray (node *arg_node)
{
    node *z = NULL;
    pattern *pat1;
    pattern *pat2;
    node *iv = NULL;
    node *q = NULL;
    node *s = NULL;

    DBUG_ENTER ();

    /* z = _sel_VxA_( iv, q); */
    pat1 = PMprf (1, PMAisPrf (F_sel_VxA), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&q), 0));
    /* z = _idx_sel( offset, q); */
    pat2 = PMprf (1, PMAisPrf (F_idx_sel), 2, PMvar (1, PMAgetNode (&iv), 0),
                  PMvar (1, PMAgetNode (&q), 0));
    if ((PMmatchFlat (pat1, arg_node) || PMmatchFlat (pat2, arg_node))) {
        // Emit el = s;
        s = WLUTgetGenarrayScalar (q, TRUE);
        if (NULL != s) {
            z = TBmakeId (s);
            DBUG_PRINT ("Replaced F_sel by %s", AVIS_NAME (s));
        }
    }
    pat1 = PMfree (pat1);
    pat2 = PMfree (pat2);

    DBUG_RETURN (z);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_idx_sel(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements idx_sel(arg1, arg2) for structural constants
 *
 *****************************************************************************/
node *
SCCFprf_idx_sel (node *arg_node, info *arg_info)
{
    node *res = NULL;

    DBUG_ENTER ();

    if (NULL == res) {
        res = IdxselEmptyScalar (arg_node, arg_info);
    }

    if (NULL == res) {
        res = IdxselModarray (arg_node, arg_info);
    }

    if (NULL == res) {
        res = IdxselModarrayCase2 (arg_node, arg_info);
    }

    if (NULL == res) {
        res = IdxselStructOpSel (arg_node, arg_info);
    }

    /* SAH claims the next two were needed to make tvd and/or tvd_abstract run
     * faster. rbe 2011-09-07
     */
    if (NULL == res) {
        res = IdxselArrayOfEqualElements (arg_node, arg_info);
    }

    if (NULL == res) {
        res = IdxselProxyArray (arg_node, arg_info);
    }

    if (NULL == res) {
        res = HandleCompositionWithGenarray (arg_node);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_SxSxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *     z = p ? x : y;
 *
 *   We implement this so that it is a primitive, rather than
 *   a CONDFUN. The intent here is to let extrema pass through
 *   the argument, which does not happen for CONDFUNs.
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *   If x and y match, the result is x, regardless of p.
 *
 *   Otherwise, we do nothing.
 *
 *   In all of the mask() CF code, we skip extrema and guards for
 *   analysis only: results are PRF_ARG values.
 *
 *****************************************************************************/
node *
SCCFprf_mask_SxSxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    pattern *pat = NULL;
    constant *p = NULL;
    node *arg2 = NULL;
    node *arg3 = NULL;

    DBUG_ENTER ();

    if (ID_AVIS (PRF_ARG2 (arg_node)) == ID_AVIS (PRF_ARG3 (arg_node))) {
        DBUG_PRINT ("Replacing mask( p, x, x) by x");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else {
        pat = PMprf (1, PMAisPrf (F_mask_SxSxS), 3, PMconst (1, PMAgetVal (&p)),
                     PMany (1, PMAgetNode (&arg2), 0), PMany (1, PMAgetNode (&arg3), 0));

        if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
            /* p is constant */
            if (COisTrue (p, TRUE)) {
                DBUG_PRINT ("Replacing mask_SxSxS result by PRF_ARG2=%s",
                            AVIS_NAME (ID_AVIS (PRF_ARG2 (arg_node))));
                res = DUPdoDupNode (PRF_ARG2 (arg_node));
            } else {
                DBUG_PRINT ("Replacing mask_SxSxS result by PRF_ARG3%s",
                            AVIS_NAME (ID_AVIS (PRF_ARG3 (arg_node))));
                res = DUPdoDupNode (PRF_ARG3 (arg_node));
            }
            p = COfreeConstant (p);
        }
        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_SxSxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z[i] = x if p;  else y[i];
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *   We also do a quick check to see if p is all TRUE or all FALSE.
 *   If so, we don't care if y is an N_array node or not.
 *
 *   Otherwise, we do nothing.
 *
 *****************************************************************************/
node *
SCCFprf_mask_SxSxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *p = NULL;
    node *x;
    node *y;
    constant *xfs = NULL;
    pattern *pat;
    node *curel;
    bool b;
    node *z = NULL;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_mask_SxSxV), 3, PMconst (1, PMAgetVal (&p)),
                 PMany (1, PMAgetNode (&x), 0),
                 PMarray (2, PMAgetNode (&y), PMAgetFS (&xfs), 1, PMskip (0)));

    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        /* p is constant, y is an N_array node */
        b = COisTrue (p, TRUE);
        p = COfreeConstant (p);
        res = DUPdoDupTree (y); /* Handy starter for result */
        FREEdoFreeTree (ARRAY_AELEMS (res));
        DBUG_PRINT ("Replacing mask(p, x, y) by %s", b ? "genarray of x" : "y");
        y = ARRAY_AELEMS (y);
        while (y != NULL) {
            curel = b ? x : EXPRS_EXPR (y);
            z = TCappendExprs (z, TBmakeExprs (DUPdoDupNode (curel), NULL));
            y = EXPRS_NEXT (y);
        }
        ARRAY_AELEMS (res) = z;
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_SxVxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z[i] = x[i] if p;  else y;
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *
 *   Otherwise, we do nothing.
 *
 *****************************************************************************/
node *
SCCFprf_mask_SxVxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *p = NULL;
    node *x;
    node *y;
    constant *xfs = NULL;
    pattern *pat;
    node *curel;
    bool b;
    node *z = NULL;

    DBUG_ENTER ();

    pat = PMprf (1, PMAisPrf (F_mask_SxVxS), 3, PMconst (1, PMAgetVal (&p)),
                 PMarray (2, PMAgetNode (&x), PMAgetFS (&xfs), 1, PMskip (0)),
                 PMany (1, PMAgetNode (&y), 0));

    if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
        b = COisTrue (p, TRUE);
        p = COfreeConstant (p);
        DBUG_PRINT ("Replacing mask(p, x, y) by %s", b ? "x" : "genarray of y");
        /* p is constant, x is N_array node */
        res = DUPdoDupTree (x); /* Handy starter for result */
        FREEdoFreeTree (ARRAY_AELEMS (res));
        x = ARRAY_AELEMS (x);
        while (x != NULL) {
            curel = b ? EXPRS_EXPR (x) : y;
            z = TCappendExprs (z, TBmakeExprs (DUPdoDupNode (curel), NULL));
            x = EXPRS_NEXT (x);
        }
        ARRAY_AELEMS (res) = z;
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_SxVxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by AWLFI, and stdlib
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z] = x if p;  else y;
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *
 *   Finally, if x and y match, the result is x, regardless of p.
 *
 *   Otherwise, we do nothing.
 *
 *****************************************************************************/
node *
SCCFprf_mask_SxVxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *p = NULL;
    node *arg2 = NULL;
    node *arg3 = NULL;
    pattern *pat;

    DBUG_ENTER ();

    if (ID_AVIS (PRF_ARG2 (arg_node)) == ID_AVIS (PRF_ARG3 (arg_node))) {
        DBUG_PRINT ("Replacing mask( p, x, x) by x");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else {
        pat = PMprf (1, PMAisPrf (F_mask_SxVxV), 3, PMconst (1, PMAgetVal (&p)),
                     PMany (1, PMAgetNode (&arg2), 0), PMany (1, PMAgetNode (&arg3), 0));

        if (PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) {
            DBUG_PRINT ("Replacing mask result by mask of arg2, arg3");
            /* p is constant. */
            if (COisTrue (p, TRUE)) {
                res = DUPdoDupNode (PRF_ARG2 (arg_node));
            } else {
                res = DUPdoDupNode (PRF_ARG3 (arg_node));
            }
            p = COfreeConstant (p);
        }
        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_VxSxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z[i] = x[i] if p;  else y;
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *   We also do a quick check to see if p is all TRUE or all FALSE.
 *
 *   Finally, if x and y match, the result is x, regardless of p.
 *
 *   Otherwise, we do nothing.
 *
 *****************************************************************************/
node *
SCCFprf_mask_VxSxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

#ifdef FIXME // Need to fix res initialization code
    node *p = NULL;
    node *x;
    node *y;
    constant *xfs = NULL;
    pattern *pat;
    constant *c;
    node *curel;
    bool b;
    node *z = NULL;

    DBUG_ENTER ();

    if (ID_AVIS (PRF_ARG2 (arg_node)) == ID_AVIS (PRF_ARG3 (arg_node))) {
        DBUG_PRINT ("Replacing mask( p, x, x) by x");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else {
        pat = PMprf (1, PMAisPrf (F_mask_VxSxS), 3,
                     PMarray (2, PMAgetNode (&p), PMAgetFS (&xfs), 1, PMskip (0)),
                     PMany (1, PMAgetNode (&x), 0), PMany (1, PMAgetNode (&y), 0));

        if ((PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) && (COisConstant (p))) {
            DBUG_PRINT ("Replacing mask result by mask of x,y");
            /* p is constant. We don't care about x and y */
            ughres = DUPdoDupTree (x); /* Handy starter for result */
            FREEdoFreeTree (ARRAY_AELEMS (res));
            p = ARRAY_AELEMS (p);
            while (p != NULL) {
                c = COaST2Constant (EXPRS_EXPR (p));
                b = COisTrue (c, TRUE);
                c = COfreeConstant (c);
                curel = b ? x : y;
                z = TCappendExprs (z, TBmakeExprs (DUPdoDupNode (curel), NULL));
                p = EXPRS_NEXT (p);
            }
            ARRAY_AELEMS (res) = z;
        }
        pat = PMfree (pat);
    }
#else  // FIXME // Need to fix res initialization code
#endif // FIXME // Need to fix res initialization code

    DBUG_ENTER ();

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_VxSxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z[i] = x if p[i];  else y;
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *   We also do a quick check to see if p is all TRUE or all FALSE.
 *   If so, we don't care if x and y are N_array nodes or not.
 *
 *   Otherwise, we do nothing.
 *
 *
 *****************************************************************************/
node *
SCCFprf_mask_VxSxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *p = NULL;
    node *x = NULL;
    node *y = NULL;
    constant *xfs = NULL;
    pattern *pat;
    constant *c;
    node *curel;
    bool b;
    node *z = NULL;

    DBUG_ENTER ();

    pat
      = PMprf (1, PMAisPrf (F_mask_VxSxV), 3,
               PMarray (2, PMAgetNode (&p), PMAgetFS (&xfs), 1, PMskip (0)),
               PMany (1, PMAgetNode (&x), 0),
               PMarray (2, PMAgetNode (&y), PMAhasFS (&xfs), 1, PMskip (0)), PMskip (0));

    if ((PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) && (COisConstant (p))) {
        DBUG_PRINT ("Replacing mask result by mask of x,y");
        /* p is constant, y is N_array node */
        res = DUPdoDupTree (y); /* Handy starter for result */
        FREEdoFreeTree (ARRAY_AELEMS (res));
        p = ARRAY_AELEMS (p);
        y = ARRAY_AELEMS (y);
        while (p != NULL) {
            c = COaST2Constant (EXPRS_EXPR (p));
            b = COisTrue (c, TRUE);
            c = COfreeConstant (c);
            curel = b ? PRF_ARG2 (arg_node) : EXPRS_EXPR (y);
            z = TCappendExprs (z, TBmakeExprs (DUPdoDupNode (curel), NULL));
            p = EXPRS_NEXT (p);
            y = EXPRS_NEXT (y);
        }
        ARRAY_AELEMS (res) = z;
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_VxVxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z[i] = x[i] if p[i];  else y;
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *   We also do a quick check to see if p is all TRUE or all FALSE.
 *   If so, we don't care if x and y are N_array nodes or not.
 *
 *   Otherwise, we do nothing.
 *
 *
 *****************************************************************************/
node *
SCCFprf_mask_VxVxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *p = NULL;
    node *x = NULL;
    constant *xfs = NULL;
    pattern *pat;
    constant *c;
    node *curel;
    bool b;
    node *z = NULL;

    DBUG_ENTER ();

    pat
      = PMprf (1, PMAisPrf (F_mask_VxVxS), 3,
               PMarray (2, PMAgetNode (&p), PMAgetFS (&xfs), 1, PMskip (0)),
               PMarray (2, PMAgetNode (&x), PMAhasFS (&xfs), 1, PMskip (0)), PMskip (0));

    if ((PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) && (COisConstant (p))) {
        DBUG_PRINT ("Replacing mask result by mask of x,y");
        /* p is constant, x is N_array node */
        res = DUPdoDupTree (x); /* Handy starter for result */
        FREEdoFreeTree (ARRAY_AELEMS (res));
        p = ARRAY_AELEMS (p);
        x = ARRAY_AELEMS (x);
        while (p != NULL) {
            c = COaST2Constant (EXPRS_EXPR (p));
            b = COisTrue (c, TRUE);
            c = COfreeConstant (c);
            curel = b ? EXPRS_EXPR (x) : PRF_ARG3 (arg_node);
            z = TCappendExprs (z, TBmakeExprs (DUPdoDupNode (curel), NULL));
            p = EXPRS_NEXT (p);
            x = EXPRS_NEXT (x);
        }
        ARRAY_AELEMS (res) = z;
    }
    pat = PMfree (pat);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_mask_VxVxV(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements mask function, used by ALWFI.
 *   The semantics of z = mask( p, x, y) are, essentially, the same as
 *   the ill-named <where> function of the SAC stdlib:
 *
 *    z[i] = x[i] if p[i];  else y[i];
 *
 * result:
 *   If p is constant, then we perform the above substutition.
 *   We also do a quick check to see if p is all TRUE or all FALSE.
 *   If so, we don't care if x and y are N_array nodes or not.
 *
 *   Finally, if x and y match, the result is x, regardless of p.
 *
 *   Otherwise, we do nothing.
 *
 *****************************************************************************/
node *
SCCFprf_mask_VxVxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *p = NULL;
    node *x;
    node *y;
    constant *xfs = NULL;
    pattern *pat;
    constant *c;
    node *curel;
    bool b;
    node *z = NULL;

    DBUG_ENTER ();

    if (ID_AVIS (PRF_ARG2 (arg_node)) == ID_AVIS (PRF_ARG3 (arg_node))) {
        DBUG_PRINT ("Replacing mask( p, x, x) by x");
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    } else {
        pat = PMprf (1, PMAisPrf (F_mask_VxVxV), 3,
                     PMarray (2, PMAgetNode (&p), PMAgetFS (&xfs), 1, PMskip (0)),
                     PMarray (2, PMAgetNode (&x), PMAhasFS (&xfs), 1, PMskip (0)),
                     PMarray (2, PMAgetNode (&y), PMAhasFS (&xfs), 1, PMskip (0)));

        if ((PMmatchFlatSkipExtremaAndGuards (pat, arg_node)) && (COisConstant (p))) {
            DBUG_PRINT ("Replacing mask result by mask of x,y");
            /* p is constant, x and y are N_array nodes */
            res = DUPdoDupTree (x); /* Handy starter for result */
            FREEdoFreeTree (ARRAY_AELEMS (res));
            p = ARRAY_AELEMS (p);
            x = ARRAY_AELEMS (x);
            y = ARRAY_AELEMS (y);
            while (p != NULL) {
                c = COaST2Constant (EXPRS_EXPR (p));
                b = COisTrue (c, TRUE);
                c = COfreeConstant (c);
                curel = b ? EXPRS_EXPR (x) : EXPRS_EXPR (y);
                z = TCappendExprs (z, TBmakeExprs (DUPdoDupNode (curel), NULL));
                p = EXPRS_NEXT (p);
                x = EXPRS_NEXT (x);
                y = EXPRS_NEXT (y);
            }
            ARRAY_AELEMS (res) = z;
        }
        pat = PMfree (pat);
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_idxs2offset(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements _idxs2offset( shp)
 *   Case 1: This is the degenerate case, when shp is an empty vector.
 *   Case 2: Indexing a vector
 *
 *   FIXME : temp kludge pending TC/CO fixup!
 *
 *****************************************************************************/
node *
SCCFprf_idxs2offset (node *arg_node, info *arg_info)
{
    node *res = NULL;
    size_t n;

    DBUG_ENTER ();

    n = TCcountExprs (PRF_ARGS (arg_node));

    if ((1 == n) && (SCSisConstantZero (PRF_ARG1 (arg_node)))) {
        res = TBmakeNum (0); /* Case 1 */
    }

    if (2 == n) { /* Case 2 */
        res = DUPdoDupNode (PRF_ARG2 (arg_node));
    }

    DBUG_RETURN (res);
}

#undef DBUG_PREFIX
