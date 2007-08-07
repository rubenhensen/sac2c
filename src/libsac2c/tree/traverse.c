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
#include "str.h"
#include "memory.h"
#include "dbug.h"
#include "tree_basic.h"
#include "tree_compound.h"
#include "phase.h"
#include "math_utils.h"
#include "sanity_checks.h"

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

#ifdef SANITYCHECKS
    if (global.sancheck) {
        SANCHKdoSanityChecksPreTraversal (arg_node, arg_info, travstack);
    }
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
        if (global.sancheck) {
            SANCHKdoSanityChecksPostTraversal (arg_node, arg_info, travstack);
        }
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

/******************************************************************************
 *
 * Function:
 *   char *TRAVtmpVar( void)
 *
 * Description:
 *   Generates string to be used as artificial variable.
 *   The variable name is different in each call of TRAVtmpVar().
 *   The string has the form "__tmp_" ++ traversal ++ consecutive number.
 *
 ******************************************************************************/

char *
TRAVtmpVar (void)
{
    static int counter = 0;
    const char *prefix;
    char *result;

    DBUG_ENTER ("TRAVtmpVar");

    prefix = TRAVgetName ();
    result = (char *)MEMmalloc ((STRlen (prefix) + MATHnumDigits (counter) + 3)
                                * sizeof (char));
    sprintf (result, "_%s_%d", prefix, counter);
    counter++;

    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   char *TRAVtmpVarName( char* postfix)
 *
 * description:
 *   creates a unique variable like TRAVtmpVar() and additionally appends
 *   an individual string.
 *
 ******************************************************************************/

char *
TRAVtmpVarName (char *postfix)
{
    const char *tmp;
    char *result, *prefix;

    DBUG_ENTER ("TRAVtmpVarName");

    /* avoid chains of same prefixes */
    tmp = TRAVgetName ();

    if ((STRlen (postfix) > (STRlen (tmp) + 1)) && (postfix[0] == '_')
        && STRprefix (tmp, postfix + 1)) {
        postfix = postfix + STRlen (tmp) + 2;
        while (postfix[0] != '_') {
            postfix++;
        }
    }

    prefix = TRAVtmpVar ();

    result = STRcatn (3, prefix, "_", postfix);

    MEMfree (prefix);

    DBUG_RETURN (result);
}
