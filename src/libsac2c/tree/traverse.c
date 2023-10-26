#include "traverse.h"

#include "traverse_tables.h"
#include "traverse_helper.h"
#include "globals.h"
#include "free.h"
#include "DupTree.h"
#include "str.h"
#include "memory.h"

#define DBUG_PREFIX "TRAVSTACK"
#include "debug.h"

#include "tree_basic.h"
#include "tree_compound.h"
#include "phase.h"
#include "math_utils.h"
#include "sanity_checks.h"
#include "memory.h"
#include "group_local_funs.h"
#include "string.h"

#define MAX_VAR_BUFFER_SIZE 30

struct TRAVSTACK_T {
    struct TRAVSTACK_T *next;
    travfun_p *funs;
    trav_t traversal;
};

typedef struct TRAVSTACK_T travstack_t;

static travstack_t *travstack = NULL;

struct LAC_INFO {
    bool lacfunok;
    int blocklevel;
};

#define LAC_INFO_LACFUNOK(n) ((n)->lacfunok)
#define LAC_INFO_BLOCKLEVEL(n) ((n)->blocklevel)

node *
TRAVdo (node *arg_node, info *arg_info)
{
    nodetype arg_node_type;
    travfunlist_t *tmpfunlist = NULL;
    size_t old_linenum = global.linenum;
    size_t old_colnum = global.colnum;
    char *old_filename = global.filename;
    static node *arg_last = NULL;
    node *special_funs;

#ifdef SANITYCHECKS
    if (global.sancheck && (travstack->traversal != TR_anonymous)) {
        SANCHKdoSanityChecksPreTraversal (arg_node, arg_info, travstack);
    }
#endif

    DBUG_ASSERT (arg_node, "OOOOOOOPS: TRAVdo() called with a NULL node!");

    /*
     * Make sure the line-number will be set
     * correctly in case MakeXxx is called.
     */
    global.linenum = NODE_LINE (arg_node);
    global.colnum = NODE_COL (arg_node);
    global.filename = (char *)NODE_FILE (arg_node);

    /*
     * Save node type as it might be modified during traversal
     */
    arg_node_type = NODE_TYPE (arg_node);

    if (global.local_funs_grouped && (travstack->traversal != TR_anonymous)
        && (NODE_TYPE (arg_node) == N_fundef) && (!GLFisLocalFun (arg_node))) {
        DBUG_ASSERT ((arg_last != NULL) || (DUPgetCopiedSpecialFundefsHook () == NULL),
                     "arg_last unset in traverse.c but copied special funs exist");

        special_funs = DUPgetCopiedSpecialFundefs ();
        if (special_funs != NULL) {
            FUNDEF_LOCALFUNS (arg_last)
              = TCappendFundef (special_funs, FUNDEF_LOCALFUNS (arg_last));
        }

        arg_last = arg_node;
    }

    if (pretable[travstack->traversal] != NULL) {
        tmpfunlist = pretable[travstack->traversal];
        while (tmpfunlist != NULL) {
            arg_node = (*tmpfunlist->fun) (arg_node, arg_info);
            tmpfunlist = tmpfunlist->next;
        }
    }

    arg_node = (travstack->funs[arg_node_type]) (arg_node, arg_info);

    if (posttable[travstack->traversal] != NULL) {
        tmpfunlist = posttable[travstack->traversal];
        while (tmpfunlist != NULL) {
            arg_node = (*tmpfunlist->fun) (arg_node, arg_info);
            tmpfunlist = tmpfunlist->next;
        }
    }

    global.linenum = old_linenum;
    global.colnum = old_colnum;
    global.filename = old_filename;

    if ((arg_node != NULL) && (travstack->traversal != TR_anonymous)) {
        switch (NODE_TYPE (arg_node)) {
        case N_module:
            MODULE_FUNS (arg_node)
              = TCappendFundef (DUPgetCopiedSpecialFundefs (), MODULE_FUNS (arg_node));

            if (MODULE_FUNS (arg_node) != NULL) {
                MODULE_FUNS (arg_node) = FREEremoveAllZombies (MODULE_FUNS (arg_node));
            }

            if (MODULE_FUNDECS (arg_node) != NULL) {
                MODULE_FUNDECS (arg_node)
                  = FREEremoveAllZombies (MODULE_FUNDECS (arg_node));
            }

            if (MODULE_FUNSPECS (arg_node) != NULL) {
                MODULE_FUNSPECS (arg_node)
                  = FREEremoveAllZombies (MODULE_FUNSPECS (arg_node));
            }
            break;
        case N_fundef:
            if (global.local_funs_grouped && !GLFisLocalFun (arg_node)) {
                FUNDEF_LOCALFUNS (arg_node)
                  = TCappendFundef (DUPgetCopiedSpecialFundefs (),
                                    FUNDEF_LOCALFUNS (arg_node));
                if (FUNDEF_LOCALFUNS (arg_node) != NULL) {
                    FUNDEF_LOCALFUNS (arg_node)
                      = FREEremoveAllZombies (FUNDEF_LOCALFUNS (arg_node));
                }
            }
            break;
        default:
            break;
        }
    }

#ifdef SANITYCHECKS
    if (arg_node != NULL) {
        if (global.sancheck && (travstack->traversal != TR_anonymous)) {
            SANCHKdoSanityChecksPostTraversal (arg_node, arg_info, travstack);
        }
    }
#endif

    return (arg_node);
}

node *
TRAVopt (node *arg_node, info *arg_info)
{
    if (arg_node != NULL) {
        arg_node = TRAVdo (arg_node, arg_info);
    }

    return (arg_node);
}

node *
TRAVcont (node *arg_node, info *arg_info)
{
    arg_node = TRAVsons (arg_node, arg_info);

    return (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn lac_info_t *TRAVlacNewInfo( bool lacfunok)
 *
 * @brief Creates a new lac information structure to be used with the
 *        special TRAVlac functions.
 *
 * @param lacfunok Set to true if LAC functions should be traversed from
 *        the top level, i.e., TRAVlacContBody will traverse into lac
 *        functions, as well. If set to false, TRAVlacDoFun will continue
 *        into LAC funs.
 *
 * @return new lac information structure
 ******************************************************************************/
lac_info_t *
TRAVlacNewInfo (bool lacfunok)
{
    lac_info_t *result;

    DBUG_ENTER ();

    result = (lac_info_t *)MEMmalloc (sizeof (lac_info_t));

    LAC_INFO_LACFUNOK (result) = lacfunok;
    LAC_INFO_BLOCKLEVEL (result) = 0;

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn lac_info_t *TRAVlacFreeInfo( lac_info_t *lac_info)
 *
 * @brief Frees a lac information structure.
 *
 * @param lac_info the information structure to be freed.
 *
 * @return NULL
 ******************************************************************************/
lac_info_t *
TRAVlacFreeInfo (lac_info_t *lac_info)
{
    DBUG_ENTER ();

    lac_info = MEMfree (lac_info);

    DBUG_RETURN (lac_info);
}

/** <!-- ****************************************************************** -->
 * @fn bool TRAVlacIsSuccOf( node *succ, node *parent, lac_info_t *lac_info)
 *
 * @brief Returns true if succ is a child of parent in the spanning tree of
 *        the syntax graph. Depending on the lac traversal mode, lac functions
 *        are either sons in the top-level N_fundef chain or children of the
 *        corresponding N_ap node.
 *
 * @param succ      node to be checked for successor
 * @param parent    potential parent node
 * @param lac_info  lac information structure
 *
 * @return true if succ is a child of parent, false otherwise
 ******************************************************************************/
bool
TRAVlacIsSuccOf (node *succ, node *parent, lac_info_t *lac_info)
{
    bool result;

    DBUG_ENTER ();

    if (succ == NULL) {
        result = FALSE;
    } else if ((NODE_TYPE (succ) == N_fundef) && (NODE_TYPE (parent) == N_ap)) {
        /* called in N_ap node to decide whether to traverse into fundef */
        result = (FUNDEF_ISLACFUN (succ) && !LAC_INFO_LACFUNOK (lac_info)
                  && !AP_ISRECURSIVEDOFUNCALL (parent));
    } else if ((NODE_TYPE (succ) == N_block) && (NODE_TYPE (parent) == N_fundef)) {
        /* called in N_fundef to decide whether to traverse the body */
        result = (!FUNDEF_ISLACFUN (parent) || LAC_INFO_LACFUNOK (lac_info)
                  || (LAC_INFO_BLOCKLEVEL (lac_info) != 0));
    } else if ((NODE_TYPE (succ) == N_fundef) && (NODE_TYPE (parent) == N_fundef)) {
        /* called in N_fundef to decide whether to follow the next */
        result = (LAC_INFO_BLOCKLEVEL (lac_info) == 0);
    } else {
        DBUG_UNREACHABLE ("TRAVlacIsSuccOf called with illegal succ/parent "
                          "combination");

        result = FALSE;
    }

    DBUG_RETURN (result);
}

/** <!-- ****************************************************************** -->
 * @fn node *TRAVlacDo( node *arg_node, info *arg_info, lac_info_t *lac_info)
 *
 * @brief Special version of TRAVdo to be used in lac traversals, at least
 *        in N_fundef nodes to traverse the body. Note that arg_node may not
 *        be NULL.
 *
 * @param arg_node node to traverse into
 * @param arg_info info structure to pass along
 * @param lac_info lac information structure
 *
 * @return the result of the traversal.
 ******************************************************************************/
node *
TRAVlacDo (node *arg_node, info *arg_info, lac_info_t *lac_info)
{
    DBUG_ASSERT (arg_node != NULL, "TRAVlacDo called with null as node");

    if (NODE_TYPE (arg_node) == N_block) {
        LAC_INFO_BLOCKLEVEL (lac_info)++;
    }

    arg_node = TRAVdo (arg_node, arg_info);

    if (NODE_TYPE (arg_node) == N_block) {
        LAC_INFO_BLOCKLEVEL (lac_info)--;
    }

    return (arg_node);
}

/** <!-- ****************************************************************** -->
 * @fn node *TRAVlacOpt( node *arg_node, info *arg_info, lac_info_t *lac_info)
 *
 * @brief Special version of TRAVdo to be used in lac traversals, at least
 *        in N_fundef nodes to traverse the body.
 *
 * @param arg_node node to traverse into
 * @param arg_info info structure to pass along
 * @param lac_info lac information structure
 *
 * @return the result of the traversal.
 ******************************************************************************/
node *
TRAVlacOpt (node *arg_node, info *arg_info, lac_info_t *lac_info)
{
    if (arg_node != NULL) {
        arg_node = TRAVlacDo (arg_node, arg_info, lac_info);
    }

    return (arg_node);
}

void
TRAVpush (trav_t traversal)
{
    travstack_t *xnew;

    DBUG_ENTER ();

    xnew = (travstack_t *)MEMmalloc (sizeof (travstack_t));

    xnew->next = travstack;
    xnew->traversal = traversal;
    xnew->funs = travtables[traversal];

    travstack = xnew;

    DBUG_RETURN ();
}

void
TRAVpushAnonymous (anontrav_t *anontraversal, travfun_p deffun)
{
    travfun_p *travmap;
    travstack_t *xnew;
    int pos;

    DBUG_ENTER ();

    DBUG_ASSERT (anontraversal != NULL, "empty anonymous traversal!");

    travmap = (travfun_p *)MEMmalloc (sizeof (travfun_p) * (MAX_NODES + 1));

    for (pos = 0; pos <= MAX_NODES; pos++) {
        travmap[pos] = deffun;
    }

    for (pos = 0; anontraversal[pos].node != 0; pos++) {
        travmap[anontraversal[pos].node] = anontraversal[pos].travfun;
    }

    xnew = (travstack_t *)MEMmalloc (sizeof (travstack_t));

    xnew->next = travstack;
    xnew->traversal = TR_anonymous;
    xnew->funs = travmap;

    travstack = xnew;

    DBUG_RETURN ();
}

trav_t
TRAVpop (void)
{
    travstack_t *tmp;
    trav_t result;

    DBUG_ENTER ();

    DBUG_ASSERT (travstack != NULL, "no traversal on stack!");

    tmp = travstack;
    travstack = tmp->next;
    result = tmp->traversal;

    if (tmp->traversal == TR_anonymous) {
        tmp->funs = MEMfree (tmp->funs);
    }
    tmp = MEMfree (tmp);

    DBUG_RETURN (result);
}

#ifdef DO_NOT_ADD_SUFFIX_IN_ANONYMOUS_TRAVERSAL

const char *
TRAVgetName (void)
{
    const char *result;

    DBUG_ENTER ();

    if (travstack == NULL) {
        result = "notrav";
    } else {
        result = travnames[travstack->traversal];
    }

    DBUG_RETURN (result);
}

#else /* DO_NOT_ADD_SUFFIX_IN_ANONYMOUS_TRAVERSAL */

#define MAX_VAR_BUFFER_SIZE 30

const char *
TRAVgetName (void)
{
    travstack_t *tmp;
    bool anonymous;
    static char buffer[MAX_VAR_BUFFER_SIZE + 1];

    DBUG_ENTER ();

    tmp = travstack;
    anonymous = FALSE;

    while ((tmp != NULL) && (tmp->traversal == TR_anonymous)) {
        tmp = tmp->next;
        anonymous = TRUE;
    }

    if (tmp == NULL) {
        strncpy (buffer, "notrav", MAX_VAR_BUFFER_SIZE);
    } else if (anonymous) {
        strncpy (buffer, travnames[tmp->traversal], MAX_VAR_BUFFER_SIZE - 5);
        strcat (buffer, "anon");
        DBUG_PRINT_TAG ("TRAVANON", "Anonymous identifier generated: %s", buffer);
    } else {
        strncpy (buffer, travnames[tmp->traversal], MAX_VAR_BUFFER_SIZE);
    }

    DBUG_RETURN (buffer);
}

#undef MAX_VAR_BUFFER_SIZE

#endif /* DO_NOT_ADD_SUFFIX_IN_ANONYMOUS_TRAVERSAL */

/******************************************************************************
 *
 * Function:
 *   char *TRAVtmpVar( void)
 *
 * Description:
 *   Generates string to be used as artificial variable.
 *   The variable name is different in each call of TRAVtmpVar().
 *   The string has the form "_" ++ traversal ++ "_" ++ consecutive number.
 *
 ******************************************************************************/

char *
TRAVtmpVar (void)
{
    static int counter = 0;
    const char *prefix;
    char *result;

    DBUG_ENTER ();

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
TRAVtmpVarName (const char *postfix)
{
    const char *tmp;
    const char *new_postfix;
    char *result, *prefix;

    DBUG_ENTER ();

    /* avoid chains of same prefixes */
    tmp = TRAVgetName ();
    new_postfix = postfix;

    if ((new_postfix[0] == '_') && STRprefix (tmp, new_postfix + 1)) {
        new_postfix = new_postfix + STRlen (tmp) + 1;

        /* check for the dividing '_' between travtag and number */
        if (new_postfix[0] == '_') {
            /* found divider */
            new_postfix++;

            /* ignore previous number tag */
            while ((new_postfix[0] >= '0') && (new_postfix[0] <= '9')) {
                new_postfix++;
            }

            if (new_postfix[0] == '_') {
                /* this is the final bit of the previous postfix */
                new_postfix++;
            } else {
                /* this does not match the pattern => roll back */
                new_postfix = postfix;
            }
        } else {
            /* this does not match the pattern => roll back */
            new_postfix = postfix;
        }
    }

    prefix = TRAVtmpVar ();

    result = STRcatn (3, prefix, "_", new_postfix);

    MEMfree (prefix);

    DBUG_RETURN (result);
}

#ifndef DBUG_OFF
/** <!-- ****************************************************************** -->
 * @fn void TRAVprintStack( void)
 *
 * @brief Prints the current traversal stack, iff -#d,TRAVSTACK is set.
 *        Mainly useful in debuggers...
 ******************************************************************************/
void
TRAVprintStack (void)
{
    travstack_t *tmp = travstack;

    DBUG_ENTER ();

    DBUG_PRINT ("Current traversal stack:");
    while (tmp != NULL) {
        DBUG_PRINT ("  %s", travnames[tmp->traversal]);
        tmp = tmp->next;
    }
    DBUG_PRINT ("End of traversal stack");

    DBUG_RETURN ();
}
#endif /* DBUG_OFF */

#undef DBUG_PREFIX
