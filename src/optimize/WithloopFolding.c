/* 	$Id$
 *
 * $Log$
 * Revision 1.3  1998/02/09 15:58:20  srs
 * *** empty log message ***
 *
 *
 */

/*******************************************************************************
 This file realizes the WL-folding for the new SAC-WLs.

 what happens:
 we search for withloops in the body of all functions.  Any found WL will be
 put in the list wl_found.Only the latest (reachable) WLs are in this list.
   E.g. a WL "A = with..." is replaced by another "A = with2..." or removed
   after "A = nonwithexpr".

 Every time we find a new WL we check if we can fold is with one of the
 already known WLs from the wl_found list.

 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "tree.h"
#include "types.h"
#include "internal_lib.h"
#include "free.h"
#include "globals.h"
#include "Error.h"
#include "dbug.h"
#include "my_debug.h"
#include "traverse.h"
#include "WithloopFolding.h"

/*************** typedefs */
typedef struct WL_FOUND_TYPE {
    char *id;
    struct WL_FOUND_TYPE *next;
} wl_found_type;

/*************** variable declarations */
wl_found_type *wl_found;

/******************************************************************************
 *
 * description:
 *   functions to handle wl_found list.
 *   ToList     : add element
 *   ClearList  : clear whole list
 *   RemoveList : remove one element
 *   SearchList : search for one element
 *
 ******************************************************************************/

wl_found_type *
ToList (char *name)
{ /* add new entry, return pointer */
    wl_found_type *tmp;
    tmp = (wl_found_type *)Malloc (sizeof (wl_found_type));
    tmp->id = name;
    tmp->next = wl_found;
    wl_found = tmp;
    return (tmp);
}

void
ClearList (void)
{ /* clear whole list */
    wl_found_type *tmp;
    while (wl_found) {
        tmp = wl_found->next;
        FREE (wl_found);
        wl_found = tmp;
    }
}

void
RemoveList (char *name)
{ /* remove one element */
    wl_found_type *tmp, *rm, *pre;
    tmp = wl_found;
    pre = NULL;
    while (tmp)
        if (!strcmp (tmp->id, name)) {
            rm = tmp;
            tmp = tmp->next;
            if (pre)
                pre->next = tmp;
            else
                wl_found = tmp;
            FREE (rm);
        } else {
            pre = tmp;
            tmp = tmp->next;
        }
}

wl_found_type *
SearchList (char *name)
{ /* search list for the entry 'name'. */
    wl_found_type *tmp;
    DBUG_ENTER ("SearchList");

    tmp = wl_found;
    while (tmp)
        if (!strcmp (tmp->id, name))
            break;
        else
            tmp = tmp->next;

    DBUG_RETURN (tmp);
}

/******************************************************************************
 *
 * function:
 *   node *WLFNwith(node *arg_node, node *arg_info)
 *
 * description:
 *
 *
 *
 ******************************************************************************/

node *
WLFNwith (node *arg_node, node *arg_info)
{
    node *tmpn;

    DBUG_ENTER ("WLFNwith");

    INFO_IS_WL (arg_info) = 1; /* we found a WL here */
    /* inside the body of this WL we may find another WL. So we better
       save this information. */
    tmpn = MakeInfo ();
    INFO_NEXT (tmpn) = arg_info;
    arg_info = tmpn;

    INFO_WL (arg_info) = arg_node; /* store the current node for later */

    /* restore arg_info */
    tmpn = arg_info;
    arg_info = INFO_NEXT (arg_info);
    FREE (tmpn);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFlet(node *arg_node, node *arg_info)
 *
 * description:
 *   adds (or replaces) entries to WL_found list if this is an assignment
 *   with a WL expression. Removes entry if assignment is not a WL.
 *
 ******************************************************************************/

node *
WLFlet (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFlet");

    INFO_IS_WL (arg_info) = 0;
    arg_node = Trav (LET_EXPR (arg_node), arg_info);

    /* now we modify the WL_found list */
    RemoveList (LET_NAME (arg_node));
    if (INFO_IS_WL (arg_info))
        ToList (LET_NAME (arg_node));

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   we search in every function separately for withloops.
 *   The folding always happens exclusively in a function body, so the
 *   so far found WLs have to be ignored for every new function traversal.
 *
 ******************************************************************************/

node *
WLFfundef (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFfundef");
    DBUG_ASSERT (!wl_found, ("There exist WLs in wl_found at the "
                             "beginning of a function body"));

    /* traverse function body */
    arg_node = Trav (FUNDEF_BODY (arg_node), arg_info);

    /* clear old wl_found list */
    ClearList ();

    /* traverse the other FUNDEFs */
    arg_node = Trav (FUNDEF_NEXT (arg_node), arg_info);

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *WLFWithloopFolding(node *arg_node, node* arg_info)
 *
 * description:
 *   starting point for the withloop folding.
 *
 *
 ******************************************************************************/

node *
WLFWithloopFolding (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("WLFWithloopFolding");

    act_tab = wlf_tab;
    wl_found = NULL;

    DBUG_ASSERT (!arg_info, ("at the beginning of WLF: arg_info != NULL"));
    arg_info = MakeInfo ();

    Trav (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}
