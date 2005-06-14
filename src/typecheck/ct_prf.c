/*
 *
 * $Log$
 * Revision 1.27  2005/06/14 09:55:10  sbs
 * support for bottom types integrated.
 *
 * Revision 1.26  2005/01/10 17:27:06  cg
 * Converted error messages from Error.h to ctinfo.c
 *
 * Revision 1.25  2004/11/26 23:48:22  sbs
 * some renamings fixed
 *
 * Revision 1.24  2004/11/24 18:14:46  sbs
 * compiles
 *
 * Revision 1.23  2004/08/08 16:05:08  sah
 * fixed some includes.
 *
 * Revision 1.22  2004/03/05 12:07:07  sbs
 * nasty sharing error in NTCCTarray eliminated.
 *
 * Revision 1.21  2004/02/27 11:49:15  sbs
 * NTCPRF_phi deleted
 *
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
 * no double usage of TEprfArg2Obj anymore as this fun uses static buffers 8-)
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
#include "new_types.h"
#include "type_errors.h"
#include "user_types.h"
#include "shape.h"
#include "constants.h"
#include "ctinfo.h"
#include "globals.h"

static constant *
ApplyCF (te_info *info, ntype *args)
{
    constant *res = NULL;

    DBUG_ENTER ("NTCApplyCF");

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
    DBUG_ENTER ("NTCCTprf_dummy");
    DBUG_ASSERT (FALSE, "prf not yet implemented");
    DBUG_RETURN (args);
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
    ntype *elem, *elem2, *res;
    constant *val, *tmp;
    shape *shp;
    int num_elems;
    char *err_msg;
    int i;

    DBUG_ENTER ("NTCCTprf_array");

    elem = TYcopyType (TYgetProductMember (elems, 0));
    num_elems = TYgetProductSize (elems);

    for (i = 1; i < num_elems; i++) {
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
        if (TYisProdOfAKV (elems)) {
            val = COcopyConstant (TYgetValue (TYgetProductMember (elems, 0)));
            for (i = 1; i < num_elems; i++) {
                tmp = val;
                val = COcat (tmp, TYgetValue (TYgetProductMember (elems, i)));
                tmp = COfreeConstant (tmp);
            }
            shp = SHcreateShape (1, num_elems);
            tmp = COmakeConstantFromShape (SHappendShapes (shp, TYgetShape (elem)));
            SHfreeShape (shp);
            res = TYmakeAKV (TYcopyType (TYgetScalar (elem)), COreshape (tmp, val));
            tmp = COfreeConstant (tmp);
            val = COfreeConstant (val);
        } else {
            switch (TYgetConstr (elem)) {
            case TC_aks:
                shp = SHcreateShape (1, num_elems);
                res = TYmakeAKS (TYgetScalar (elem),
                                 SHappendShapes (shp, TYgetShape (elem)));
                SHfreeShape (shp);
                break;
            case TC_akd:
                res
                  = TYmakeAKD (TYgetScalar (elem), TYgetDim (elem) + 1, SHmakeShape (0));
                break;
            case TC_audgz:
            case TC_aud:
                res = TYmakeAUDGZ (TYgetScalar (elem));
                break;
            default:
                DBUG_ASSERT ((FALSE),
                             "array elements of non array types not yet supported");
                res = NULL; /* just to please gcc */
            }
        }
    }

    TYfreeTypeConstructor (elem);

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

    DBUG_ENTER ("NTCCTprf_cast");

    cast_t = TYgetProductMember (elems, 0);
    cast_bt = TYeliminateUser (cast_t);
    expr_t = TYgetProductMember (elems, 1);
    expr_bt = TYeliminateUser (expr_t);

    TEassureSameScalarType ("cast-type", cast_bt, "expr-type", expr_bt);
    res_bt = TEassureSameShape ("cast-type", cast_bt, "expr-type", expr_bt);
    cast_bt = TYfreeType (cast_bt);
    expr_bt = TYfreeType (expr_bt);

    /*
     * Unfortunately, this TEassureSameShape in certain situations does not detect
     * incompatabilities. The problem arises from the application of TYeliminateUser:
     * e.g.  TYeliminateUser( complex[.])  =>   double[.,.]   which does not contain
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
     * the best possible return type. Usualy, this is res_bt. However, if
     * "cast_t" turns out to be based on a user defined type, we have to "de-nest" the
     * return type, i.e., we have to cut off the shape of the (base) defining type of the
     * user type from the back of "res_bt".
     */

    if (TYisArray (cast_t) && TYisUser (TYgetScalar (cast_t))) {
        res
          = TYdeNestTypes (res_bt, UTgetBaseType (TYgetUserType (TYgetScalar (cast_t))));
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
 *    ntype *NTCCTprf_dim( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_dim (te_info *info, ntype *args)
{
    ntype *array;
    ntype *res;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_dim");
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "dim called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array);
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
 *    ntype *NTCCTprf_shape( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_shape (te_info *info, ntype *args)
{
    ntype *arg;
    ntype *res = NULL;
    shape *shp;
    char *err_msg;

    int n;

    DBUG_ENTER ("NTCCTprf_shape");
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "shape called with incorrect number of arguments");

    arg = TYgetProductMember (args, 0);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), arg);
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
            DBUG_ASSERT (FALSE, "NTCCTprf_shape applied to non-array type");
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_reshape( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_reshape (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *new_shp, *array, *scalar;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_reshape");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "shape called with incorrect number of arguments");

    new_shp = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntVect (TEprfArg2Obj (TEgetNameStr (info), 1), new_shp);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

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
                res = TYmakeAKD (TYcopyType (scalar),
                                 SHgetExtent (TYgetShape (new_shp), 0), SHmakeShape (0));
                break;
            case TC_akd:
            case TC_audgz:
            case TC_aud:
                res = TYmakeAUD (TYcopyType (scalar));
                break;
            default:
                DBUG_ASSERT (FALSE, "NTCPRF_reshape applied to non-array type");
            }
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_selS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_selS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_selS");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "selS called with incorrect number of arguments");

    idx = TYgetProductMember (args, 0);
    array = TYgetProductMember (args, 1);

    TEassureIntVect (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
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
 *    ntype *NTCCTprf_modarrayS( te_info *info, ntype *args)
 *
 * description:
 *
 ******************************************************************************/

ntype *
NTCCTprf_modarrayS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *idx, *array, *val;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_modarrayS");
    DBUG_ASSERT (TYgetProductSize (args) == 3,
                 "modarrayS called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);
    idx = TYgetProductMember (args, 1);
    val = TYgetProductMember (args, 2);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 3), val);
    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 3), val);
    TEassureSameSimpleType (TEarg2Obj (2), array, TEprfArg2Obj (TEgetNameStr (info), 3),
                            val);
    TEassureIntVect (TEprfArg2Obj (TEgetNameStr (info), 1), idx);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array);
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

static ntype *
ConvS (te_info *info, ntype *args, simpletype st)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ("ConvS");
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "ConvS called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureNumS (TEprfArg2Obj (TEgetNameStr (info), 1), array);
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

static ntype *
ConvA (te_info *info, ntype *args, simpletype st)
{
    ntype *res = NULL;
    ntype *array, *scal;
    char *err_msg;

    DBUG_ENTER ("ConvA");
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "ConvA called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureNumA (TEprfArg2Obj (TEgetNameStr (info), 1), array);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array)) {
            res = TYmakeAKV (TYmakeSimpleType (st), ApplyCF (info, args));
        } else {
            res = TYcopyType (array);
            scal = TYgetScalar (res);
            scal = TYsetSimpleType (scal, st);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toiS( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toi_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toi_S_ is applied to
 *   @return       the result type of applying _toi_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toiS (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCCTprf_toiS");

    res = ConvS (info, args, T_int);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_toiA( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _toi_A_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _toi_A_ is applied to
 *   @return       the result type of applying _toi_A_
 *
 ******************************************************************************/

ntype *
NTCCTprf_toiA (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCCTprf_toiA");

    res = ConvA (info, args, T_int);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tofS( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tof_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tof_S_ is applied to
 *   @return       the result type of applying _tof_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tofS (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCCTprf_tofS");

    res = ConvS (info, args, T_float);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_tofA( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tof_A_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tof_A_ is applied to
 *   @return       the result type of applying _tof_A_
 *
 ******************************************************************************/

ntype *
NTCCTprf_tofA (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCCTprf_tofA");

    res = ConvA (info, args, T_float);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_todS( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tod_S_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tod_S_ is applied to
 *   @return       the result type of applying _tod_S_
 *
 ******************************************************************************/

ntype *
NTCCTprf_todS (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCCTprf_todS");

    res = ConvS (info, args, T_double);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *NTCCTprf_todA( te_info *info, ntype *args)
 *
 *   @brief  computes the return type of an application of _tod_A_
 *
 *   @param info   info needed for type errors and for applying CF
 *   @param args   product of argument types _tod_A_ is applied to
 *   @return       the result type of applying _tod_A_
 *
 ******************************************************************************/

ntype *
NTCCTprf_todA (te_info *info, ntype *args)
{
    ntype *res = NULL;

    DBUG_ENTER ("NTCCTprf_todA");

    res = ConvA (info, args, T_double);

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

    DBUG_ENTER ("NTCCTprf_ari_op_SxA");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_SxA called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureSameSimpleType (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                            array2);
    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
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
 *    ntype *NTCCTprf_ari_op_SxA( te_info *info, ntype *args)
 *
 * description:
 *    simple []  x  simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_SxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_ari_op_SxA");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_SxA called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureSameSimpleType (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                            array2);
    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYcopyType (array2);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_AxS( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  x  simple []  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_AxS (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_ari_op_AxS");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_AxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureSameSimpleType (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                            array2);
    TEassureScalar (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        if (TYisAKV (array1) && TYisAKV (array2)) {
            res = TYmakeAKV (TYcopyType (TYgetScalar (array1)), ApplyCF (info, args));
        } else {
            res = TYcopyType (array1);
        }
    }

    DBUG_RETURN (TYmakeProductType (1, res));
}

/******************************************************************************
 *
 * function:
 *    ntype *NTCCTprf_ari_op_AxA( te_info *info, ntype *args)
 *
 * description:
 *    simple [shp]  x  simple [shp]  ->  simple [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_ari_op_AxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_ari_op_AxA");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "ari_op_AxA called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureSameSimpleType (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                            array2);
    res = TEassureSameShape (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                             array2);
    err_msg = TEfetchErrors ();
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

    DBUG_ENTER ("NTCCTprf_ari_op_A");
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
 *    ntype *NTCCTprf_rel_op_AxA( te_info *info, ntype *args)
 *
 * description:
 *     simple [shp]  x  simple [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_rel_op_AxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_rel_op_AxA");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "rel_op_AxA called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureSameSimpleType (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                            array2);
    res = TEassureSameShape (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                             array2);
    err_msg = TEfetchErrors ();
    if (err_msg != NULL) {
        res = TYmakeBottomType (err_msg);
    } else {

        res = TYsetScalar (res, TYmakeSimpleType (T_bool));

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
 *    ntype *NTCCTprf_log_op_AxA( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  x  bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_AxA (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array1, *array2;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_log_op_AxA");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "log_op_AxA called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureBoolA (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureBoolA (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
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
 *    ntype *NTCCTprf_log_op_A( te_info *info, ntype *args)
 *
 * description:
 *    bool [shp]  ->  bool [shp]
 *
 ******************************************************************************/

ntype *
NTCCTprf_log_op_A (te_info *info, ntype *args)
{
    ntype *res = NULL;
    ntype *array;
    char *err_msg;

    DBUG_ENTER ("NTCCTprf_log_op_A");
    DBUG_ASSERT (TYgetProductSize (args) == 1,
                 "log_op_A called with incorrect number of arguments");

    array = TYgetProductMember (args, 0);

    TEassureBoolA (TEprfArg2Obj (TEgetNameStr (info), 1), array);
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

    DBUG_ENTER ("NTCCTprf_int_op_SxS");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "int_op_SxS called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureIntS (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
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

    DBUG_ENTER ("NTCCTprf_take_SxV");
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

    DBUG_ENTER ("NTCCTprf_drop_SxV");
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

    DBUG_ENTER ("NTCCTprf_cat_VxV");
    DBUG_ASSERT (TYgetProductSize (args) == 2,
                 "cat_VxV called with incorrect number of arguments");

    array1 = TYgetProductMember (args, 0);
    array2 = TYgetProductMember (args, 1);

    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureSimpleType (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    TEassureSameSimpleType (TEarg2Obj (1), array1, TEprfArg2Obj (TEgetNameStr (info), 2),
                            array2);
    TEassureVect (TEprfArg2Obj (TEgetNameStr (info), 1), array1);
    TEassureVect (TEprfArg2Obj (TEgetNameStr (info), 2), array2);
    err_msg = TEfetchErrors ();
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
