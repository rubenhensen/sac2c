/*
 *
 * $Id$
 *
 */

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "ct_prf.h"
#include "constants.h"
#include "new_types.h"
#include "type_errors.h"
#include "type_utils.h"
#include "user_types.h"
#include "shape.h"
#include "constants.h"
#include "ctinfo.h"
#include "globals.h"

static constant *
ApplyCF (te_info *info, ntype *args)
{
    constant *res = NULL;

    DBUG_ENTER ();

    switch (TYgetProductSize (args)) {
    case 1:
        res = ((monCF)TEgetCFFun (info)) (TYgetValue (TYgetProductMember (args, 0)));
        break;
    case 2:
        res = ((binCF)TEgetCFFun (info)) (TYgetValue (TYgetProductMember (args, 0)),
                                          TYgetValue (TYgetProductMember (args, 1)));
        break;
    case 3:
        res = ((triCF)TEgetCFFun (info)) (TYgetValue (TYgetProductMember (args, 0)),
                                          TYgetValue (TYgetProductMember (args, 1)),
                                          TYgetValue (TYgetProductMember (args, 2)));
        break;
    default:
        DBUG_ASSERT (FALSE, "Constant Folding failed for the given number of arguments!");
    }
    DBUG_ASSERT (res != NULL,
                 "Constant Folding failed despite legal arguments were found!");

    DBUG_RETURN (res);
}

/******************************************************************************
 ***
 ***          Now the functions for computing result types:
 ***          ---------------------------------------------
 ***
 ******************************************************************************/

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_dummy( te_info *info, ntype *args)
 *
 * description:
 *    generic version which, in the end 8-), should never be called!
 *
 ******************************************************************************/

ntype *
NTCCTprf_dummy (te_info *info, ntype *args)
{
    DBUG_ENTER ();
    DBUG_ASSERT (FALSE, "prf not yet implemented");
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_id( te_info *info, ntype *args)
 *
 * description:
 *    generic version which simply returns its argument type.
 *
 ******************************************************************************/

ntype *
NTCCTprf_id (te_info *info, ntype *args)
{
    DBUG_ENTER ();
    DBUG_RETURN (TYcopyType (args));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_array( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_array (te_info *info, ntype *elems)
{
    ntype *outer, *elem, *elem2, *res;
    constant *val, *tmp;
    shape *shp;
    int num_elems;
    char *err_msg;
    int i;

    DBUG_ENTER ();

    outer = TYgetProductMember (elems, 0);
    elem = TYcopyType (TYgetProductMember (elems, 1));
    num_elems = TYgetProductSize (elems);

    for (i = 2; i < num_elems; i++) {
        elem2 = TYgetProductMember (elems, i);
        TEassureSameScalarType ("array element #0", elem, TEarrayElem2Obj (i), elem2);
        elem2 = TEassureSameShape ("array element #0", elem, TEarrayElem2Obj (i), elem2);
        TYfreeType (elem);
        elem = elem2;
    }
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);

    } else {
        if (TYisProdOfAKVafter (elems, 1)) {
            val = COcopyConstant (TYgetValue (TYgetProductMember (elems, 1)));
            for (i = 2; i < num_elems; i++) {
                tmp = val;
                val = COcat (tmp, TYgetValue (TYgetProductMember (elems, i)), NULL);
                tmp = COfreeConstant (tmp);
            }
            shp = TYgetShape (outer);
            tmp = COmakeConstantFromShape (SHappendShapes (shp, TYgetShape (elem)));
            res = TYmakeAKV (TYcopyType (TYgetScalar (elem)), COreshape (tmp, val, NULL));
            tmp = COfreeConstant (tmp);
            val = COfreeConstant (val);
        } else {
            res = TYnestTypes (outer, elem);
        }
    }
    TYfreeType (elem);

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_cast( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_cast (te_info *info, ntype *elems)
{
    ntype *cast_t, *cast_bt, *expr_t, *expr_bt;
    ntype *res, *res_bt;
    shape *shp, *d_shp, *s_shp;
    char *err_msg;

    DBUG_ENTER ();

    cast_t = TYgetProductMember (elems, 0);
    cast_bt = TYeliminateUser (cast_t);
    expr_t = TYgetProductMember (elems, 1);
    expr_bt = TYeliminateUser (expr_t);

    /*
     * First we check whether the base types match. If they don't, we
     * instantly bail out, as we do not support ant kind of basetype
     * polymorphism and thus the program is incorrect no matter what.
     */
    TEassureSameScalarType ("cast-type", cast_bt, "expr-type", expr_bt);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        CTIerror (err_msg);
        TEextendedAbort ();
    }

    /*
     * For matching shapes, we only bail out if this code is reachable.
     * This is to allow shape polymorphic programming (e.g., recursion
     * over shapes).
     *
     * The actual error processing can be found further below...
     */
    res_bt = TEassureSameShape ("cast-type", cast_bt, "expr-type", expr_bt);
    cast_bt = TYfreeType (cast_bt);
    expr_bt = TYfreeType (expr_bt);

    /*
     * Unfortunately, this TEassureSameShape in certain situations does not detect
     * incompatibilities. The problem arises from the application of
     * TYeliminateUser: e.g.
     *
     *    TYeliminateUser( complex[.])  =>   double[.,.]
     *
     * which does not contain the information that complex[.] in fact is
     * double[.,2]! As a consequence, it can be casted into double[3,4] which
     * obviously is wrong!! Such situations can accur, if
     *
     *   (a) "res_bt" is an AKS type and
     *   (b) at least one of "cast_t" and "expr_t" are based on a user type;
     *
     * or if
     *
     *   (a) both, "cast_t" and "expr_t" are based on a user type.
     *
     * Hence, the shapes (if available) of "res_bt" and the definitions of
     * the user types of "cast_t" and "expr_t" have to be compared:
     */

    if (TYisAKS (res_bt)) {
        shp = TYgetShape (res_bt);
        if (TYisArray (cast_t) && TYisUser (TYgetScalar (cast_t))) {
            d_shp = TYgetShape (UTgetBaseType (TYgetUserType (TYgetScalar (cast_t))));
            s_shp = SHdropFromShape (SHgetDim (shp) - SHgetDim (d_shp), shp);
            if (!SHcompareShapes (d_shp, s_shp)) {
                CTIerrorLine (global.linenum,
                              "Cast type %s does not match expression type %s "
                              "as \"%s\" is defined as %s",
                              TYtype2String (cast_t, FALSE, 0),
                              TYtype2String (expr_t, FALSE, 0),
                              UTgetName (TYgetUserType (TYgetScalar (cast_t))),
                              TYtype2String (UTgetBaseType (
                                               TYgetUserType (TYgetScalar (cast_t))),
                                             FALSE, 0));
                TEextendedAbort ();
            }
        }
        if (TYisArray (expr_t) && TYisUser (TYgetScalar (expr_t))) {
            d_shp = TYgetShape (UTgetBaseType (TYgetUserType (TYgetScalar (expr_t))));
            s_shp = SHdropFromShape (SHgetDim (shp) - SHgetDim (d_shp), shp);
            if (!SHcompareShapes (d_shp, s_shp)) {
                CTIerrorLine (global.linenum,
                              "Cast type %s does not match expression type %s "
                              "as \"%s\" is defined as %s",
                              TYtype2String (cast_t, FALSE, 0),
                              TYtype2String (expr_t, FALSE, 0),
                              UTgetName (TYgetUserType (TYgetScalar (expr_t))),
                              TYtype2String (UTgetBaseType (
                                               TYgetUserType (TYgetScalar (expr_t))),
                                             FALSE, 0));
                TEextendedAbort ();
            }
        }
    } else {
        if (TYisArray (cast_t) && TYisUser (TYgetScalar (cast_t)) && TYisArray (expr_t)
            && TYisUser (TYgetScalar (expr_t))) {
            shp = TYgetShape (UTgetBaseType (TYgetUserType (TYgetScalar (cast_t))));
            d_shp = TYgetShape (UTgetBaseType (TYgetUserType (TYgetScalar (expr_t))));
            if (SHgetDim (shp) < SHgetDim (d_shp)
                  ? !SHcompareShapes (shp,
                                      SHdropFromShape (SHgetDim (d_shp) - SHgetDim (shp),
                                                       d_shp))
                  : !SHcompareShapes (SHdropFromShape (SHgetDim (shp) - SHgetDim (d_shp),
                                                       shp),
                                      d_shp)) {
                CTIerrorLine (global.linenum,
                              "Cast type %s does not match expression type %s "
                              "as \"%s\" is defined as %s whereas \"%s\" is defined as "
                              "%s",
                              TYtype2String (cast_t, FALSE, 0),
                              TYtype2String (expr_t, FALSE, 0),
                              UTgetName (TYgetUserType (TYgetScalar (cast_t))),
                              TYtype2String (UTgetBaseType (
                                               TYgetUserType (TYgetScalar (cast_t))),
                                             FALSE, 0),
                              UTgetName (TYgetUserType (TYgetScalar (expr_t))),
                              TYtype2String (UTgetBaseType (
                                               TYgetUserType (TYgetScalar (expr_t))),
                                             FALSE, 0));
                TEextendedAbort ();
            }
        }
    }

    /*
     * Now, that we have checked for potential compatibility, we can compute
     * the best possible return type. Usual, this is res_bt. However, if
     * "cast_t" turns out to be based on a user defined type, we have to
     * "de-nest" the return type, i.e., we have to cut off the shape of the
     * (base) defining type of the user type from the back of "res_bt".
     *
     * REMARK: We first have to perform some error checking in case only
     *         the first first shape conformity check above went wrong!
     */

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else if (TYisArray (cast_t) && TYisUser (TYgetScalar (cast_t))) {
        res
          = TYdeNestTypeFromInner (res_bt,
                                   UTgetBaseType (TYgetUserType (TYgetScalar (cast_t))));
        res = TYsetScalar (res, TYcopyType (TYgetScalar (cast_t)));
        res_bt = TYfreeType (res_bt);
    } else {
        res = res_bt;
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_type_conv( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_type_conv (te_info *info, ntype *args)
{
    ntype *type;
    ntype *arg;
    ntype *res;
    ct_res cmp;
    char *err_msg;

    DBUG_ENTER ();

    type = TYgetProductMember (args, 0);
    arg = TYgetProductMember (args, 1);

    cmp = TYcmpTypes (type, arg);

    if ((cmp == TY_eq) || (cmp == TY_lt)) {
        res = TYcopyType (type);
    } else if (cmp == TY_gt) {
        res = TYcopyType (arg);
    } else {
        TEhandleError (TEgetLine (info), TEgetFile (info),
                       "inferred type %s should match declared type %s",
                       TYtype2String (arg, FALSE, 0), TYtype2String (type, FALSE, 0));
        err_msg = TEfetchErrors ();
        res = TYmakeBottomType (err_msg);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_nest( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_enclose (te_info *info, ntype *args)
{
    ntype *to;
    ntype *res;

    DBUG_ENTER ();

    to = TYgetProductMember (args, 1);

    res = TYcopyType (to);

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_denest( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_disclose (te_info *info, ntype *args)
{
    ntype *to;
    ntype *res;

    DBUG_ENTER ();

    to = TYgetProductMember (args, 1);

    res = TYcopyType (to);

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_dispatch_error(te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_dispatch_error (te_info *info, ntype *args)
{
    ntype *num_rets_t, *res;
    constant *co;
    int num_rets, i;

    DBUG_ENTER ();

    num_rets_t = TYgetProductMember (args, 0);
    DBUG_ASSERT (TYisAKV (num_rets_t), "illegal construction of _dispatch_error_:"
                                       " first argument not a constant");
    co = TYgetValue (num_rets_t);
    DBUG_ASSERT (COgetType (co) == T_int, "illegal construction of _dispatch_error_:"
                                          " first argument not an integer");
    DBUG_ASSERT (COgetDim (co) == 0, "illegal construction of _dispatch_error_:"
                                     " first argument not a scalar");
    num_rets = ((int *)COgetDataVec (co))[0];

    res = TYmakeEmptyProductType (num_rets);
    for (i = 0; i < num_rets; i++) {
        TYsetProductMember (res, i, TYcopyType (TYgetProductMember (args, i + 1)));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_guard( te_info *info, ntype *elems)
 *
 * description: X', Y', Z'... = guard( X, Y, Z, ..., pred);
 *             pred is Boolean scalar
 *             X, Y, Z... are anything.
 *             If( pred), X' = X; Y' = Y; Z' = Z; ...
 *             else error.
 *
 ******************************************************************************/

ntype *
NTCCTprf_guard (te_info *info, ntype *args)
{
    ntype *res;
    char *err_msg;
    ntype *pred;
    int num_rets;
    int i;

    DBUG_ENTER ();

    num_rets = TYgetProductSize (args) - 1;

    pred = TYgetProductMember (args, num_rets);
    TEassureBoolS ("predicate (last argument of guard())", pred);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        CTIabort (err_msg);
    }

    res = TYmakeEmptyProductType (num_rets);
    for (i = 0; i < num_rets; i++) {
        TYsetProductMember (res, i, TYcopyType (TYgetProductMember (args, i)));
    }

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_guardhold( te_info *info, ntype *elems)
 *
 * description: pred' = guard( pred);  pred is Boolean scalar
 *              guardhold means the pred is always evaluated to true;
 *
 ******************************************************************************/

ntype *
NTCCTprf_guardhold (te_info *info, ntype *args)
{
    ntype *res = NULL;
    char *err_msg;
    ntype *pred;

    DBUG_ENTER ();

    pred = TYgetProductMember (args, 0);
    TEassureBoolS ("requires predicate", pred);
    if (err_msg != NULL) {
        CTIabort (err_msg);
    }

    res = TYcopyType (pred);

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_afterguard( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_afterguard (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res = NULL, *pred;
    char *err_msg;
    int i;
    bool all_true = TRUE;

    DBUG_ENTER ();
    arg = TYgetProductMember (args, 0);

    for (i = 1; (i < TYgetProductSize (args)) && (res == NULL); i++) {
        pred = TYgetProductMember (args, i);
        TEassureBoolS ("requires expression", pred);
        err_msg = TEfetchErrors ();

        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {
            if (TYisAKV (pred)) {
                if (COisFalse (TYgetValue (pred), TRUE)) {
                    res = TYmakeBottomType (err_msg);
                }
            } else {
                all_true = FALSE;
            }
        }
    }
    if (res == NULL) {
        if (all_true) {
            res = TYcopyType (arg);
        } else {
            res = TYeliminateAKV (arg);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_noteminval( te_info *info, ntype *elems)
 *
 * description:
 *   _note_minval is used to preserve a temp assigned
 *   from WITHIDs in WLs. If -ssaiv ever works, we can just
 *   scrap this, and attach AVIS_MIN and AVIS_MAX to
 *   the WITHIDs directly. At present, we can't do that,
 *   because the extrema are different for each partition,
 *   but the WITHIDs are not different, hence this kludge.
 *
 *   The semantics of the _noteminval/notemaxval are:
 *
 *    iv'  = _noteminval(iv, GENERATOR_BOUND1)
 *    iv'' = _notemaxval(iv', GENERATOR_BOUND2)
 *
 *    The duty of the code using iv'' (SWLFI/SWLF) is to delete the
 *    guard in the fullness of time, after making use of the
 *    extrema. The scc phase will delete these guards before
 *    they ever reach the code generator. In both cases,
 *    the above call is replaced by:
 *
 *    iv'' = iv;
 *
 ******************************************************************************/

ntype *
NTCCTprf_noteminval (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res;

    DBUG_ENTER ();

    arg = TYgetProductMember (args, 0);
    res = TYcopyType (arg);
    res = TYmakeProductType (1, res);
    DBUG_RETURN (res);
}

ntype *
NTCCTprf_notemaxval (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res;

    DBUG_ENTER ();

    arg = TYgetProductMember (args, 0);
    res = TYcopyType (arg);
    res = TYmakeProductType (1, res);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_noteintersect( te_info *info, ntype *elems)
 *
 * description:
 *   This guard acts as an anchor for index vector intersect
 *   computations, preceeding an _sel_( idx, foldeeWL) operation.
 *
 *   This is intended as a data-flow approach for
 *   adding ancillary information to the ast.
 *
 *   The semantics of the _noteintersect are:
 *   If we start with:
 *
 *    z   = _sel_VxA_( iv, foldeeWL);
 *
 *   We end up with:
 *
 *    iv' = _noteintersect(iv, indexsetmin, indexsetmax)
 *    z   = _sel_VxA_( iv', foldeeWL);
 *
 *   The indexsetmin and indexsetmax are the result
 *   of the EWLF intersect computation
 *   between iv' and the foldeeWL partition(s).
 *   If they exactly match a partition of the foldeeWL,
 *   then EWLF may proceed.
 *
 ******************************************************************************/

ntype *
NTCCTprf_noteintersect (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res;

    DBUG_ENTER ();

    arg = TYgetProductMember (args, 0);
    arg = TYeliminateAKV (arg);
    res = TYcopyType (arg);
    res = TYmakeProductType (1, res);
    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_type_constraint( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_type_constraint (te_info *info, ntype *args)
{
    ntype *type;
    ntype *arg;
    ntype *res, *pred;
    ct_res cmp;
    char *err_msg;

    DBUG_ENTER ();

    type = TYgetProductMember (args, 0);
    arg = TYgetProductMember (args, 1);

    cmp = TYcmpTypes (type, arg);

    if ((cmp == TY_eq) || (cmp == TY_gt)) {
        res = TYcopyType (arg);
        pred = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
    } else if (cmp == TY_lt) {
        res = TYcopyType (type);
        pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
    } else {
        TEhandleError (TEgetLine (info), TEgetFile (info),
                       "inferred type %s should match required type %s",
                       TYtype2String (arg, FALSE, 0), TYtype2String (type, FALSE, 0));
        err_msg = TEfetchErrors ();
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_same_shape( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_same_shape (te_info *info, ntype *args)
{
    ntype *array1, *array2, *res1, *res2, *pred;
    char *err_msg;

    DBUG_ENTER ();

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    res1 = TEassureSameShape (TEarg2Obj (1), array1,
                              TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if (err_msg != NULL) {
        res1 = TYfreeType (res1);
        res1 = TYmakeBottomType (err_msg);
        res2 = TYcopyType (res1);
        pred = TYcopyType (res1);
    } else {

        if (TUshapeKnown (array1) && TUshapeKnown (array2)) {
            res1 = TYfreeType (res1);
            res1 = TYcopyType (array1);
            res2 = TYcopyType (array2);
            pred = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
        } else {
            res2 = TYcopyType (res1);
            pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (3, res1, res2, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_shape_dim( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_shape_dim (te_info *info, ntype *args)
{
    ntype *idx, *array, *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {

        TEassureShpMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 1), idx, TEarg2Obj (2),
                               array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {
            if (TUshapeKnown (idx) && TUdimKnown (array)) {
                res = TYcopyType (idx);
                pred
                  = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
            } else {
                res = TYeliminateAKV (idx);
                pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_non_neg_S)( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_non_neg_S (te_info *info, ntype *args)
{
    ntype *idx;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    idx = TYgetProductMember (args, 0);
    TEassureNonNegativeValues_S (TEprfArg2Obj (TEgetNameStr (info), 1), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {
        res = TYcopyType (idx);
        if (TYisAKV (idx)) {
            pred = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
        } else {
            pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_non_neg_V)( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_non_neg_V (te_info *info, ntype *args)
{
    ntype *idx;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    idx = TYgetProductMember (args, 0);
    TEassureNonNegativeValues_V (TEprfArg2Obj (TEgetNameStr (info), 1), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {
        res = TYcopyType (idx);
        if (TYisAKV (idx)) {
            pred = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
        } else {
            pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_val_shape_S( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_val_shape_S (te_info *info, ntype *args)
{
    ntype *idx, *array;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {

        err_msg = TEfetchErrors ();

        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {

            err_msg = TEfetchErrors ();

            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
                pred = TYcopyType (res);
            } else {
                if (TYisAKV (idx) && TUshapeKnown (array)) {
                    res = TYcopyType (idx);
                    pred = TYmakeAKV (TYmakeSimpleType (T_bool),
                                      COmakeTrue (SHcreateShape (0)));
                } else {
                    res = TYeliminateAKV (idx);
                    pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_val_shape_V( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_val_shape_V (te_info *info, ntype *args)
{
    ntype *idx, *array;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {

        TEassureShpMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 1), idx, TEarg2Obj (2),
                               array);
        err_msg = TEfetchErrors ();

        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {

            TEassureValMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 1), idx,
                                     TEarg2Obj (2), array);
            err_msg = TEfetchErrors ();

            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
                pred = TYcopyType (res);
            } else {
                if (TYisAKV (idx) && TUshapeKnown (array)) {
                    res = TYcopyType (idx);
                    pred = TYmakeAKV (TYmakeSimpleType (T_bool),
                                      COmakeTrue (SHcreateShape (0)));
                } else {
                    res = TYeliminateAKV (idx);
                    pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_val_lt_val_SxS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_val_lt_val_SxS (te_info *info, ntype *args)
{
    ntype *iv1, *iv2;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    iv1 = TYgetProductMember (args, 0);
    iv2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), iv1);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), iv2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {
        TEassureValLtVal_SxS (TEprfArg2Obj (TEgetNameStr (info), 1), iv1, TEarg2Obj (2),
                              iv2);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {
            if (TYisAKV (iv1) && TYisAKV (iv2)) {
                res = TYcopyType (iv1);
                pred
                  = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
            } else {
                res = TYeliminateAKV (iv1);
                pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_val_le_val_SxS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_val_le_val_SxS (te_info *info, ntype *args)
{
    ntype *iv1, *iv2;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    iv1 = TYgetProductMember (args, 0);
    iv2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), iv1);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), iv2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {
        TEassureValLeVal_SxS (TEprfArg2Obj (TEgetNameStr (info), 1), iv1, TEarg2Obj (2),
                              iv2);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {
            if (TYisAKV (iv1) && TYisAKV (iv2)) {
                res = TYcopyType (iv1);
                pred
                  = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
            } else {
                res = TYeliminateAKV (iv1);
                pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_val_le_val_VxV( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_val_le_val_VxV (te_info *info, ntype *args)
{
    ntype *iv1, *iv2;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    iv1 = TYgetProductMember (args, 0);
    iv2 = TYgetProductMember (args, 1);

    TEassureIntV ("vect", iv1);
    TEassureIntV ("vect", iv2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {
        res = TEassureSameShape ("vect", iv1, "vect", iv2);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYfreeType (res);
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {
            TEassureValLeVal (TEprfArg2Obj (TEgetNameStr (info), 1), iv1, TEarg2Obj (2),
                              iv2);

            err_msg = TEfetchErrors ();

            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
                pred = TYcopyType (res);
            } else {
                if (TYisAKV (iv1) && TYisAKV (iv2)) {
                    res = TYcopyType (iv1);
                    pred = TYmakeAKV (TYmakeSimpleType (T_bool),
                                      COmakeTrue (SHcreateShape (0)));
                } else {
                    res = TYeliminateAKV (iv1);
                    pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_prod_shape( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_prod_shape (te_info *info, ntype *args)
{
    ntype *new_shp, *array;
    ntype *res, *pred;
    char *err_msg;

    DBUG_ENTER ();

    new_shp = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), new_shp);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
        pred = TYcopyType (res);
    } else {

        TEassureProdValMatchesProdShape (TEprfArg2Obj (TEgetNameStr (info), 1), new_shp,
                                         TEarg2Obj (2), array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
            pred = TYcopyType (res);
        } else {
            if (TYisAKV (new_shp) && TUshapeKnown (array)) {
                res = TYcopyType (new_shp);
                pred
                  = TYmakeAKV (TYmakeSimpleType (T_bool), COmakeTrue (SHcreateShape (0)));
            } else {
                res = TYeliminateAKV (new_shp);
                pred = TYmakeAKS (TYmakeSimpleType (T_bool), SHcreateShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (2, res, pred));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_nested_shape( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_nested_shape (te_info *info, ntype *args)
{
    ntype *type;
    usertype udt;
    ntype *res;
    shape *shp;
    char *err_msg;

    DBUG_ENTER ();

    type = TYgetProductMember (args, 0);

    if (!TUisArrayOfUser (type)) {
        TEhandleError (TEgetLine (info), TEgetFile (info),
                       "nested_shape applied to non user-type %s.",
                       TYtype2String (type, FALSE, 0));
        err_msg = TEfetchErrors ();
        res = TYmakeBottomType (err_msg);
    } else {
        udt = TYgetUserType (TYgetScalar (type));
        shp = TYgetShape (UTgetBaseType (udt));
        res = TYmakeAKV (TYmakeSimpleType (T_int), COmakeConstantFromShape (shp));
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_saabind( te_info *info, ntype *args)
 *
 *****************************************************************************/
ntype *
NTCCTprf_saabind (te_info *info, ntype *args)
{
    ntype *dim;
    ntype *shape;
    ntype *type;
    ntype *res;

    DBUG_ENTER ();

    dim = TYgetProductMember (args, 0);
    shape = TYgetProductMember (args, 1);
    type = TYgetProductMember (args, 2);

    if (TUshapeKnown (type)) {
        res = TYcopyType (type);
    } else if (TYisAKV (shape)) {
        res = TYmakeAKS (TYcopyType (TYgetScalar (type)),
                         COconstant2Shape (TYgetValue (shape)));
    } else if (TYisAKV (dim)) {
        res = TYmakeAKD (TYcopyType (TYgetScalar (type)),
                         *((int *)COgetDataVec (TYgetValue (dim))), SHmakeShape (0));
    } else {
        res = TYcopyType (type);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_dim_A( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_dim_A (te_info *info, ntype *args)
{
    ntype *array;
    ntype *res;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "dim called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    if (!(TYisUser (TYgetScalar (array))
          && UTisNested (TYgetUserType (TYgetScalar (array))))) {
        TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    }
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array) || TYisAKS (array) || TYisAKD (array)) {
            res = TYmakeAKV (TYmakeSimpleType (T_int),
                             COmakeConstantFromInt (TYgetDim (array)));
        } else {
            res = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_shape_A( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_shape_A (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res = NULL;
    shape *shp;
    char *err_msg;

    int n;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "shape called with incorrect number of arguments");

    arg = TYgetProductMember (args, 0);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        switch (TYgetConstr (arg)) {
        case TC_akv:
        case TC_aks:
            shp = TYgetShape (arg);
            res = TYmakeAKV (TYmakeSimpleType (T_int), COmakeConstantFromShape (shp));
            break;
        case TC_akd:
            n = TYgetDim (arg);
            res = TYmakeAKS (TYmakeSimpleType (T_int), SHcreateShape (1, n));
            break;
        case TC_audgz:
        case TC_aud:
            res = TYmakeAKD (TYmakeSimpleType (T_int), 1, SHmakeShape (0));
            break;
        default:
            DBUG_ASSERT (FALSE, "NTCCTprf_shape_A applied to non-array type");
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_reshape_VxA( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_reshape_VxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *new_shp, *array, *scalar;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "reshape called with incorrect number of arguments");

    new_shp = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), new_shp);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureNonNegativeValues_V (TEprfArg2Obj (TEgetNameStr (info), 1), new_shp);
        TEassureProdValMatchesProdShape (TEprfArg2Obj (TEgetNameStr (info), 1), new_shp,
                                         TEarg2Obj (2), array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            scalar = TYgetScalar (array);

            switch (TYgetConstr (new_shp)) {
            case TC_akv:
                if (TYgetConstr (array) == TC_akv) {
                    res = TYmakeAKV (TYcopyType (TYgetScalar (array)),
                                     ApplyCF (info, args));
                } else {
                    res = TYmakeAKS (TYcopyType (scalar),
                                     COconstant2Shape (TYgetValue (new_shp)));
                }
                break;
            case TC_aks:
                if (SHgetUnrLen (TYgetShape (new_shp)) == 0) {
                    res = TYmakeAKS (TYcopyType (scalar), SHmakeShape (0));

                } else {
                    res = TYmakeAKD (TYcopyType (scalar),
                                     SHgetExtent (TYgetShape (new_shp), 0),
                                     SHmakeShape (0));
                }
                break;
            case TC_akd:
            case TC_audgz:
            case TC_aud:
                res = TYmakeAUD (TYcopyType (scalar));
                break;
            default:
                DBUG_ASSERT (FALSE, "NTCPRF_reshape_VxA applied to non-array type");
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_sel_VxA( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_sel_VxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "selS called with incorrect number of arguments");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureShpMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 1), idx, TEarg2Obj (2),
                               array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            TEassureValMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 1), idx,
                                     TEarg2Obj (2), array);
            err_msg = TEfetchErrors ();
            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
            } else {
                if (TYisAKV (idx) && TYisAKV (array)) {
                    res = TYmakeAKV (TYcopyType (TYgetScalar (array)),
                                     ApplyCF (info, args));
                } else {
                    res = TYmakeAKS (TYcopyType (TYgetScalar (array)), SHmakeShape (0));
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_sel_VxIA( te_info *info, ntype *args)
 *
 * description:
 *
 * TODO: Implement properly!
 ******************************************************************************/

ntype *
NTCCTprf_sel_VxIA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "selIS called with incorrect number of arguments");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureShpMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 1), idx, TEarg2Obj (2),
                               array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            printf ("ERROR!\n");
            res = TYmakeBottomType (err_msg);
        } else {

            TEassureValMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 1), idx,
                                     TEarg2Obj (2), array);
            err_msg = TEfetchErrors ();
            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
            } else {
                if (TYisAKV (idx) && TYisAKV (array)) {
                    printf ("BOTH AKS\n");
                    res = TYmakeAKV (TYcopyType (TYgetScalar (array)),
                                     ApplyCF (info, args));

                } else {
                    printf ("NOT BOTH AKS\n");
                    res = TYmakeAKS (TYcopyType (TYgetScalar (array)), SHmakeShape (0));
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_idx_selS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_idx_selS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "selS called with incorrect number of arguments");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        TEassureIdxMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 1), idx,
                                 TEarg2Obj (2), array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {
            if (TYisAKV (idx) && TYisAKV (array)) {
                res = TYmakeAKV (TYcopyType (TYgetScalar (array)), ApplyCF (info, args));
            } else {
                res = TYmakeAKS (TYcopyType (TYgetScalar (array)), SHmakeShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_shape_sel( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_shape_sel (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "selS called with incorrect number of arguments");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntVectLengthOne (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        TEassureValMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 1), idx, TEarg2Obj (2),
                               array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {
            if (TYisAKV (idx) && (TYisAKS (array) || TYisAKV (array))) {
                int i = ((int *)COgetDataVec (TYgetValue (idx)))[0];
                res = TYmakeAKV (TYmakeSimpleType (T_int),
                                 COmakeConstantFromInt (
                                   SHgetExtent (TYgetShape (array), i)));
            } else {
                res = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_idx_shape_sel( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_idx_shape_sel (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "selS called with incorrect number of arguments");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        TEassureValMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 1), idx, TEarg2Obj (2),
                               array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {
            if (TYisAKV (idx) && (TYisAKS (array) || TYisAKV (array))) {
                int i = ((int *)COgetDataVec (TYgetValue (idx)))[0];
                res = TYmakeAKV (TYmakeSimpleType (T_int),
                                 COmakeConstantFromInt (
                                   SHgetExtent (TYgetShape (array), i)));
            } else {
                res = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_modarray_AxSxS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_modarray_AxSxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array, *val;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 3,
                 "modarray_AxSxS called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    val = TYgetProductMember (args, 2);

    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 3), val);
    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 3), idx);
    TEassureSameScalarType (TEarg2Obj (1), array, TEprfArg2Obj (TEgetNameStr (info), 3),
                            val);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        if (TYisAKV (array)) {
            if (TYisAKV (idx) && TYisAKV (val)) {
                res = TYmakeAKV (TYcopyType (TYgetScalar (array)), ApplyCF (info, args));
            } else {
                res = TYmakeAKS (TYcopyType (TYgetScalar (array)),
                                 SHcopyShape (TYgetShape (array)));
            }
        } else {
            res = TYcopyType (array);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_modarray_AxVxS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_modarray_AxVxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array, *val;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 3,
                 "modarrayS called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    val = TYgetProductMember (args, 2);

    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 3), val);
    TEassureSameScalarType (TEarg2Obj (1), array, TEprfArg2Obj (TEgetNameStr (info), 3),
                            val);
    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 2), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureShpMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 2), idx, TEarg2Obj (1),
                               array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            TEassureValMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 2), idx,
                                     TEarg2Obj (1), array);
            err_msg = TEfetchErrors ();
            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
            } else {

                if (TYisAKV (array)) {
                    if (TYisAKV (idx) && TYisAKV (val)) {
                        res = TYmakeAKV (TYcopyType (TYgetScalar (array)),
                                         ApplyCF (info, args));
                    } else {
                        res = TYmakeAKS (TYcopyType (TYgetScalar (array)),
                                         SHcopyShape (TYgetShape (array)));
                    }
                } else {
                    res = TYcopyType (array);
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_modarray_AxVxA( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_modarray_AxVxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array, *val;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 3,
                 "modarrayA called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    val = TYgetProductMember (args, 2);

    TEassureSameScalarType (TEarg2Obj (1), array, TEprfArg2Obj (TEgetNameStr (info), 3),
                            val);
    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureShpPlusDimMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 2), idx,
                                      TEarg2Obj (3), val, TEanotherArg2Obj (1), array);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            TEassureShpIsPostfixOfShp (TEprfArg2Obj (TEgetNameStr (info), 3), val,
                                       TEarg2Obj (1), array);
            TEassureValMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 2), idx,
                                     TEarg2Obj (1), array);
            err_msg = TEfetchErrors ();
            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
            } else {

                if (TYisAKV (array)) {
                    if (TYisAKV (idx) && TYisAKV (val)) {
                        res = TYmakeAKV (TYcopyType (TYgetScalar (array)),
                                         ApplyCF (info, args));
                    } else {
                        res = TYmakeAKS (TYcopyType (TYgetScalar (array)),
                                         SHcopyShape (TYgetShape (array)));
                    }
                } else {
                    res = TYcopyType (array);
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_modarray_AxSxA( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_modarray_AxSxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array, *val;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 3,
                 "modarrayA called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    val = TYgetProductMember (args, 2);

    TEassureSameScalarType (TEarg2Obj (1), array, TEprfArg2Obj (TEgetNameStr (info), 3),
                            val);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
#ifdef FIXME // This has to be fixed, but I'm not sure what should
             // be here...
        TEassureShpPlusDimMatchesDim (TEprfArg2Obj (TEgetNameStr (info), 2), idx,
                                      TEarg2Obj (3), val, TEanotherArg2Obj (1), array);
#endif // FIXME
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            TEassureShpIsPostfixOfShp (TEprfArg2Obj (TEgetNameStr (info), 3), val,
                                       TEarg2Obj (1), array);
#ifdef FIXME // ditto.
            TEassureValMatchesShape (TEprfArg2Obj (TEgetNameStr (info), 2), idx,
                                     TEarg2Obj (1), array);
#endif // FIXME
            err_msg = TEfetchErrors ();
            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
            } else {

                if (TYisAKV (array)) {
                    if (TYisAKV (idx) && TYisAKV (val)) {
                        res = TYmakeAKV (TYcopyType (TYgetScalar (array)),
                                         ApplyCF (info, args));
                    } else {
                        res = TYmakeAKS (TYcopyType (TYgetScalar (array)),
                                         SHcopyShape (TYgetShape (array)));
                    }
                } else {
                    res = TYcopyType (array);
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

static ntype *
ConvS (te_info *info, ntype *args, simpletype st)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "ConvS called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYmakeSimpleType (st), ApplyCF (info, args));
        } else {
            res = TYmakeAKS (TYmakeSimpleType (st), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tob_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tob_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tob_S_ is applied to
 *   @return       the result type of applying _tob_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tob_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_byte);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tos_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tos_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tos_S_ is applied to
 *   @return       the result type of applying _tos_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tos_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_short);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toi_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toi_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toi_S_ is applied to
 *   @return       the result type of applying _toi_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toi_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_int);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tol_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tol_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tol_S_ is applied to
 *   @return       the result type of applying _tol_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tol_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_long);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toll_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toll_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toll_S_ is applied to
 *   @return       the result type of applying _toll_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toll_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_longlong);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toub_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toub_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toub_S_ is applied to
 *   @return       the result type of applying _toub_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toub_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_ubyte);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tous_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tous_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tous_S_ is applied to
 *   @return       the result type of applying _tous_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tous_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_ushort);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toui_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toui_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toui_S_ is applied to
 *   @return       the result type of applying _toui_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toui_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_uint);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toul_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toul_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toul_S_ is applied to
 *   @return       the result type of applying _toul_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toul_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_ulong);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toull_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toull_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toull_S_ is applied to
 *   @return       the result type of applying _toull_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toull_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_ulonglong);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tof_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tof_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tof_S_ is applied to
 *   @return       the result type of applying _tof_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tof_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_float);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tod_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tod_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tod_S_ is applied to
 *   @return       the result type of applying _tod_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tod_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_double);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tobool_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tobool_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tobool_S_ is applied to
 *   @return       the result type of applying _tobool_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tobool_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_bool);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toc_S( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toc_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tod_S_ is applied to
 *   @return       the result type of applying _tod_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toc_S (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ();

    res = ConvS (info, args, T_char);

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_SxS( te_info *info, ntype *args)
 *
 * description:
 *    simple []  x  simple []  ->  simple []
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_SxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureNumS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureNumS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if ((err_msg == NULL) && TEgetPrf (info) == F_div_SxS) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYmakeAKS (TYcopyType (TYgetScalar (array1)), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_SxV( te_info *info, ntype *args)
 *
 * description:
 *    simple []  x  simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureNumS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureNumV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if ((err_msg == NULL) && TEgetPrf (info) == F_div_SxV) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYeliminateAKV (array2);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_VxS( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  x  simple []  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_VxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_VxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureNumV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureNumS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if ((err_msg == NULL) && TEgetPrf (info) == F_div_VxS) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYeliminateAKV (array1);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_VxV( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  x  simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_VxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_VxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureNumV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureNumV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    res = TEassureSameShape (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                             array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if ((err_msg == NULL) && TEgetPrf (info) == F_div_VxV) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYfreeType (res);
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_S( te_info *info, ntype *args)
 *
 * description:
 *    simple  ->  simple
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_S (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "ari_op_A called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureNumS (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array)), ApplyCF (info, args));
        } else {
            res = TYcopyType (array);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_V( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_V (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "ari_op_A called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureNumV (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array)), ApplyCF (info, args));
        } else {
            res = TYcopyType (array);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_A( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_A (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "ari_op_A called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array)), ApplyCF (info, args));
        } else {
            res = TYcopyType (array);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_rel_op_SxS( te_info *info, ntype *args)
 *
 * description:
 *     simple [shp]  x  simple [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_rel_op_SxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "rel_op_SxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_rel_op_SxV( te_info *info, ntype *args)
 *
 * description:
 *     simple [shp]  x  simple [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_rel_op_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "rel_op_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYeliminateAKV (array2);
            res = TYsetScalar (res, TYmakeSimpleType (T_bool));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_rel_op_VxS( te_info *info, ntype *args)
 *
 * description:
 *     simple [shp]  x  simple [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_rel_op_VxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "rel_op_VxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYeliminateAKV (array1);
            res = TYsetScalar (res, TYmakeSimpleType (T_bool));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_rel_op_VxV( te_info *info, ntype *args)
 *
 * description:
 *     simple [shp]  x  simple [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_rel_op_VxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "rel_op_AxA called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    res = TEassureSameShape (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                             array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYfreeType (res);
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYsetScalar (res, TYmakeSimpleType (T_bool));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_log_op_SxS( te_info *info, ntype *args)
 *
 * description:
 *    bool   x  bool   ->  bool
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_SxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "log_op_SxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureBoolS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureBoolS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYmakeAKS (TYmakeSimpleType (T_bool), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_log_op_SxV( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  x  bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "log_op_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureBoolS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureBoolV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYcopyType (array2);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_log_op_VxS( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  x  bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_VxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "log_op_VxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureBoolV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureBoolS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYcopyType (array1);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_log_op_VxV( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  x  bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_VxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "log_op_VxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureBoolV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureBoolV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    res = TEassureSameShape (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                             array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYfreeType (res);
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_log_op_V( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_V (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "log_op_V called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureBoolV (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYcopyType (array);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_log_op_S( te_info *info, ntype *args)
 *
 * description:
 *    bool   ->  bool
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_S (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ();

    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "log_op_S called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureBoolS (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYmakeSimpleType (T_bool), ApplyCF (info, args));
        } else {
            res = TYcopyType (array);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_int_op_SxS( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  int []  ->  int []
 *
 ******************************************************************************/

ntype *
NTCCTprf_int_op_SxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "int_op_SxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if ((err_msg == NULL) && TEgetPrf (info) == F_mod_SxS) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYmakeSimpleType (T_int), ApplyCF (info, args));
        } else {
            res = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_int_op_SxV( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  int []  ->  int []
 *
 ******************************************************************************/

ntype *
NTCCTprf_int_op_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "int_op_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if ((err_msg == NULL) && TEgetPrf (info) == F_mod_SxV) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYeliminateAKV (array2);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_int_op_VxS( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  int []  ->  int []
 *
 ******************************************************************************/

ntype *
NTCCTprf_int_op_VxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "int_op_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if ((err_msg == NULL) && TEgetPrf (info) == F_mod_VxS) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYeliminateAKV (array1);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_int_op_VxV( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  int []  ->  int []
 *
 ******************************************************************************/

ntype *
NTCCTprf_int_op_VxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "int_op_VxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    res = TEassureSameShape (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                             array2);
    err_msg = TEfetchErrors ();
    if ((err_msg == NULL) && TEgetPrf (info) == F_mod_VxV) {
        TEassureValNonZero (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }

    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYfreeType (res);
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_take_SxV( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  simple [.]  ->  simple [.]
 *
 ******************************************************************************/

ntype *
NTCCTprf_take_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    shape *shp;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "take_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureVect (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureAbsValFitsShape (TEarg2Obj (1), array1,
                                 TEprfArg2Obj (TEgetNameStr (info), 2), array2);

        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            if (TYisAKV (array1)) {
                if (TYisAKV (array2)) {
                    res = TYmakeAKV (TYcopyType (TYgetScalar (array2)),
                                     ApplyCF (info, args));
                } else {
                    shp = SHcreateShape (1, abs (((int *)COgetDataVec (
                                              TYgetValue (array1)))[0]));
                    res = TYmakeAKS (TYcopyType (TYgetScalar (array2)), shp);
                }
            } else {
                res = TYmakeAKD (TYcopyType (TYgetScalar (array2)), 1, SHmakeShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_drop_SxV( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  simple [.]  ->  simple [.]
 *
 ******************************************************************************/

ntype *
NTCCTprf_drop_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    shape *shp;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "drop_SxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureVect (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureAbsValFitsShape (TEarg2Obj (1), array1,
                                 TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            if (TYisAKV (array1) && (TYisAKV (array2) || TYisAKS (array2))) {
                if (TYisAKV (array2)) {
                    res = TYmakeAKV (TYcopyType (TYgetScalar (array2)),
                                     ApplyCF (info, args));
                } else {
                    shp = SHcopyShape (TYgetShape (array2));
                    shp = SHsetExtent (shp, 0,
                                       SHgetExtent (shp, 0)
                                         - abs (((int *)COgetDataVec (
                                             TYgetValue (array1)))[0]));
                    res = TYmakeAKS (TYcopyType (TYgetScalar (array2)), shp);
                }
            } else {
                res = TYmakeAKD (TYcopyType (TYgetScalar (array2)), 1, SHmakeShape (0));
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_cat_VxV( te_info *info, ntype *args)
 *
 * description:
 *    simple [.]  x  simple [.]  ->  simple [.]
 *
 ******************************************************************************/

ntype *
NTCCTprf_cat_VxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "cat_VxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureVect (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureVect (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg == NULL) {
        TEassureSameSimpleType (TEarg2Obj (1), array1,
                                TEprfArg2Obj (TEgetNameStr (info), 2), array2);
        err_msg = TEfetchErrors ();
    }
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else if ((TYisAKV (array1) || TYisAKS (array1))
                   && (TYisAKV (array2) || TYisAKS (array2))) {
            res = TYmakeAKS (TYcopyType (TYgetScalar (array1)),
                             SHcreateShape (1, SHgetExtent (TYgetShape (array1), 0)
                                                 + SHgetExtent (TYgetShape (array2), 0)));
        } else {
            res = TYmakeAKD (TYcopyType (TYgetScalar (array1)), 1, SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_mask_VxVxS( te_info *info, ntype *elems)
 *
 * description:
 *    _mask_VxVxV_( p, x, y) masks the values of x and y,
 *     based on the boolean predicate, p. It selects x[i] if
 *     p[i], else y. First two arguments must be the same shape;
 *     y must be a scalar.
 *     This is an internal-use-only primitive, used as part of AWLF.
 *     It does not survive the optimization phase.
 *
 *     The idea below is to make the result have the same type as x,
 *     but without any AKV-ness. This should work, because x and y
 *     should have the same types, although not the same ranks.
 *
 ******************************************************************************/

ntype *
NTCCTprf_mask_VxVxS (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res;

    DBUG_ENTER ();

    arg = TYgetProductMember (args, 1);
    arg = TYeliminateAKV (arg);
    res = TYcopyType (arg);
    res = TYmakeProductType (1, res);

    /* FIXME We should check that x and y are the same shape */

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_mask_VxVxV( te_info *info, ntype *elems)
 *
 * description:
 *    _mask_VxVxV_( p, x, y) masks the values of x and y,
 *     based on the boolean predicate, p. It selects x[i] if
 *     p[i], else y[i]. All arguments must be the same shape.
 *     This is an internal-use-only primitive, used as part of AWLF.
 *     It does not survive the optimization phase.
 *
 *     The idea below is to make the result have the same type as x,
 *     but without any AKV-ness. This should work, because x and y
 *     should have the same types.
 *
 ******************************************************************************/

ntype *
NTCCTprf_mask_VxVxV (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res;

    DBUG_ENTER ();

    arg = TYgetProductMember (args, 1);
    arg = TYeliminateAKV (arg);
    res = TYcopyType (arg);
    res = TYmakeProductType (1, res);

    /* FIXME We should check that all 3 arguments are the same shape */

    DBUG_RETURN (res);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_vect2offset( te_info *info, ntype *args)
 *
 * description:
 *
 *    See COvect2offset for semantics.
 *    We would like to have a way to check for shape(iv)>shape(shp).
 *    But, we do not.
 *
 ******************************************************************************/
ntype *
NTCCTprf_vect2offset (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *shp;
    char *err_msg;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "vect2offset called with incorrect number of arguments");

    shp = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), shp);

    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 2), shp);
    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 2), idx);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        if (TYisAKV (shp) && TYisAKV (idx)) {
            res = TYmakeAKV (TYmakeSimpleType (T_int), ApplyCF (info, args));
        } else {
            res = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_idxs2offset( te_info *info, ntype *args)
 *
 * description:
 *  Any number of arguments.
 *
      TEassureIntS( TEprfArg2Obj( TEgetNameStr( info), 2), idx);
 ******************************************************************************/
ntype *
NTCCTprf_idxs2offset (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *shp;
    int len;
    int i;
    char *err_msg;

    DBUG_ENTER ();

    len = TYgetProductSize (args) - 1;
    shp = TYgetProductMember (args, 0);
    TEassureIntV (TEprfArg2Obj (TEgetNameStr (info), 2), shp);
    TEassureShpMatchesInt (TEprfArg2Obj (TEgetNameStr (info), 2), shp, len);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        for (i = 0; i < len; i++) {
            idx = TYgetProductMember (args, i + 1);
            TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), idx);
        }
    }
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        res = TYmakeAKS (TYmakeSimpleType (T_int), SHmakeShape (0));
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_hideValue_SxA( te_info *info, ntype *args)
 *
 * description:
 *
 *****************************************************************************/

ntype *
NTCCTprf_hideValue_SxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "hideValue called with incorrect number of arguments");

    array = TYgetProductMember (args, 1);

    if (TYisAKV (array)) {
        res = TYmakeAKS (TYcopyType (TYgetScalar (array)),
                         SHcopyShape (TYgetShape (array)));
    } else {
        res = TYcopyType (array);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_hideShape_SxA( te_info *info, ntype *args)
 *
 * description:
 *
 *****************************************************************************/

ntype *
NTCCTprf_hideShape_SxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "hideShape called with incorrect number of arguments");

    array = TYgetProductMember (args, 1);

    if (TUisScalar (array)) {
        res = TYmakeAKS (TYcopyType (TYgetScalar (array)),
                         SHcopyShape (TYgetShape (array)));
    } else if (TUshapeKnown (array)) {
        res = TYmakeAKD (TYcopyType (TYgetScalar (array)), SHgetDim (TYgetShape (array)),
                         SHmakeShape (0));
    } else {
        res = TYcopyType (array);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_hideDim_SxA( te_info *info, ntype *args)
 *
 * description:
 *
 *****************************************************************************/

ntype *
NTCCTprf_hideDim_SxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;

    DBUG_ENTER ();
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "hideDim called with incorrect number of arguments");

    array = TYgetProductMember (args, 1);

    res = TYmakeAUD (TYcopyType (TYgetScalar (array)));

    DBUG_RETURN (TYmakeProductType (1, res));
}

#undef DBUG_PREFIX
