/*
 *
 * $Log$
 * Revision 1.20  2004/02/03 11:25:44  sbs
 * NTCPRF_phi added.
 *
 * Revision 1.19  2003/09/10 09:42:35  sbs
 * NTCPRF_drop_SxV improved /
 * NTCPRF_take_SxV added.
 *
 * Revision 1.18  2003/09/09 14:56:11  sbs
 * extended type error reporting added
 *
 * Revision 1.17  2003/05/27 09:33:02  sbs
 * error in NTCPRF_modarrayS eliminated.
 *
 * Revision 1.16  2003/04/14 10:49:49  sbs
 * no double usage of TEPrfArg2Obj anymore as this fun uses static buffers 8-)
 *
 * Revision 1.15  2003/04/11 17:56:37  sbs
 * implementation of NTCPRF_reshape extended for akv types.
 *
 * Revision 1.14  2003/04/09 15:35:57  sbs
 * NTCPRF_toiS, NTCPRF_toiA, NTCPRF_tofS, NTCPRF_tofA, NTCPRF_todS, NTCPRF_todA,
 * NTCPRF_ari_op_A, NTCPRF_log_op_A added.
 *
 * Revision 1.13  2003/04/08 12:26:19  sbs
 * ApplyCF extended for non-binary functions;
 * modarray now folds as well 8-)
 *
 * Revision 1.12  2003/04/07 14:34:41  sbs
 * (most) type computations extended for AKV types 8-)
 *
 * Revision 1.11  2003/03/19 10:34:30  sbs
 * NTCPRF_drop_SxV and NTCPRF_cat_VxV added.
 *
 * Revision 1.10  2002/11/04 17:40:11  sbs
 * computation of cast further improved; incompatibilities of
 * user defined types are recognized much better now!
 *
 * Revision 1.9  2002/11/04 13:18:11  sbs
 * cast computation improved:  (:xx[*])expr   now yields
 * most specific subtype of xx[*] that may be derived from the
 * type of expr. E.g., ((:complex[*])expr::double[5,2)) :: complex[5]  now!!
 *
 * Revision 1.8  2002/10/30 12:11:56  sbs
 * cast modified; now casts between defined type and its base definition type are
 * legal as well ;-)
 *
 * Revision 1.7  2002/10/28 14:04:15  sbs
 * NTCPRF_cast added
 *
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
#include "constants.h"
#include "type_errors.h"
#include "user_types.h"

ct_funptr NTCPRF_funtab[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) g
#include "prf_node_info.mac"
#undef PRF_IF
};

void *NTCPRF_cffuntab[] = {
#define PRF_IF(a, b, c, d, e, f, g, h) (void *)h
#include "prf_node_info.mac"
#undef PRF_IF
};

static constant *
ApplyCF (te_info *info, ntype *args)
{
    constant *res = NULL;

    DBUG_ENTER ("NTCApplyCF");

    switch (TYGetProductSize (args)) {
    case 1:
        res = ((monCF)TEGetCFFun (info)) (TYGetValue (TYGetProductMember (args, 0)));
        break;
    case 2:
        res = ((binCF)TEGetCFFun (info)) (TYGetValue (TYGetProductMember (args, 0)),
                                          TYGetValue (TYGetProductMember (args, 1)));
        break;
    case 3:
        res = ((triCF)TEGetCFFun (info)) (TYGetValue (TYGetProductMember (args, 0)),
                                          TYGetValue (TYGetProductMember (args, 1)),
                                          TYGetValue (TYGetProductMember (args, 2)));
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
    constant *val, *tmp;
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

    if (TYIsProdOfAKV (elems)) {
        val = TYGetValue (TYGetProductMember (elems, 0));
        for (i = 1; i < num_elems; i++) {
            tmp = val;
            val = COCat (tmp, TYGetValue (TYGetProductMember (elems, i)));
            tmp = COFreeConstant (tmp);
        }
        shp = SHCreateShape (1, num_elems);
        tmp = COMakeConstantFromShape (SHAppendShapes (shp, TYGetShape (elem)));
        SHFreeShape (shp);
        res = TYMakeAKV (TYCopyType (TYGetScalar (elem)), COReshape (tmp, val));
        tmp = COFreeConstant (tmp);
        val = COFreeConstant (val);
    } else {
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
    }

    TYFreeTypeConstructor (elem);

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_cast( te_info *info, ntype *elems)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCPRF_cast (te_info *info, ntype *elems)
{
    ntype *cast_t, *cast_bt, *expr_t, *expr_bt;
    ntype *res, *res_bt;
    shape *shp, *d_shp, *s_shp;

    DBUG_ENTER ("NTCPRF_cast");

    cast_t = TYGetProductMember (elems, 0);
    cast_bt = TYEliminateUser (cast_t);
    expr_t = TYGetProductMember (elems, 1);
    expr_bt = TYEliminateUser (expr_t);

    TEAssureSameScalarType ("cast-type", cast_bt, "expr-type", expr_bt);
    res_bt = TEAssureSameShape ("cast-type", cast_bt, "expr-type", expr_bt);
    cast_bt = TYFreeType (cast_bt);
    expr_bt = TYFreeType (expr_bt);

    /*
     * Unfortunately, this TEAssureSameShape in certain situations does not detect
     * incompatabilities. The problem arises from the application of TYEliminateUser:
     * e.g.  TYEliminateUser( complex[.])  =>   double[.,.]   which does not contain
     *       the information that complex[.] in fact is double[.,2]! As a consequence,
     * it can be casted into double[3,4] which obviously is wrong!!
     * Such situations can accur, if
     *   (a) "res_bt" is an AKS type and
     *   (b) at least one of "cast_t" and "expr_t" are based on a user type;
     * or if
     *   (a) both, "cast_t" and "expr_t" are based on a user type.
     * Hence, the shapes (if available) of "res_bt" and the definitions of
     * the user types of "cast_t" and "expr_t" have to be compared:
     */

    if (TYIsAKS (res_bt)) {
        shp = TYGetShape (res_bt);
        if (TYIsArray (cast_t) && TYIsUser (TYGetScalar (cast_t))) {
            d_shp = TYGetShape (UTGetBaseType (TYGetUserType (TYGetScalar (cast_t))));
            s_shp = SHDropFromShape (SHGetDim (shp) - SHGetDim (d_shp), shp);
            if (!SHCompareShapes (d_shp, s_shp)) {
                ERROR (linenum, ("cast type %s does not match expression type %s "
                                 "as \"%s\" is defined as %s",
                                 TYType2String (cast_t, FALSE, 0),
                                 TYType2String (expr_t, FALSE, 0),
                                 UTGetName (TYGetUserType (TYGetScalar (cast_t))),
                                 TYType2String (UTGetBaseType (
                                                  TYGetUserType (TYGetScalar (cast_t))),
                                                FALSE, 0)));
                TEExtendedAbort ();
            }
        }
        if (TYIsArray (expr_t) && TYIsUser (TYGetScalar (expr_t))) {
            d_shp = TYGetShape (UTGetBaseType (TYGetUserType (TYGetScalar (expr_t))));
            s_shp = SHDropFromShape (SHGetDim (shp) - SHGetDim (d_shp), shp);
            if (!SHCompareShapes (d_shp, s_shp)) {
                ERROR (linenum, ("cast type %s does not match expression type %s "
                                 "as \"%s\" is defined as %s",
                                 TYType2String (cast_t, FALSE, 0),
                                 TYType2String (expr_t, FALSE, 0),
                                 UTGetName (TYGetUserType (TYGetScalar (expr_t))),
                                 TYType2String (UTGetBaseType (
                                                  TYGetUserType (TYGetScalar (expr_t))),
                                                FALSE, 0)));
                TEExtendedAbort ();
            }
        }
    } else {
        if (TYIsArray (cast_t) && TYIsUser (TYGetScalar (cast_t)) && TYIsArray (expr_t)
            && TYIsUser (TYGetScalar (expr_t))) {
            shp = TYGetShape (UTGetBaseType (TYGetUserType (TYGetScalar (cast_t))));
            d_shp = TYGetShape (UTGetBaseType (TYGetUserType (TYGetScalar (expr_t))));
            if (SHGetDim (shp) < SHGetDim (d_shp)
                  ? !SHCompareShapes (shp,
                                      SHDropFromShape (SHGetDim (d_shp) - SHGetDim (shp),
                                                       d_shp))
                  : !SHCompareShapes (SHDropFromShape (SHGetDim (shp) - SHGetDim (d_shp),
                                                       shp),
                                      d_shp)) {
                ERROR (linenum,
                       ("cast type %s does not match expression type %s "
                        "as \"%s\" is defined as %s whereas \"%s\" is defined as %s",
                        TYType2String (cast_t, FALSE, 0),
                        TYType2String (expr_t, FALSE, 0),
                        UTGetName (TYGetUserType (TYGetScalar (cast_t))),
                        TYType2String (UTGetBaseType (
                                         TYGetUserType (TYGetScalar (cast_t))),
                                       FALSE, 0),
                        UTGetName (TYGetUserType (TYGetScalar (expr_t))),
                        TYType2String (UTGetBaseType (
                                         TYGetUserType (TYGetScalar (expr_t))),
                                       FALSE, 0)));
                TEExtendedAbort ();
            }
        }
    }

    /*
     * Now, that we have checked for potential compatibility, we can compute
     * the best possible return type. Usualy, this is res_bt. However, if
     * "cast_t" turns out to be based on a user defined type, we have to "de-nest" the
     * return type, i.e., we have to cut off the shape of the (base) defining type of the
     * user type from the back of "res_bt".
     */

    if (TYIsArray (cast_t) && TYIsUser (TYGetScalar (cast_t))) {
        res
          = TYDeNestTypes (res_bt, UTGetBaseType (TYGetUserType (TYGetScalar (cast_t))));
        res = TYSetScalar (res, TYCopyType (TYGetScalar (cast_t)));
        res_bt = TYFreeType (res_bt);
    } else {
        res = res_bt;
    }

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
    ntype *array;
    ntype *res;

    DBUG_ENTER ("NTCPRF_dim");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "dim called with incorrect number of arguments");

    array = TYGetProductMember (args, 0);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array);

    if (TYIsAKV (array) || TYIsAKS (array) || TYIsAKD (array)) {
        res = TYMakeAKV (TYMakeSimpleType (T_int),
                         COMakeConstantFromInt (TYGetDim (array)));
    } else {
        res = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));
    }

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
    shape *shp;
    int n;

    DBUG_ENTER ("NTCPRF_shape");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "shape called with incorrect number of arguments");

    arg = TYGetProductMember (args, 0);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), arg);

    switch (TYGetConstr (arg)) {
    case TC_akv:
    case TC_aks:
        shp = TYGetShape (arg);
        res = TYMakeAKV (TYMakeSimpleType (T_int), COMakeConstantFromShape (shp));
        break;
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
    TEAssureProdValMatchesProdShape (TEPrfArg2Obj (TEGetNameStr (info), 1), new_shp,
                                     TEArg2Obj (2), array);

    scalar = TYGetScalar (array);

    switch (TYGetConstr (new_shp)) {
    case TC_akv:
        if (TYGetConstr (array) == TC_akv) {
            res = TYMakeAKV (TYCopyType (TYGetScalar (array)), ApplyCF (info, args));
        } else {
            res
              = TYMakeAKS (TYCopyType (scalar), COConstant2Shape (TYGetValue (new_shp)));
        }
        break;
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
    TEAssureValMatchesShape (TEPrfArg2Obj (TEGetNameStr (info), 1), idx, TEArg2Obj (2),
                             array);

    if (TYIsAKV (idx) && TYIsAKV (array)) {
        res = TYMakeAKV (TYCopyType (TYGetScalar (array)), ApplyCF (info, args));
    } else {
        res = TYMakeAKS (TYCopyType (TYGetScalar (array)), SHMakeShape (0));
    }

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
    TEAssureValMatchesShape (TEPrfArg2Obj (TEGetNameStr (info), 2), idx, TEArg2Obj (1),
                             array);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 3), val);
    TEAssureScalar (TEPrfArg2Obj (TEGetNameStr (info), 3), val);
    TEAssureSameSimpleType (TEArg2Obj (2), array, TEPrfArg2Obj (TEGetNameStr (info), 3),
                            val);

    if (TYIsAKV (array)) {
        if (TYIsAKV (idx) && TYIsAKV (val)) {
            res = TYMakeAKV (TYCopyType (TYGetScalar (array)), ApplyCF (info, args));
        } else {
            res = TYMakeAKS (TYCopyType (TYGetScalar (array)),
                             SHCopyShape (TYGetShape (array)));
        }
    } else {
        res = TYCopyType (array);
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

static ntype *
ConvS (te_info *info, ntype *args, simpletype st)
{
    ntype *res = NULL;
    ntype *array;

    DBUG_ENTER ("ConvS");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "ConvS called with incorrect number of arguments");

    array = TYGetProductMember (args, 0);

    TEAssureNumS (TEPrfArg2Obj (TEGetNameStr (info), 1), array);

    if (TYIsAKV (array)) {
        res = TYMakeAKV (TYMakeSimpleType (st), ApplyCF (info, args));
    } else {
        res = TYMakeAKS (TYMakeSimpleType (st), SHMakeShape (0));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

static ntype *
ConvA (te_info *info, ntype *args, simpletype st)
{
    ntype *res = NULL;
    ntype *array, *scal;

    DBUG_ENTER ("ConvA");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "ConvA called with incorrect number of arguments");

    array = TYGetProductMember (args, 0);

    TEAssureNumA (TEPrfArg2Obj (TEGetNameStr (info), 1), array);

    if (TYIsAKV (array)) {
        res = TYMakeAKV (TYMakeSimpleType (st), ApplyCF (info, args));
    } else {
        res = TYCopyType (array);
        scal = TYGetScalar (res);
        scal = TYSetSimpleType (scal, st);
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCPRF_toiS( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toi_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toi_S_ is applied to
 *   @return       the result type of applying _toi_S_
 *
 ******************************************************************************/

ntype *
NTCPRF_toiS (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCPRF_toiS");

    res = ConvS (info, args, T_int);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCPRF_toiA( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toi_A_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toi_A_ is applied to
 *   @return       the result type of applying _toi_A_
 *
 ******************************************************************************/

ntype *
NTCPRF_toiA (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCPRF_toiA");

    res = ConvA (info, args, T_int);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCPRF_tofS( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tof_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tof_S_ is applied to
 *   @return       the result type of applying _tof_S_
 *
 ******************************************************************************/

ntype *
NTCPRF_tofS (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCPRF_tofS");

    res = ConvS (info, args, T_float);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCPRF_tofA( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tof_A_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tof_A_ is applied to
 *   @return       the result type of applying _tof_A_
 *
 ******************************************************************************/

ntype *
NTCPRF_tofA (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCPRF_tofA");

    res = ConvA (info, args, T_float);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCPRF_todS( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tod_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tod_S_ is applied to
 *   @return       the result type of applying _tod_S_
 *
 ******************************************************************************/

ntype *
NTCPRF_todS (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCPRF_todS");

    res = ConvS (info, args, T_double);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCPRF_todA( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tod_A_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tod_A_ is applied to
 *   @return       the result type of applying _tod_A_
 *
 ******************************************************************************/

ntype *
NTCPRF_todA (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCPRF_todA");

    res = ConvA (info, args, T_double);

    DBUG_RETURN (res);
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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYMakeAKV (TYCopyType (TYGetScalar (array1)), ApplyCF (info, args));
    } else {
        res = TYMakeAKS (TYCopyType (TYGetScalar (array1)), SHMakeShape (0));
    }

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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYMakeAKV (TYCopyType (TYGetScalar (array1)), ApplyCF (info, args));
    } else {
        res = TYCopyType (array2);
    }

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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYMakeAKV (TYCopyType (TYGetScalar (array1)), ApplyCF (info, args));
    } else {
        res = TYCopyType (array1);
    }

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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYFreeType (res);
        res = TYMakeAKV (TYCopyType (TYGetScalar (array1)), ApplyCF (info, args));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_ari_op_A( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_ari_op_A (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;

    DBUG_ENTER ("NTCPRF_ari_op_A");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "ari_op_A called with incorrect number of arguments");

    array = TYGetProductMember (args, 0);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array);

    if (TYIsAKV (array)) {
        res = TYMakeAKV (TYCopyType (TYGetScalar (array)), ApplyCF (info, args));
    } else {
        res = TYCopyType (array);
    }

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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYFreeType (res);
        res = TYMakeAKV (TYMakeSimpleType (T_bool), ApplyCF (info, args));
    }

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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYFreeType (res);
        res = TYMakeAKV (TYMakeSimpleType (T_bool), ApplyCF (info, args));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_log_op_A( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCPRF_log_op_A (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;

    DBUG_ENTER ("NTCPRF_log_op_A");
    DBUG_ASSERT (TYGetProductSize (args) == 1,
                 "log_op_A called with incorrect number of arguments");

    array = TYGetProductMember (args, 0);

    TEAssureBoolA (TEPrfArg2Obj (TEGetNameStr (info), 1), array);

    if (TYIsAKV (array)) {
        res = TYMakeAKV (TYMakeSimpleType (T_bool), ApplyCF (info, args));
    } else {
        res = TYCopyType (array);
    }

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

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYMakeAKV (TYMakeSimpleType (T_int), ApplyCF (info, args));
    } else {
        res = TYMakeAKS (TYMakeSimpleType (T_int), SHMakeShape (0));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_take_SxV( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  simple [.]  ->  simple [.]
 *
 ******************************************************************************/

ntype *
NTCPRF_take_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    shape *shp;

    DBUG_ENTER ("NTCPRF_take_SxV");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "take_SxV called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureIntS (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureVect (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureAbsValFitsShape (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                             array2);

    if (TYIsAKV (array1)) {
        if (TYIsAKV (array2)) {
            res = TYMakeAKV (TYCopyType (TYGetScalar (array2)), ApplyCF (info, args));
        } else {
            shp = SHCreateShape (1, abs (((int *)COGetDataVec (TYGetValue (array1)))[0]));
            res = TYMakeAKS (TYCopyType (TYGetScalar (array2)), shp);
        }
    } else {
        res = TYMakeAKD (TYCopyType (TYGetScalar (array2)), 1, SHMakeShape (0));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_drop_SxV( te_info *info, ntype *args)
 *
 * description:
 *    int []  x  simple [.]  ->  simple [.]
 *
 ******************************************************************************/

ntype *
NTCPRF_drop_SxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    shape *shp;

    DBUG_ENTER ("NTCPRF_drop_SxV");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "drop_SxV called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureIntS (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureVect (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureAbsValFitsShape (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                             array2);

    if (TYIsAKV (array1) && (TYIsAKV (array2) || TYIsAKS (array2))) {
        if (TYIsAKV (array2)) {
            res = TYMakeAKV (TYCopyType (TYGetScalar (array2)), ApplyCF (info, args));
        } else {
            shp = SHCopyShape (TYGetShape (array2));
            shp = SHSetExtent (shp, 0,
                               SHGetExtent (shp, 0)
                                 - abs (((int *)COGetDataVec (TYGetValue (array1)))[0]));
            res = TYMakeAKS (TYCopyType (TYGetScalar (array2)), shp);
        }
    } else {
        res = TYMakeAKD (TYCopyType (TYGetScalar (array2)), 1, SHMakeShape (0));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_cat_VxV( te_info *info, ntype *args)
 *
 * description:
 *    simple [.]  x  simple [.]  ->  simple [.]
 *
 ******************************************************************************/

ntype *
NTCPRF_cat_VxV (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_cat_VxV");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "cat_VxV called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureSimpleType (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);
    TEAssureSameSimpleType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    TEAssureVect (TEPrfArg2Obj (TEGetNameStr (info), 1), array1);
    TEAssureVect (TEPrfArg2Obj (TEGetNameStr (info), 2), array2);

    if (TYIsAKV (array1) && TYIsAKV (array2)) {
        res = TYMakeAKV (TYCopyType (TYGetScalar (array1)), ApplyCF (info, args));
    } else if ((TYIsAKV (array1) || TYIsAKS (array1))
               && (TYIsAKV (array2) || TYIsAKS (array2))) {
        res = TYMakeAKS (TYCopyType (TYGetScalar (array1)),
                         SHCreateShape (1, SHGetExtent (TYGetShape (array1), 0)
                                             + SHGetExtent (TYGetShape (array2), 0)));
    } else {
        res = TYMakeAKD (TYCopyType (TYGetScalar (array1)), 1, SHMakeShape (0));
    }

    DBUG_RETURN (TYMakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCPRF_phi( te_info *info, ntype *args)
 *
 * description:
 *    alpha [*]  x alpha [*]  ->  alpha [*]
 *
 ******************************************************************************/

ntype *
NTCPRF_phi (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;

    DBUG_ENTER ("NTCPRF_phi");
    DBUG_ASSERT (TYGetProductSize (args) == 2,
                 "phi called with incorrect number of arguments");

    array1 = TYGetProductMember (args, 0);
    array2 = TYGetProductMember (args, 1);

    TEAssureSameScalarType (TEArg2Obj (1), array1, TEPrfArg2Obj (TEGetNameStr (info), 2),
                            array2);
    res = TYLubOfTypes (array1, array2);

    DBUG_RETURN (TYMakeProductType (1, res));
}
