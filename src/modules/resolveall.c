/*
 *
 * $Log$
 * Revision 1.3  2004/11/17 19:48:18  sah
 * added visibility checking
 *
 * Revision 1.2  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.1  2004/10/21 17:18:55  sah
 * Initial revision
 *
 *
 *
 */

#define NEW_INFO

#include "resolveall.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"
#include "modulemanager.h"
#include "symboltable.h"
#include "Error.h"
#include "free.h"

static void
SubSymbols (STtable_t *table, node *symbols)
{
    DBUG_ENTER ("SubSymbols");

    while (symbols != NULL) {
        STRemove (SYMBOL_ID (symbols), table);
        symbols = SYMBOL_NEXT (symbols);
    }

    DBUG_VOID_RETURN;
}

static node *
Symboltable2Symbols (STsymboliterator_t *iterator, bool exportedonly)
{
    node *result;

    DBUG_ENTER ("Symboltable2Symbols");

    if (STSymbolIteratorHasMore (iterator)) {
        STsymbol_t *symb = STSymbolIteratorNext (iterator);
        node *next = Symboltable2Symbols (iterator, exportedonly);
        result = MakeSymbol (StringCopy (STSymbolName (symb)), next);
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

static node *
CheckSymbolExistsRec (const char *mod, STtable_t *table, node *symbols, bool exportedonly)
{
    STsymbol_t *symbol;

    DBUG_ENTER ("CheckSymbolExistsRec");

    if (symbols != NULL) {

        SYMBOL_NEXT (symbols)
          = CheckSymbolExistsRec (mod, table, SYMBOL_NEXT (symbols), exportedonly);

        /* check visibility */
        symbol = STGet (SYMBOL_ID (symbols), table);
        if ((symbol == NULL)
            || ((!(STSymbolVisibility (symbol) == SVT_exported))
                && ((!(STSymbolVisibility (symbol) == SVT_provided)) || exportedonly))) {
            node *tmp;

            WARN (NODE_LINE (symbols),
                  ("Symbol `%s:%s' is undefined. Ignoring...", mod, SYMBOL_ID (symbols)));

            tmp = symbols;
            symbols = SYMBOL_NEXT (symbols);

            tmp = FreeNode (tmp);
        }
    }

    DBUG_RETURN (symbols);
}

static node *
CheckSymbolExists (const char *mod, node *symbols, bool exportedonly)
{
    module_t *module;
    STtable_t *table;

    DBUG_ENTER ("CheckSymbolExists");

    module = LoadModule (mod);

    table = GetSymbolTable (module);

    symbols = CheckSymbolExistsRec (mod, table, symbols, exportedonly);

    table = STDestroy (table);
    module = UnLoadModule (module);

    DBUG_RETURN (symbols);
}

static node *
ResolveAllFlag (char *module, node *symbols, bool exportedonly)
{
    module_t *mod;
    STtable_t *symtab;
    STsymboliterator_t *iterator;
    node *result;

    DBUG_ENTER ("ResolveAllFlag");

    /* get symbol table */

    mod = LoadModule (module);
    symtab = GetSymbolTable (mod);

    SubSymbols (symtab, symbols);

    iterator = STSymbolIteratorGet (symtab);

    result = Symboltable2Symbols (iterator, exportedonly);
    iterator = STSymbolIteratorRelease (iterator);

    symtab = STDestroy (symtab);
    mod = UnLoadModule (mod);

    if (symbols != NULL) {
        symbols = FreeTree (symbols);
    }

    DBUG_RETURN (result);
}

node *
RSAUse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAUse");

    USE_SYMBOL (arg_node)
      = CheckSymbolExists (USE_MOD (arg_node), USE_SYMBOL (arg_node), FALSE);

    if (USE_ALL (arg_node)) {
        USE_SYMBOL (arg_node)
          = ResolveAllFlag (USE_MOD (arg_node), USE_SYMBOL (arg_node), FALSE);
        USE_ALL (arg_node) = FALSE;
    }

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = Trav (USE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAImport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAImport");

    IMPORT_SYMBOL (arg_node)
      = CheckSymbolExists (IMPORT_MOD (arg_node), IMPORT_SYMBOL (arg_node), TRUE);

    if (IMPORT_ALL (arg_node)) {
        IMPORT_SYMBOL (arg_node)
          = ResolveAllFlag (IMPORT_MOD (arg_node), IMPORT_SYMBOL (arg_node), TRUE);
        IMPORT_ALL (arg_node) = FALSE;
    }

    if (IMPORT_NEXT (arg_node) != NULL) {
        IMPORT_NEXT (arg_node) = Trav (IMPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAProvide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAProvide");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = Trav (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAExport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAExport");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = Trav (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAModul (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAModul");

    if (MODUL_IMPORTS (arg_node) != NULL) {
        MODUL_IMPORTS (arg_node) = Trav (MODUL_IMPORTS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

void
ResolveAll (node *modul)
{
    funtab *store_tab;

    DBUG_ENTER ("ResolveAll");

    store_tab = act_tab;
    act_tab = rsa_tab;

    modul = Trav (modul, NULL);

    act_tab = store_tab;

    DBUG_VOID_RETURN;
}
