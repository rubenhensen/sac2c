/*
 *
 * $Log$
 * Revision 3.121  2005/03/04 21:21:42  cg
 * Whenever the traversal reaches an N_module node, the
 * situation is considered save to add silently duplicated
 * LaC functions and to remove zombified functions from the
 * fundef chain. This is done once here and reliefs all other
 * compiler modules from caring about the issue.
 *
 * Revision 3.120  2004/12/01 15:23:53  sah
 * fixed pre/posttables
 *
 * Revision 3.119  2004/12/01 14:33:07  sah
 * added support for TRAVsetPreFun TRAVsetPostFun
 *
 * Revision 3.118  2004/11/28 22:15:57  ktr
 * NODE_TYPE is rescued before traversal as it might return NULL.
 *
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
    nodetype arg_node_type;
    int old_linenum = global.linenum;
    char *old_filename = global.filename;

    DBUG_ENTER ("TRAVdo");

    DBUG_ASSERT ((arg_node != NULL), "Trav: tried to traverse into subtree NULL !");

    DBUG_ASSERT ((NODE_TYPE (arg_node) <= MAX_NODES), "Trav: illegal node type !");

    DBUG_ASSERT ((travstack != NULL), "no traversal on stack!");

    if ((NODE_TYPE (arg_node) == N_module) && (NULL != DUPgetCopiedSpecialFundefs ())) {
        DBUG_ASSERT (FALSE, "Unresolved copies of special functions at beginning"
                            " of new traversal. Maybe a previous traversal is not"
                            " organized properly in the sense that it has a traversal"
                            " function for the N_module node.");
        /*
         * This assertion is written in this special style to allow for setting
         * breakpoints in debuggers.
         */
    }

    /*
     * Make sure the line-number will be set
     * correctly in case MakeXxx is called.
     */
    global.linenum = NODE_LINE (arg_node);
    global.filename = NODE_FILE (arg_node);

    /*
     * Save node type as it might be modified during traversal
     */
    arg_node_type = NODE_TYPE (arg_node);

    if (pretable[travstack->traversal] != NULL) {
        arg_node = pretable[travstack->traversal](arg_node, arg_info);
    }

    arg_node = (travstack->funs[arg_node_type]) (arg_node, arg_info);

    if (posttable[travstack->traversal] != NULL) {
        arg_node = posttable[travstack->traversal](arg_node, arg_info);
    }

    global.linenum = old_linenum;
    global.filename = old_filename;

    if ((arg_node != NULL) && (NODE_TYPE (arg_node) == N_module)) {
        /*
         * arg_node may have become NULL during traversal.
         */
        MODULE_FUNS (arg_node)
          = TCappendFundef (DUPgetCopiedSpecialFundefs (), MODULE_FUNS (arg_node));
        if (MODULE_FUNS (arg_node) != NULL) {
            MODULE_FUNS (arg_node) = FREEremoveAllZombies (MODULE_FUNS (arg_node));
        }

        if (MODULE_FUNDECS (arg_node) != NULL) {
            MODULE_FUNDECS (arg_node) = FREEremoveAllZombies (MODULE_FUNDECS (arg_node));
        }
    }

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

void
TRAVsetPreFun (trav_t traversal, travfun_p prefun)
{
    DBUG_ENTER ("TRAVsetPreFun");

    pretable[traversal] = prefun;

    DBUG_VOID_RETURN;
}

void
TRAVsetPostFun (trav_t traversal, travfun_p postfun)
{
    DBUG_ENTER ("TRAVsetPreFun");

    posttable[traversal] = postfun;

    DBUG_VOID_RETURN;
}
