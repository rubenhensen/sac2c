/*
 *
 * $Id$
 *
 */

#include "traverse.h"
#include "traverse_tables.h"
#include "traverse_helper.h"
#include "globals.h"
#include "free.h"
#include "DupTree.h"
#include "internal_lib.h"
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "phase.h"

struct TRAVSTACK_T {
    struct TRAVSTACK_T *next;
    travfun_p *funs;
    trav_t traversal;
};

typedef struct TRAVSTACK_T travstack_t;

static travstack_t *travstack = NULL;

#ifdef SANITYCHECKS

static node *fundef = NULL;

static void
DoSanityChecksPreTraversal (node *arg_node, info *arg_info)
{
    node *ids;

    DBUG_ASSERT ((arg_node != NULL), "Pre Traversal Sanity Check\n"
                                     "Tried to traverse into subtree NULL !");

    DBUG_ASSERT ((NODE_TYPE (arg_node) <= MAX_NODES),
                 "Pre Traversal Sanity Check\n"
                 "Traversed into illegal node type !");

    DBUG_ASSERT ((travstack != NULL), "Pre Traversal Sanity Check\n"
                                      "No traversal on stack!");

    if ((NODE_TYPE (arg_node) == N_module) && (NULL != DUPgetCopiedSpecialFundefs ())) {
        DBUG_ASSERT (FALSE, "Pre Traversal Sanity Check\n"
                            "Unresolved copies of special functions at beginning\n"
                            "of new traversal. Maybe a previous traversal is not\n"
                            "organized properly in the sense that it has a traversal\n"
                            "function for the N_module node.");
        /*
         * This assertion is written in this special style to allow for setting
         * breakpoints in debuggers.
         */
    }

    if (global.valid_ssaform && (NODE_TYPE (arg_node) == N_assign)
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)) {
        ids = LET_IDS (ASSIGN_INSTR (arg_node));

        while (ids != NULL) {
            DBUG_ASSERTF (AVIS_SSAASSIGN (IDS_AVIS (ids)) == arg_node,
                          ("Pre Traversal Sanity Check\n"
                           "Broken SSAASSIGN link for variable %s in function %s\n"
                           "Compiler phase:    %s\n"
                           "Compiler subphase: %s\n"
                           "Traversal:         %s",
                           AVIS_NAME (IDS_AVIS (ids)), FUNDEF_NAME (fundef),
                           PHphaseName (global.compiler_phase),
                           PHsubPhaseName (global.compiler_subphase), TRAVgetName ()));
            ids = IDS_NEXT (ids);
        }
    }

    if (NODE_TYPE (arg_node) == N_fundef) {
        fundef = arg_node;
    }
}

static void
DoSanityChecksPostTraversal (node *arg_node, info *arg_info)
{
    node *ids;

    if (global.valid_ssaform && (NODE_TYPE (arg_node) == N_assign)
        && (NODE_TYPE (ASSIGN_INSTR (arg_node)) == N_let)) {
        ids = LET_IDS (ASSIGN_INSTR (arg_node));

        while (ids != NULL) {
            DBUG_ASSERTF (AVIS_SSAASSIGN (IDS_AVIS (ids)) == arg_node,
                          ("Post Traversal Sanity Check\n"
                           "Broken SSAASSIGN link for variable %s in function %s\n"
                           "Compiler phase:    %s\n"
                           "Compiler subphase: %s\n"
                           "Traversal:         %s",
                           AVIS_NAME (IDS_AVIS (ids)), FUNDEF_NAME (fundef),
                           PHphaseName (global.compiler_phase),
                           PHsubPhaseName (global.compiler_subphase), TRAVgetName ()));
            ids = IDS_NEXT (ids);
        }
    }

    if (NODE_TYPE (arg_node) == N_fundef) {
        fundef = NULL;
    }
}

#endif

node *
TRAVdo (node *arg_node, info *arg_info)
{
    nodetype arg_node_type;
    int old_linenum = global.linenum;
    char *old_filename = global.filename;

    /*
     * check whether the node is non null as this
     * is a common reason for segfaults
     */
    DBUG_ASSERT ((arg_node != NULL), "tried to traverse a node which is NULL!");

#ifdef SANITYCHECKS
    DoSanityChecksPreTraversal (arg_node, arg_info);
#endif

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

        if (MODULE_FUNSPECS (arg_node) != NULL) {
            MODULE_FUNSPECS (arg_node)
              = FREEremoveAllZombies (MODULE_FUNSPECS (arg_node));
        }
    }

#ifdef SANITYCHECKS
    if (arg_node != NULL) {
        DoSanityChecksPostTraversal (arg_node, arg_info);
    }
#endif

    return (arg_node);
}

node *
TRAVcont (node *arg_node, info *arg_info)
{
    arg_node = TRAVsons (arg_node, arg_info);

    return (arg_node);
}

void
TRAVpush (trav_t traversal)
{
    travstack_t *new;

    DBUG_ENTER ("TRAVpush");

    new = MEMmalloc (sizeof (travstack_t));

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

    tmp = MEMfree (tmp);

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
