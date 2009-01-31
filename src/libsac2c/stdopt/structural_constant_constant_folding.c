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
#include "print.h"

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
        tmpXid = DUPdoDupTree (TCgetNthExprsExpr (offset, ARRAY_AELEMS (arg2)));
        if (iv_len == X_dim) {
            // Case 1 : Exact selection: do the sel operation now.
            DBUG_PRINT ("CF", ("StructOpSel exact selection performed."));
            result = tmpXid;
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
            DBUG_PRINT ("CF", ("StructOpSel sel(iv,X) replaced iv: old: %s; new: %s",
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
    int dropcount;
    int argxrho;
    int resxrho;

    DBUG_ENTER ("SCCFprf_take_SxV");
    if (PM (
          PMarrayConstructor (&fs2, &arg2,
                              PMintConst (&con1, &arg1, PMprf (F_take_SxV, arg_node))))) {
        takecount = COconst2Int (con1);
        resxrho = abs (takecount);
        argxrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2));
        DBUG_ASSERT ((resxrho <= argxrho), ("SCCFprf_take_SxV attempted overtake"));
        if (argxrho == resxrho) {
            res = DUPdoDupTree (arg2);
        } else {
            dropcount = (takecount >= 0) ? 0 : argxrho + takecount;
            tail = TCtakeDropExprs (resxrho, dropcount, ARRAY_AELEMS (arg2));
            DBUG_PRINT ("CF", ("SCCFprf_take performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2)),
                               SHcreateShape (1, resxrho), tail);
        }
        con1 = COfreeConstant (con1);
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
    int dc;
    int resxrho;
    int arg2xrho;

    DBUG_ENTER ("SCCFprf_drop_SxV");
    if (PM (
          PMarrayConstructor (&arg2fs, &arg2,
                              PMintConst (&con1, &arg1, PMprf (F_drop_SxV, arg_node))))) {
        dc = COconst2Int (con1);
        if (0 == dc) {
            res = DUPdoDupTree (arg2);
        } else {
            dropcount = (dc < 0) ? 0 : dc;
            arg2xrho = SHgetUnrLen (ARRAY_FRAMESHAPE (arg2));
            resxrho = arg2xrho - abs (dc);
            if (resxrho < 0) {
                CTIerrorLine (global.linenum,
                              "SCCFprf_drop_SxV attempted overdrop of size %d on vector "
                              "of shape %d",
                              resxrho, arg2xrho);
                CTIabort ("Compilation terminated");
            }
            tail = TCtakeDropExprs (resxrho, dropcount, ARRAY_AELEMS (arg2));
            DBUG_PRINT ("CF", ("SCCFprf_drop performed "));
            res = TBmakeArray (TYcopyType (ARRAY_ELEMTYPE (arg2)),
                               SHcreateShape (1, resxrho), tail);
        }
        con1 = COfreeConstant (con1);
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
 *   z = _modarray_AxVxS_(X, iv, val)
 *
 *****************************************************************************/
node *
SCCFprf_modarray_AxVxS (node *arg_node, info *arg_info)
{
    node *res = NULL;

    node *X = NULL;
    node *iv = NULL;
    node *val = NULL;
    node *exprs;
    constant *emptyVec;
    constant *coiv = NULL;
    constant *fsX = NULL;
    int offset;

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
    /**
     * match F_modarray_AxVxS( X, [], val)
     */
    emptyVec = COmakeConstant (T_int, SHcreateShape (1, 0), NULL);
    if (PM (PMvar (&val, PMconst (&emptyVec, &iv,
                                  PMvar (&X, PMprf (F_modarray_AxVxS, arg_node)))))) {
        res = DUPdoDupTree (val);
    } else {
        /**
         * match F_modarray_AxVxS( X = [...], iv = [c0,...,cn], val)
         */
        if (PM (PMvar (&val, PMconst (&coiv, &iv,
                                      PMarrayConstructor (&fsX, &X,
                                                          PMprf (F_modarray_AxVxS,
                                                                 arg_node)))))) {
            if (SHcompareShapes (COgetShape (fsX), COgetShape (coiv))) {
                offset = COvect2offset (fsX, coiv);
                res = DUPdoDupTree (X);
                exprs = TCgetNthExprs (offset, ARRAY_AELEMS (res));
                EXPRS_EXPR (exprs) = FREEdoFreeTree (EXPRS_EXPR (exprs));
                EXPRS_EXPR (exprs) = DUPdoDupTree (val);
            }
            fsX = COfreeConstant (fsX);
            coiv = COfreeConstant (coiv);
        } else {
            if (fsX != NULL) {
                fsX = COfreeConstant (fsX);
                if (coiv != NULL) {
                    coiv = COfreeConstant (coiv);
                }
            }
        }
    }
    emptyVec = COfreeConstant (emptyVec);
    DBUG_RETURN (res);
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
    node *iv = NULL;
    node *val = NULL;
    node *exprs, *val_exprs;
    constant *emptyVec;
    constant *coiv = NULL;
    constant *fsX = NULL;
    constant *fsval = NULL;
    constant *ivlen = NULL;
    constant *fsX_tail = NULL;
    int offset;

    DBUG_ENTER ("SCCFprf_modarray_AxVxA");

    /**
     * match F_modarray_AxVxA( X, [], val)
     */
    emptyVec = COmakeConstant (T_int, SHcreateShape (1, 0), NULL);
    if (PM (PMvar (&val, PMconst (&emptyVec, &iv,
                                  PMvar (&X, PMprf (F_modarray_AxVxS, arg_node)))))) {
        res = DUPdoDupTree (val);
    } else {
        /**
         *   match F_modarray_AxVxA( X = [...], iv = [c0,...,cn], val = [...])
         */
        if (PM (PMarrayConstructor (&fsval, &val,
                                    PMconst (&coiv, &iv,
                                             PMarrayConstructor (&fsX, &X,
                                                                 PMprf (F_modarray_AxVxA,
                                                                        arg_node)))))) {

            /**
             *  The only way to get this version of modarray is by means
             *  of WLUR!! Therefore, it is guaranteed that in fact shape(iv) <= dim(X)!
             *  However, this does not guarantee that shape(iv) <= shape( frameshape( X))!
             *  Example:
             *    foo( int[2,2] a)
             *    {
             *      X = [a,a];
             *      ...
             *    }
             *  => frameshape( X) = [2]  but shape(X) = [2,2,2] !!
             */
            DBUG_ASSERT ((COgetDim (fsX) == 1),
                         "illegal frameshape on first arg to modarray");
            if ((COgetDim (coiv) == 1)
                && (COgetExtent (coiv, 0) <= COgetExtent (fsX, 0))) {
                DBUG_ASSERT ((COgetDim (fsval) == 1),
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
                if ((COgetExtent (coiv, 0) + COgetExtent (fsval, 0))
                    == COgetExtent (fsX, 0)) {
                    ivlen = COmakeConstantFromInt (COgetExtent (coiv, 0));
                    fsX_tail = COdrop (ivlen, fsX);
                    if (COcompareConstants (fsval, fsX_tail)) {
                        offset = COvect2offset (fsX, coiv);
                        res = DUPdoDupTree (X);
                        exprs = TCgetNthExprs (offset, ARRAY_AELEMS (res));
                        val_exprs = ARRAY_AELEMS (val);
                        while (val_exprs != NULL) {
                            EXPRS_EXPR (exprs) = FREEdoFreeTree (EXPRS_EXPR (exprs));
                            EXPRS_EXPR (exprs) = DUPdoDupTree (EXPRS_EXPR (val_exprs));
                            exprs = EXPRS_NEXT (exprs);
                            val_exprs = EXPRS_NEXT (val_exprs);
                        }
                    }
                    ivlen = COfreeConstant (ivlen);
                    fsX_tail = COfreeConstant (fsX_tail);
                }
            }
            fsX = COfreeConstant (fsX);
            coiv = COfreeConstant (coiv);
            fsval = COfreeConstant (fsval);
        } else {
            if (fsX != NULL) {
                fsX = COfreeConstant (fsX);
                if (coiv != NULL) {
                    coiv = COfreeConstant (coiv);
                    if (fsval != NULL) {
                        fsval = COfreeConstant (fsval);
                    }
                }
            }
            /*
             * match F_modarray_AxVxA( X = [...], iv = [co,...,cn], val = V)
             *
             * we only do the simple case (case a above) where V fits neatly
             * into X as an element.
             */
            if (PM (PMvar (&val, PMconst (&coiv, &iv,
                                          PMarrayConstructor (&fsX, &X,
                                                              PMprf (F_modarray_AxVxA,
                                                                     arg_node)))))
                && (COgetExtent (coiv, 0) == COgetExtent (fsX, 0))) {
                offset = COvect2offset (fsX, coiv);
                res = DUPdoDupTree (X);
                exprs = TCgetNthExprs (offset, ARRAY_AELEMS (res));
                EXPRS_EXPR (exprs) = FREEdoFreeTree (EXPRS_EXPR (exprs));
                EXPRS_EXPR (exprs) = DUPdoDupTree (val);
            } else {
                if (fsX != NULL) {
                    fsX = COfreeConstant (fsX);
                    if (coiv != NULL) {
                        coiv = COfreeConstant (coiv);
                    }
                }
            }
        }
    }
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

            DBUG_PRINT ("CF", ("SCCFprf_cat performed const1++const2"));
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
 * @param arg_node N_prf node of sel
 * @param arg_info info structure
 *
 * @return if prf matches the above pattern it returns i
 ******************************************************************************/
static node *
SelArrayOfEqualElements (node *arg_node, info *arg_info)
{
    node *res = NULL;
    node *iv = NULL;
    node *aelems = NULL;
    node *elem = NULL;
    constant *frameshape = NULL;
    bool matches = TRUE;

    DBUG_ENTER ("SelArrayOfEqualElements");

    if (PM (PMexprs (&aelems, PMarray (&frameshape, NULL,
                                       PMvar (&iv, PMprf (F_sel_VxA, arg_node)))))
        && TUshapeKnown (AVIS_TYPE (ID_AVIS (iv)))
        && (SHgetExtent (TYgetShape (AVIS_TYPE (ID_AVIS (iv))), 0)
            == COgetExtent (frameshape, 0))) {
        while (matches && (aelems != NULL)) {
            matches = PM (PMany (&elem, aelems));
            aelems = EXPRS_NEXT (aelems);
        }

        if (matches) {
            res = DUPdoDupTree (elem);
        }
    }

    if (frameshape != NULL) {
        frameshape = COfreeConstant (frameshape);
    }

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
 *        we can ignore iv_1 as it has to be 0 anyways. We loose a check here
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
static void *IPS_FAILED = "";

static void *
IsProxySel (constant *idx, void *sels, void *template)
{
    node *index;

    DBUG_ENTER ("IsProxySel");

    if (sels == NULL) {
        DBUG_ASSERT (TRUE, "ran out of selection operations!");

        sels = IPS_FAILED;
    } else if (sels != IPS_FAILED) {
        index = COconstant2AST (idx);

        DBUG_ASSERT ((NODE_TYPE (index) == N_array), "index not array?!?");

        if (!PM (PMexprs (&ARRAY_AELEMS (index),
                          PMpartExprs ((node *)template,
                                       PMarray (NULL, NULL,
                                                PMprf (F_sel_VxA,
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
SelProxyArray (node *arg_node, info *arg_info)
{
    node *var_P = NULL;
    node *iv = NULL;
    node *aelems_P = NULL;
    node *arr_P = NULL;
    node *tmp;
    node *filter_iv;
    node *var_A = NULL;
    node *template = NULL;
    constant *fs_iv = NULL;
    constant *fs_P = NULL;
    shape *fs_P_shp;
    shape *iter_shp;
    node *iv_avis;
    bool all_sels = TRUE;
    bool match;
    int pos, off, tlen, flen;

    node *res = NULL;

    DBUG_ENTER ("SelProxyArray");

    if (PM (PMvar (&var_P, PMarrayConstructor (&fs_iv, &iv, PMprf (F_sel_VxA, arg_node))))
        && PM (PMexprs (&aelems_P, PMarray (&fs_P, &arr_P, var_P)))) {
        /*
         * before we check that P is a proxy, we check whether it is defined
         * by sel operations on a single source array. This test is way
         * cheaper, so testing twice is worth it.
         */
        DBUG_PRINT ("CF_PROXY", ("Found matching sel!"));
        tmp = aelems_P;
        while (all_sels && (tmp != NULL)) {
            all_sels
              = PM (PMvar (&var_A, PMany (NULL, PMprf (F_sel_VxA, EXPRS_EXPR (tmp)))));
            tmp = EXPRS_NEXT (tmp);
        }

        if (all_sels) {
            DBUG_PRINT ("CF_PROXY", ("Might have found a proxy!"));

            /*
             * first of all, we filter out the prefix of indices that correspond
             * to a 1 extent dimension in P and the corresponding elements
             * from the frameshape of P to get the iteration space over A
             */
            filter_iv = DUPdoDupTree (ARRAY_AELEMS (iv));
            fs_P_shp = ARRAY_FRAMESHAPE (arr_P);

            pos = 0;
            while ((SHgetExtent (fs_P_shp, pos) == 1) && (filter_iv != NULL)) {
                filter_iv = FREEdoFreeNode (filter_iv);
                pos++;
            }

            DBUG_ASSERT ((filter_iv != NULL), "selection from weird array!");

            flen = TCcountExprs (filter_iv);
            iter_shp = SHmakeShape (flen);
            off = 0;
            for (pos = 0; pos < SHgetDim (fs_P_shp); pos++) {
                if (SHgetExtent (fs_P_shp, pos) == 0) {
                    SHsetExtent (iter_shp, off, SHgetExtent (fs_P_shp, pos));
                    off++;
                }
            }
            /*
             * now the final step:
             *
             * check whether all sels are of the form
             *
             * sel_VxA( [v1, ..., vn, c1, ..., cn], A)
             *
             * To do so, we first extract a template for the v1, ..., vn
             * from the first element in the array.
             */
            match = PM (
              PMexprs (&template, PMarray (NULL, NULL, PMprf (F_sel_VxA, aelems_P))));

            DBUG_ASSERT (match, "code has unexpected pattern!");

            tlen = TCcountExprs (template);
            DBUG_ASSERT ((tlen >= flen), "sel operations do not match!");

            if (tlen == flen) {
                template = NULL; /* no non-index part */
            } else {
                template = DUPdoDupTree (template);
                tmp = TCgetNthExprs (tlen - flen - 1, template);
                EXPRS_NEXT (tmp) = FREEdoFreeTree (EXPRS_NEXT (tmp));
            }

            /*
             * now we check whether all selections are
             *
             * template ++ some constants
             */
            tmp = COcreateAllIndicesAndFold (iter_shp, IsProxySel, aelems_P, template);

            /*
             * if that worked out, we can replace the selection by
             *
             * sel ( [v1, ..., vn, i1, ..., in]], A)
             */
            if (tmp != IPS_FAILED) {
                DBUG_PRINT ("CF_PROXY", ("Replacing a proxy sel!"));
                iv_avis = TBmakeAvis (TRAVtmpVar (), TYmakeAKS (TYmakeSimpleType (T_int),
                                                                SHcreateShape (1, tlen)));
                INFO_VARDECS (arg_info) = TBmakeVardec (iv_avis, INFO_VARDECS (arg_info));
                INFO_PREASSIGN (arg_info)
                  = TBmakeAssign (TBmakeLet (TBmakeIds (iv_avis, NULL),
                                             TCmakeIntVector (
                                               TCappendExprs (template, filter_iv))),
                                  INFO_PREASSIGN (arg_info));
                AVIS_SSAASSIGN (iv_avis) = INFO_PREASSIGN (arg_info);

                res = TCmakePrf2 (F_sel_VxA, TBmakeId (iv_avis), DUPdoDupNode (var_A));
            } else {
                if (template != NULL) {
                    template = FREEdoFreeTree (template);
                }
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

    if (NULL == res) {
        res = SelArrayOfEqualElements (arg_node, arg_info);
    }

    if (NULL == res) {
        res = SelProxyArray (arg_node, arg_info);
    }
    DBUG_RETURN (res);
}
