/*
 * $Log$
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
    case TC_aks:
        res = TYMakeAKD (TYCopyType (scalar), SHGetExtent (TYGetShape (idx), 0),
                         SHMakeShape (0));
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
    ntype *idx, *shp, *expr, *res;
    ntype *dummy;

    DBUG_ENTER ("NTCWL_gen");

    idx = TYGetProductMember (args, 0);
    shp = TYGetProductMember (args, 1);
    expr = TYGetProductMember (args, 2);

    TEAssureIntVect ("shape expression of genarray with loop", shp);
    idx = TEAssureSameShape ("shape expression", shp,
                             "generator boundaries of genarray with loop", idx);

    dummy = Idx2Outer (idx);
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
