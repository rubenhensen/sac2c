/*
 *
 * $Log$
 * Revision 1.3  2004/10/27 08:41:20  sah
 * main is always provided now
 *
 * Revision 1.2  2004/10/26 09:33:29  sah
 * uses EXPORTED/PROVIDED flag now
 *
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
#include "free.h"
#include "tree_basic.h"
#include "Error.h"

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
    int filetype;
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
#define INFO_EXP_FILETYPE(n) (n->filetype)

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
    INFO_EXP_FILETYPE (result) = 0;

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

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = Trav (PROVIDE_NEXT (arg_node), arg_info);
    }

    if (INFO_EXP_FILETYPE (arg_info) != F_prog) {
        if (CheckExport (PROVIDE_ALL (arg_node), PROVIDE_SYMBOL (arg_node), arg_info)) {
            INFO_EXP_PROVIDED (arg_info) = TRUE;
        }
    } else {
        WARN (NODE_LINE (arg_node), ("provide is only allowed in modules."));

        arg_node = FreeNode (arg_node);
    }

    DBUG_RETURN (arg_node);
}

node *
EXPExport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("EXPExport");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = Trav (EXPORT_NEXT (arg_node), arg_info);
    }

    if (INFO_EXP_FILETYPE (arg_info) != F_prog) {
        if (CheckExport (PROVIDE_ALL (arg_node), PROVIDE_SYMBOL (arg_node), arg_info)) {
            INFO_EXP_EXPORTED (arg_info) = TRUE;
        }
    } else {
        WARN (NODE_LINE (arg_node), ("export is only allowed in modules."));

        arg_node = FreeNode (arg_node);
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

    if (((FUNDEF_STATUS (arg_node) == ST_regular)
         || (FUNDEF_STATUS (arg_node) == ST_wrapperfun))
        && (!strcmp (FUNDEF_MOD (arg_node), INFO_EXP_MODNAME (arg_info)))) {
        INFO_EXP_SYMBOL (arg_info) = FUNDEF_NAME (arg_node);
        INFO_EXP_EXPORTED (arg_info) = FALSE;
        INFO_EXP_PROVIDED (arg_info) = FALSE;

        if (INFO_EXP_INTERFACE (arg_info) != NULL) {
            INFO_EXP_INTERFACE (arg_info)
              = Trav (INFO_EXP_INTERFACE (arg_info), arg_info);
        }

        if (INFO_EXP_EXPORTED (arg_info)) {
            SET_FLAG (FUNDEF, arg_node, IS_EXPORTED, TRUE);
            SET_FLAG (FUNDEF, arg_node, IS_PROVIDED, TRUE);
        } else if (INFO_EXP_PROVIDED (arg_info)) {
            SET_FLAG (FUNDEF, arg_node, IS_EXPORTED, FALSE);
            SET_FLAG (FUNDEF, arg_node, IS_PROVIDED, TRUE);
        } else {
            SET_FLAG (FUNDEF, arg_node, IS_EXPORTED, FALSE);
            SET_FLAG (FUNDEF, arg_node, IS_PROVIDED, FALSE);
        }

        /* override exports/provide for function main in a
         * program! Main is always provided.
         */
        if (INFO_EXP_FILETYPE (arg_info) == F_prog) {
            if (!strcmp (FUNDEF_NAME (arg_node), "main")) {
                SET_FLAG (FUNDEF, arg_node, IS_EXPORTED, FALSE);
                SET_FLAG (FUNDEF, arg_node, IS_PROVIDED, TRUE);
            }
        }
    } else {
        SET_FLAG (FUNDEF, arg_node, IS_EXPORTED, FALSE);
        SET_FLAG (FUNDEF, arg_node, IS_PROVIDED, FALSE);
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
    INFO_EXP_FILETYPE (arg_info) = MODUL_FILETYPE (arg_node);

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    MODUL_IMPORTS (arg_node) = INFO_EXP_INTERFACE (arg_info);

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

    if (MODUL_FILETYPE (syntax_tree) != F_prog) {
        SerializeModule (syntax_tree);
    }

    DBUG_VOID_RETURN;
}
