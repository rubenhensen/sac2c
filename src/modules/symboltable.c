/*
 *
 * $Log$
 * Revision 1.1  2004/09/22 11:37:27  sah
 * Initial revision
 *
 *
 *
 */

#include "symboltable.h"
#include "dbug.h"
#include "internal_lib.h"
#include <string.h>

typedef struct SYMBOLTABLESYMBOL_T symboltablesymbol_t;

struct SYMBOLENTRY_T {
    char *name;
    symbolentrytype_t type;
    symbolentry_t *next;
};

struct SYMBOLCHAIN_T {
    symbolentry_t *head;
    symbolentry_t *pos;
};

struct SYMBOLTABLESYMBOL_T {
    char *name;
    symbolentry_t *head;
    symboltablesymbol_t *next;
};

struct SYMBOLTABLE_T {
    symboltablesymbol_t *head;
};

static symbolentry_t *
SymbolTableEntryInit (char *name, symbolentrytype_t type)
{
    symbolentry_t *result;

    DBUG_ENTER ("SymbolTableEntryInit");

    result = (symbolentry_t *)Malloc (sizeof (symbolentry_t));

    result->name = StringCopy (name);
    result->type = type;
    result->next = NULL;

    DBUG_RETURN (result);
}

static symbolentry_t *
SymbolTableEntryDestroy (symbolentry_t *entry)
{
    symbolentry_t *result;

    DBUG_ENTER ("SymbolTableEntryDestroy");

    entry->name = Free (entry->name);

    result = entry->next;

    entry = Free (entry);

    DBUG_RETURN (result);
}

static symboltablesymbol_t *
SymbolTableSymbolInit (char *symbol)
{
    symboltablesymbol_t *result;

    DBUG_ENTER ("SymbolTableSymbolInit");

    result = (symboltablesymbol_t *)Malloc (sizeof (symboltablesymbol_t));

    result->name = StringCopy (symbol);
    result->head = NULL;
    result->next = NULL;

    DBUG_RETURN (result);
}

static symboltablesymbol_t *
SymbolTableSymbolDestroy (symboltablesymbol_t *symbol)
{
    symboltablesymbol_t *result;

    DBUG_ENTER ("SymbolTableSymbolDestroy");

    while (symbol->head != NULL)
        symbol->head = SymbolTableEntryDestroy (symbol->head);

    symbol->name = Free (symbol->name);

    result = symbol->next;

    symbol = Free (symbol);

    DBUG_RETURN (result);
}

static void
SymbolTableSymbolAdd (symboltablesymbol_t *symbol, symboltable_t *table)
{
    DBUG_ENTER ("SymbolTableSymbolAdd");

    symbol->next = table->head;
    table->head = symbol;

    DBUG_VOID_RETURN;
}

static symboltablesymbol_t *
SymbolTableSymbolLookup (char *symbol, symboltable_t *table)
{
    symboltablesymbol_t *result;

    DBUG_ENTER ("SymbolTableLookupSymbol");

    result = table->head;

    while ((result != NULL) && (strcmp (result->name, symbol))) {
        result = result->next;
    }

    DBUG_RETURN (result);
}

static void
SymbolTableSymbolEntryAdd (symbolentry_t *entry, symboltablesymbol_t *symbol)
{
    DBUG_ENTER ("SymbolTableSymbolEntryAdd");

    entry->next = symbol->head;
    symbol->head = entry;

    DBUG_VOID_RETURN;
}

symboltable_t *
SymbolTableInit ()
{
    symboltable_t *result;

    DBUG_ENTER ("SymbolTableInit");

    result = (symboltable_t *)Malloc (sizeof (symboltable_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

symboltable_t *
SymbolTableDestroy (symboltable_t *table)
{
    DBUG_ENTER ("SymbolTableDestroy");

    while (table->head != NULL) {
        table->head = SymbolTableSymbolDestroy (table->head);
    }

    table = Free (table);

    DBUG_RETURN (table);
}

static void
SymbolTableEntryAdd (char *symbolname, symbolentry_t *entry, symboltable_t *table)
{
    symboltablesymbol_t *symbol;

    DBUG_ENTER ("SymbolTableAdd");

    symbol = SymbolTableSymbolLookup (symbolname, table);

    if (symbol == NULL) {
        symbol = SymbolTableSymbolInit (symbolname);
        SymbolTableSymbolAdd (symbol, table);
        SymbolTableSymbolEntryAdd (entry, symbol);
    }

    DBUG_VOID_RETURN;
}

void
SymbolTableAdd (char *symbol, char *name, symbolentrytype_t type, symboltable_t *table)
{
    symbolentry_t *entry;

    DBUG_ENTER ("SymbolTableAdd");

    entry = SymbolTableEntryInit (name, type);
    SymbolTableEntryAdd (symbol, entry, table);

    DBUG_VOID_RETURN;
}

static symbolchain_t *
SymbolTableChainInit (symboltablesymbol_t *symbol)
{
    symbolchain_t *result;

    DBUG_ENTER ("SymbolTableChainInit");

    result = (symbolchain_t *)Malloc (sizeof (symbolchain_t));

    result->head = symbol->head;
    result->pos = symbol->head;

    DBUG_RETURN (result);
}

symbolchain_t *
SymbolTableChainGet (char *symbolname, symboltable_t *table)
{
    symbolchain_t *result;
    symboltablesymbol_t *symbol;

    DBUG_ENTER ("SymbolTableChainGet");

    symbol = SymbolTableSymbolLookup (symbolname, table);

    result = SymbolTableChainInit (symbol);

    DBUG_RETURN (result);
}

symbolentry_t *
SymbolTableChainNext (symbolchain_t *chain)
{
    symbolentry_t *result;

    DBUG_ENTER ("SymbolTableChainNext");

    result = chain->pos;

    if (chain->pos != NULL)
        chain->pos = chain->pos->next;

    DBUG_RETURN (result);
}

void
SymbolTableChainReset (symbolchain_t *chain)
{
    DBUG_ENTER ("SymbolTableChainReset");

    chain->pos = chain->head;

    DBUG_VOID_RETURN;
}

char *
SymbolTableEntryName (symbolentry_t *entry)
{
    DBUG_ENTER ("SymbolTableEntryName");

    DBUG_ASSERT ((entry != NULL), "SymbolTableEntryName called with NULL argument");

    DBUG_RETURN (entry->name);
}

symbolentrytype_t
SymbolTableEntryType (symbolentry_t *entry)
{
    DBUG_ENTER ("SymbolTableEntryType");

    DBUG_ASSERT ((entry != NULL), "SymbolTableEntryType called with NULL argument");

    DBUG_RETURN (entry->type);
}
