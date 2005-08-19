/*
 *
 * $Log$
 * Revision 1.18  2005/08/19 17:25:06  sbs
 * changed TUrettypes2alpha into TUrettypes2alphaFix, etc.
 *
 * Revision 1.17  2005/08/09 10:17:05  sah
 * argtypes2UnknownAUD now processes DECLTYPE as well
 *
 * Revision 1.16  2005/07/21 12:02:24  ktr
 * added TUdimKnown
 *
 * Revision 1.15  2005/07/16 19:06:08  sbs
 * TUshapeKnown added.
 *
 * Revision 1.14  2005/07/16 12:30:50  sbs
 * TUisIntVect added.
 *
 * Revision 1.13  2005/06/18 13:52:03  sah
 * moved SignatureMatches and ActualArgs2Ntype from
 * create_wrapper_code to type_utils
 *
 * Revision 1.12  2005/06/01 20:08:57  sah
 * TYisHidden now is aware of the structure of hidden types
 *
 * Revision 1.11  2005/05/31 18:15:35  sah
 * made the alpha-a-lizer functions symbol-type aware
 *
 * Revision 1.10  2005/05/24 08:26:34  sbs
 * TUretypes2alpha modified.
 *
 * Revision 1.9  2004/12/09 12:32:27  sbs
 * TUargtypes2AUD and TYrettypes2AUD eliminated
 *
 * Revision 1.8  2004/12/09 00:36:57  sbs
 * handling of alphas in replaceAUD changed
 * ,
 *
 * Revision 1.7  2004/12/07 14:36:16  sbs
 * added TUtypeSignature2String
 *
 * Revision 1.6  2004/12/06 17:30:25  sbs
 * TUreplaceRettypes is now non-destructive!
 *
 * Revision 1.5  2004/12/05 19:19:55  sbs
 * return type of LaC funs changed into alphas.
 *
 * Revision 1.4  2004/11/26 22:58:51  sbs
 * some new utils added
 * \.
 *
 * Revision 1.3  2004/11/25 17:52:55  sbs
 * compiles
 *
 * Revision 1.2  2004/11/24 17:42:48  sbs
 * not yet
 *
 * Revision 1.1  2004/11/24 09:50:08  sbs
 * Initial revision
 *
 *
 */

#include "type_utils.h"
#include "dbug.h"
#include "internal_lib.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "new_types.h"
#include "new_typecheck.h"
#include "ssi.h"
#include "user_types.h"

/** <!--********************************************************************-->
 *
 * @fn node *TUcreateTmpVardecsFromRets( node *rets)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUcreateTmpVardecsFromRets (node *rets)
{
    node *vardecs = NULL;

    DBUG_ENTER ("TUcreateTmpVardecsFromRets");

    while (rets != NULL) {
        vardecs = TBmakeVardec (TBmakeAvis (ILIBtmpVar (), TYcopyType (RET_TYPE (rets))),
                                vardecs);
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN (vardecs);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *TUmakeProductTypeFromRets( node *rets)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

ntype *
TUmakeProductTypeFromRets (node *rets)
{
    ntype *type = NULL;
    int i = 0;

    DBUG_ENTER ("TUcreateTmpVardecsFromRets");

    type = TYmakeEmptyProductType (TCcountRets (rets));
    while (rets != NULL) {
        type = TYsetProductMember (type, i, TYcopyType (RET_TYPE (rets)));
        rets = RET_NEXT (rets);
        i++;
    }

    DBUG_RETURN (type);
}

/** <!--********************************************************************-->
 *
 * @fn node *TUreplaceRetTypes( node *rets, ntype* prodt)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUreplaceRetTypes (node *rets, ntype *prodt)
{
    ntype *type = NULL;
    node *tmp = rets;
    int i = 0;

    DBUG_ENTER ("TUreplaceRetTypes");

    DBUG_ASSERT (TCcountRets (tmp) == TYgetProductSize (prodt),
                 "lengths of N_rets and returntype do notmatch!");
    while (tmp != NULL) {
        type = TYgetProductMember (prodt, i);
        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYcopyType (type);
        tmp = RET_NEXT (tmp);
        i++;
    }

    DBUG_RETURN (rets);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUrettypes2unknownAUD( node *rets);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUrettypes2unknownAUD (node *rets)
{
    node *tmp = rets;

    DBUG_ENTER ("TUrettypes2unknownAUD");

    while (tmp != NULL) {
        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYmakeAUD (TYmakeSimpleType (T_unknown));
        tmp = RET_NEXT (tmp);
    }

    DBUG_RETURN (rets);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUargtypes2unknownAUD( node *args);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUargtypes2unknownAUD (node *args)
{
    node *tmp = args;

    DBUG_ENTER ("TUargtypes2unknownAUD");

    while (tmp != NULL) {
        ARG_NTYPE (tmp) = TYfreeType (ARG_NTYPE (tmp));
        ARG_NTYPE (tmp) = TYmakeAUD (TYmakeSimpleType (T_unknown));

        AVIS_DECLTYPE (ARG_AVIS (tmp)) = TYfreeType (AVIS_DECLTYPE (ARG_AVIS (tmp)));
        AVIS_DECLTYPE (ARG_AVIS (tmp)) = TYcopyType (ARG_NTYPE (tmp));

        tmp = ARG_NEXT (tmp);
    }

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn ntype  *TUtype2alphaMax( ntype *type);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

ntype *
TUtype2alphaMax (ntype *type)
{
    ntype *new, *scalar;
    tvar *tv;

    DBUG_ENTER ("TUtype2alphaMax");

    if (TYisAlpha (type)) {
        tv = TYgetAlpha (type);
        if (SSIgetMax (tv) != NULL) {
            new = TYmakeAlphaType (TYcopyType (SSIgetMax (tv)));
        } else if (SSIgetMin (tv) != NULL) {
            new = TYmakeAlphaType (TYmakeAUD (TYcopyType (TYgetScalar (SSIgetMin (tv)))));
        } else {
            new = TYmakeAlphaType (NULL);
        }
    } else {
        scalar = TYgetScalar (type);
        if ((TYisSimple (scalar) && (TYgetSimpleType (scalar) == T_unknown))) {
            new = TYmakeAlphaType (NULL);
        } else {
            new = TYmakeAlphaType (TYcopyType (type));
        }
    }

    DBUG_RETURN (new);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUrettypes2alphaMax( node *rets);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUrettypes2alphaMax (node *rets)
{
    node *tmp = rets;
    ntype *new;

    DBUG_ENTER ("TUrettypes2alphaMax");

    while (tmp != NULL) {

        new = TUtype2alphaMax (RET_TYPE (tmp));

        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = new;
        tmp = RET_NEXT (tmp);
    }

    DBUG_RETURN (rets);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUrettypes2alphaFix( node *rets);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUrettypes2alphaFix (node *rets)
{
    node *tmp = rets;
    ntype *new, *scalar;

    DBUG_ENTER ("TUrettypes2alphaFix");

    while (tmp != NULL) {
        if (!TYisAlpha (RET_TYPE (tmp))) {
            scalar = TYgetScalar (RET_TYPE (tmp));
            DBUG_ASSERT ((!TYisSimple (scalar)
                          || (TYgetSimpleType (scalar) != T_unknown)),
                         "TUrettypes2alphaFix applied to rettype with T_unknown");
            new = TYmakeAlphaType (TYcopyType (RET_TYPE (tmp)));
            SSInewMin (TYgetAlpha (new), RET_TYPE (tmp));
            RET_TYPE (tmp) = new;
        } else {
            DBUG_ASSERT (TYisFixedAlpha (RET_TYPE (tmp)),
                         "TUrettypes2alphaFix applied to rettype with non-fix alpha");
        }
        tmp = RET_NEXT (tmp);
    }

    DBUG_RETURN (rets);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUdimKnown( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUdimKnown (ntype *ty)
{
    DBUG_ENTER ("TUdimKnown");
    DBUG_RETURN (TYisAKD (ty) || TYisAKS (ty) || TYisAKV (ty));
}

/** <!--********************************************************************-->
 *
 * @fn bool TUshapeKnown( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUshapeKnown (ntype *ty)
{
    DBUG_ENTER ("TUshapeKnown");
    DBUG_RETURN (TYisAKS (ty) || TYisAKV (ty));
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisIntVect( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisIntVect (ntype *ty)
{
    bool res;

    DBUG_ENTER ("TUisIntVect");
    res = ((TYgetSimpleType (TYgetScalar (ty)) == T_int)
           && (TYisAKD (ty) || TYisAKS (ty) || TYisAKV (ty)) && (TYgetDim (ty) == 1));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisUniqueUserType( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisUniqueUserType (ntype *ty)
{
    bool res = FALSE;

    DBUG_ENTER ("TUisUniqueUserType");

    if (TYisUser (ty)) {
        node *tdef = UTgetTdef (TYgetUserType (ty));
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if (TYPEDEF_ISUNIQUE (tdef)) {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisHidden( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisHidden (ntype *ty)
{
    bool res = FALSE;
#ifndef DBUG_OFF
    char *tmp;
#endif

    DBUG_ENTER ("TUisHidden");

    if (TYisUser (TYgetScalar (ty))) {
        ntype *base = UTgetBaseType (TYgetUserType (TYgetScalar (ty)));

        DBUG_EXECUTE ("TU", tmp = TYtype2DebugString (base, FALSE, 0););

        DBUG_PRINT ("TU", ("found basetype %s", tmp));

        DBUG_EXECUTE ("TU", tmp = ILIBfree (tmp););

        res = (TYgetSimpleType (TYgetScalar (base)) == T_hidden);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisArrayOfUser( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisArrayOfUser (ntype *type)
{
    bool res;

    DBUG_ENTER ("TUisArrayOfUser");

    res = (TYisUser (TYgetScalar (type)));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisBoxed( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisBoxed (ntype *type)
{
    bool res = FALSE;
    ntype *impl;

    DBUG_ENTER ("TUisBoxed");

    if (TUisHidden (type)) {
        impl = TUcomputeImplementationType (type);
        DBUG_ASSERT (!TYisAUD (impl),
                     "TUisBoxed called with type of unknown dimensionality");
        res = (TYisAUDGZ (impl) ? TRUE : TYgetDim (type) > 0);
        impl = TYfreeType (impl);
    }

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *TUcomputeImplementationType( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

ntype *
TUcomputeImplementationType (ntype *ty)
{
    ntype *res;

    DBUG_ENTER ("TUgetImplementationType");

    if (TUisArrayOfUser (ty)) {
        res = TYnestTypes (ty, UTgetBaseType (TYgetUserType (TYgetScalar (ty))));
    } else {
        res = TYcopyType (ty);
    }
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn char *TUtypeSignature2String( node *fundef)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

char *
TUtypeSignature2String (node *fundef)
{
    static str_buf *buf = NULL;
    char *tmp_str;
    node *arg;

    DBUG_ENTER ("TUtypeSignature2String");

    if (buf == NULL) {
        buf = ILIBstrBufCreate (100);
    }

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        tmp_str = TYtype2String (ARG_NTYPE (arg), FALSE, 0);
        buf = ILIBstrBufPrintf (buf, "%s ", tmp_str);
        tmp_str = ILIBfree (tmp_str);
        arg = ARG_NEXT (arg);
    }

    buf = ILIBstrBufPrint (buf, "-> ");

    arg = FUNDEF_RETS (fundef);
    while (arg != NULL) {
        tmp_str = TYtype2String (RET_TYPE (arg), FALSE, 0);
        buf = ILIBstrBufPrintf (buf, "%s ", tmp_str);
        tmp_str = ILIBfree (tmp_str);
        arg = RET_NEXT (arg);
    }

    tmp_str = ILIBstrBuf2String (buf);
    ILIBstrBufFlush (buf);

    DBUG_RETURN (tmp_str);
}

/******************************************************************************
 *
 * Function:
 *   ntype *TUactualArgs2Ntype( node *actual)
 *
 * Description:
 *   Returns the appropriate product type for the given actual arguments.
 *
 ******************************************************************************/

ntype *
TUactualArgs2Ntype (node *actual)
{
    ntype *actual_type, *tmp_type, *prod_type;
    int size, pos;

    DBUG_ENTER ("TUactualArgs2Ntype");

    size = TCcountExprs (actual);
    prod_type = TYmakeEmptyProductType (size);

    pos = 0;
    while (actual != NULL) {
        tmp_type = NTCnewTypeCheck_Expr (EXPRS_EXPR (actual));
        actual_type = TYfixAndEliminateAlpha (tmp_type);
        tmp_type = TYfreeType (tmp_type);

        TYsetProductMember (prod_type, pos, actual_type);
        actual = EXPRS_NEXT (actual);
        pos++;
    }

    DBUG_RETURN (prod_type);
}

/******************************************************************************
 *
 * Function:
 *   bool TUsignatureMatches( node *formal, ntype *actual_prod_type)
 *
 * Description:
 *   Checks whether TYPE('formal') is a supertype of 'actual_prod_type'.
 *
 ******************************************************************************/

bool
TUsignatureMatches (node *formal, ntype *actual_prod_type)
{
    ntype *actual_type, *formal_type;
    int pos;
    bool match = TRUE;
#ifndef DBUG_OFF
    char *tmp_str, *tmp2_str;
#endif

    DBUG_ENTER ("TUsignatureMatches");

    pos = 0;
    while ((formal != NULL) && (ARG_NTYPE (formal) != NULL)) {
        DBUG_ASSERT ((NODE_TYPE (formal) == N_arg), "illegal args found!");

        formal_type = AVIS_TYPE (ARG_AVIS (formal));
        actual_type = TYgetProductMember (actual_prod_type, pos);
        DBUG_EXECUTE ("TU", tmp_str = TYtype2String (formal_type, FALSE, 0);
                      tmp2_str = TYtype2String (actual_type, FALSE, 0););
        DBUG_PRINT ("TU", ("    comparing formal type %s with actual type %s", tmp_str,
                           tmp2_str));
        DBUG_EXECUTE ("TU", tmp_str = ILIBfree (tmp_str);
                      tmp2_str = ILIBfree (tmp2_str););

        if (!TYleTypes (actual_type, formal_type)) {
            match = FALSE;
            break;
        }

        formal = ARG_NEXT (formal);
        pos++;
    }
    DBUG_PRINT ("TU", ("    result: %d", match));

    DBUG_RETURN (match);
}
