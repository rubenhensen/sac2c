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
        tmpXid = (node *)CFsel (COtake (take_vec, iv_co), arg2);

        if (iv_len == X_dim) {
            // Case 1 : Exact selection: do the sel operation now.
            result = DUPdoDupTree (tmpXid);
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
    int resshape;
    int offset;

    DBUG_ENTER ("SCCFprf_drop_SxV");
    arg1 = COaST2Constant (PRF_ARG1 (arg_node));
    arg2Narray = PMfollowId (PRF_ARG2 (arg_node));
    if ((NULL != arg1) && (NULL != arg2Narray) && (N_array == NODE_TYPE (arg2Narray))) {
        dropcount = COconst2Int (arg1);
        if (0 == dropcount) {
            res = DUPdoDupTree (arg2Narray);
        } else {
            resshape = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2Narray)) - dropcount;
            offset = (dropcount < 0) ? 0 : dropcount;
            tail = TCgetExprsSection (offset, resshape, ARRAY_AELEMS (arg2Narray));
            DBUG_PRINT ("CF", ("SCCFprf_drop performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2Narray)),
                               SHcreateShape (1, resshape), tail);
        }
        if (NULL != arg1) {
            arg1 = COfreeConstant (arg1);
        }
    }

    DBUG_RETURN (res);
}

#ifdef LATERONMAYBE
/******************************************************************************
 *
 * function:
 *   node * ARtake(int n, node *X)
 *
 * description:
 *   Implements undertake for N_array. If n is negative,
 *   take is from tail end of array, ala APL.
 *
 *****************************************************************************/
static node *
ARtake (int n, node *X)
{
    node *res;
    node *tail;
    int framexrho;
    int offset;

    DBUG_ENTER ("ARtake");
    framexrho = SHgetUnrLen (ARRAY_FRAMESHAPE (X));
    offset = (n >= 0) ? 0 : framexrho - n;
    tail = TCgetExprsSection (offset, n, ARRAY_AELEMS (X));
    res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (X)), SHcreateShape (1, n), tail);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * ARdrop(int n, node *X)
 *
 * description:
 *   Implements drop for N_array.
 *   If n<0, drop is from tail end of array.
 *
 *****************************************************************************/
static node *
ARdrop (int n, node *X)
{
    node *res;

    DBUG_ENTER ("ARdrop");
    res = X;
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * ARsel(int i, node *X)
 *
 * description:
 *   Implements indexed reference for N_array:  X[i]
 *
 *****************************************************************************/
static node *
ARsel (int i, node *X)
{
    node *res;

    DBUG_ENTER ("ARsel");
    res = X;
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *   node * ARcatenate(node *X, node *Y)
 *
 * description:
 *   Implements catenate for N_array
 *
 *****************************************************************************/
static node *
ARcatenate (node *X, node *Y)
{
    node *res;

    DBUG_ENTER ("ARcatenate");
    res = X;
    DBUG_RETURN (res);
}

#endif // LATERONMAYBE
/******************************************************************************
 *
 * function:
 *   node * ARmodarray( node *X, constant *iv, node *val)
 *
 * description:
 *   Implements modarray for structural constant:  X[iv] = val;
 *   iv expected to be an integer vector
 *
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
     * index error here, so we don't check for such things.
     */

    /*
     * need to figure out how to do rho(constantvector)...
    DBUG_ASSERT(( SHgetDim( ARRAY_FRAMESHAPE( arg1)) == COconst2Int( iv)),
                ("ARmodarray confused about N_array frameshape"));
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
 *   node *SCCFprf_modarray(node *arg_node, info *arg_info)
 *
 * description:
 *   Implements modarray for structural constant X.
 *   z = _modarray_AxVxS_(X, iv, val)
 *
 *****************************************************************************/
node *
SCCFprf_modarray (node *arg_node, info *arg_info)
{
    node *res = NULL;
    constant *arg2const;
    node *arg1;
    node *arg1Nid = NULL;
    node *arg1Narray;
    node *arg2Narray = NULL;

    DBUG_ENTER ("SCCFprf_modarray");
    arg1 = PRF_ARG1 (arg_node);

    /* Attempt to find non-empty iv N_array for arg2const. */
    if (PM (
          PMarray (&arg2Narray, PMvar (&arg1Nid, PMprf (F_modarray_AxVxS, arg_node))))) {
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
        frameshape = arg1shape + arg1shape;

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
            DBUG_ASSERT ((0 == EXPRS_NEXT (tail)), "SCCFprf_cat_VxV missed arg1 tail");
            EXPRS_NEXT (tail) = arg2aelems;
            DBUG_PRINT ("CF", ("SCCFprf_cat replaced const1++const2"));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg1)),
                               SHcreateShape (1, frameshape), arg1aelems);
        }
    }
    DBUG_RETURN (res);
}

#ifdef TOMORROW

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
    if (arg1 == arg2) {
        res = TRUE;
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

#endif // TOMORROW

/******************************************************************************
 *
 * function:
 *   node *SelModarray(node *arg_node)
 *
 * description:
 *   tries a special sel-modarray optimization for the following cases:
 *
 *   1. iv is an unknown expression:
 *      b = modarray(arr, iv, value)
 *      x = sel(iv, b)    ->   x = value;
 *
 *   2. iv is an expression with known constant value:
 *      b = modarray(arr, [5], val5);
 *      c = modarray(b, [3], val3);
 *      d = modarray(c, [2], val2);
 *      x = sel([5], d)   ->  x = val5;
 *      In the above, if the statements setting c or d contain
 *      non-constants as iv, then we must NOT perform the
 *      optimization, because we are unable to assert that
 *      the iv is not [5].
 *
 *   We still need to ensure that iv is a valid index for arr.
 *   I think -ecc should now handle that check properly.
 *
 *****************************************************************************/

static node *
SelModarray (node *arg_node)
{
    node *res = NULL;

#ifdef OLDWAY
    node *iv = NULL;
    node *arr = NULL;
    node *val = NULL;
    node *ivmod = NULL;

    node *prf_mod;
    node *mod_arr_expr;
    node *mod_idx_expr;
    node *mod_elem_expr;
    constant *mod_idx_co;
    constant *idx_co;

    DBUG_ENTER ("SelModarray");

    /*
     * checks if array_expr is an array identifier defined by a primitive
     * modarray operation
     */
    if ((NODE_TYPE (array_expr) == N_id)
        && (AVIS_SSAASSIGN (ID_AVIS (array_expr)) != NULL)
        && (NODE_TYPE (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)))) == N_prf)) {

        switch (PRF_PRF (ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr))))) {

        case F_modarray_AxVxS:
            prf_mod = ASSIGN_RHS (AVIS_SSAASSIGN (ID_AVIS (array_expr)));

            /* get parameter of modarray */
            DBUG_ASSERT ((PRF_ARGS (prf_mod) != NULL), "missing 1. arg for modarray");
            mod_arr_expr = EXPRS_EXPR (PRF_ARGS (prf_mod));

            DBUG_ASSERT ((EXPRS_NEXT (PRF_ARGS (prf_mod)) != NULL),
                         "missing 2. arg for modarray");
            mod_idx_expr = EXPRS_EXPR (EXPRS_NEXT (PRF_ARGS (prf_mod)));

            DBUG_ASSERT ((EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (prf_mod))) != NULL),
                         "missing 3. arg for modarray");
            mod_elem_expr = EXPRS_EXPR (EXPRS_NEXT (EXPRS_NEXT (PRF_ARGS (prf_mod))));

            /* try to build up constants from index vectors */
            idx_co = COaST2Constant (idx_expr);
            mod_idx_co = COaST2Constant (mod_idx_expr);

            if ((CMPTdoCompareTree (idx_expr, mod_idx_expr) == CMPT_EQ)
                || ((idx_co != NULL) && (mod_idx_co != NULL)
                    && (COcompareConstants (idx_co, mod_idx_co)))) {
                /*
                 * idx vectors in sel and modarray are equal
                 * - replace sel() with element
                 */
                res = DUPdoDupTree (mod_elem_expr);

                DBUG_PRINT ("CF", ("sel-modarray optimization done"));
            }

            if (idx_co != NULL) {
                idx_co = COfreeConstant (idx_co);
            }
            if (mod_idx_co != NULL) {
                mod_idx_co = COfreeConstant (mod_idx_co);
            }

            break;

        default:
            break;
        }
    }
#else // OLDWAY
    DBUG_ENTER ("SelModarray");

    /* Pattern-match on arg2 of the sel() for a modarray that
     * has the same iv as the sel(). If we find one, replace
     * the sel() with arg3 of the modarray.
     * If we find a modarray that does not match, but both
     * sel iv and the modarray iv are constants (that don't match),
     * continue the search up the modarray chain.
     */

    /* We have to distinguish the two ivs, or we may stumble
     * past the first modarray
     */
#ifdef TOMORROW

    this is complicated by the fact that we can not perform this optimization blindly
      - if we use isValueMatch,
      we fail to let (-ecc - check c) perform their vetting of iv.

      if (PM (PMvar (&val, PMvar (&ivmod,
                                  PMvar (&arr, PMprf (F_modarray_AxVxS,
                                                      PMvar (&iv, PMprf (F_sel_VxA,
                                                                         arg_node))))))))
    {
        if ((ivmod == iv) || (isValueMatch (ivmod, iv))) {
            /* If CSE is disabled, the above comparison misses constants */
            res = DUPdoDupTree (val);
        } else {
            /* Look further up the food chain from this modarray */
            /* I have to think on this some more... */
            while (something) {
                val = NULL;
                if (PM (PMvar (&val,
                               PMvar (&ivmod,
                                      PMvar (&arr,
                                             PMprf (F_modarray_AxVxS,
                                                    PMvar (&iv, PMprf (F_sel_VxA,
                                                                       arg_node)))))))) {
                    .Both ivs must be constant and different
                }
            }
        }
    }
#endif // TOMORROW
#endif // OLDWAY
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
        res = StructOpSel (arg_node, arg_info);
    }
    if (NULL == res) {
        res = SelModarray (arg_node);
    }
    DBUG_RETURN (res);
}
