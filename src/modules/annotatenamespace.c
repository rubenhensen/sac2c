/*
 *
 * $Log$
 * Revision 1.8  2004/11/02 12:15:37  sah
 * namespaces are annotated correctly now
 *
 * Revision 1.7  2004/11/01 21:49:31  sah
 * asdded adding of dependencies for directly referenced namespaces
 *
 * Revision 1.6  2004/10/28 17:19:32  sah
 * now when annotating namespaces, the used namespaces
 * are added to the list of dependencies
 *
 * Revision 1.5  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.4  2004/10/22 13:40:55  sah
 * the use chain is freed during the
 * tarversal as it is no more needed
 * afterwards
 *
 * Revision 1.3  2004/10/22 13:23:51  sah
 * working implementation for fundefs
 *
 * Revision 1.2  2004/10/22 09:02:16  sah
 * added ANSSymbol
 *
 * Revision 1.1  2004/10/22 08:49:55  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "annotatenamespace.h"
#include "traverse.h"
#include "tree_basic.h"
#include "symboltable.h"
#include "stringset.h"
#include "free.h"
#include "Error.h"

/*
 * INFO structure
 */
struct INFO {
    STtable_t *symbols;
    const char *current;
    node *module;
};

/*
 * INFO macros
 */
#define INFO_ANS_SYMBOLS(info) (info->symbols)
#define INFO_ANS_CURRENT(info) (info->current)
#define INFO_ANS_MODULE(info) (info->module)

/*
 * INFO functions
 */
static info *
MakeInfo ()
{
    info *result;

    DBUG_ENTER ("MakeInfo");

    result = Malloc (sizeof (info));

    INFO_ANS_SYMBOLS (result) = STInit ();
    INFO_ANS_CURRENT (result) = NULL;
    INFO_ANS_MODULE (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ("FreeInfo");

    INFO_ANS_SYMBOLS (info) = STDestroy (INFO_ANS_SYMBOLS (info));

    info = Free (info);

    DBUG_RETURN (info);
}

/*
 * local helper functions
 */
static void
CheckUseUnique (STtable_t *table)
{
    STsymboliterator_t *iterator;

    DBUG_ENTER ("CheckUseUnique");

    iterator = STSymbolIteratorGet (table);

    while (STSymbolIteratorHasMore (iterator)) {
        const char *symbol = STSymbolIteratorNext (iterator);
        STentryiterator_t *entries = STEntryIteratorGet (symbol, table);

        if (STEntryIteratorHasMore (entries)) {
            STentry_t *entry = STEntryIteratorNext (entries);

            if (STEntryIteratorHasMore (entries)) {
                ERROR (0, ("Symbol `%s' used more than once", symbol));
                CONT_ERROR (("... from module `%s'", STEntryName (entry)));

                while (STEntryIteratorHasMore (entries)) {
                    entry = STEntryIteratorNext (entries);
                    CONT_ERROR (("... from module `%s'", STEntryName (entry)));
                }
            }
        }

        entries = STEntryIteratorRelease (entries);
    }

    iterator = STSymbolIteratorRelease (iterator);

    DBUG_VOID_RETURN;
}

static void
CheckLocalNameClash (const char *symbol, STtable_t *table, int lineno)
{
    DBUG_ENTER ("CheckLocalNameClash");

    if (STContains (symbol, table)) {
        ERROR (lineno, ("Symbol `%s' used and locally defined", symbol));
    }

    DBUG_VOID_RETURN;
}

/*
 * Traversal functions
 */

node *
ANSSymbol (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSSymbol");

    STAdd (SYMBOL_ID (arg_node), INFO_ANS_CURRENT (arg_info), SET_namespace,
           INFO_ANS_SYMBOLS (arg_info));

    if (SYMBOL_NEXT (arg_node) != NULL) {
        SYMBOL_NEXT (arg_node) = Trav (SYMBOL_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSUse (node *arg_node, info *arg_info)
{
    node *result;

    DBUG_ENTER ("ANSUse");

    INFO_ANS_CURRENT (arg_info) = USE_MOD (arg_node);

    USE_SYMBOL (arg_node) = Trav (USE_SYMBOL (arg_node), arg_info);

    INFO_ANS_CURRENT (arg_info) = NULL;

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = Trav (USE_NEXT (arg_node), arg_info);
    }

    /* the use information is no more needed from
     * this point on, so we can free this use
     */

    result = USE_NEXT (arg_node);
    arg_node = FreeNode (arg_node);

    DBUG_RETURN (result);
}

node *
ANSImport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSImport");

    /* imports are ignored in this traversal as imported
       functions are treated like local functions         */

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = Trav (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSExport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSExport");

    /* exports are ignored in this traversal */

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = Trav (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSProvide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSProvide");

    /* provides are ignored in this traversal */

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = Trav (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSFundef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSFundef");

    CheckLocalNameClash (FUNDEF_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info),
                         NODE_LINE (arg_node));

    if (FUNDEF_MOD (arg_node) == NULL) {
        FUNDEF_MOD (arg_node) = StringCopy (MODUL_NAME (INFO_ANS_MODULE (arg_info)));
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSTypedef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSTypedef");

    CheckLocalNameClash (TYPEDEF_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info),
                         NODE_LINE (arg_node));

    if (TYPEDEF_MOD (arg_node) == NULL) {
        TYPEDEF_MOD (arg_node) = StringCopy (MODUL_NAME (INFO_ANS_MODULE (arg_info)));
    }

    if (TYPEDEF_NEXT (arg_node) != NULL) {
        TYPEDEF_NEXT (arg_node) = Trav (TYPEDEF_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSObjdef (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSObjdef");

    CheckLocalNameClash (OBJDEF_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info),
                         NODE_LINE (arg_node));

    if (OBJDEF_MOD (arg_node) == NULL) {
        OBJDEF_MOD (arg_node) = StringCopy (MODUL_NAME (INFO_ANS_MODULE (arg_info)));
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSAp (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSAp");

    if (AP_MOD (arg_node) == NULL) {
        /*
         * look up the correct namespace
         */
        if (STContains (AP_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info))) {
            /*
             * There is a namespace annotated to this symbolname, e.g. it was used
             * -> use that namespace
             */
            STentry_t *entry
              = STGetFirstEntry (AP_NAME (arg_node), INFO_ANS_SYMBOLS (arg_info));
            AP_MOD (arg_node) = StringCopy (STEntryName (entry));

            MODUL_DEPENDENCIES (INFO_ANS_MODULE (arg_info))
              = SSAdd (AP_MOD (arg_node),
                       MODUL_DEPENDENCIES (INFO_ANS_MODULE (arg_info)));
        } else {
            /*
             * must be a local function
             * -> use local namespace
             */
            AP_MOD (arg_node) = StringCopy (MODUL_NAME (INFO_ANS_MODULE (arg_info)));
        }
    } else if (strcmp (MODUL_NAME (INFO_ANS_MODULE (arg_info)), AP_MOD (arg_node))) {
        /*
         * this function comes from another namespace
         *  -> add the namespace to the dependency list
         */
        MODUL_DEPENDENCIES (INFO_ANS_MODULE (arg_info))
          = SSAdd (AP_MOD (arg_node), MODUL_DEPENDENCIES (INFO_ANS_MODULE (arg_info)));
    }

    arg_node = TravSons (arg_node, arg_info);

    DBUG_RETURN (arg_node);
}

node *
ANSArg (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSArg");

    if (ARG_NEXT (arg_node) != NULL) {
        ARG_NEXT (arg_node) = Trav (ARG_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSVardec (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSVardec");

    if (VARDEC_NEXT (arg_node) != NULL) {
        VARDEC_NEXT (arg_node) = Trav (VARDEC_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
ANSModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("ANSModul");

    INFO_ANS_MODULE (arg_info) = arg_node;

    /* traverse use/import statements */

    if (MODUL_IMPORTS (arg_node) != NULL) {
        MODUL_IMPORTS (arg_node) = Trav (MODUL_IMPORTS (arg_node), arg_info);
    }

    /* check uniqueness property of symbols in use */

    CheckUseUnique (INFO_ANS_SYMBOLS (arg_info));

    /* traverse fundecs */

    if (MODUL_FUNDECS (arg_node) != NULL) {
        MODUL_FUNDECS (arg_node) = Trav (MODUL_FUNDECS (arg_node), arg_info);
    }

    /* traverse fundefs */

    if (MODUL_FUNS (arg_node) != NULL) {
        MODUL_FUNS (arg_node) = Trav (MODUL_FUNS (arg_node), arg_info);
    }

    /* traverse typedefs */

    if (MODUL_TYPES (arg_node) != NULL) {
        MODUL_TYPES (arg_node) = Trav (MODUL_TYPES (arg_node), arg_info);
    }

    /* traverse objdefs */

    if (MODUL_OBJS (arg_node) != NULL) {
        MODUL_OBJS (arg_node) = Trav (MODUL_OBJS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

void
DoAnnotateNamespace (node *module)
{
    funtab *store_tab;
    info *info;

    DBUG_ENTER ("DoAnnotateNamespace");

    info = MakeInfo ();

    store_tab = act_tab;
    act_tab = ans_tab;

    module = Trav (module, info);

    act_tab = store_tab;

    info = FreeInfo (info);

    DBUG_VOID_RETURN;
}
