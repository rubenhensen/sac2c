/*
 *
 * $Log$
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
#include "free.h"

static void
SubSymbols (symboltable_t *table, node *symbols)
{
    DBUG_ENTER ("SubSymbols");

    while (symbols != NULL) {
        SymbolTableRemove (SYMBOL_ID (symbols), table);
        symbols = SYMBOL_NEXT (symbols);
    }

    DBUG_VOID_RETURN;
}

static node *
Symboltable2Symbols (symbolchain_t *chain, bool exportedonly)
{
    node *result;

    DBUG_ENTER ("Symboltable2Symbols");

    if (SymbolTableSymbolChainHasMore (chain)) {
        char *id = StringCopy (SymbolTableSymbolChainNext (chain));
        node *next = Symboltable2Symbols (chain, exportedonly);
        result = MakeSymbol (id, next);
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

static node *
ResolveAllFlag (char *module, node *symbols, bool exportedonly)
{
    module_t *mod;
    symboltable_t *symtab;
    symbolchain_t *chain;
    node *result;

    DBUG_ENTER ("ResolveAllFlag");

    /* get symbol table */

    mod = LoadModule (module);
    symtab = GetSymbolTable (mod);

    SubSymbols (symtab, symbols);

    chain = SymbolTableSymbolChainGet (symtab);

    result = Symboltable2Symbols (chain, exportedonly);
    chain = SymbolTableSymbolChainRelease (chain);

    symtab = SymbolTableDestroy (symtab);
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
