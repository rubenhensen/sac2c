/*
 *
 * $Log$
 * Revision 1.4  2002/07/24 10:40:35  dkr
 * FreeDotInfo: DBUG_VOID_RETURN added
 * tabs removed
 *
 * Revision 1.2  2002/07/19 13:24:49  sah
 * added functions for traversal.
 *
 * Revision 1.1  2002/07/09 12:54:25  sbs
 * Initial revision
 *
 */

#include "handle_dots.h"
#include "traverse.h"
#include "dbug.h"
#include "free.h"

#define HD_DOTINFO(n) ((dotinfo *)n->info2)
#define HD_DI_LEFT(n) HD_DOTINFO (n)->left;
#define HD_DI_RIGHT(n) HD_DOTINFO (n)->right;
#define HD_DI_DOTCNT(n) HD_DOTINFO (n)->dotcnt;
#define HD_DI_TDOTCNT(n) HD_DOTINFO (n)->tripledotcnt;
#define HD_DI_SELCNT(n) HD_DOTINFO (n)->selcnt;

typedef struct DOTLIST {
    int position;         /* position of dot within selection */
    int dotcount;         /* type of dot; 1:'.' 3: '...' */
    struct DOTLIST *next; /* for building up a list ;) */
    struct DOTLIST *last;
} dotlist;

typedef struct DOTINFO {
    dotlist *left;    /* left end of dotlist */
    dotlist *right;   /* right end */
    int dotcnt;       /* amount of dots found */
    int tripledotcnt; /* amount of tripedots found */
    int selcnt;       /* amount of selectors at all */
} dotinfo;

dotinfo *
MakeDotInfo ()
{
    dotinfo *result;

    DBUG_ENTER ("MakeDotInfo");

    result = (dotinfo *)Malloc (sizeof (dotinfo));

    result->left = NULL;
    result->right = NULL;
    result->dotcnt = 0;
    result->tripledotcnt = 0;
    result->selcnt = 0;

    DBUG_RETURN (result);
}

void
FreeDotInfo (dotinfo *node)
{
    DBUG_ENTER ("FreeDotInfo");

    while (node->left != NULL) {
        dotlist *tmp = node->left;
        node->left = node->left->next;
        Free (tmp);
    }

    Free (node);

    DBUG_VOID_RETURN;
}

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
 *    node * HDdot( node *arg_node, node *arg_info);
 *
 * description:
 *    ...
 *
 ******************************************************************************/

node *
HDdot (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("HDdot");

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
    dotinfo *old_dotinfo;

    DBUG_ENTER ("HDprf");

    old_dotinfo = HD_DOTINFO (arg_info);
    HD_DOTINFO (arg_info) = MakeDotInfo ();

    FreeDotInfo (HD_DOTINFO (arg_info));
    HD_DOTINFO (arg_info) = old_dotinfo;

    DBUG_RETURN (arg_node);
}
