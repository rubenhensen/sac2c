/*
 * $Log$
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

#include "dbug.h"
#include "ct_with.h"
#include "type_errors.h"

/******************************************************************************
 ***
 ***          Local helper functions:
 ***          -----------------------
 ***
 ******************************************************************************/

ntype *
Idx2Outer (ntype *idx)
{
    ntype *scalar;
    ntype *res;

    DBUG_ENTER ("Idx2Outer");

    scalar = TYGetScalar (idx);
    switch (TYGetConstr (idx)) {
    case TC_akv:
        res = TYMakeAKS (TYCopyType (scalar), COConstant2Shape (TYGetValue (idx)));
        break;
    case TC_aks:
        if (SHGetExtent (TYGetShape (idx), 0) == 0) {
            res = TYMakeAKS (TYCopyType (scalar), SHMakeShape (0));
        } else {
            res = TYMakeAKD (TYCopyType (scalar), SHGetExtent (TYGetShape (idx), 0),
                             SHMakeShape (0));
        }
        break;
    case TC_akd:
    case TC_audgz:
    case TC_aud:
        res = TYMakeAUD (TYCopyType (scalar));
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
 *    ntype *NTCWL_idx( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCWL_idx (te_info *info, ntype *args)
{
    ntype *lb, *idx, *ub, *sv, *wv, *res;

    DBUG_ENTER ("NTCWL_idx");

    lb = TYGetProductMember (args, 0);
    idx = TYGetProductMember (args, 1);
    ub = TYGetProductMember (args, 2);

    TEAssureIntVect ("lower bound of with loop", lb);
    TEAssureIntVect ("upper bound of with loop", ub);
    res = TEAssureSameShape ("lower bound", lb, "upper bound of with loop", ub);
    res = TEAssureSameShape ("index variables", idx, "generator boundaries", res);

    if (TYGetProductSize (args) >= 4) {
        sv = TYGetProductMember (args, 3);
        TEAssureIntVect ("step vector of with loop", sv);
        res = TEAssureSameShape ("step vector", sv, "generator boundaries", res);

        if (TYGetProductSize (args) == 5) {
            wv = TYGetProductMember (args, 4);
            TEAssureIntVect ("width vector of with loop", wv);
            res = TEAssureSameShape ("width vector", wv, "generator boundaries", res);
        }
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCWL_gen( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCWL_gen (te_info *info, ntype *args)
{
    ntype *idx, *shp, *expr, *dexpr, *res;
    ntype *dummy;

    DBUG_ENTER ("NTCWL_gen");

    idx = TYGetProductMember (args, 0);
    shp = TYGetProductMember (args, 1);
    expr = TYGetProductMember (args, 2);
    dexpr = TYGetProductMember (args, 3);

    TEAssureIntVect ("shape expression of genarray with loop", shp);
    TEAssureNonNegativeValues ("shape expression of genarray with loop", shp);
    idx = TEAssureSameShape ("shape expression", shp,
                             "generator boundaries of genarray with loop", idx);

    TEAssureSameScalarType ("body expression", expr, "default expression", dexpr);
    expr = TEAssureSameShape ("body expression", expr, "default expression", dexpr);

    if (TYGetConstr (shp) == TC_akv) {
        dummy = Idx2Outer (shp);
    } else {
        dummy = Idx2Outer (idx);
    }
    res = TYNestTypes (dummy, expr);
    TYFreeType (dummy);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCWL_mod( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCWL_mod (te_info *info, ntype *args)
{
    ntype *idx, *array, *expr, *res;
    ntype *dummy;

    DBUG_ENTER ("NTCWL_mod");

    idx = TYGetProductMember (args, 0);
    array = TYGetProductMember (args, 1);
    expr = TYGetProductMember (args, 2);

    dummy = Idx2Outer (idx);
    res = TYNestTypes (dummy, expr);
    TYFreeType (dummy);

    res = TEAssureSameShape ("array expression", array, "result of modarray with loop",
                             res);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCWL_fold( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCWL_fold (te_info *info, ntype *args)
{
    ntype *neutr, *expr, *res;

    DBUG_ENTER ("NTCWL_foldfun");

    neutr = TYGetProductMember (args, 0);
    expr = TYGetProductMember (args, 1);

    TEAssureSameSimpleType ("neutral element", neutr, "body expression of fold with loop",
                            expr);
    res = TYLubOfTypes (neutr, expr);

    DBUG_RETURN (TYMakeProductType (1, res));
}
