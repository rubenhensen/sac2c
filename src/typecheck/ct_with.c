/*
 * $Log$
 * Revision 1.8  2004/11/24 17:42:07  sbs
 * compiles
 *
 * Revision 1.7  2004/03/05 12:08:00  sbs
 * avoided the creation of AKD of dimensionality 0.
 *
 * Revision 1.6  2003/12/02 09:53:22  sbs
 * genarray Wls with non-negative entries will be rejected by the TC now!
 *
 * Revision 1.5  2003/11/26 14:22:44  sbs
 * default value of new genarray WLs now is checked as well.
 *
 * Revision 1.4  2003/04/11 17:55:59  sbs
 * COConstant2Shape used in Idx2Outer.
 *
 * Revision 1.3  2003/04/07 14:33:41  sbs
 * genarray variant and Idx2Outer extended for AKV types.
 *
 * Revision 1.2  2002/08/06 08:26:49  sbs
 * some vars initialized to please gcc for the product version.
 *
 * Revision 1.1  2002/08/05 16:57:53  sbs
 * Initial revision
 *
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

    DBUG_ENTER ("NTCCTwl_idx");

    lb = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    ub = TYgetProductMember (args, 2);

    TEassureIntVect ("lower bound of with loop", lb);
    TEassureIntVect ("upper bound of with loop", ub);
    res = TEassureSameShape ("lower bound", lb, "upper bound of with loop", ub);
    res = TEassureSameShape ("index variables", idx, "generator boundaries", res);

    if (TYgetProductSize (args) >= 4) {
        sv = TYgetProductMember (args, 3);
        TEassureIntVect ("step vector of with loop", sv);
        res = TEassureSameShape ("step vector", sv, "generator boundaries", res);

        if (TYgetProductSize (args) == 5) {
            wv = TYgetProductMember (args, 4);
            TEassureIntVect ("width vector of with loop", wv);
            res = TEassureSameShape ("width vector", wv, "generator boundaries", res);
        }
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

    DBUG_ENTER ("NTCCTwl_gen");

    idx = TYgetProductMember (args, 0);
    shp = TYgetProductMember (args, 1);
    expr = TYgetProductMember (args, 2);
    dexpr = TYgetProductMember (args, 3);

    TEassureIntVect ("shape expression of genarray with loop", shp);
    TEassureNonNegativeValues ("shape expression of genarray with loop", shp);
    idx = TEassureSameShape ("shape expression", shp,
                             "generator boundaries of genarray with loop", idx);

    TEassureSameScalarType ("body expression", expr, "default expression", dexpr);
    expr = TEassureSameShape ("body expression", expr, "default expression", dexpr);

    if (TYgetConstr (shp) == TC_akv) {
        dummy = Idx2Outer (shp);
    } else {
        dummy = Idx2Outer (idx);
    }
    res = TYnestTypes (dummy, expr);
    TYfreeType (dummy);

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

    DBUG_ENTER ("NTCCTwl_mod");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);
    expr = TYgetProductMember (args, 2);

    dummy = Idx2Outer (idx);
    res = TYnestTypes (dummy, expr);
    TYfreeType (dummy);

    res = TEassureSameShape ("array expression", array, "result of modarray with loop",
                             res);

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
    ntype *neutr, *expr, *res;

    DBUG_ENTER ("NTCCTwl_foldfun");

    neutr = TYgetProductMember (args, 0);
    expr = TYgetProductMember (args, 1);

    TEassureSameSimpleType ("neutral element", neutr, "body expression of fold with loop",
                            expr);
    res = TYlubOfTypes (neutr, expr);

    DBUG_RETURN (TYmakeProductType (1, res));
}
