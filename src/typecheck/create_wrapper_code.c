/*
 *
 * $Log$
 * Revision 1.2  2002/08/09 13:15:20  dkr
 * CWCmodul, CWCfundef added
 *
 * Revision 1.1  2002/08/09 13:00:02  dkr
 * Initial revision
 *
 */

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "dbug.h"
#include "free.h"
#include "DupTree.h"
#include "new_types.h"

/******************************************************************************
 *
 * Function:
 *   node *CWCmodul( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *CWCmodul (node *arg_node, node *arg_info);
{
    DBUG_ENTER ("CWCmodul");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CWCfundef( node *arg_node, node *arg_info);
 *
 * Description:
 *
 *
 ******************************************************************************/

node *CWCfundef (node *arg_node, node *arg_info);
{
    DBUG_ENTER ("CWCfundef");

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * Function:
 *   node *CreateWrapperCode( node *ast)
 *
 * Description:
 *
 *
 ******************************************************************************/

node *
CreateWrapperCode (node *ast)
{
    DBUG_ENTER ("CreateWrapperCode");

    DBUG_RETURN (ast);
}
