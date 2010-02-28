/*
 * $Id$
 *
 */

#include "ct_with.h"
#include "dbug.h"
#include "type_errors.h"
#include "new_types.h"
#include "constants.h"
#include "shape.h"

/******************************************************************************
 ***
 ***          Local helper functions:
 ***          -----------------------
 ***
 ******************************************************************************/

static ntype *
Idx2Outer (ntype *idx)
{
    ntype *scalar;
    ntype *res;

    DBUG_ENTER ("Idx2Outer");

    scalar = TYgetScalar (idx);
    switch (TYgetConstr (idx)) {
    case TC_akv:
        res = TYmakeAKS (TYcopyType (scalar), COconstant2Shape (TYgetValue (idx)));
        break;
    case TC_aks:
        if (SHgetExtent (TYgetShape (idx), 0) == 0) {
            res = TYmakeAKS (TYcopyType (scalar), SHmakeShape (0));
        } else {
            res = TYmakeAKD (TYcopyType (scalar), SHgetExtent (TYgetShape (idx), 0),
                             SHmakeShape (0));
        }
        break;
    case TC_akd:
    case TC_audgz:
    case TC_aud:
        res = TYmakeAUD (TYcopyType (scalar));
        break;
    default:
        DBUG_ASSERT (FALSE, "Idx2Outer applied to non-array type idx");
        res = NULL; /* just to please gcc 8-) */
    }

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
 *    ntype *NTCCTwl_idx( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_idx (te_info *info, ntype *args)
{
    ntype *lb, *idx, *ub, *sv, *wv, *res;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_idx");

    lb = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    ub = TYgetProductMember (args, 2);

    TEassureIntV ("lower bound of with loop", lb);
    TEassureIntV ("upper bound of with loop", ub);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        res = TEassureSameShape ("lower bound", lb, "upper bound of with loop", ub);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            res = TEassureSameShape ("index variables", idx, "generator boundaries", res);
            err_msg = TEfetchErrors ();
            if (err_msg != NULL) {
                res = TYmakeBottomType (err_msg);
            } else {

                if (TYgetProductSize (args) >= 4) {
                    sv = TYgetProductMember (args, 3);
                    TEassureIntV ("step vector of with loop", sv);
                    err_msg = TEfetchErrors ();
                    if (err_msg != NULL) {
                        res = TYmakeBottomType (err_msg);
                    } else {

                        res = TEassureSameShape ("step vector", sv,
                                                 "generator boundaries", res);
                        err_msg = TEfetchErrors ();
                        if (err_msg != NULL) {
                            res = TYmakeBottomType (err_msg);
                        } else {

                            if (TYgetProductSize (args) == 5) {
                                wv = TYgetProductMember (args, 4);
                                TEassureIntV ("width vector of with loop", wv);
                                err_msg = TEfetchErrors ();
                                if (err_msg != NULL) {
                                    res = TYmakeBottomType (err_msg);
                                } else {

                                    res = TEassureSameShape ("width vector", wv,
                                                             "generator boundaries", res);
                                    err_msg = TEfetchErrors ();
                                    if (err_msg != NULL) {
                                        res = TYmakeBottomType (err_msg);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTwl_multipart( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_multipart (te_info *info, ntype *args)
{
    ntype *expr1, *expr2, *res;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_multipart");

    expr1 = TYgetProductMember (args, 0);
    expr2 = TYgetProductMember (args, 1);

    res = TEassureSameShape ("one generator index", expr1, "another generator index",
                             expr2);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTwl_multicode( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_multicode (te_info *info, ntype *args)
{
    ntype *expr1, *expr2, *res;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_multicode");

    expr1 = TYgetProductMember (args, 0);
    expr2 = TYgetProductMember (args, 1);

    TEassureSameScalarType ("one generator body expression", expr1,
                            "another generator body expression", expr2);

    res = TEassureSameShape ("one generator body expression", expr1,
                             "another generator body expression", expr2);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTwl_multifoldcode( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_multifoldcode (te_info *info, ntype *args)
{
    ntype *expr1, *expr2, *res;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_multifoldcode");

    expr1 = TYgetProductMember (args, 0);
    expr2 = TYgetProductMember (args, 1);

    TEassureSameScalarType ("one generator body expression", expr1,
                            "another generator body expression", expr2);

    res = TYlubOfTypes (expr1, expr2);

    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTwl_gen( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_gen (te_info *info, ntype *args)
{
    ntype *idx, *shp, *expr, *dexpr, *res;
    ntype *dummy;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_gen");

    idx = TYgetProductMember (args, 0);
    shp = TYgetProductMember (args, 1);
    expr = TYgetProductMember (args, 2);
    dexpr = TYgetProductMember (args, 3);

    idx = TEassureSameShape ("shape expression", shp,
                             "generator boundaries of genarray with loop", idx);

    TEassureSameScalarType ("body expression", expr, "default expression", dexpr);
    expr = TEassureSameShape ("body expression", expr, "default expression", dexpr);
    TEassureIntV ("shape expression of genarray with loop", shp);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        TEassureNonNegativeValues_V ("shape expression of genarray with loop", shp);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        } else {

            if (TYgetConstr (shp) == TC_akv) {
                dummy = Idx2Outer (shp);
            } else {
                dummy = Idx2Outer (idx);
            }
            res = TYnestTypes (dummy, expr);
            TYfreeType (dummy);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTwl_mod( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_mod (te_info *info, ntype *args)
{
    ntype *idx, *array, *expr, *res;
    ntype *dummy;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_mod");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);
    expr = TYgetProductMember (args, 2);

    dummy = Idx2Outer (idx);
    res = TYnestTypes (dummy, expr);
    TYfreeType (dummy);

    TEassureIntV ("index expression of modarray with loop", idx);
    TEassureSameScalarType ("array to be modified", array, "body expression", expr);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        res = TEassureSameShape ("array expression", array,
                                 "result of modarray with loop", res);
        err_msg = TEfetchErrors ();
        if (err_msg != NULL) {
            res = TYmakeBottomType (err_msg);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTwl_fold( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTwl_fold (te_info *info, ntype *args)
{
    ntype *idx, *neutr, *expr, *res;
    char *err_msg;

    DBUG_ENTER ("NTCCTwl_foldfun");

    idx = TYgetProductMember (args, 0);
    neutr = TYgetProductMember (args, 1);
    expr = TYgetProductMember (args, 2);

    TEassureIntV ("index expression of fold with loop", idx);
    TEassureSameScalarType ("neutral element", neutr, "body expression of fold with loop",
                            expr);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {
        res = TYlubOfTypes (neutr, expr);
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}
