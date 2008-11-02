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
#include "memory.h"

struct TRAVSTACK_T {
    struct TRAVSTACK_T *next;
    travfun_p *funs;
    trav_t traversal;
};

typedef struct TRAVSTACK_T travstack_t;

static travstack_t *travstack = NULL;

struct LAC_INFO {
    bool lacfunok;
    bool travinlac;
};

#define LAC_INFO_LACFUNOK(n) ((n)->lacfunok)
#define LAC_INFO_TRAVINLAC(n) ((n)->travinlac)

node *
TRAVdo (node *arg_node, info *arg_info)
{
    nodetype arg_node_type;
    int old_linenum = global.linenum;
    char *old_filename = global.filename;

#ifdef SANITYCHECKS
    if (global.sancheck && (travstack->traversal != TR_anonymous)) {
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

    if ((arg_node != NULL) && (travstack->traversal != TR_anonymous)
        && (NODE_TYPE (arg_node) == N_module)) {
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

    DBUG_ENTER ("TRAVlacNewInfo");

    result = MEMmalloc (sizeof (lac_info_t));

    LAC_INFO_LACFUNOK (result) = lacfunok;
    LAC_INFO_TRAVINLAC (result) = FALSE;

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
    DBUG_ENTER ("TRAVlacFreeInfo");

    lac_info = MEMfree (lac_info);

    DBUG_RETURN (lac_info);
}

/** <!-- ****************************************************************** -->
 * @fn node *TRAVlacContFun( node *ap, info *arg_info, lac_info_t *lac_info)
 *
 * @brief This function is to be used to continue traversal into N_ap's fundef
 *        attribute. Depending on the traversal mode, it either continues
 *        with the LAC function (if not in top-level mode) or directly
 *        returns. Furthermore, the function correctly handles the recursive
 *        call in loop functions.
 *
 * @param ap N_ap node to start traversal from
 * @param arg_info info structure to be passed along
 * @param lac_info lac info structure
 *
 * @return node N_ap node with fundef attribute updated by traversal
 ******************************************************************************/
node *
TRAVlacContFun (node *ap, info *arg_info, lac_info_t *lac_info)
{
    bool wasinlac;

    DBUG_ENTER ("TRAVlacDoFun");

    DBUG_ASSERT ((NODE_TYPE (ap) == N_ap), "TRAVlacContFun called on non-ap node");

    DBUG_ASSERT ((AP_FUNDEF (ap) != NULL),
                 "TRAVlacContFun called with null fundef attribute");

    if (FUNDEF_ISLACFUN (AP_FUNDEF (ap)) && !LAC_INFO_LACFUNOK (lac_info)
        && !AP_ISRECURSIVEDOFUNCALL (ap)) {
        wasinlac = LAC_INFO_TRAVINLAC (lac_info);
        LAC_INFO_TRAVINLAC (lac_info) = TRUE;

        AP_FUNDEF (ap) = TRAVdo (AP_FUNDEF (ap), arg_info);

        LAC_INFO_TRAVINLAC (lac_info) = wasinlac;
    }

    DBUG_RETURN (ap);
}

/** <!-- ****************************************************************** -->
 * @fn node *TRAVlacContBody( node *fundef, info *arg_info,
 *                            lac_info_t *lac_info)
 *
 * @brief This function is to be used to continue traversal into the body
 *        son of a N_fundef node. If in top-level mode, the function
 *        continues into every non-null body. Otherwise, it continues only
 *        into non-lac functions.
 *
 * @param fundef N_fundef node whose body is to be traversed
 * @param arg_info info structure to be passed on
 * @param lac_info lac info structure
 *
 * @return N_fundef node with updated body son
 ******************************************************************************/
node *
TRAVlacContBody (node *fundef, info *arg_info, lac_info_t *lac_info)
{
    DBUG_ENTER ("TRAVlacContBody");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "TRAVlacContBody called on non-fundef node");

    if (!FUNDEF_ISLACFUN (fundef) || LAC_INFO_LACFUNOK (lac_info)
        || LAC_INFO_TRAVINLAC (lac_info)) {
        TRAVopt (FUNDEF_BODY (fundef), arg_info);
    }

    DBUG_RETURN (fundef);
}

/** <!-- ****************************************************************** -->
 * @fn node *TRAVlacOptNext( node *fundef, info *arg_info,
 *                           lac_info_t *lac_info)
 *
 * @brief This function has to be used to continue to the next fundef when
 *        using the lac traversal functions. Depending on whether currently
 *        a lac function or a top-level function is traversed, this function
 *        either returns directly or continues with the next son of the
 *        N_fundef argument.
 *
 *
 * @param fundef N_fundef node whose next son is to be traversed
 * @param arg_info info structure to be passed on
 * @param lac_info lac info structure
 *
 * @return N_fundef node with updated next son.
 ******************************************************************************/
node *
TRAVlacOptNext (node *fundef, info *arg_info, lac_info_t *lac_info)
{
    DBUG_ENTER ("TRAVlacOptNext");

    DBUG_ASSERT ((NODE_TYPE (fundef) == N_fundef),
                 "TRAVlacOptNext called on non-fundef node");

    if (!LAC_INFO_TRAVINLAC (lac_info)) {
        TRAVopt (FUNDEF_NEXT (fundef), arg_info);
    }

    DBUG_RETURN (fundef);
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

void
TRAVpushAnonymous (anontrav_t *anontraversal, travfun_p deffun)
{
    travfun_p *travmap;
    travstack_t *new;
    int pos;

    DBUG_ENTER ("TRAVpushAnonymous");

    DBUG_ASSERT ((anontraversal != NULL), "empty anonymous traversal!");

    travmap = MEMmalloc (sizeof (travfun_p) * (MAX_NODES + 1));

    for (pos = 0; pos <= MAX_NODES; pos++) {
        travmap[pos] = deffun;
    }

    for (pos = 0; anontraversal[pos].node != 0; pos++) {
        travmap[anontraversal[pos].node] = anontraversal[pos].travfun;
    }

    new = MEMmalloc (sizeof (travstack_t));

    new->next = travstack;
    new->traversal = TR_anonymous;
    new->funs = travmap;

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

    if (tmp->traversal == TR_anonymous) {
        tmp->funs = MEMfree (tmp->funs);
    }
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

#ifndef DBUG_OFF
/** <!-- ****************************************************************** -->
 * @fn void TRAVprintStack()
 *
 * @brief Prints the current traversal stack, iff -#d,TRAVSTACK is set.
 *        Mainly useful in debuggers...
 ******************************************************************************/
void
TRAVprintStack ()
{
    travstack_t *tmp = travstack;

    DBUG_ENTER ("TRAVprintStack");

    DBUG_PRINT ("TRAVSTACK", ("Current traversal stack:"));
    while (tmp != NULL) {
        DBUG_PRINT ("TRAVSTACK", ("  %s", travnames[tmp->traversal]));
        tmp = tmp->next;
    }
    DBUG_PRINT ("TRAVSTACK", ("End of traversal stack"));

    DBUG_VOID_RETURN;
}
#endif /* DBUG_OFF */
