/*
 *
 * $Log$
 * Revision 1.1  2004/10/17 14:51:07  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "export.h"
#include "DeadFunctionRemoval.h"
#include "serialize.h"
#include "traverse.h"
#include "types.h"
#include "tree_basic.h"

#include <string.h>

/*
 * INFO structure
 */
struct INFO {
    char *symbol;
    bool exported;
    bool provided;
    bool result;
    node *interface;
    char *modname;
};

/*
 * INFO macros
 */
#define INFO_EXP_SYMBOL(n) (n->symbol)
#define INFO_EXP_EXPORTED(n) (n->exported)
#define INFO_EXP_PROVIDED(n) (n->provided)
#define INFO_EXP_RESULT(n) (n->result)
#define INFO_EXP_INTERFACE(n) (n->interface)
#define INFO_EXP_MODNAME(n) (n->modname)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_EXP_SYMBOL (result) = NULL;
    INFO_EXP_EXPORTED (result) = FALSE;
    INFO_EXP_PROVIDED (result) = FALSE;
    INFO_EXP_RESULT (result) = FALSE;
    INFO_EXP_INTERFACE (result) = NULL;
    INFO_EXP_MODNAME (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_EXP_SYMBOL (info) = NULL;
    INFO_EXP_INTERFACE (info) = NULL;

    info = Free (info);

    DBUG_RETURN (info);
}

/*
 * Helper Functions
 */

static bool
CheckExport (bool all, node *symbol, info *arg_info)
{
    DBUG_ENTER ("CheckExport");

    INFO_EXP_RESULT (arg_info) = FALSE;

    if (symbol != NULL) {
        symbol = Trav (symbol, arg_info);
    }

    if (all) {
        /* in case of a all flag, the symbollist was
         * an except, thus the result has to be inversed
         */
        INFO_EXP_RESULT (arg_info) = !INFO_EXP_RESULT (arg_info);
    }

    DBUG_RETURN (INFO_EXP_RESULT (arg_info));
}

/*
 * Traversal Functions
 */

node *
EXPUse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPUse");

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = Trav (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPImport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPImport");

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = Trav (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPProvide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPProvide");

    if (CheckExport (PROVIDE_ALL (arg_node), PROVIDE_SYMBOL (arg_node), arg_info)) {
        INFO_EXP_PROVIDED (arg_info) = TRUE;
    }

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = Trav (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPExport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPExport");

    if (CheckExport (PROVIDE_ALL (arg_node), PROVIDE_SYMBOL (arg_node), arg_info)) {
        INFO_EXP_EXPORTED (arg_info) = TRUE;
    }

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = Trav (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPSymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPSymbol");

    if (!strcmp (INFO_EXP_SYMBOL (arg_info), SYMBOL_ID (arg_node))) {
        INFO_EXP_RESULT (arg_info) = TRUE;
    }

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = Trav (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPFundef");

    if ((FUNDEF_STATUS (arg_node) == ST_regular)
        && (!strcmp (FUNDEF_MOD (arg_node), INFO_EXP_MODNAME (arg_info)))) {
        INFO_EXP_SYMBOL (arg_info) = FUNDEF_NAME (arg_node);
        INFO_EXP_EXPORTED (arg_info) = FALSE;
        INFO_EXP_PROVIDED (arg_info) = FALSE;

        INFO_EXP_INTERFACE (arg_info) = Trav (INFO_EXP_INTERFACE (arg_info), arg_info);

        if (INFO_EXP_EXPORTED (arg_info) || INFO_EXP_PROVIDED (arg_info)) {
            FUNDEF_STATUS (arg_node) = ST_exported;
        }
    }

    if (FUNDEF_NEXT (arg_node) != NULL) {
        FUNDEF_NEXT (arg_node) = Trav (FUNDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPModul");

    INFO_EXP_INTERFACE (arg_info) = MODUL_IMPORTS (arg_node);
    INFO_EXP_MODNAME (arg_info) = MODUL_NAME (arg_node);

    if (INFO_EXP_INTERFACE (arg_info) != NULL) {
        if (MODUL_FUNS (arg_node) != NULL) {
            MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
        }
    }

    DBUG_RETURN (arg_node);
}

/*
 * Start of Traversal
 */

static node *
StartExpTraversal (node *modul)
{
    funtab *store_tab;
    info *info;

    DBUG_ENTER ("StartExpTraversal");

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = exp_tab;

    modul = Trav (modul, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    DBUG_RETURN (modul);
}

void
DoExport (node *syntax_tree)
{
    DBUG_ENTER ("DoExport");

    syntax_tree = StartExpTraversal (syntax_tree);

    syntax_tree = DeadFunctionRemoval (syntax_tree);

    SerializeModule (syntax_tree);

    DBUG_VOID_RETURN;
}
