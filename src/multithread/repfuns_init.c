/*
 *
 * $Log$
 * Revision 1.2  2000/02/04 14:44:43  jhs
 * Added infrastructure.
 *
 * Revision 1.1  2000/02/04 13:48:36  jhs
 * Initial revision
 *
 *
 */

/******************************************************************************
 *
 * file:   blocks_init.c
 *
 * prefix: RFIN
 *
 * description:
 *   ####
 *
 ******************************************************************************/

#include "dbug.h"

#include "types.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "traverse.h"
#include "scheduling.h"
#include "DupTree.h"
#include "generatemasks.h"
#include "globals.h"

#include "internal_lib.h"

/******************************************************************************
 *
 * function:
 *   node *RepfunsInit(node *arg_node, node *arg_info)
 *
 * description:
 *   ####
 *
 ******************************************************************************/
node *
RepfunsInit (node *arg_node, node *arg_info)
{
    funtab *old_tab;
    int old_withinwith;

    DBUG_ENTER ("RepfunsInit");

    old_tab = act_tab;
    act_tab = rfin_tab;

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);
    INFO_RFIN_WITHINWITH (arg_info) = FALSE;

    FUNDEF_BODY (arg_node) = Trav (FUNDEF_BODY (arg_node), arg_info);

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;

    act_tab = old_tab;

    DBUG_RETURN (arg_node);
}

node *
RFINnwith2 (node *arg_node, node *arg_info)
{
    int old_withinwith;

    DBUG_ENTER ("RFINnwith2");

    old_withinwith = INFO_RFIN_WITHINWITH (arg_info);
    INFO_RFIN_WITHINWITH (arg_info) = TRUE;

    /* trav segments  */

    INFO_RFIN_WITHINWITH (arg_info) = old_withinwith;
    ;

    DBUG_RETURN (arg_node);
}

node *
RFINlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("RFINlet");

    /* if lhs application then lift */

    DBUG_RETURN (arg_node);
}
