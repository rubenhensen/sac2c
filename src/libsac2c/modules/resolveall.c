#include "resolveall.h"
#include "tree_basic.h"
#include "traverse.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "modulemanager.h"
#include "symboltable.h"
#include "ctinfo.h"
#include "free.h"
#include "str.h"
#include "memory.h"
#include "namespaces.h"

/**
 * INFO structure
 */
struct INFO {
    namespace_t *currentns;
};

/**
 * INFO macros
 */
#define INFO_CURRENTNS(n) ((n)->currentns)

/**
 * INFO functions
 */
static info *
MakeInfo (void)
{
    info *result;

    DBUG_ENTER ();

    result = (info *)MEMmalloc (sizeof (info));

    INFO_CURRENTNS (result) = NULL;

    DBUG_RETURN (result);
}

static info *
FreeInfo (info *info)
{
    DBUG_ENTER ();

    info = MEMfree (info);

    DBUG_RETURN (info);
}

/**
 * helper functions
 */

static void
SubSymbols (sttable_t *table, node *symbols)
{
    DBUG_ENTER ();

    while (symbols != NULL) {
        STremove (SYMBOL_ID (symbols), table);
        symbols = SYMBOL_NEXT (symbols);
    }

    DBUG_RETURN ();
}

static node *
Symboltable2Symbols (stsymboliterator_t *iterator, bool exportedonly)
{
    node *result;

    DBUG_ENTER ();

    if (STsymbolIteratorHasMore (iterator)) {
        stsymbol_t *symb = STsymbolIteratorNext (iterator);
        node *next = Symboltable2Symbols (iterator, exportedonly);

        if (exportedonly) {
            if (STsymbolVisibility (symb) == SVT_exported) {
                result = TBmakeSymbol (STRcpy (STsymbolName (symb)), next);
            } else {
                result = next;
            }
        } else {
            if (STsymbolVisibility (symb) != SVT_local) {
                result = TBmakeSymbol (STRcpy (STsymbolName (symb)), next);
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

    DBUG_ENTER ();

    if (symbols != NULL) {

        SYMBOL_NEXT (symbols)
          = CheckSymbolExistsRec (mod, table, SYMBOL_NEXT (symbols), exportedonly);

        /* check visibility */
        symbol = STget (SYMBOL_ID (symbols), table);
        if ((symbol == NULL)
            || ((!(STsymbolVisibility (symbol) == SVT_exported))
                && ((!(STsymbolVisibility (symbol) == SVT_provided)) || exportedonly))) {
            node *tmp;

            CTIwarnLoc (NODE_LOCATION (symbols),
                        "Symbol `%s::%s' is undefined. Ignoring...", mod,
                        SYMBOL_ID (symbols));

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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
        CTIwarn (EMPTY_LOC, "All flag resolved to empty set of symbols.");
    }

    DBUG_RETURN (result);
}

/**
 * traversal functions
 */

node *
RSAuse (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (STReq (USE_MOD (arg_node), NSgetModule (INFO_CURRENTNS (arg_info)))) {
        CTIerror (NODE_LOCATION (arg_node),
                  "The namespace of the module being compiled cannot be "
                  "referenced in use statements.");

        if (USE_NEXT (arg_node) != NULL) {
            USE_NEXT (arg_node) = TRAVdo (USE_NEXT (arg_node), arg_info);
        }
    } else {
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

            CTIwarnLoc (NODE_LOCATION (arg_node),
                        "Use statement has empty set of symbols. Ignoring...");

            tmp = USE_NEXT (arg_node);
            arg_node = FREEdoFreeNode (arg_node);

            arg_node = tmp;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
RSAimport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (STReq (IMPORT_MOD (arg_node), NSgetModule (INFO_CURRENTNS (arg_info)))) {
        CTIerror (NODE_LOCATION (arg_node),
                  "The namespace of the module being compiled cannot be "
                  "referenced in import statements.");

        if (IMPORT_NEXT (arg_node) != NULL) {
            IMPORT_NEXT (arg_node) = TRAVdo (IMPORT_NEXT (arg_node), arg_info);
        }
    } else {
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

            CTIwarnLoc (NODE_LOCATION (arg_node),
                        "Import statement has empty set of symbols. Ignoring...");

            tmp = IMPORT_NEXT (arg_node);
            arg_node = FREEdoFreeNode (arg_node);

            arg_node = tmp;
        }
    }

    DBUG_RETURN (arg_node);
}

node *
RSAprovide (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (PROVIDE_NEXT (arg_node) != NULL) {
        PROVIDE_NEXT (arg_node) = TRAVdo (PROVIDE_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAexport (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    if (EXPORT_NEXT (arg_node) != NULL) {
        EXPORT_NEXT (arg_node) = TRAVdo (EXPORT_NEXT (arg_node), arg_info);
    }

    DBUG_RETURN (arg_node);
}

node *
RSAmodule (node *arg_node, info *arg_info)
{
    DBUG_ENTER ();

    INFO_CURRENTNS (arg_info) = MODULE_NAMESPACE (arg_node);

    if (MODULE_INTERFACE (arg_node) != NULL) {
        MODULE_INTERFACE (arg_node) = TRAVdo (MODULE_INTERFACE (arg_node), arg_info);
    }

    INFO_CURRENTNS (arg_info) = NULL;

    DBUG_RETURN (arg_node);
}

node *
RSAdoResolveAll (node *modul)
{
    info *arg_info;

    DBUG_ENTER ();

    arg_info = MakeInfo ();

    TRAVpush (TR_rsa);

    modul = TRAVdo (modul, arg_info);

    TRAVpop ();

    arg_info = FreeInfo (arg_info);

    CTIabortOnError ();

    DBUG_RETURN (modul);
}

#undef DBUG_PREFIX
