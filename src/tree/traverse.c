/*
 *
 * $Log$
 * Revision 3.117  2004/11/27 05:05:25  ktr
 * bugfix.
 *
 * Revision 3.116  2004/11/27 01:33:21  sah
 * implemented TRAVgetName
 *
 * Revision 3.115  2004/11/26 11:56:41  sah
 * *** empty log message ***
 *
 * Revision 3.114  2004/11/23 22:22:03  sah
 * rewrite
 *
 *
 * ... [eliminated] ...
 *
 * Revision 1.2  1994/11/10  15:44:34  sbs
 * RCS-header inserted
 *
 */

#include "traverse.h"
#include "traverse_tables.h"
#include "traverse_helper.h"
#include "globals.h"
#include "internal_lib.h"
#include "dbug.h"
#include "tree_basic.h"

struct TRAVSTACK_T {
    struct TRAVSTACK_T *next;
    travfun_p *funs;
    trav_t traversal;
};

typedef struct TRAVSTACK_T travstack_t;

static travstack_t *travstack = NULL;

node *
TRAVdo (node *arg_node, info *arg_info)
{
    int old_linenum = global.linenum;
    char *old_filename = global.filename;

    DBUG_ENTER ("TRAVdo");

    DBUG_ASSERT ((arg_node != NULL), "Trav: tried to traverse into subtree NULL !");

    DBUG_ASSERT ((NODE_TYPE (arg_node) <= MAX_NODES), "Trav: illegal node type !");

    DBUG_ASSERT ((travstack != NULL), "no traversal on stack!");

    /*
     * Make sure the line-number will be set
     * correctly in case MakeXxx is called.
     */
    global.linenum = NODE_LINE (arg_node);
    global.filename = NODE_FILE (arg_node);

    if (pretable[NODE_TYPE (arg_node)] != NULL) {
        arg_node = pretable[NODE_TYPE (arg_node)](arg_node, arg_info);
    }

    arg_node = (travstack->funs[NODE_TYPE (arg_node)]) (arg_node, arg_info);

    if (posttable[NODE_TYPE (arg_node)] != NULL) {
        arg_node = posttable[NODE_TYPE (arg_node)](arg_node, arg_info);
    }

    global.linenum = old_linenum;
    global.filename = old_filename;

    DBUG_RETURN (arg_node);
}

node *
TRAVcont (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("TRAVcont");

    arg_node = TRAVsons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

void
TRAVpush (trav_t traversal)
{
    travstack_t *new;

    DBUG_ENTER ("TRAVpush");

    new = ILIBmalloc (sizeof (travstack_t));

    new->next = travstack;
    new->traversal = traversal;
    new->funs = travtables[traversal];

    travstack = new;

    DBUG_VOID_RETURN;
}

trav_t
TRAVpop ()
{
    travstack_t *tmp;
    trav_t result;

    DBUG_ENTER ("TRAVpop");

    DBUG_ASSERT ((travstack != NULL), "no traversal on stack!");

    tmp = travstack;
    travstack = tmp->next;
    result = tmp->traversal;

    tmp = ILIBfree (tmp);

    DBUG_RETURN (result);
}

const char *
TRAVgetName ()
{
    const char *result;

    DBUG_ENTER ("TRAVgetName");

    if (travstack == NULL) {
        result = "no_active_traversal";
    } else {
        result = travnames[travstack->traversal];
    }

    DBUG_RETURN (result);
}
