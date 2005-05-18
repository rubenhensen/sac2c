/*
 *
 * $Log$
 * Revision 1.8  2005/05/18 14:06:07  sah
 * fixed generation of warning message
 *
 * Revision 1.7  2005/05/18 13:56:51  sah
 * enabled caching of symboltables which
 * leads to a huge speedup when analysing use and import
 * from big modules
 *
 * Revision 1.6  2005/04/26 16:39:20  sah
 * added nice error messages and import capabilities
 *
 * Revision 1.5  2005/01/11 12:32:52  cg
 * Converted output from Error.h to ctinfo.c
 *
 * Revision 1.4  2004/11/25 20:51:53  sah
 * COMPILES
 *
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
#include "ctinfo.h"
#include "free.h"
#include "internal_lib.h"

static void
SubSymbols (sttable_t *table, node *symbols)
{
    DBUG_ENTER ("SubSymbols");

    while (symbols != NULL) {
        STremove (SYMBOL_ID (symbols), table);
        symbols = SYMBOL_NEXT (symbols);
    }

    DBUG_VOID_RETURN;
}

static node *
Symboltable2Symbols (stsymboliterator_t *iterator, bool exportedonly)
{
    node *result;

    DBUG_ENTER ("Symboltable2Symbols");

    if (STsymbolIteratorHasMore (iterator)) {
        stsymbol_t *symb = STsymbolIteratorNext (iterator);
        node *next = Symboltable2Symbols (iterator, exportedonly);

        if (exportedonly) {
            if (STsymbolVisibility (symb) == SVT_exported) {
                result = TBmakeSymbol (ILIBstringCopy (STsymbolName (symb)), next);
            } else {
                result = next;
            }
        } else {
            if (STsymbolVisibility (symb) != SVT_local) {
                result = TBmakeSymbol (ILIBstringCopy (STsymbolName (symb)), next);
            } else {
                result = next;
            }
        }
    } else {
        result = NULL;
    }

    DBUG_RETURN (result);
}

static node *
CheckSymbolExistsRec (const char *mod, const sttable_t *table, node *symbols,
                      bool exportedonly)
{
    stsymbol_t *symbol;

    DBUG_ENTER ("CheckSymbolExistsRec");

    if (symbols != NULL) {

        SYMBOL_NEXT (symbols)
          = CheckSymbolExistsRec (mod, table, SYMBOL_NEXT (symbols), exportedonly);

        /* check visibility */
        symbol = STget (SYMBOL_ID (symbols), table);
        if ((symbol == NULL)
            || ((!(STsymbolVisibility (symbol) == SVT_exported))
                && ((!(STsymbolVisibility (symbol) == SVT_provided)) || exportedonly))) {
            node *tmp;

            CTIwarnLine (NODE_LINE (symbols), "Symbol `%s:%s' is undefined. Ignoring...",
                         mod, SYMBOL_ID (symbols));

            tmp = symbols;
            symbols = SYMBOL_NEXT (symbols);

            tmp = FREEdoFreeNode (tmp);
        }
    }

    DBUG_RETURN (symbols);
}

static node *
CheckSymbolExists (const char *mod, node *symbols, bool exportedonly)
{
    module_t *module;
    const sttable_t *table;

    DBUG_ENTER ("CheckSymbolExists");

    module = MODMloadModule (mod);

    table = MODMgetSymbolTable (module);

    symbols = CheckSymbolExistsRec (mod, table, symbols, exportedonly);

    module = MODMunLoadModule (module);

    DBUG_RETURN (symbols);
}

static node *
ResolveAllFlag (char *module, node *symbols, bool exportedonly)
{
    module_t *mod;
    sttable_t *symtab;
    stsymboliterator_t *iterator;
    node *result;

    DBUG_ENTER ("ResolveAllFlag");

    /* get symbol table */

    mod = MODMloadModule (module);
    symtab = STcopy (MODMgetSymbolTable (mod));

    SubSymbols (symtab, symbols);

    iterator = STsymbolIteratorGet (symtab);

    result = Symboltable2Symbols (iterator, exportedonly);
    iterator = STsymbolIteratorRelease (iterator);

    symtab = STdestroy (symtab);

    mod = MODMunLoadModule (mod);

    if (symbols != NULL) {
        symbols = FREEdoFreeTree (symbols);
    }

    if (result == NULL) {
        CTIwarn ("All flag resolved to empty set of symbols.");
    }

    DBUG_RETURN (result);
}

node *
RSAuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAuse");

    USE_SYMBOL (arg_node)
      = CheckSymbolExists (USE_MOD (arg_node), USE_SYMBOL (arg_node), FALSE);

    if (USE_ALL (arg_node)) {
        USE_SYMBOL (arg_node)
          = ResolveAllFlag (USE_MOD (arg_node), USE_SYMBOL (arg_node), FALSE);
        USE_ALL (arg_node) = FALSE;
    }

    if (USE_NEXT (arg_node) != NULL) {
        USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
    }

    if (USE_SYMBOL (arg_node) == NULL) {
        node *tmp;

        CTIwarnLine (NODE_LINE (arg_node),
                     "Use statement has empty set of symbols. Ignoring...");

        tmp = USE_NEXT (arg_node);
        arg_node = FREEdoFreeNode (arg_node);

        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

node *
RSAimport (node *arg_node, info *arg_info)
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
        IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
    }

    if (IMPORT_SYMBOL (arg_node) == NULL) {
        node *tmp;

        CTIwarnLine (NODE_LINE (arg_node),
                     "Import statement has empty set of symbols. Ignoring...");

        tmp = IMPORT_NEXT (arg_node);
        arg_node = FREEdoFreeNode (arg_node);

        arg_node = tmp;
    }

    DBUG_RETURN (arg_node);
}

node *
RSAprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAProvide");

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAExport");

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ("RSAmodule");

    if (MODULE_IMPORTS (arg_node) != NULL) {
        MODULE_IMPORTS (arg_node) = TRAVdo (MODULE_IMPORTS (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAdoResolveAll (node *modul)
{
    DBUG_ENTER ("RSAdoResolveAll");

    TRAVpush (TR_rsa);

    modul = TRAVdo (modul, NULL);

    TRAVpop ();

    DBUG_RETURN (modul);
}
