/*
 *
 * $Log$
 * Revision 1.1  2002/07/09 12:54:25  sbs
 * Initial revision
 *
 *
 */

#include "handle_dots.h"
#include "traverse.h"
#include "dbug.h"
#include "free.h"

/******************************************************************************
 *
 * function:
 *    node * EliminateSelDots( node * arg_node);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
EliminateSelDots (node *arg_node)
{
    funtab *tmp_tab;
    node *info_node;

    DBUG_ENTER ("EliminateSelDots");

    tmp_tab = act_tab;
    act_tab = hd_tab;

    info_node = MakeInfo ();

    arg_node = Trav (arg_node, info_node);

    info_node = FreeNode (info_node);

    act_tab = tmp_tab;

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *    node * HDprf( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDprf (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDprf");

    DBUG_PRINT ("HDSEL", ("Hallo hier bin ich!"));

    DBUG_RETURN (arg_node);
}
