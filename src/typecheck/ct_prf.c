/*
 *
 * $Log$
 * Revision 1.6  2002/10/10 12:17:51  sbs
 * PRF_IF macro definition adjusted to changes made in prf_node_info.mac
 *
 * Revision 1.5  2002/09/25 11:38:26  sbs
 * some minor warnings eliminated
 *
 * Revision 1.4  2002/09/11 23:18:22  dkr
 * prf_node_info.mac modified
 *
 * Revision 1.3  2002/09/04 12:59:46  sbs
 * type checking of arrays changed; now sig deps will be created as well.
 *
 * Revision 1.2  2002/08/07 09:49:58  sbs
 * modulo added
 *
 * Revision 1.1  2002/08/05 16:57:50  sbs
 * Initial revision
 *
 */

#include "dbug.h"
#include "ct_prf.h"
#include "type_errors.h"

ct_funptr NTCPRF_funtab[] = {
#define PRF_IF(a, b, c, d, e, f, g) g
#include "prf_node_info.mac"
#undef PRF_IF
};

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
NTCPRF_dummy (te_info *info, ntype *args)
{
    DBUG_ENTER ("NTCPRF_dummy");
    DBUG_ASSERT (FALSE, "prf not yet implemented");
    DBUG_RETURN (args);
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_array( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_array (te_info *info, ntype *elems)
{
    ntype *elem, *elem2, *res;
    shape *shp;
    int num_elems;
    int i;

    DBUG_ENTER ("NTCPRF_array");

    elem = TYCopyType (TYGetProductMember (elems, 0));
    num_elems = TYGetProductSize (elems);

    for (i = 1; i < num_elems; i++) {
        elem2 = TYGetProductMember (elems, i);
        TEAssureSameScalarType ("array element #0", elem, TEArrayElem2Obj (i), elem2);
        elem2 = TEAssureSameShape ("array element #0", elem, TEArrayElem2Obj (i), elem2);
        TYFreeType (elem);
        elem = elem2;
    }

    switch (TYGetConstr (elem)) {
    case TC_aks:
        shp = SHCreateShape (1, num_elems);
        res = TYMakeAKS (TYGetScalar (elem), SHAppendShapes (shp, TYGetShape (elem)));
        SHFreeShape (shp);
        break;
    case TC_akd:
        res = TYMakeAKD (TYGetScalar (elem), TYGetDim (elem) + 1, SHMakeShape (0));
        break;
    case TC_audgz:
    case TC_aud:
        res = TYMakeAUDGZ (TYGetScalar (elem));
        break;
    default:
        DBUG_ASSERT ((FALSE), "array elements of non array types not yet supported");
        res = NULL; /* just to please gcc */
    }

    TYFreeTypeConstructor (elem);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_dim( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_dim (te_info *info, ntype *args)
{
    ntype *res;

    DBUG_ENTER ("NTCPRF_dim");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "dim called with incorrect number of arguments");

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1),
                        TYGetProductMember (args, 0));

    res = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_shape( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_shape (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res = NULL;
    int n;

    DBUG_ENTER ("NTCPRF_shape");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "shape called with incorrect number of arguments");

    arg = TYGetProductMember (args, 0);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), arg);

    switch (TYGetConstr (arg)) {
    case TC_aks:
    case TC_akd:
        n = TYGetDim (arg);
        res = TYMakeAKS (TYMakeSimpleType (T_int), SHCreateShape (1, n));
        break;
    case TC_audgz:
    case TC_aud:
        res = TYMakeAKD (TYMakeSimpleType (T_int), 1, SHMakeShape (0));
        break;
    default:
        DBUG_ASSERT (FALSE, "NTCPRF_shape applied to non-array type");
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_reshape( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_reshape (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *new_shp, *array, *scalar;

    DBUG_ENTER ("NTCPRF_reshape");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "shape called with incorrect number of arguments");

    new_shp = TYGetProductMember (args, 0);
    array = TYGetProductMember (args, 1);

    TEAssureIntVect (TEPrfArg2Obj (TEGetNameStr (info), 1), new_shp);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array);

    scalar = TYGetScalar (array);

    switch (TYGetConstr (new_shp)) {
    case TC_aks:
        res = TYMakeAKD (TYCopyType (scalar), SHGetExtent (TYGetShape (new_shp), 0),
                         SHMakeShape (0));
        break;
    case TC_akd:
    case TC_audgz:
    case TC_aud:
        res = TYMakeAUD (TYCopyType (scalar));
        break;
    default:
        DBUG_ASSERT (FALSE, "NTCPRF_reshape applied to non-array type");
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_selS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_selS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;

    DBUG_ENTER ("NTCPRF_selS");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "selS called with incorrect number of arguments");

    idx = TYGetProductMember (args, 0);
    array = TYGetProductMember (args, 1);

    TEAssureIntVect (TEPrfArg2Obj (TEGetNameStr (info), 1), idx);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array);
    TEAssureShpMatchesDim (TEPrfArg2Obj (TEGetNameStr (info), 1), idx, TEArg2Obj (2),
                           array);

    res = TYMakeAKS (TYCopyType (TYGetScalar (array)), SHMakeShape (0));

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_modarrayS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_modarrayS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array, *val;

    DBUG_ENTER ("NTCPRF_modarrayS");
    DBUG_ASSERT (TYGetProductSize (args) == 3,
                 "modarrayS called with incorrect number of arguments");

    array = TYGetProductMember (args, 0);
    idx = TYGetProductMember (args, 1);
    val = TYGetProductMember (args, 2);

    TEAssureIntVect (TEPrfArg2Obj (TEGetNameStr (info), 1), idx);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array);
    TEAssureShpMatchesDim (TEPrfArg2Obj (TEGetNameStr (info), 2), idx, TEArg2Obj (1),
                           array);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 3), val);
    TEAssureScalar (TEPrfArg2Obj (TEGetNameStr (info), 3), val);
    TEAssureSameSimpleType (TEArg2Obj (2), idx, TEPrfArg2Obj (TEGetNameStr (info), 3),
                            val);

    res = TYCopyType (array);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_ari_op_SxS( te_info *info, ntype *args)
 *
 * description:
 *    simple []  x  simple []  ->  simple []
 *
 ******************************************************************************/

ntype *
NTCPRF_ari_op_SxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_ari_op_SxA");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "ari_op_SxA called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureSameSimpleType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    TEAssureScalar (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureScalar (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);

    res = TYMakeAKS (TYCopyType (TYGetScalar (array1)), SHMakeShape (0));

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_ari_op_SxA( te_info *info, ntype *args)
 *
 * description:
 *    simple []  x  simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_ari_op_SxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_ari_op_SxA");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "ari_op_SxA called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureSameSimpleType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    TEAssureScalar (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);

    res = TYCopyType (array2);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_ari_op_AxS( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  x  simple []  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_ari_op_AxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_ari_op_AxS");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "ari_op_AxS called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureSameSimpleType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    TEAssureScalar (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);

    res = TYCopyType (array1);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_ari_op_AxA( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  x  simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_ari_op_AxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_ari_op_AxA");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "ari_op_AxA called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureSameSimpleType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    res = TEAssureSameShape (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                             array2);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_rel_op_AxA( te_info *info, ntype *args)
 *
 * description:
 *     simple [shp]  x  simple [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_rel_op_AxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_rel_op_AxA");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "rel_op_AxA called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureSameSimpleType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    res = TEAssureSameShape (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                             array2);

    res = TYSetScalar (res, TYMakeSimpleType (T_bool));

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_log_op_AxA( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  x  bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_log_op_AxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_log_op_AxA");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "log_op_AxA called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureBoolA (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureBoolA (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    res = TEAssureSameShape (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                             array2);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_int_op_SxS( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  int []  ->  int []
 *
 ******************************************************************************/

ntype *
NTCPRF_int_op_SxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_int_op_SxS");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "int_op_SxS called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureIntS (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureIntS (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);

    res = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));

    DBUG_RETURN (TYMakeProductType (1, res));
}
