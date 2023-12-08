#include "type_utils.h"

#define DBUG_PREFIX "TU"
#include "debug.h"

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
#include "constants.h"

/** <!--********************************************************************-->
 *
 * @fn ntype *TUcreateFuntype( node *fundef)
 * @fn ntype *TUcreateFuntypeIgnoreArtificials( node *fundef)
 *
 *   @brief creates a function type from the given arg/return types.
 *   @param
 *   @return
 *
 ******************************************************************************/

static ntype *
FuntypeFromArgs (ntype *res, node *args, node *fundef, bool all)
{
    DBUG_ENTER ();

    if (args != NULL) {
        res = FuntypeFromArgs (res, ARG_NEXT (args), fundef, all);
        if (all || !ARG_ISARTIFICIAL (args)) {
            res = TYmakeFunType (TYcopyType (ARG_NTYPE (args)), res, fundef);
        }
    }

    DBUG_RETURN (res);
}

ntype *
TUcreateFuntype (node *fundef)
{
    ntype *res;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "TUcreateFuntype applied to non-fundef node!");

    res = FuntypeFromArgs (TUmakeProductTypeFromRets (FUNDEF_RETS (fundef)),
                           FUNDEF_ARGS (fundef), fundef, TRUE);

    DBUG_RETURN (res);
}

ntype *
TUcreateFuntypeIgnoreArtificials (node *fundef)
{
    ntype *res;
    node *rets;

    DBUG_ENTER ();

    DBUG_ASSERT (NODE_TYPE (fundef) == N_fundef,
                 "TUcreateFuntypeIgnoreArtificials applied to non-fundef node!");

    rets = FUNDEF_RETS (fundef);

    while ((rets != NULL) && RET_ISARTIFICIAL (rets)) {
        rets = RET_NEXT (rets);
    }

    res = FuntypeFromArgs (TUmakeProductTypeFromRets (rets), FUNDEF_ARGS (fundef), fundef,
                           FALSE);

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *TUrebuildWrapperTypeAlphaFix( ntype *)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

static ntype *
buildWrapperAlphaFix (node *fundef, ntype *type)
{
    DBUG_ENTER ();

    DBUG_PRINT_TAG ("TUWRAP", "buildWrapperAlphaFix (%s, type)", CTIitemName (fundef));
    /*
     * set this instances return types to alpha[*]
     */
    FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));

    /*
     * add the fundef to the wrappertype
     */
    type = TYmakeOverloadedFunType (TUcreateFuntype (fundef), type);

    DBUG_RETURN (type);
}

ntype *
TUrebuildWrapperTypeAlphaFix (ntype *type)
{
    ntype *new_type;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisFun (type), "TUrebuildWrapperType called on non-fun type!");

    new_type
      = (ntype *)TYfoldFunctionInstances (type,
                                          (void *(*)(node *, void *))buildWrapperAlphaFix,
                                          NULL);

    DBUG_RETURN (new_type);
}

/** <!--********************************************************************-->
 *
 * @fn ntype *TUrebuildWrapperTypeAlpha( ntype *)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

static ntype *
buildWrapperAlpha (node *fundef, ntype *type)
{
    DBUG_ENTER ();

    /*
     * set this instances return types to alpha[*]
     */
    DBUG_PRINT_TAG ("TUWRAP", "opening return types of %s", CTIitemName (fundef));
    if (FUNDEF_BODY (fundef) != NULL) {
        FUNDEF_RETS (fundef) = TUrettypes2alphaAUDMax (FUNDEF_RETS (fundef));
    } else {
        FUNDEF_RETS (fundef) = TUrettypes2alphaFix (FUNDEF_RETS (fundef));
    }

    /*
     * add the fundef to the wrappertype
     */
    type = TYmakeOverloadedFunType (TUcreateFuntype (fundef), type);

    DBUG_RETURN (type);
}

ntype *
TUrebuildWrapperTypeAlpha (ntype *type)
{
    ntype *new_type;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisFun (type), "TUrebuildWrapperType called on non-fun type!");

    new_type
      = (ntype *)TYfoldFunctionInstances (type,
                                          (void *(*)(node *, void *))buildWrapperAlpha,
                                          NULL);

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

    DBUG_ENTER ();

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
    size_t i = 0;

    DBUG_ENTER ();

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
    size_t i = 0;

    DBUG_ENTER ();

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
 * @fn node *TUmakeTypeExprsFromRets( node *rets)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUmakeTypeExprsFromRets (node *rets)
{
    node *exprs;

    DBUG_ENTER ();

    if (rets == NULL) {
        exprs = NULL;
    } else {
        exprs = TUmakeTypeExprsFromRets (RET_NEXT (rets));
        exprs = TBmakeExprs (TBmakeType (TYcopyType (RET_TYPE (rets))), exprs);
    }

    DBUG_RETURN (exprs);
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
    size_t i = 0;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    while (tmp != NULL) {
        ARG_NTYPE (tmp) = TYfreeType (ARG_NTYPE (tmp));
        ARG_NTYPE (tmp) = TYmakeAUD (TYmakeSimpleType (T_unknown));

        /*
         * update DECLTYPE if there is one
         */
        if (AVIS_DECLTYPE (ARG_AVIS (tmp)) != NULL) {
            AVIS_DECLTYPE (ARG_AVIS (tmp)) = TYfreeType (AVIS_DECLTYPE (ARG_AVIS (tmp)));
            AVIS_DECLTYPE (ARG_AVIS (tmp)) = TYcopyType (ARG_NTYPE (tmp));
        }

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
    ntype *xnew, *scalar, *mint;
    tvar *tv;

    DBUG_ENTER ();

    if (TYisAlpha (type)) {
        tv = TYgetAlpha (type);
        mint = SSIgetMin (tv);
        if (SSIgetMax (tv) != NULL) {
            xnew = TYmakeAlphaType (TYcopyType (SSIgetMax (tv)));
        } else if (mint !=NULL) {
            if (TYisBottom (mint)) {
                xnew = TYmakeAlphaType (TYcopyType (mint));
            } else {
                xnew = TYmakeAlphaType (TYmakeAUD (TYcopyType (TYgetScalar (mint))));
            }
        } else {
            xnew = TYmakeAlphaType (NULL);
        }
    } else if (TYisBottom (type)) {
        xnew = TYmakeAlphaType (TYcopyType (type));
    } else {
        scalar = TYgetScalar (type);
        if ((TYisSimple (scalar) && (TYgetSimpleType (scalar) == T_unknown))) {
            xnew = TYmakeAlphaType (NULL);
        } else {
            xnew = TYmakeAlphaType (TYcopyType (type));
        }
    }

    DBUG_RETURN (xnew);
}

/** <!--********************************************************************-->
 *
 * @fn ntype  *TUtype2alphaAUDMax( ntype *type);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

ntype *
TUtype2alphaAUDMax (ntype *type)
{
    ntype *xnew, *scalar;

    DBUG_ENTER ();

    if (TYisAlpha (type)) {
        xnew = TYcopyType (type);

        tvar *tv = TYgetAlpha (type);

        DBUG_ASSERT (SSIgetMax (tv) != NULL,
                     "trying to TUtype2alphaAUDMax alpha without max!");
        DBUG_ASSERT (TYisAUD (SSIgetMax (tv)),
                     "trying to TUtype2alphaAUDMax alpha with non-AUD max!");
    } else if (TYisBottom (type)) {
        xnew = TYmakeAlphaType (TYcopyType (type));
    } else {
        scalar = TYgetScalar (type);
        if ((TYisSimple (scalar) && (TYgetSimpleType (scalar) == T_unknown))) {
            xnew = TYmakeAlphaType (NULL);
        } else {
            xnew = TYmakeAlphaType (TYmakeAUD (TYcopyType (scalar)));
        }
    }

    DBUG_RETURN (xnew);
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
TUrettypes2alphaAUDMax (node *rets)
{
    node *tmp = rets;
    ntype *xnew;

    DBUG_ENTER ();

    while (tmp != NULL) {

        xnew = TUtype2alphaAUDMax (RET_TYPE (tmp));

        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = xnew;
        tmp = RET_NEXT (tmp);
    }

    DBUG_RETURN (rets);
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
    ntype *xnew;

    DBUG_ENTER ();

    while (tmp != NULL) {

        xnew = TUtype2alphaMax (RET_TYPE (tmp));

        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = xnew;
        tmp = RET_NEXT (tmp);
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

    DBUG_ENTER ();

    while (tmp != NULL) {

        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYmakeAlphaType (NULL);
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
    ntype *xnew, *scalar;

    DBUG_ENTER ();

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

            xnew = TYmakeAlphaType (TYcopyType (RET_TYPE (tmp)));
            SSInewMin (TYgetAlpha (xnew), RET_TYPE (tmp));

            RET_TYPE (tmp) = xnew;
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
 * @fn node  *TUalphaRettypes2bottom( node *rets, const char *msg);
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

node *
TUalphaRettypes2bottom (node *rets, const char *msg)
{
    node *tmp = rets;

    DBUG_ENTER ();

    while (tmp != NULL) {
        if (TYisAlpha (RET_TYPE (tmp))) {
            RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
            RET_TYPE (tmp) = TYmakeBottomType (STRcpy (msg));
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
    DBUG_ENTER ();
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
    DBUG_ENTER ();
    DBUG_RETURN (TYisAKS (ty) || TYisAKV (ty));
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisBoolScalar( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisBoolScalar (ntype *ty)
{
    bool res;

    DBUG_ENTER ();
    res = ((TYgetSimpleType (TYgetScalar (ty)) == T_bool)
           && (TYisAKD (ty) || TYisAKS (ty) || TYisAKV (ty)) && (TYgetDim (ty) == 0));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisIntScalar( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisIntScalar (ntype *ty)
{
    bool res;

    DBUG_ENTER ();
    res = ((TYgetSimpleType (TYgetScalar (ty)) == T_int)
           && (TYisAKD (ty) || TYisAKS (ty) || TYisAKV (ty)) && (TYgetDim (ty) == 0));
    DBUG_RETURN (res);
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

    DBUG_ENTER ();
    res = ((TYgetSimpleType (TYgetScalar (ty)) == T_int)
           && (TYisAKD (ty) || TYisAKS (ty) || TYisAKV (ty)) && (TYgetDim (ty) == 1));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisEmptyVect( ntype *ty)
 *
 *   @brief Predicate for empty (integer) vector.
 *   @param an ntype
 *   @return boolean true if argument is empty integer vector.
 *
 ******************************************************************************/

bool
TUisEmptyVect (ntype *ty)
{
    bool res;

    DBUG_ENTER ();
    res = ((TYgetSimpleType (TYgetScalar (ty)) == T_int) && (TUshapeKnown (ty))
           && (TYgetDim (ty) == 1) && (0 == SHgetExtent (TYgetShape (ty), 0)));
    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUisUnsigned( ntype *ty)
 *
 *   @brief Predicate for array of unsigned integer (int/long/etc ) type.
 *   @param an ntype
 *   @return boolean true if it is an untyped integer type
 *
 ******************************************************************************/

bool
TUisUnsigned (ntype *ty)
{
    bool res;
    simpletype st;

    DBUG_ENTER ();
    st = TYgetSimpleType (TYgetScalar (ty));
    res = ((st == T_ubyte)
           || (st == T_ushort)
           || (st == T_uint)
           || (st == T_ulong)
           || (st == T_ulonglong));

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
 * @fn bool TUisHidden( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUisNested (ntype *ty)
{
    bool res = FALSE;

    DBUG_ENTER ();

    if (TUisHidden (ty) && TYisUser (ty)) {
        res = UTisNested (TYgetUserType (ty));
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

    DBUG_ENTER ();

    res = (TYisArray (type) && TYisUser (TYgetScalar (type)));

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
TUisArrayOfSimple (ntype *type)
{
    bool res;

    DBUG_ENTER ();

    res = (TYisArray (type) && TYisSimple (TYgetScalar (type)));

    DBUG_RETURN (res);
}

/** <!--********************************************************************-->
 *
 * @fn bool TUcontainsUser( ntype *ty)
 *
 *   @brief
 *   @param
 *   @return
 *
 ******************************************************************************/

bool
TUcontainsUser (ntype *type)
{
    bool res = FALSE;
    size_t cnt;

    DBUG_ENTER ();

    if (TYisArray (type)) {
        res = TYisUser (TYgetScalar (type));
    } else if (TYisProd (type)) {
        size_t max = TYgetProductSize (type);
        for (cnt = 0; cnt < max; cnt++) {
            res = res || TUcontainsUser (TYgetProductMember (type, cnt));
        }
    } else {
        DBUG_UNREACHABLE ("type not implemented yet");
    }

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

    DBUG_ENTER ();

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

/** <!-- ****************************************************************** -->
 * @fn bool TUisPolymorphic( ntype *type)
 *
 * @brief Checks whether type is polymorphic.
 *
 * @param type ntype structure
 *
 * @return true if type contains either a poly or a polyuser type
 ******************************************************************************/
bool
TUisPolymorphic (ntype *type)
{
    DBUG_ENTER ();

    if (TYisArray (type)) {
        type = TYgetScalar (type);
    }

    DBUG_RETURN (TYisPoly (type) || TYisPolyUser (type));
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUstripImplicitNestingOperations( ntype *poly)
 *
 * @brief Removes implicit nesting operations from PolyUser types.
 *
 * @param poly a (possibly) polymorphic user type
 *
 * @return copy of that type with no implicit nesting/denesting.
 ******************************************************************************/
extern ntype *
TUstripImplicitNestingOperations (ntype *poly)
{
    ntype *res;

    DBUG_ENTER ();

    if (TUisPolymorphic (poly)) {
        if (TYisArray (poly)) {
            res = TYcopyType (poly);
            res
              = TYsetScalar (res, TUstripImplicitNestingOperations (TYgetScalar (poly)));
        } else {
            if (TYisPolyUser (poly)) {
                res
                  = TYmakePolyUserType (STRcpy (TYgetPolyUserOuter (poly)),
                                        STRcpy (TYgetPolyUserInner (poly)),
                                        STRcpy (TYgetPolyUserShape (poly)), FALSE, FALSE);
            } else {
                res = TYcopyType (poly);
            }
        }
    } else {
        res = TYcopyType (poly);
    }

    DBUG_RETURN (res);
}

/** <!--*******************************************************************-->
 *
 * @fn bool TUeqShapes ( ntype *a, ntype *b)
 * @brief Predicate to tell if two AKS arrays have the same shape
 *
 *****************************************************************************/
bool
TUeqShapes (ntype *a, ntype *b)
{
    bool res;

    DBUG_ENTER ();

    res = SHcompareShapes (TYgetShape (a), TYgetShape (b));

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 * @fn bool TUleShapeInfo( ntype *a, ntype *b)
 *
 * @brief Returns true if the type a holds at least as much shape information
 *        as the type b. However, the types need not to be related in any
 *        kind.
 *
 * @param a first ntype to compare
 * @param b second ntype to compare
 *
 * @return true iff a hold at least as much shape info as b
 ******************************************************************************/
bool
TUleShapeInfo (ntype *a, ntype *b)
{
    bool result;

    DBUG_ENTER ();

    switch (TYgetConstr (a)) {
    case TC_akv:
    case TC_aks:
        result = TRUE;
        break;

    case TC_akd:
        switch (TYgetConstr (b)) {
        case TC_akv:
        case TC_aks:
            result = FALSE;
            break;

        default:
            result = TRUE;
            break;
        }
        break;

    case TC_audgz:
        switch (TYgetConstr (b)) {
        case TC_audgz:
        case TC_aud:
            result = TRUE;
            break;

        default:
            result = FALSE;
            break;
        }
        break;

    case TC_aud:
        result = (TYgetConstr (b) == TC_aud);
        break;

    default:
        DBUG_UNREACHABLE ("illegal argument");
        result = FALSE;
        break;
    }

    DBUG_RETURN (result);
}

bool
TUeqElementSize (ntype *a, ntype *b)
{
    bool result;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisArray (a), "first argument is not an array type");
    DBUG_ASSERT (TYisArray (b), "second argument is not an array type");
    DBUG_ASSERT (TYisSimple (TYgetScalar (a)),
                 "first argument is not an array of a built-in type");
    DBUG_ASSERT (TYisSimple (TYgetScalar (b)),
                 "second argument is not an array of a built-in type");

    result = global.basetype_size[TYgetSimpleType (TYgetScalar (a))]
             == global.basetype_size[TYgetSimpleType (TYgetScalar (b))];

    DBUG_RETURN (result);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

/** <!--********************************************************************-->
 *
 * @fn char *TUwrapperTypeSignature2String( node *fundef)
 *
 *   @brief creates a multi-line string for wrapper functions; one line
 *          per existing instance. To do so a local helper function 
 *          createFunSigString is needed that is mapped onto all instances.
 *   @param
 *   @return
 *
 ******************************************************************************/
static node *
createFunSigString( node *fundef, info *arg_info)
{
    char *tmp;
    str_buf **buf;

    buf = (str_buf **)arg_info;

    tmp = TUtypeSignature2String (fundef);
    *buf = SBUFprintf (*buf, "\\n***   %s :: %s", CTIitemName (fundef), tmp);
    tmp = MEMfree (tmp);
 
    arg_info = (info *)buf;
    
    return fundef;
}

char *
TUwrapperTypeSignature2String (node *fundef)
{
    static str_buf *buf = NULL;
    char *res_str;
    char *tmp;

    DBUG_ENTER ();
    
    DBUG_ASSERT (FUNDEF_WRAPPERTYPE (fundef) != NULL,
                 "TUwrapperTypeSignature2String called on function"
                 " without wrapper type!");

    if (buf == NULL) {
        buf = SBUFcreate (100);
    }

    tmp = TUtypeSignature2String (fundef);
    buf = SBUFprintf (buf, "\\n***   %s :: %s", CTIitemName (fundef), tmp);
    tmp = MEMfree (tmp);

    buf = SBUFprintf (buf, "\\n*** instances present:");
    FUNDEF_WRAPPERTYPE (fundef)
        = TYmapFunctionInstances (FUNDEF_WRAPPERTYPE (fundef),
                                  createFunSigString,
                                  (info *)&buf);

    res_str = SBUF2str (buf);
    SBUFflush (buf);

    DBUG_RETURN (res_str);
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
    size_t size;
    size_t pos;

    DBUG_ENTER ();

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
    size_t pos;
    bool match = TRUE;
#ifndef DBUG_OFF
    char *tmp_str = NULL, *tmp2_str = NULL;
#endif

    DBUG_ENTER ();

    pos = 0;
    while ((formal != NULL) && (ARG_NTYPE (formal) != NULL)) {
        DBUG_ASSERT (NODE_TYPE (formal) == N_arg, "illegal args found!");

        formal_type = AVIS_TYPE (ARG_AVIS (formal));
        actual_type = TYgetProductMember (actual_prod_type, pos);
        DBUG_EXECUTE (tmp_str = TYtype2String (formal_type, FALSE, 0);
                      tmp2_str = TYtype2String (actual_type, FALSE, 0));
        DBUG_PRINT ("    comparing formal type %s with actual type %s", tmp_str,
                    tmp2_str);
        DBUG_EXECUTE (tmp_str = MEMfree (tmp_str); tmp2_str = MEMfree (tmp2_str));

        if (!(TYleTypes (actual_type, formal_type)
              || (!exact && TYgetSimpleType (TYgetScalar (formal_type)) == T_unknown))) {
            match = FALSE;
            break;
        }

        formal = ARG_NEXT (formal);
        pos++;
    }
    DBUG_PRINT ("    result: %d", match);

    DBUG_RETURN (match);
}

/** <!-- ****************************************************************** -->
 * @brief Returns true if the ravels of arrays of type t1 and t2 have a
 *        compatible structure wrt. to indexing operations, i.e., that
 *        for t1 A and t2 B and iv an index vector, A[iv] and B[iv] use the
 *        same index offset.
 *
 * @param t1 type of array
 * @param t2 type of array
 *
 * @return true if t1 and t2 are compatible
 ******************************************************************************/
bool
TUravelsHaveSameStructure (ntype *t1, ntype *t2)
{
    ntype *aks1, *aks2;
    shape *shp1, *shp2;

    bool res = FALSE;

    DBUG_ENTER ();

    aks1 = TYeliminateAKV (t1);
    aks2 = TYeliminateAKV (t2);

    /*
     * We check whether the ravel has a compatible structure,
     * i.e., we need to know that the shape only differs
     * on the outermost dimension. For vectors, this is
     * always true (AKD case). Otherwise, we have to
     * inspect the shape here.
     */
    if (TUdimKnown (aks1) && TUdimKnown (aks2) && (TYgetDim (aks1) == TYgetDim (aks2))) {
        /* vector case */
        res = TRUE;
    } else if (TYisAKS (aks1) && TYisAKS (aks2) && (TYgetDim (aks1) > 1)
               && (TYgetDim (aks2) > 1)) {
        /*
         * AKS non-vector case.
         * Check that drop(1, shape( aks1)) == drop(1, shape( aks2))
         */
        shp1 = SHdropFromShape (1, TYgetShape (aks1));
        shp2 = SHdropFromShape (1, TYgetShape (aks2));

        res = SHcompareShapes (shp1, shp2);

        shp1 = SHfreeShape (shp1);
        shp2 = SHfreeShape (shp2);
    }

    aks1 = TYfreeType (aks1);
    aks2 = TYfreeType (aks2);

    DBUG_RETURN (res);
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    result = ((rets == NULL)
              || (TYisAKV (RET_TYPE (rets)) && TUretsAreConstant (RET_NEXT (rets))));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUcombineBottom( ntype *left, ntype *right)
 *
 * @brief returns a new bottom ntype containing the concatenation of
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

    DBUG_ENTER ();

    result = TYcopyType (left);
    TYextendBottomError (result, TYgetBottomError (right));

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn ntype *TUcombineBottoms( ntype *prod)
 *
 * @brief returns a new bottom ntype containing the concatenation of
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
    size_t i;

    DBUG_ENTER ();

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
    size_t i;

    DBUG_ENTER ();

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
 * @brief returns a new bottom ntype containing the concatenation of all
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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    base = UTgetBaseType (udt);
    if (base == NULL) {
        base = UTgetTypedef (udt);
        if (UTisNested (udt)) {
            /*
             * Since this is a nested type, the basetype needs to
             * be a scalar hidden type of itself!
             */
            base_elem = TYmakeHiddenSimpleType (udt);
            base = TYmakeAKS (base_elem, SHmakeShape (0));
            if (visited != NULL) {
                visited = MEMfree (visited);
            }
        } else {
            /*
             * Here, we know that we are either dealing with
             * AKS{ User{}}, AKS{ Symb{}}, or AKS{ Simple{}}.
             */
            DBUG_ASSERT ((TYisAKS (base)), "non AKS type in non-nested"
                         "typedef for \"%s\" found", UTgetName (udt));
            base_elem = TYgetScalar (base);
            if (TYisAKSUdt (base) || TYisAKSSymb (base)) {
                inner_udt = TYisAKSUdt (base)
                              ? TYgetUserType (base_elem)
                              : UTfindUserType (TYgetName (base_elem),
                                                TYgetNamespace (base_elem));
                if (inner_udt == UT_NOT_DEFINED) {
                    CTIerror (LINE_TO_LOC (global.linenum),
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
                        visited = (int *)MEMmalloc (sizeof (int) * (size_t)num_udt);
                        for (i = 0; i < num_udt; i++)
                            visited[i] = 0;
                    }
                    /*
                     * if we have not yet checked the inner_udt, recursively call
                     * CheckUdtAndSetBaseType!
                     */
                    if (visited[inner_udt] == 1) {
                        CTIerror (LINE_TO_LOC (global.linenum),
                                  "Type %s:%s recursively defined",
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

/** <!-- ****************************************************************** -->
 * @fn TUisScalar( ntype *ty)
 *
 * @brief Simple but useful utility to check whether a type definitely
 *        represents a scalar. This function has the functionality that
 *        TYisScalar concerning the name claims to have.
 *
 * @param type
 *
 * @return boolean
 ******************************************************************************/

bool
TUisScalar (ntype *ty)
{
    DBUG_ENTER ();

    DBUG_RETURN (TUdimKnown (ty) && (TYgetDim (ty) == 0));
}

/** <!-- ****************************************************************** -->
 * @fn TUisVector( ntype *ty)
 *
 * @brief Simple but useful utility to check whether a type definitely
 *        represents a vector.
 *
 * @param type
 *
 * @return boolean
 ******************************************************************************/

bool
TUisVector (ntype *ty)
{
    DBUG_ENTER ();

    DBUG_RETURN (TUdimKnown (ty) && (TYgetDim (ty) == 1));
}

/** <!-- ****************************************************************** -->
 * @fn bool TUhasBasetype( ntype *ty, simpletype smpl)
 *
 * @brief Checks whether the given type is an array type with the given
 *        basetype
 *
 * @param ty    array type to check
 * @param smpl  basetype to check against
 *
 * @return true if array is an arraytype with given basetype
 ******************************************************************************/
bool
TUhasBasetype (ntype *ty, simpletype smpl)
{
    bool result;

    DBUG_ENTER ();

    result = TYisArray (ty) && TYisSimple (TYgetScalar (ty))
             && (TYgetSimpleType (TYgetScalar (ty)) == smpl);

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn simpletype TUgetBaseSimpleType( ntype *type)
 *
 * @brief Returns the simpletype of the innermost basetype of a typedef
 *        chain
 *
 * @param type type to start search with
 *
 * @return the types innermost basetype
 ******************************************************************************/
simpletype
TUgetBaseSimpleType (ntype *type)
{
    usertype udt;

    DBUG_ENTER ();
    while (TUisArrayOfUser (type)) {
        udt = TYgetUserType (TYgetScalar (type));
        udt = UTgetUnAliasedType (udt);
        type = UTgetBaseType (udt);
    }

    DBUG_ASSERT (TYisArray (type), "Non array type found!");
    DBUG_ASSERT (TYisSimple (TYgetScalar (type)), "non simple type as base!");
    DBUG_RETURN (TYgetSimpleType (TYgetScalar (type)));
}

/** <!-- ****************************************************************** -->
 *
 * @fn int TUakvScalInt2Int( ntype *ty)
 *
 * @brief: Extract integer scalar constant from an AKV integer scalar ntype
 *
 * @param: ty: ntype
 *
 * @return the integer value
 *
 ******************************************************************************/
int
TUakvScalInt2Int (ntype *ty)
{
    int z;
    constant *con = NULL;

    DBUG_ENTER ();

    DBUG_ASSERT (TYisAKV (ty) && TUisIntScalar (ty), "Expected integer scalar constant");
    con = TYgetValue (ty); // This is NOT a copy!
    z = COconst2Int (con);

    DBUG_RETURN (z);
}

/** <!-- ****************************************************************** -->
 *
 * @fn node *TUint2akv( int val)
 *
 * @brief: generate an AKV type for the integer value val
 *
 * @param: val: value
 *
 * @return the AKV type
 *
 ******************************************************************************/
ntype* TUint2akv (int val)
{
    DBUG_ENTER ();
    DBUG_RETURN (TYmakeAKV (TYmakeSimpleType (T_int),
                            COmakeConstantFromInt (val)));
}

/** <!-- ****************************************************************** -->
 *
 * @fn int TUgetFullDimEncoding( ntype *type)
 *
 * @brief: produces the array info encoding needed by the backend:
 *         >= 0 : AKS with result == DIM
 *         <  -2: AKD with result == -2 - DIM
 *         == -1: AUSGZ
 *         == -2: AUD
 *         
 *
 * @param: type: ntype
 *
 * @return the encoding of the dimensionality.
 *
 ******************************************************************************/
int TUgetFullDimEncoding (ntype *type)
{
    int res;
    
    DBUG_ENTER ();

    if (TYisAUDGZ (type)) {
        res = -1;
    } else if (TYisAUD (type)) {
        res = -2;
    } else if (TYisAKD (type)) {
        res = -2 - TYgetDim (type);
    } else {
        res = TYgetDim (type);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int TUgetDimEncoding( ntype *type)
 *
 * @brief: produces the array info encoding needed by the backend:
 *         >= 0 : AKS / AKD with result == DIM
 *         == -1: AUDGZ
 *         == -2: AUD
 *
 *
 * @param: type: ntype
 *
 * @return the encoding of the dimensionality.
 *
 ******************************************************************************/
int TUgetDimEncoding (ntype *type)
{
    int res;

    DBUG_ENTER ();

    if (TYisAUDGZ (type)) {
        res = -1;
    } else if (TYisAUD (type)) {
        res = -2;
    } else {
        res = TYgetDim (type);
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn int TUgetLengthEncoding( ntype *type)
 *
 * @brief: produces the ravel info encoding needed by the backend:
 *         >= 0 : non-scalar AKS with result == prod (shape)
 *         == 0 : scalar
 *         == -1: AKD, AUDGZ, or AUD
 *
 *
 * @param: type: ntype
 *
 * @return the encoding of the dimensionality.
 *
 ******************************************************************************/
int TUgetLengthEncoding (ntype *type)
{
    int res;

    DBUG_ENTER ();

    if (TYisAUDGZ (type) || TYisAUD (type) || TYisAKD (type)) {
        res = -1;
    } else if (TUisScalar (type)) {
        res = 0;
    } else {
        res = (int)SHgetUnrLen (TYgetShape (type));
    }

    DBUG_RETURN (res);
}

/** <!-- ****************************************************************** -->
 *
 * @fn simpletype TUgetSimpleImplementationType (ntype *type)
 *
 * @brief: computes the implementation type and picks the element type of
 *         it. This should always be a SimpleType since all User-types are
 *         being followed through to their base.
 *
 * @param: type: ntype
 *
 * @return the simpletype in the implementation (can be T-hidden if nested
 *         or external.
 *
 ******************************************************************************/
simpletype TUgetSimpleImplementationType (ntype *type)
{
    ntype *itype;
    simpletype res;

    DBUG_ENTER ();

    itype = TUcomputeImplementationType (type);
    res = TYgetSimpleType (TYgetScalar (itype));
    itype = TYfreeType (itype);

    DBUG_RETURN (res);
}


#undef DBUG_PREFIX
