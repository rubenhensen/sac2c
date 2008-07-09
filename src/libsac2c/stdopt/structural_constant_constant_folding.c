/*
 * $Id$
 */

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

#include "dbug.h"
#include "tree_basic.h"
#include "node_basic.h"
#include "tree_compound.h"
#include "str.h"
#include "memory.h"
#include "new_types.h"
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
#include "pattern_match.h"

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
 *
 *   CF performs the select, producing:
 *
 *      z = c;
 *
 * Case 2: (shape(iv) > frame_dim(X)):
 *
 *      z = sel([2,1], X);
 *
 *    CF performs a partial selection:
 *
 *      tmpX   = c;
 *      tmpiv = [1];
 *      z = sel(tmpiv, tmpX);
 *
 * Case 3: (shape(iv) < frame_dim(X):
 *
 *    Illegal.
 *
 *****************************************************************************/

static node *
StructOpSel (node *arg_node, info *arg_info)
{
    node *result = NULL;

    int iv_len;
    int X_dim;

    constant *take_vec;
    constant *con1 = NULL;
    constant *con2 = NULL;
    constant *arg2fs = NULL;
    int offset;
    node *tmpivid;
    node *tmpivval;
    node *tmpivavis;
    node *tmpXid;
    node *arg1 = NULL;
    node *arg2 = NULL;

    DBUG_ENTER ("StructOpSel");
    // Match for _sel_VxA_( constant, N_array)
    if (PM (
          PMarrayConstructor (&arg2fs, &arg2,
                              PMintConst (&con1, &arg1, PMprf (F_sel_VxA, arg_node))))) {
        X_dim = SHgetExtent (COgetShape (arg2fs), 0);
        arg2fs = COfreeConstant (arg2fs);
        iv_len = SHgetUnrLen (COgetShape (con1));
        DBUG_ASSERT ((iv_len >= X_dim), ("shape(iv) <  dim(X)"));
        take_vec = COmakeConstantFromInt (X_dim);

        // Select the N_array element we want w/iv prefix
        con2 = COtake (take_vec, con1);
        offset = Idx2OffsetArray (con2, arg2);
        con2 = COfreeConstant (con2);
        tmpXid = DUPdoDupTree ((node *)TCgetNthExprsExpr (offset, ARRAY_AELEMS (arg2)));
        if (iv_len == X_dim) {
            if (N_id == NODE_TYPE (tmpXid)) {
                // FIXME: tvd2d gets wrong answers if we allow the return of an N_num
                //        or an N_double (not sure if it's either or both...)
                //        2008-06-27
                // Case 1 : Exact selection: do the sel operation now.
                DBUG_PRINT ("CF", ("StructOpSel exact selection."));
                result = tmpXid;
            }
        } else {
            // Case 2: Selection vector has more elements than frame_dim(X):
            // Perform partial selection on X now; build new selection for
            // run-time.
            DBUG_ASSERT (N_id == NODE_TYPE (tmpXid), "StructOpSel X element not N_id");
            con1 = COdrop (take_vec, con1); // iv suffix
            take_vec = COfreeConstant (take_vec);
            tmpivavis
              = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node)))),
                            TYmakeAKS (TYmakeSimpleType (T_int),
                                       SHcreateShape (1, iv_len - X_dim)));
            AVIS_DIM (tmpivavis) = TBmakeNum (1);
            // Following is really GenIntVector call
            AVIS_SHAPE (tmpivavis)
              = TCmakeIntVector (TBmakeExprs (TBmakeNum (iv_len - X_dim), NULL));
            tmpivval = COconstant2AST (con1);
            INFO_VARDECS (arg_info) = TBmakeVardec (tmpivavis, INFO_VARDECS (arg_info));
            tmpivid = TBmakeId (tmpivavis);
            INFO_PREASSIGN (arg_info)
              = TBmakeAssign (TBmakeLet (TBmakeIds (tmpivavis, NULL), tmpivval),
                              INFO_PREASSIGN (arg_info));

            AVIS_SSAASSIGN (tmpivavis) = INFO_PREASSIGN (arg_info);
            DBUG_PRINT ("CF", ("StructOpSel sel(iv,X) created new iv: old: %s; new: %s",
                               AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node))),
                               AVIS_NAME (tmpivavis)));

            // Create new sel() operation  _sel_VxA_(tmpiv, tmpX);
            result = TCmakePrf2 (F_sel_VxA, tmpivid, tmpXid);
        }
        con1 = COfreeConstant (con1);
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
 *      reshape( shp, arr)

 *   when it can determine that shp is a constant,
 *   and that ( times reduce shp) == times reduce shape(arr)
 *
 *****************************************************************************/
node *
SCCFprf_reshape (node *arg_node, info *arg_info)
{
    node *res = NULL;
    shape *resshape;
    constant *con1 = NULL;
    constant *fs2 = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    int timesrhoarg2;
    int prodarg1;

    DBUG_ENTER ("SCCFprf_reshape");
    /*  reshape(shp, arr):
     *    constant shp, non-constant arr, with
     *    arr having same element count prod(shp)
     *    and rank-0 ELEMTYPE
     */
    if (PM (PMarrayConstructor (&fs2, &arg2,
                                PMintConst (&con1, &arg1,
                                            PMprf (F_reshape_VxA, arg_node))))) {
        if (0 == TYgetDim (ARRAY_ELEMTYPE (arg2))) {
            resshape = COconstant2Shape (con1);
            prodarg1 = SHgetUnrLen (resshape);
            timesrhoarg2 = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2));
            if (prodarg1 == timesrhoarg2) {
                /* If the result is a scalar, return that. Otherwise,
                 * create an N_array.
                 */
                if (0 == SHgetDim (resshape)) {
                    res = DUPdoDupTree (TCgetNthExprsExpr (0, ARRAY_AELEMS (arg2)));
                } else {
                    res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2)), resshape,
                                       DUPdoDupTree (ARRAY_AELEMS (arg2)));
                }
                DBUG_PRINT ("CF", ("SCCFprf_reshape performed "));
            }
            con1 = COfreeConstant (con1);
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_take_SxV(node *arg_node, info *arg_info)
 *
 * description:
 *   computes structural undertake on array expressions with constant arg1.
 *   Implements take for constants
 *   If both arguments are constant, CF was done by the typechecker.
 *   This handles the case where arg1 is constant, and arg2 is
 *   an N_array.
 *
 *****************************************************************************/
node *
SCCFprf_take_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *tail;
    node *arg1 = NULL;
    node *arg2 = NULL;
    constant *con1 = NULL;
    constant *fs2 = NULL;
    int takecount;
    int resshape;
    int argshape;
    int offset;

    DBUG_ENTER ("SCCFprf_take_SxV");
    if (PM (
          PMarrayConstructor (&fs2, &arg2,
                              PMintConst (&con1, &arg1, PMprf (F_take_SxV, arg_node))))) {
        takecount = COconst2Int (con1);
        resshape = abs (takecount);
        argshape = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2));
        DBUG_ASSERT ((resshape <= argshape), ("SCCFprf_take_SxV attempted overtake"));
        if (argshape == takecount) {
            res = DUPdoDupTree (arg2);
        } else {
            offset = (takecount >= 0) ? 0 : argshape - takecount;

            tail = TCgetExprsSection (offset, resshape, ARRAY_AELEMS (arg2));
            DBUG_PRINT ("CF", ("SCCFprf_take performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2)),
                               SHcreateShape (1, resshape), tail);
        }

        if (NULL != con1) {
            con1 = COfreeConstant (con1);
        }
    }
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
 *   This handles the case where arg1 is constant, and arg2 is
 *   an N_array.
 *
 *****************************************************************************/
node *
SCCFprf_drop_SxV (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *tail;
    node *arg1 = NULL;
    node *arg2 = NULL;
    constant *con1 = NULL;
    constant *arg2fs = NULL;
    int dropcount;
    int resxrho;
    int offset;

    DBUG_ENTER ("SCCFprf_drop_SxV");
    if (PM (
          PMarrayConstructor (&arg2fs, &arg2,
                              PMintConst (&con1, &arg1, PMprf (F_drop_SxV, arg_node))))) {
        dropcount = COconst2Int (con1);
        if (0 == dropcount) {
            res = DUPdoDupTree (arg2);
        } else {
            resxrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2)) - dropcount;
            resxrho = (resxrho < 0) ? 0 : resxrho;
            offset = (dropcount < 0) ? 0 : dropcount;
            tail = TCgetExprsSection (offset, resxrho, ARRAY_AELEMS (arg2));
            DBUG_PRINT ("CF", ("SCCFprf_drop performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2)),
                               SHcreateShape (1, resxrho), tail);
        }
        if (NULL != con1) {
            con1 = COfreeConstant (con1);
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * ARmodarray( node *arg_node, constant *iv)
 *
 * description:
 *   Implements modarray for structural constant:  X[iv] = val;
 *   X must be an N_array.
 *   iv must be a constant integer vector.
 *   val must be a scalar.
 *
 * @param: _modarray_AxVxS( X, iv, val)
 * @result:
 *****************************************************************************/
static node *
ARmodarray (node *arg1, constant *iv, node *val)
{
    node *res = NULL;
    node *oldval;
    node *newval;
    int offset;

    DBUG_ENTER ("ARmodarray");
    offset = Idx2OffsetArray (iv, arg1);
    /* If -ecc is active, we should not be able to get
     * index error here, so we don't check for it.
     */
    res = DUPdoDupTree (arg1);
    newval = TCgetNthExprs (offset, ARRAY_AELEMS (res));
    oldval = FREEdoFreeTree (EXPRS_EXPR (newval));
    EXPRS_EXPR (newval) = DUPdoDupTree (val);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_modarray_AxVxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxS_(X, iv, scalarval)
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    constant *fs1 = NULL;
    constant *fs2 = NULL;

    DBUG_ENTER ("SCCFprf_modarray_AxVxS");
    /**
     * if iv is an empty vector, we simply replace the entire
     * expression by val!
     * Well, not quite!!! This is only valid, iff
     *      shape( val) ==  shape(X)
     * If we do not know this, then the only thing we can do is
     * to replace the modarray by
     *      _type_conv_( type(X), val))
     * iff a is AKS! * cf bug246 !!!
     *
     * 2008-05-09: We can ignore all the above, because the
     * inclusion of guards via "sac2c -ecc" ensures this can never happen.
     * It ensures that iv is valid, and that val is scalar.
     */
    if (PM (PMintConst (&fs2, &arg2, PMvar (&arg1, PMprf (F_modarray_AxVxS, arg_node))))
        && (COisEmptyVect (fs2))) {               /* z = modarray(X, [], 42) */
        res = DUPdoDupTree (PRF_ARG3 (arg_node)); /* val is scalar */
        fs2 = COfreeConstant (fs2);

        /* Attempt to find non-empty iv N_array for arg2const. */
    } else {
        fs1 = NULL;
        fs2 = NULL;
        arg1 = NULL;
        arg2 = NULL;
        if (PM (PMintConst (&fs2, &arg2,
                            PMarrayConstructor (&fs1, &arg1,
                                                PMprf (F_modarray_AxVxS, arg_node))))) {
            res = ARmodarray (arg1, fs2, PRF_ARG3 (arg_node));
            fs1 = COfreeConstant (fs1);
            fs2 = COfreeConstant (fs2);
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_modarray_AxVxA(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxA_(X, iv, array)
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1 = NULL;
    node *arg2 = NULL;
    constant *fs1 = NULL;
    constant *fs2 = NULL;

    DBUG_ENTER ("SCCFprf_modarray_AxVxA");
    if (PM (PMintConst (&fs2, &arg2,
                        PMarrayConstructor (&fs1, &arg1,
                                            PMprf (F_modarray_AxVxA, arg_node))))) {
        res = ARmodarray (arg1, fs2, PRF_ARG3 (arg_node));
        fs1 = COfreeConstant (fs1);
        fs2 = COfreeConstant (fs2);
    } else {
        fs1 = NULL;
        fs2 = NULL;
        arg1 = NULL;
        arg2 = NULL;
        if (PM (PMintConst (&fs2, &arg2,
                            PMarrayConstructor (&fs1, &arg1,
                                                PMprf (F_modarray_AxVxA, arg_node))))) {
            res = ARmodarray (arg1, fs2, PRF_ARG3 (arg_node));
            fs1 = COfreeConstant (fs1);
            fs2 = COfreeConstant (fs2);
        }
    }
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

    DBUG_ENTER ("SCCFprf_cat_VxV");
    DBUG_ASSERT ((N_id == NODE_TYPE (PRF_ARG1 (arg_node))),
                 ("SCCFprf_cat_VxV arg1 not N_id"));
    DBUG_ASSERT ((N_id == NODE_TYPE (PRF_ARG2 (arg_node))),
                 ("SCCFprf_cat_VxV arg2 not N_id"));

    /* This catches the empty-vector case when one argument is not an N_array */
    if (TUisEmptyVect (AVIS_TYPE (ID_AVIS (PRF_ARG1 (arg_node))))) {
        DBUG_PRINT ("CF", ("SCCFprf_cat (1) removed []++var"));
        res = DUPdoDupTree (PRF_ARG2 (arg_node));
    }
    if (TUisEmptyVect (AVIS_TYPE (ID_AVIS (PRF_ARG2 (arg_node))))) {
        DBUG_PRINT ("CF", ("SCCFprf_cat (1) removed var++[]"));
        res = DUPdoDupTree (PRF_ARG1 (arg_node));
    }

    if ((NULL == res)
        && PM (PMarrayConstructor (&fs2, &arg2,
                                   PMarrayConstructor (&fs1, &arg1,
                                                       PMprf (F_cat_VxV, arg_node))))) {

        DBUG_ASSERT ((1 == SHgetDim (ARRAY_FRAMESHAPE (arg1))),
                     "SCCFprf_cat expected vector arg1 frameshape");
        DBUG_ASSERT ((1 == SHgetDim (ARRAY_FRAMESHAPE (arg2))),
                     "SCCFprf_cat expected vector arg2 frameshape");

        DBUG_ASSERT (TUeqShapes (ARRAY_ELEMTYPE (arg1), ARRAY_ELEMTYPE (arg2)),
                     ("SCCFprf_cat args have different element types"));

        /* Both arguments are constants or structure constants */
        arg1xrho = COconst2Int (fs1);
        arg2xrho = COconst2Int (fs2);
        frameshape = COadd (fs1, fs2);
        frameshaperes = COconstant2Shape (frameshape);
        frameshape = COfreeConstant (frameshape);
        fs1 = COfreeConstant (fs1);
        fs2 = COfreeConstant (fs2);

        /* Perform the actual element catenation */
        if (0 == arg1xrho) { /* []++arg2 */
            DBUG_PRINT ("CF", ("SCCFprf_cat (2)removed []++var"));
            res = DUPdoDupTree (arg2);
        } else if (0 == arg2xrho) { /* arg2++[] */
            DBUG_PRINT ("CF", ("SCCFprf_cat (2) removed var++[]"));
            res = DUPdoDupTree (arg1);
        } else { /* arg1++arg2 */
            arg1aelems = DUPdoDupTree (ARRAY_AELEMS (arg1));
            arg2aelems = DUPdoDupTree (ARRAY_AELEMS (arg2));
            els = TCappendExprs (arg1aelems, arg2aelems);
#define CRUD
#ifdef CRUD // FIXME
            /* FIXME: KLUDGE to avoid catenating N_num and N_id */
            if (((N_id == NODE_TYPE (EXPRS_EXPR (arg1aelems)))
                 & (N_id == NODE_TYPE (EXPRS_EXPR (arg2aelems))))
                || ((N_id != NODE_TYPE (EXPRS_EXPR (arg1aelems)))
                    & (N_id != NODE_TYPE (EXPRS_EXPR (arg2aelems))))) {
                DBUG_PRINT ("CF", ("SCCFprf_cat performed const1++const2"));
                res
                  = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg1)), frameshaperes, els);
            }
#else  // CRUD
            DBUG_PRINT ("CF", ("SCCFprf_cat performed const1++const2"));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg1)), frameshaperes, els);
#endif // CRUD // FIXME sudoku bug
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
 * @result: if the selection can be folded, the result is the
 * val from in the earlier modarray. Otherwise, NULL.
 *
 * @brief:
 *   tries a sel-modarray optimization for the following cases:
 *
 *  Case 1. iv is an unknown expression:
 *      b = modarray(arr, iv, val)
 *      c = b;              NB. Perhaps a few assigns in the way
 *      x = sel(iv, c)    ->   x = val;
 *
 *      I do not know if other facilities (e.g., WLF) perform this
 *      optimization. If they do not, this opt is important for loop fusion
 *      and array contraction. Consider the example of vector-vector
 *      operations in:  Z = B + C * D;
 *
 *        for (i=0; i<shape(C); i++) {
 *          t1     = C[i] * D[i];
 *          tmp[i] = t1;
 *        }
 *        for (i=0; i<shape(B); i++) {
 *          t2     = B[i] + tmp[i];
 *          Z[i]   = t2;
 *        }
 *
 *      Loop fusion will (assuming B and C have same shape)  turn this into:
 *        for (i=0; i<shape(C); i++) {
 *          t1     = C[i] * D[i];
 *          tmp[i] = t1;
 *          tmp2   = tmp;              NB. Just to complicate things a bit:
 *                                     NB. Hence, the N_array search in PM.
 *          t2     = B[i] + tmp2[i];
 *          Z[i]   = t2;
 *        }
 *
 *      Next, this case of SelModArray will turn the code into:
 *
 *        for (i=0; i<shape(C); i++) {
 *          t1     = C[i] * D[i];
 *          tmp[i] = t1;
 *          t2     = B[i] + t1;
 *          Z[i]   = t2;
 *        }
 *      After this point, tmp is likely dead, so DCR will remove it
 *      sometime later.
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
 *
 *****************************************************************************/

static node *
SelModarray (node *arg_node)
{
    node *res = NULL;
    node *iv = NULL;
    node *X = NULL;
    node *M = NULL;
    node *val = NULL;

    DBUG_ENTER ("SelModarray");
    /* We are looking for one of:
     *
     *    X = modarray_AxVxS_( M, iv, val);
     *     or
     *    X = modarray_AxVxA_( M, iv, val);
     *    sel_VxA( iv, X);
     *     or
     *    X = idx_modarray_AxVxS_( M, iv, val);
     *    idx_sel_VxA( iv, X);
     *
     * NB: these ||'s do only work here as the execution order of these
     *    is guaranteed to be left to right AND as PMvar( &val, ...)
     *    is guaranteed not to be executed if PMprf does not match!!
     */

    if (PM (PMvar (&X, /* _sel_VxA_( iv, X) */
                   PMvar (&iv, PMprf (F_sel_VxA, arg_node))))
        &&

        (PM (PMvar (&val, /* _modarray_AxVxS_( M, iv, val) */
                    PMvar (&iv, PMvar (&M, PMprf (F_modarray_AxVxS, X)))))
         || PM (PMvar (&val, /* _modarray_AxSxA_( M, iv, val) */
                       PMvar (&iv, PMvar (&M, PMprf (F_modarray_AxVxA, X))))))) {
        res = DUPdoDupTree (val);
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * @function:
 *   node *SelModarrayCase2(node *arg_node)
 *
 * @param: arg_node is a _sel_ N_prf.
 *
 * @result: if the selection can be folded, the result is the
 * val from in the earlier modarray. Otherwise, NULL.
 *
 * @brief:
 *  Case 2. ivc is a constant:
 *      b = modarray(arr, ivc, val5);
 *      c = modarray(b, [3], val3);
 *      d = modarray(c, [2], val2);
 *      x = sel(ivc, d)   ->  x = val5;
 *
 *      In the above, if the statements setting c or d contain
 *      non-constants as ivc, then we must NOT perform the
 *      optimization, because we are unable to assert that
 *      the constant is not [5].
 *
 *****************************************************************************/
static node *
SelModarrayCase2 (node *arg_node)
{
    node *res = NULL;
    constant *iv1c = NULL;
    constant *iv2c = NULL;
    node *iv1 = NULL;
    node *iv2 = NULL;
    node *val = NULL;
    node *X2 = NULL;
    node *X = NULL;

    DBUG_ENTER ("SelModarrayCase2");

    if (PM (PMvar (&X, /* _sel_VxA_( iv1, X) */
                   PMintConst (&iv1c, &iv1, PMprf (F_sel_VxA, arg_node))))) {

        while (PM (
          PMvar (&val, /* X = _modarray_AxVxS_( X2, iv2, val)  */
                 PMintConst (&iv2c, &iv2, PMvar (&X2, PMprf (F_modarray_AxVxS, X)))))) {
            /* FIXME: Bodo: Does this need an F_modarray_AxVxA case ?? */
            if (COcompareConstants (iv1c, iv2c)) {
                break;
            } else { /* Chase the modarray chain */
                val = NULL;
                iv2c = NULL;
                iv2 = NULL;
                X = X2;
                X2 = NULL;
            }
        }
        if (NULL != val) {
            res = DUPdoDupTree (val);
        }
    }
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

    DBUG_ENTER ("SelEmptyScalar");

    arg1 = PRF_ARG1 (arg_node);
    arg2 = PRF_ARG2 (arg_node);
    DBUG_ASSERT ((N_id == NODE_TYPE (arg1)), ("arg1 not N_id"));
    DBUG_ASSERT ((N_id == NODE_TYPE (arg2)), ("arg2 not N_id"));

    if (TUisScalar (AVIS_TYPE (ID_AVIS (arg2)))
        && TUisEmptyVect (AVIS_TYPE (ID_AVIS (arg1)))) {
        DBUG_PRINT ("CF", ("SelEmptyScalar removed sel([], scalar)"));
        res = DUPdoDupTree (arg2);
    }

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

    DBUG_ENTER ("SCCFprf_sel");
    res = SelEmptyScalar (arg_node, arg_info);

    if (NULL == res) {
        res = SelModarray (arg_node);
    }

    if (NULL == res) {
        res = SelModarrayCase2 (arg_node);
    }

    if (NULL == res) {
        res = StructOpSel (arg_node, arg_info);
    }
    DBUG_RETURN (res);
}
