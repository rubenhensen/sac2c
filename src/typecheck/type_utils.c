/*
 *
 * $Log$
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
