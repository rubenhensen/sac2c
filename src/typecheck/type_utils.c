/*
 *
 * $Log$
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
        RET_TYPE (tmp) = type;
        tmp = RET_NEXT (tmp);
        i++;
    }
    TYfreeTypeConstructor (prodt);

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

    DBUG_ENTER ("TUrettypes2AUD");

    while (tmp != NULL) {
        scalar = TYcopyType (TYgetScalar (RET_TYPE (tmp)));
        RET_TYPE (tmp) = TYfreeType (RET_TYPE (tmp));
        RET_TYPE (tmp) = TYmakeAUD (TYmakeSimpleType (T_unknown));
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

    DBUG_ENTER ("TUrettypes2alphaAUD");

    while (tmp != NULL) {
        scalar = TYcopyType (TYgetScalar (RET_TYPE (tmp)));
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

    DBUG_ENTER ("IsUniqueNT");

    if (TYisUser (ty)) {
        node *tdef = UTgetTdef (TYgetUserType (ty));
        DBUG_ASSERT (tdef != NULL, "Failed attempt to look up typedef");

        if (TYPEDEF_ISUNIQUE (tdef)) {
            res = TRUE;
        }
    }

    DBUG_RETURN (res);
}
