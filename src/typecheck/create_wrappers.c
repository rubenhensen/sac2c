/*
 *
 * $Log$
 * Revision 1.3  2002/03/05 15:51:17  sbs
 * *** empty log message ***
 *
 * Revision 1.2  2002/03/05 15:40:40  sbs
 * CRTWRP traversal embedded.
 *
 * Revision 1.1  2002/03/05 13:59:27  sbs
 * Initial revision
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "internal_lib.h"
#include "traverse.h"
#include "globals.h"

#include "user_types.h"
#include "new_types.h"

#include "create_wrappers.h"

/******************************************************************************
 *
 * function:
 *    node *CreateWrappers(node *arg_node)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CreateWrappers (node *arg_node)
{
    funtab *tmp_tab;

    DBUG_ENTER ("CreateWrappers");

    tmp_tab = act_tab;
    act_tab = crtwrp_tab;

    arg_node = Trav (arg_node, NULL);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node *CRTWRPfundef(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 ******************************************************************************/

node *
CRTWRPfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("CRTWRPfundef");

    DBUG_RETURN (arg_node);
}
