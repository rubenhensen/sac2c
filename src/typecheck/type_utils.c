/*
 *
 * $Log$
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
 * @fn node  *TUrettypes2alpha( node *rets);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUrettypes2alpha (node *rets)
{
    node *tmp = rets;

    DBUG_ENTER ("TUrettypes2alpha");

    while (tmp != NULL) {
        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYmakeAlphaType (NULL);
        tmp = RET_NEXT (tmp);
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
        tmp = ARG_NEXT (tmp);
    }

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUrettypes2AUD( node *rets);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUrettypes2AUD (node *rets)
{
    node *tmp = rets;
    ntype *scalar;
    tvar *tv;

    DBUG_ENTER ("TUrettypes2AUD");

    while (tmp != NULL) {
        if (TYisAlpha (RET_TYPE (tmp))) {
            tv = TYgetAlpha (RET_TYPE (tmp));
            if (SSIgetMax (tv) != NULL) {
                scalar = TYcopyType (TYgetScalar (SSIgetMax (tv)));
            } else if (SSIgetMin (tv) != NULL) {
                scalar = TYcopyType (TYgetScalar (SSIgetMin (tv)));
            } else {
                scalar = TYmakeSimpleType (T_unknown);
            }
        } else {
            scalar = TYcopyType (TYgetScalar (RET_TYPE (tmp)));
        }
        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYmakeAUD (scalar);
        tmp = RET_NEXT (tmp);
    }

    DBUG_RETURN (rets);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUargtypes2AUD( node *args);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUargtypes2AUD (node *args)
{
    node *tmp = args;
    ntype *scalar;

    DBUG_ENTER ("TUargtypes2AUD");

    while (tmp != NULL) {
        scalar = TYcopyType (TYgetScalar (ARG_NTYPE (tmp)));
        ARG_NTYPE (tmp) = TYfreeType (ARG_NTYPE (tmp));
        ARG_NTYPE (tmp) = TYmakeAUD (scalar);
        tmp = ARG_NEXT (tmp);
    }

    DBUG_RETURN (args);
}

/** <!--********************************************************************-->
 *
 * @fn node  *TUrettypes2alphaAUD( node *rets);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUrettypes2alphaAUD (node *rets)
{
    node *tmp = rets;
    ntype *scalar;
    tvar *tv;

    DBUG_ENTER ("TUrettypes2alphaAUD");

    while (tmp != NULL) {
        if (TYisAlpha (RET_TYPE (tmp))) {
            tv = TYgetAlpha (RET_TYPE (tmp));
            if (SSIgetMax (tv) != NULL) {
                scalar = TYcopyType (TYgetScalar (SSIgetMax (tv)));
            } else if (SSIgetMin (tv) != NULL) {
                scalar = TYcopyType (TYgetScalar (SSIgetMin (tv)));
            } else {
                scalar = TYmakeSimpleType (T_unknown);
            }
        } else {
            scalar = TYcopyType (TYgetScalar (RET_TYPE (tmp)));
        }
        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYmakeAlphaType (TYmakeAUD (scalar));
        tmp = RET_NEXT (tmp);
    }

    DBUG_RETURN (rets);
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

    DBUG_ENTER ("TUisHidden");

    if (TYisUser (TYgetScalar (ty))) {
        res = (TYgetSimpleType (UTgetBaseType (TYgetUserType (TYgetScalar (ty))))
               == T_hidden);
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
