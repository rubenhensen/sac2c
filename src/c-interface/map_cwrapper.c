/*
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tree.h"
#include "map_cwrapper.h"
#include "my_debug.h"
#include "dbug.h"
#include "traverse.h"
#include "Error.h"
#include "convert.h"
#include "filemgr.h"
#include "globals.h"
#include "free.h"
#include "resource.h"
#include "shape.h"

/* only local used utility functions */
static int CountFunResults (node *fundef);
static int CountFunArgs (node *fundef);

/******************************************************************************
 *
 * function:
 *   node *MCWmodul(node *arg_node, node *arg_info)
 *
 * description:
 *   Traverses only in functions of module
 *
 ******************************************************************************/

node *
MCWmodul (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MCWmodul");

    if (MODUL_OBJS (arg_node) != NULL) {
        SYSWARN (("the c interface cannot handle global objects so far"));
        CONT_WARN (("creating no c-library and interface"));
        generatelibrary &= (!GENERATELIBRARY_C);
    } else {
        if (MODUL_FUNS (arg_node) != NULL) {
            /* if there are some fundefs, traverse them */
            NOTE (("analyse overloading of sac-functions...\n"));
            INFO_MCW_MODUL (arg_info) = arg_node;

            /* the modul node is needed to hang the wrapperchain in N_module */
            Trav (MODUL_FUNS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MCWfundef(node *arg_node, node *arg_info)
 *
 * description:
 *   Builds up N_cwrapper nodes for wrapper and nodelist with fundefs
 *
 ******************************************************************************/

node *
MCWfundef (node *arg_node, node *arg_info)
{
    node *result;
    node *cwrapper;

    DBUG_ENTER ("MCWfundef");

    if (FUNDEF_STATUS (arg_node) == ST_regular) {
        /* export only functions defined in this module */
        DBUG_ASSERT ((FUNDEF_BODY (arg_node) != NULL),
                     ("MapFunctionToWrapper: Found fundef ST_regular with empty body!"));

        /* start traversal of cwrapper nodes, searching a mapping wrapper */
        if (MODUL_CWRAPPER (INFO_MCW_MODUL (arg_info)) != NULL) {
            INFO_MCW_FUNDEF (arg_info) = arg_node; /* map this fundef */
            result = Trav (MODUL_CWRAPPER (INFO_MCW_MODUL (arg_info)), arg_info);
        } else {
            /* no existing wrapper */
            result = NULL;
        }

        if (result == NULL) {
            /* new wrapper needed - setup node with data for new wrapper */
            cwrapper = MakeCWrapper (MODUL_CWRAPPER (INFO_MCW_MODUL (arg_info)),
                                     StringCopy (FUNDEF_NAME (arg_node)), modulename,
                                     CountFunArgs (arg_node), CountFunResults (arg_node));
            CWRAPPER_FUNS (cwrapper)
              = NodeListAppend (CWRAPPER_FUNS (cwrapper), arg_node, NULL);
            MODUL_CWRAPPER (INFO_MCW_MODUL (arg_info)) = cwrapper;
        }
    }

    /* step to next fundef */
    if (FUNDEF_NEXT (arg_node) != NULL) {
        Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MCWarg(node *arg_node, node *arg_info)
 *
 * description:
 *   traverses N_arg nodes, count artificial args
 *
 ******************************************************************************/

node *
MCWarg (node *arg_node, node *arg_info)
{
    DBUG_ENTER ("MCWarg");

    if (ARG_ATTRIB (arg_node) == ST_regular) {
        INFO_MCW_CNT_STANDARD (arg_info)++;
    }

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

/******************************************************************************
 *
 * function:
 *   node *MCWcwrapper(node *arg_node, node *arg_info)
 *
 * description:
 *   searches cwarppers for wrapper matching specified function
 *   if    match: include fundef in wrappers function nodelist, return arg_node
 *   if no match: return NULL
 *
 ******************************************************************************/

node *
MCWcwrapper (node *arg_node, node *arg_info)
{
    node *result;
    bool isfound;
    DBUG_ENTER ("MCWcwrapper");

    result = NULL;
    isfound = FALSE;

    if (strcmp (CWRAPPER_NAME (arg_node), FUNDEF_NAME (INFO_MCW_FUNDEF (arg_info)))
        == 0) {
        /* same name, now check number of args and results */
        if ((CWRAPPER_ARGCOUNT (arg_node) == CountFunArgs (INFO_MCW_FUNDEF (arg_info)))
            && (CWRAPPER_RESCOUNT (arg_node)
                == CountFunResults (INFO_MCW_FUNDEF (arg_info)))) {
            /* fundef matches this wrapper,
             * so add fundef to funs nodelist */
            CWRAPPER_FUNS (arg_node) = NodeListAppend (CWRAPPER_FUNS (arg_node),
                                                       INFO_MCW_FUNDEF (arg_info), NULL);
            isfound = TRUE;
        }
    }

    if ((!isfound) && (CWRAPPER_NEXT (arg_node) != NULL)) {
        /* traverse to next wrapper, and continue search */
        result = Trav (CWRAPPER_NEXT (arg_node), arg_info);
        if (result != NULL) {
            isfound = TRUE;
        }
    }

    if (isfound) {
        result = arg_node;
    } else {
        result = NULL;
    }
    DBUG_RETURN (result);
}

/******************************************************************************
 *
 * function:
 *   node *MapCWrapper(node *syntaxtree)
 *
 * description:
 *   Builds up a mappingtree by traversing syntaxtree looking for fundefs to
 *   export. N_cwrapper nodes are located as son of N_modul
 *
 *
 ******************************************************************************/

node *
MapCWrapper (node *syntax_tree)
{
    node *arg_info;
    funtab *old_tab;

    DBUG_ENTER ("MapCWrapper");

    if (generatelibrary & GENERATELIBRARY_C) {
        arg_info = MakeInfo ();

        old_tab = act_tab;
        act_tab = mapcw_tab;

        syntax_tree = Trav (syntax_tree, arg_info);

        act_tab = old_tab;

        FREE (arg_info);
    }

    DBUG_RETURN (syntax_tree);
}

/******************************************************************************
 *
 * function:
 *   int CountFunArgs( node *fundef)
 *
 * description:
 *   counts the number of arguments of this function
 *
 * return:
 *   number of args
 *
 ******************************************************************************/

static int
CountFunArgs (node *fundef)
{
    node *arg_info;
    int counter;

    DBUG_ENTER ("CountFunArgs");

    arg_info = MakeInfo ();

    INFO_MCW_CNT_STANDARD (arg_info) = 0;

    if (FUNDEF_ARGS (fundef) != NULL) {
        Trav (FUNDEF_ARGS (fundef), arg_info);
    }
    counter = INFO_MCW_CNT_STANDARD (arg_info);

    FREE (arg_info);

    DBUG_RETURN (counter);
}

/******************************************************************************
 *
 * function:
 *   int CountFunResults( node *fundef)
 *
 * description:
 *   counts the number of standard results of this function
 *
 * return:
 *   number of results
 *
 ******************************************************************************/

static int
CountFunResults (node *fundef)
{
    int count = 0;
    types *rettypes;

    DBUG_ENTER ("CountFunResults");

    rettypes = FUNDEF_TYPES (fundef);

    while (rettypes != NULL) {
        if (TYPES_STATUS (rettypes) == ST_regular) {
            count++;
        }
        rettypes = TYPES_NEXT (rettypes);
    }

    DBUG_RETURN (count);
}
