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
 *    node *CFsel( constant *idx, node *a)
 *
 * description:
 *    selects a single element from N_array a.
 *
 ******************************************************************************/
static node *
CFsel (constant *idx, node *a)
{
    node *result;
    int offset;

    DBUG_ENTER ("CFsel");
    DBUG_ASSERT ((N_array == NODE_TYPE (a)), "CFsel arg2 not N_array");

    offset = Idx2OffsetArray (idx, a);
    result = TCgetNthExprsExpr (offset, ARRAY_AELEMS (a));
    DBUG_RETURN (result);
}

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
    constant *iv_co;

    node *tmpivid;
    node *tmpivval;
    node *tmpivavis;
    node *tmpXid;

    node *arg1 = NULL;
    node *arg2 = NULL;

    DBUG_ENTER ("StructOpSel");
    // Match for _sel_VxA_( constant, N_array)
    if (PM (PMarray (&arg2, PMconst (&arg1, PMprf (F_sel_VxA, arg_node))))) {
        iv_co = COaST2Constant (arg1);
        iv_len = SHgetUnrLen (COgetShape (iv_co));
        X_dim = SHgetDim (ARRAY_FRAMESHAPE (arg2));
        DBUG_ASSERT ((iv_len >= X_dim), ("shape(iv) <  dim(X)"));
        take_vec = COmakeConstantFromInt (X_dim);

        // Select the N_array element we want w/iv prefix
        tmpXid = DUPdoDupTree ((node *)CFsel (COtake (take_vec, iv_co), arg2));

        if (iv_len == X_dim) {
            // Case 1 : Exact selection: do the sel operation now.
            result = tmpXid;
        } else {
            // Case 2: Selection vector has more elements than frame_dim(X):
            // Perform partial selection on X now; build new selection for
            // run-time.

            DBUG_ASSERT (N_id == NODE_TYPE (tmpXid), "StructOpSel X element not N_id");
            iv_co = COdrop (take_vec, iv_co); // iv suffix
            take_vec = COfreeConstant (take_vec);
            tmpivavis
              = TBmakeAvis (TRAVtmpVarName (AVIS_NAME (ID_AVIS (PRF_ARG1 (arg_node)))),
                            TYmakeAKS (TYmakeSimpleType (T_int),
                                       SHcreateShape (1, iv_len - X_dim)));
            AVIS_DIM (tmpivavis) = TBmakeNum (1);
            // Following is really GenIntVector call
            AVIS_SHAPE (tmpivavis)
              = TCmakeIntVector (TBmakeExprs (TBmakeNum (iv_len - X_dim), NULL));
            tmpivval = COconstant2AST (iv_co);
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
        iv_co = COfreeConstant (iv_co);
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
    constant *arg1const;
    node *arg1 = NULL;
    node *arg2Narray = NULL;
    int timesrhoarg2;
    int prodarg1;

    DBUG_ENTER ("SCCFprf_reshape");
    /*  reshape(shp, arr):
     *    constant shp, non-constant arr, with
     *    arr having same element count prod(shp)
     *    and rank-0 ELEMTYPE
     */
    if (PM (PMarray (&arg2Narray, PMintConst (&arg1, PMprf (F_reshape_VxA, arg_node))))) {
        if ((NULL != arg1) && (0 == TYgetDim (ARRAY_ELEMTYPE (arg2Narray)))) {
            arg1const = COaST2Constant (arg1);
            if (NULL != arg1const) {
                resshape = COconstant2Shape (arg1const);
                prodarg1 = SHgetUnrLen (resshape);
                timesrhoarg2 = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2Narray));
                if (prodarg1 == timesrhoarg2) {
                    /* If the result is a scalar, return that. Otherwise,
                     * create an N_array.
                     */
                    if (0 == SHgetDim (resshape)) {
                        res = DUPdoDupTree (
                          TCgetNthExprsExpr (0, ARRAY_AELEMS (arg2Narray)));
                    } else {
                        res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2Narray)),
                                           resshape,
                                           DUPdoDupTree (ARRAY_AELEMS (arg2Narray)));
                    }
                    DBUG_PRINT ("CF", ("SCCFprf_reshape performed "));
                }
                arg1const = COfreeConstant (arg1const);
            }
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
    node *arg2Narray;
    constant *arg1;
    int takecount;
    int resshape;
    int argshape;
    int offset;

    DBUG_ENTER ("SCCFprf_take_SxV");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));
    arg2Narray = PMfollowId (PRF_ARG2 (arg_node));

    if ((NULL != arg1) && (NULL != arg2Narray) && (N_array == NODE_TYPE (arg2Narray))) {
        takecount = COconst2Int (arg1);
        resshape = abs (takecount);
        argshape = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2Narray));
        DBUG_ASSERT ((resshape <= argshape), ("SCCFprf_take_SxV attempted overtake"));
        if (argshape == takecount) {
            res = DUPdoDupTree (arg2Narray);
        } else {
            offset = (takecount >= 0) ? 0 : argshape - takecount;

            tail = TCgetExprsSection (offset, resshape, ARRAY_AELEMS (arg2Narray));
            DBUG_PRINT ("CF", ("SCCFprf_take performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2Narray)),
                               SHcreateShape (1, resshape), tail);
        }

        if (NULL != arg1) {
            arg1 = COfreeConstant (arg1);
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
    node *arg2Narray;
    constant *arg1;
    int dropcount;
    int resxrho;
    int offset;

    DBUG_ENTER ("SCCFprf_drop_SxV");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));
    arg2Narray = PMfollowId (PRF_ARG2 (arg_node));
    if ((NULL != arg1) && (NULL != arg2Narray) && (N_array == NODE_TYPE (arg2Narray))) {
        dropcount = COconst2Int (arg1);
        if (0 == dropcount) {
            res = DUPdoDupTree (arg2Narray);
        } else {
            resxrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2Narray)) - dropcount;
            resxrho = (resxrho < 0) ? 0 : resxrho;
            offset = (dropcount < 0) ? 0 : dropcount;
            tail = TCgetExprsSection (offset, resxrho, ARRAY_AELEMS (arg2Narray));
            DBUG_PRINT ("CF", ("SCCFprf_drop performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2Narray)),
                               SHcreateShape (1, resxrho), tail);
        }
        if (NULL != arg1) {
            arg1 = COfreeConstant (arg1);
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * ARmodarray( node *X, constant *iv, node *val)
 *
 * description:
 *   Implements modarray for structural constant:  X[iv] = val;
 *   X is expected to be an N_array.
 *   iv is expected to be a constant integer vector.
 *   val is expected to be a scalar.
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
    int framexrho;

    DBUG_ENTER ("ARmodarray");
    DBUG_ASSERT ((N_array == NODE_TYPE (arg1)), "ARmodarray expected N_array as arg1");
    framexrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg1));
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
 * static  node *prf_modarray(node *arg_node, info *arg_info, node *arg1Nid,
 *                            node *arg2Narray)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxS_(X, iv, val)
 *
 *****************************************************************************/
static node *
prf_modarray (node *arg_node, info *arg_info, node *arg1Nid, node *arg2Narray)
{
    node *res = NULL;
    constant *arg2const;
    node *arg1;
    node *arg1Narray;

    DBUG_ENTER ("prf_modarray");
    arg1 = PRF_ARG1 (arg_node);

    /* Attempt to find non-empty iv N_array for arg2const. */
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
     * It also ensures that iv is valid.
     * Hence, we can blindly do this replacement. */

    arg2const = COaST2Constant (arg2Narray);
    if (NULL != arg2const) {
        if (COisEmptyVect (arg2const)) {              /*  modarray(X, [], 42) */
            res = DUPdoDupTree (PRF_ARG3 (arg_node)); /* X and val are scalar */
            arg2const = COfreeConstant (arg2const);
        } else {
            /* arg2 non-empty constant */
            arg1Narray = PMfollowId (arg1Nid);
            if ((NULL != arg1Narray) && (N_array == NODE_TYPE (arg1Narray))) {
                DBUG_ASSERT ((N_array == NODE_TYPE (arg1Narray)),
                             "SCCFprf_modarray unable to find arg1 N_array");
                res = ARmodarray (arg1Narray, arg2const, PRF_ARG3 (arg_node));
            }
        }
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node *SCCFprf_modarray_AxVxS(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxS_(X, iv, scalaral)
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxS (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1Nid = NULL;
    node *arg2Narray = NULL;

    DBUG_ENTER ("SCCFprf_modarray_AxVxS");
    if (PM (
          PMarray (&arg2Narray, PMvar (&arg1Nid, PMprf (F_modarray_AxVxS, arg_node))))) {
        res = prf_modarray (arg_node, arg_info, arg1Nid, arg2Narray);
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
 *   z = _modarray_AxVxS_(X, iv, array)
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxA (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *arg1Nid = NULL;
    node *arg2Narray = NULL;

    DBUG_ENTER ("SCCFprf_modarray_AxVxA");
    if (PM (
          PMarray (&arg2Narray, PMvar (&arg1Nid, PMprf (F_modarray_AxVxA, arg_node))))) {
        res = prf_modarray (arg_node, arg_info, arg1Nid, arg2Narray);
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
    node *tail;
    int arg1shape;
    int arg2shape;
    int frameshape;

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
        && PM (PMarray (&arg2, PMarray (&arg1, PMprf (F_cat_VxV, arg_node))))) {
        /* Both arguments are constants or structure constants */
        arg1shape = SHgetUnrLen (ARRAY_FRAMESHAPE (arg1));
        arg2shape = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2));
        frameshape = arg1shape + arg2shape;

        /* Perform the actual element catenation */
        if (0 == arg1shape) { /* []++arg2 */
            DBUG_PRINT ("CF", ("SCCFprf_cat (2)removed []++var"));
            res = DUPdoDupTree (arg2);
        } else if (0 == arg2shape) { /* arg2++[] */
            DBUG_PRINT ("CF", ("SCCFprf_cat (2) removed var++[]"));
            res = DUPdoDupTree (arg1);
        } else { /* arg1++arg2 */
            arg1aelems = DUPdoDupTree (ARRAY_AELEMS (arg1));
            arg2aelems = DUPdoDupTree (ARRAY_AELEMS (arg2));
            tail = TCgetNthExprs (arg1shape - 1, arg1aelems);
            DBUG_ASSERT ((NULL == EXPRS_NEXT (tail)), "SCCFprf_cat_VxV missed arg1 tail");
            EXPRS_NEXT (tail) = arg2aelems;
            DBUG_PRINT ("CF", ("SCCFprf_cat replaced const1++const2"));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg1)),
                               SHcreateShape (1, frameshape), arg1aelems);
        }
    }
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   bool isValueMatch(node *arg1, node *arg2)
 *
 * description: Compares two N_id nodes for the same value
 *
 *****************************************************************************/
static bool
isValueMatch (node *arg1, node *arg2)
{
    bool res = FALSE;
    constant *arg1c;
    constant *arg2c;

    DBUG_ENTER ("isValueMatch");
    arg1 = PMfollowId (arg1); /* Follow the streams to their source */
    arg2 = PMfollowId (arg2);

    if (arg1 == arg2) {
        res = TRUE; /* If CSE has done its job, this is all we need */
    } else {

        arg1c = COaST2Constant (arg1);
        arg2c = COaST2Constant (arg2);
        if ((NULL != arg1c) && (NULL != arg2c)) {
            res = COcompareConstants (arg1c, arg2c);
        }
        if (NULL != arg1c) {
            arg1c = COfreeConstant (arg1c);
        }
        if (NULL != arg2c) {
            arg2c = COfreeConstant (arg2c);
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
 *   1. iv is an unknown expression:
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
 *   2. ivc is a constant:
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
 *   3. Essentially the inverse of case (2): iv is not a constant:
 *      b = modarray(arr, iv, val5);
 *      c = modarray(b, constantiv val3);
 *      x = sel(iv, c)
 *
 *      Since we do not know the value of iv, we must not make
 *      any assertions about its relationship to constantiv.
 *      Hence, this case must not be optimized.
 *
 *
 *   We still need to ensure that iv is a valid index for arr.
 *   I think -ecc should now handle that check properly.
 *
 *****************************************************************************/

/* FIXME: As of 2008-06-19, caes 2 and 3 of above are likely not implemented */

static node *
SelModarray (node *arg_node)
{
    node *res = NULL;
    node *iv = NULL;
    node *X = NULL;

    DBUG_ENTER ("SelModarray");
    if (PM (PMvar (&X, PMvar (&iv, PMprf (F_sel_VxA, arg_node))))) {
        X = PMfollowId (X);
        if ((NULL != X) && (N_prf == NODE_TYPE (X))
            && ((F_modarray_AxVxS == PRF_PRF (X)) || (F_idx_modarray_AxSxS == PRF_PRF (X))
                || (F_modarray_AxVxA == PRF_PRF (X)))) {
            if (isValueMatch (iv, PRF_ARG2 (X))) {
                res = DUPdoDupTree (PRF_ARG3 (X));
            }
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
    node *res;

    DBUG_ENTER ("SCCFprf_sel");
    res = SelEmptyScalar (arg_node, arg_info);
    if (NULL == res) {
        res = SelModarray (arg_node);
    }
    if (NULL == res) {
        res = StructOpSel (arg_node, arg_info);
    }
    DBUG_RETURN (res);
}
