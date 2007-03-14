/*
 * $Id$
 */

#include "type_utils.h"
#include "dbug.h"
#include "str.h"
#include "str_buffer.h"
#include "memory.h"
#include "tree_basic.h"
#include "tree_compound.h"

#include "new_types.h"
#include "shape.h"
#include "namespaces.h"
#include "new_typecheck.h"
#include "create_wrappers.h"
#include "ssi.h"
#include "user_types.h"
#include "globals.h"
#include "traverse.h"
#include "ctinfo.h"

/** <!--********************************************************************-->
 *
 * @fn ntype *TUrebuildWrapperType( ntype *)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

static ntype *
buildWrapper (node *fundef, ntype *type)
{
    DBUG_ENTER ("buildWrapper");

    /*
     * set this instances return types to AUD[*]
     */
    FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));

    /*
     * add the fundef to the wrappertype
     */
    type = TYmakeOverloadedFunType (CRTWRPcreateFuntype (fundef), type);

    DBUG_RETURN (type);
}

ntype *
TUrebuildWrapperType (ntype *type)
{
    ntype *new_type;

    DBUG_ENTER ("TUrebuildWrapperType");

    DBUG_ASSERT (TYisFun (type), "TUrebuildWrapperType called on non-fun type!");

    new_type
      = TYfoldFunctionInstances (type, (void *(*)(node *, void *))buildWrapper, NULL);

    DBUG_RETURN (new_type);
}

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
        vardecs = TBmakeVardec (TBmakeAvis (TRAVtmpVar (), TYcopyType (RET_TYPE (rets))),
                                vardecs);
        rets = RET_NEXT (rets);
    }

    DBUG_RETURN (vardecs);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *TUmakeProductTypeFromArgs( node *args)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

ntype *
TUmakeProductTypeFromArgs (node *args)
{
    ntype *type = NULL;
    int i = 0;

    DBUG_ENTER ("TUcreateTmpVardecsFromArgs");

    type = TYmakeEmptyProductType (TCcountArgs (args));
    while (args != NULL) {
        type = TYsetProductMember (type, i, TYcopyType (ARG_NTYPE (args)));
        args = ARG_NEXT (args);
        i++;
    }

    DBUG_RETURN (type);
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
    } else if (TYisBottom (type)) {
        new = TYmakeAlphaType (TYcopyType (type));
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
            if (TYisBottom (RET_TYPE (tmp))) {
                scalar = RET_TYPE (tmp);
            } else {
                scalar = TYgetScalar (RET_TYPE (tmp));
            }

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

    DBUG_ENTER ("TUisHidden");

    if (!TYisBottom (ty) && !TYisSymb (TYgetScalar (ty))) {
        if (TYisUser (TYgetScalar (ty))) {
            ty = UTgetBaseType (TYgetUserType (TYgetScalar (ty)));
        }

        res = (TYgetSimpleType (TYgetScalar (ty)) == T_hidden);
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

    res = (!TYisBottom (type) && TYisUser (TYgetScalar (type)));

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

    if (!TUisHidden (type)) {
        /*
         * all types except for scalars are boxed */
        impl = TUcomputeImplementationType (type);
        res = ((TYisAUD (impl) || TYisAUDGZ (impl)) ? TRUE : TYgetDim (type) > 0);
        impl = TYfreeType (impl);
    } else {
        /* hidden types are _always_ boxed */
        res = TRUE;
    }

    DBUG_RETURN (res);
}

/** <!--*******************************************************************-->
 *
 * @fn bool SCIeqShapes ( ntype *a, ntype *b)
 * @brief Predicate to tell if two AKS arrays have the same shape
 *
 *****************************************************************************/
bool
TUeqShapes (ntype *a, ntype *b)
{
    bool res;

    DBUG_ENTER ("TUeqShapes");

    res = SHcompareShapes (TYgetShape (a), TYgetShape (b));

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
        buf = SBUFcreate (100);
    }

    arg = FUNDEF_ARGS (fundef);
    while (arg != NULL) {
        tmp_str = TYtype2String (ARG_NTYPE (arg), FALSE, 0);
        buf = SBUFprintf (buf, "%s ", tmp_str);
        tmp_str = MEMfree (tmp_str);
        arg = ARG_NEXT (arg);
    }

    buf = SBUFprint (buf, "-> ");

    arg = FUNDEF_RETS (fundef);
    while (arg != NULL) {
        tmp_str = TYtype2String (RET_TYPE (arg), FALSE, 0);
        buf = SBUFprintf (buf, "%s ", tmp_str);
        tmp_str = MEMfree (tmp_str);
        arg = RET_NEXT (arg);
    }

    tmp_str = SBUF2str (buf);
    SBUFflush (buf);

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
 *   bool TUsignatureMatches( node *formal, ntype *actual_prod_type, bool exact)
 *
 * Description:
 *   Checks whether TYPE('formal') is a supertype of 'actual_prod_type'.
 *   if exact==true it does NOT match unknown, otherwise it does.
 *
 ******************************************************************************/

bool
TUsignatureMatches (node *formal, ntype *actual_prod_type, bool exact)
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
        DBUG_EXECUTE ("TU", tmp_str = MEMfree (tmp_str); tmp2_str = MEMfree (tmp2_str););

        if (!(TYleTypes (actual_type, formal_type)
              || (!exact && TYgetSimpleType (TYgetScalar (formal_type)) == T_unknown))) {
            match = FALSE;
            break;
        }

        formal = ARG_NEXT (formal);
        pos++;
    }
    DBUG_PRINT ("TU", ("    result: %d", match));

    DBUG_RETURN (match);
}

/** <!-- ****************************************************************** -->
 * @fn bool TUretsContainBottom( node *rets)
 *
 * @brief Returns true iff at least one return has a bottom type assigned.
 *
 * @param rets rets chain
 *
 * @return true if at least one rets has bottom type
 ******************************************************************************/
bool
TUretsContainBottom (node *rets)
{
    bool result;

    DBUG_ENTER ("TUretsContainConstantOrBottom");

    if (rets == NULL) {
        result = FALSE;
    } else {
        result = TYisBottom (RET_TYPE (rets)) || TUretsContainBottom (RET_NEXT (rets));
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn bool TUretsAreConstant( node *rets)
 *
 * @brief Returns true iff all rets have an akv type assigned.
 *
 * @param rets rets chain
 *
 * @return true iff all rets have akv type
 ******************************************************************************/
bool
TUretsAreConstant (node *rets)
{
    bool result;

    DBUG_ENTER ("TUretsAreConstant");

    result = ((rets == NULL)
              || (TYisAKV (RET_TYPE (rets)) && TUretsAreConstant (RET_NEXT (rets))));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUcombineBottom( ntype *left, ntype *right)
 *
 * @brief resturns a new bottom ntype containing the concatenation of
 *        the error messages of the given two bottom types.
 *
 * @param left bottom type
 * @param right bottom type
 *
 * @return freshly allocated bottom type
 ******************************************************************************/

ntype *
TUcombineBottom (ntype *left, ntype *right)
{
    ntype *result = NULL;

    DBUG_ENTER ("TUcombineBottom");

    result = TYcopyType (left);
    TYextendBottomError (result, TYgetBottomError (right));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUcombineBottoms( ntype *prod)
 *
 * @brief resturns a new bottom ntype containing the concatenation of
 *        the error messages contained in the given type.
 *
 * @param type
 *
 * @return freshly allocated bottom type
 ******************************************************************************/

ntype *
TUcombineBottoms (ntype *prod)
{
    ntype *res = NULL, *next = NULL;
    int i;

    DBUG_ENTER ("TUcombineBottoms");

    if (TYisProd (prod)) {
        for (i = 0; i < TYgetProductSize (prod); i++) {
            next = TUcombineBottoms (TYgetProductMember (prod, i));
            if (res != NULL) {
                if (next != NULL) {
                    TYextendBottomError (res, TYgetBottomError (next));
                    next = TYfreeType (next);
                }
            } else {
                res = next;
            }
        }
    } else {
        if (TYisBottom (prod)) {
            res = TYcopyType (prod);
        }
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUspreadBottoms( ntype *prod)
 *
 * @brief replaces all non-bottom types by combinations of all bottom
 *        types found
 *
 * @param prod product type potentially containing bottom types.
 *
 * @return a copy of the type where all types have been replaced by identical
 *         bottom types.
 ******************************************************************************/

ntype *
TUspreadBottoms (ntype *prod)
{
    ntype *result = NULL, *bottoms = NULL;
    int i;

    DBUG_ENTER ("TUspreadBottoms");

    bottoms = TUcombineBottoms (prod);
    if (bottoms == NULL) {
        result = TYcopyType (prod);
    } else {
        result = TYmakeEmptyProductType (TYgetProductSize (prod));
        for (i = 0; i < TYgetProductSize (prod); i++) {
            TYsetProductMember (result, i, TYcopyType (bottoms));
        }
        bottoms = TYfreeType (bottoms);
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUcombineBottomsFromRets( node *rets)
 *
 * @brief resturns a new bottom ntype containing the concatenation of all
 *        error messages of the given rets chain. if the chain does not
 *        contain any bottom type, NULL is returned.
 *
 * @param rets a N_ret chain
 *
 * @return freshly allocated bottom type or NULL
 ******************************************************************************/
ntype *
TUcombineBottomsFromRets (node *rets)
{
    ntype *result = NULL;

    DBUG_ENTER ("TUcombineBottomsFromRets");

    if (rets != NULL) {
        result = TUcombineBottomsFromRets (RET_NEXT (rets));

        if (TYisBottom (RET_TYPE (rets))) {
            if (result == NULL) {
                result = TYcopyType (RET_TYPE (rets));
            } else {
                TYextendBottomError (result, TYgetBottomError (RET_TYPE (rets)));
            }
        }
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUcheckUdtAndSetBaseType( usertype udt, int* visited)
 *
 * @brief
 *  This function checks the integrity of a user defined type, and while doing
 *  so it converts Symb{} types into Udt{} types, it computes its base-type,
 *  AND stores it in the udt-repository!
 *  At the time being, the following restrictions apply:
 *  - the defining type has to be one of AKS{ Symb{}}, AKS{ Udt{}}
 *    or AKS{ Simple{}}.
 *  - if the defining type contains a Symb{} type, this type and all further
 *    decendents must be defined without any recursion in type definitions!
 *
 *  We ASSUME, that the existence of a basetype indicates that the udt has
 *  been checked already!!!
 *  Furthermore, we ASSUME that iff basetype is not yet set, the defining
 *  type either is a user- or a symbol-type.
 *
 * @param udt udt# to infer the basetype for
 * @param visited The second parameter ("visited") is needed for detecting
 *        recusive definitions only. Therefore, the initial call should be
 *        made with (visited == NULL)!
 *
 * @return
 ******************************************************************************/
ntype *
TUcheckUdtAndSetBaseType (usertype udt, int *visited)
{
    ntype *base, *base_elem;
    usertype inner_udt;
    ntype *inner_base;
    ntype *new_base, *new_base_elem;
    int num_udt, i;

    DBUG_ENTER ("TUcheckUdtandSetBaseType");

    base = UTgetBaseType (udt);
    if (base == NULL) {
        base = UTgetTypedef (udt);
        if (!TYisAKS (base)) {
            CTIerrorLine (global.linenum,
                          "Typedef of %s::%s is illegal; should be either"
                          " scalar type or array type of fixed shape",
                          NSgetName (UTgetNamespace (udt)), UTgetName (udt));
        } else {
            /*
             * Here, we know that we are either dealing with
             * AKS{ User{}}, AKS{ Symb{}}, or AKS{ Simple{}}.
             */
            if (TYisAKSUdt (base) || TYisAKSSymb (base)) {
                base_elem = TYgetScalar (base);
                inner_udt = TYisAKSUdt (base)
                              ? TYgetUserType (base_elem)
                              : UTfindUserType (TYgetName (base_elem),
                                                TYgetNamespace (base_elem));
                if (inner_udt == UT_NOT_DEFINED) {
                    CTIerrorLine (global.linenum,
                                  "Typedef of %s::%s is illegal; type %s::%s unknown",
                                  NSgetName (UTgetNamespace (udt)), UTgetName (udt),
                                  NSgetName (TYgetNamespace (base_elem)),
                                  TYgetName (base_elem));
                } else {
                    /*
                     * First, we replace the defining symbol type by the appropriate
                     * user-defined-type, i.e., inner_udt!
                     */
                    new_base_elem = TYmakeUserType (inner_udt);
                    new_base = TYmakeAKS (new_base_elem, SHcopyShape (TYgetShape (base)));
                    UTsetTypedef (udt, new_base);
                    TYfreeType (base);
                    base = new_base;

                    /*
                     * If this is the initial call, we have to allocate and
                     * initialize our recursion detection mask "visited".
                     */
                    if (visited == NULL) {
                        /* This is the initial call, so visited has to be initialized! */
                        num_udt = UTgetNumberOfUserTypes ();
                        visited = (int *)MEMmalloc (sizeof (int) * num_udt);
                        for (i = 0; i < num_udt; i++)
                            visited[i] = 0;
                    }
                    /*
                     * if we have not yet checked the inner_udt, recursively call
                     * CheckUdtAndSetBaseType!
                     */
                    if (visited[inner_udt] == 1) {
                        CTIerrorLine (global.linenum, "Type %s:%s recursively defined",
                                      NSgetName (UTgetNamespace (udt)), UTgetName (udt));
                    } else {
                        visited[udt] = 1;
                        inner_base = TUcheckUdtAndSetBaseType (inner_udt, visited);
                        /*
                         * Finally, we compute the resulting base-type by nesting
                         * the inner base type with the actual typedef!
                         */
                        base = TYnestTypes (base, inner_base);
                    }
                }
            } else {
                /*
                 * Here, we deal with AKS{ Simple{}}. Hence
                 * base is the base type. Therefore, there will be no further
                 * recursice call. This allows us to free "visited".
                 * To be precise, we would have to free "visited in all ERROR-cases
                 * as well, but we neglect that since in case of an error the
                 * program will terminate soon anyways!
                 */
                if (visited != NULL)
                    visited = MEMfree (visited);
            }
        }
        UTsetBaseType (udt, base);
    }

    DBUG_RETURN (base);
}