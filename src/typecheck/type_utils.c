/*
 *
 * $Log$
 * Revision 1.1  2004/11/24 09:50:08  sbs
 * Initial revision
 *
 *
 */

#include "type_utils.h"
#include "dbug.h"
#include "internal_lib.h"
#include "tree_basic.h"

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
    }

    DBUG_RETURN (vardecs);
}
