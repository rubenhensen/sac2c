/*
 *
 * $Log$
 * Revision 1.3  2004/10/21 17:20:13  sah
 * Added SymbolTableRemove
 *
 * Revision 1.2  2004/09/23 21:14:23  sah
 * ongoing implementation
 *
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

struct SYMBOLCHAIN_T {
    symboltablesymbol_t *head;
    symboltablesymbol_t *pos;
};

struct SYMBOLENTRY_T {
    char *name;
    symbolentrytype_t type;
    symbolentry_t *next;
};

struct SYMBOLENTRYCHAIN_T {
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
SymbolTableEntryInit (const char *name, symbolentrytype_t type)
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
SymbolTableSymbolInit (const char *symbol)
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
SymbolTableSymbolLookup (const char *symbol, symboltable_t *table)
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
SymbolTableEntryAdd (const char *symbolname, symbolentry_t *entry, symboltable_t *table)
{
    symboltablesymbol_t *symbol;

    DBUG_ENTER ("SymbolTableAdd");

    symbol = SymbolTableSymbolLookup (symbolname, table);

    if (symbol == NULL) {
        symbol = SymbolTableSymbolInit (symbolname);
        SymbolTableSymbolAdd (symbol, table);
    }

    SymbolTableSymbolEntryAdd (entry, symbol);

    DBUG_VOID_RETURN;
}

void
SymbolTableAdd (const char *symbol, const char *name, symbolentrytype_t type,
                symboltable_t *table)
{
    symbolentry_t *entry;

    DBUG_ENTER ("SymbolTableAdd");

    entry = SymbolTableEntryInit (name, type);
    SymbolTableEntryAdd (symbol, entry, table);

    DBUG_VOID_RETURN;
}

void
SymbolTableRemove (const char *symbol, symboltable_t *table)
{
    symboltablesymbol_t *symp;

    DBUG_ENTER ("SymbolTableRemove");

    symp = SymbolTableSymbolLookup (symbol, table);

    if (symp != NULL) {
        if (table->head == symp) {
            table->head = symp->next;
        } else {
            symboltablesymbol_t *pos = table->head;

            while (pos->next != symp) {
                pos = pos->next;
            }

            pos->next = symp->next;
        }

        symp = SymbolTableSymbolDestroy (symp);
    }

    DBUG_VOID_RETURN;
}

symbolchain_t *
SymbolTableSymbolChainGet (symboltable_t *table)
{
    symbolchain_t *result;

    DBUG_ENTER ("SymbolTableSymbolChainGet");

    result = (symbolchain_t *)Malloc (sizeof (symbolchain_t));

    result->head = table->head;
    result->pos = table->head;

    DBUG_RETURN (result);
}

symbolchain_t *
SymbolTableSymbolChainRelease (symbolchain_t *chain)
{
    DBUG_ENTER ("SymbolTableSymbolChainRelease");

    chain = Free (chain);

    DBUG_RETURN (chain);
}

const char *
SymbolTableSymbolChainNext (symbolchain_t *chain)
{
    char *result;

    DBUG_ENTER ("SymbolTableSymbolChainNext");

    if (chain->pos == NULL) {
        result = NULL;
    } else {
        result = chain->pos->name;
        chain->pos = chain->pos->next;
    }

    DBUG_RETURN (result);
}

void
SymbolTableSymbolChainReset (symbolchain_t *chain)
{
    DBUG_ENTER ("SymbolTableEntryChainReset");

    chain->head = chain->pos;

    DBUG_VOID_RETURN;
}

int
SymbolTableSymbolChainHasMore (symbolchain_t *chain)
{
    DBUG_ENTER ("SymbolTableSymbolChainHasMore");

    DBUG_RETURN (chain->pos != NULL);
}

static symbolentrychain_t *
SymbolTableEntryChainInit (symboltablesymbol_t *symbol)
{
    symbolentrychain_t *result;

    DBUG_ENTER ("SymbolTableEntryChainInit");

    result = (symbolentrychain_t *)Malloc (sizeof (symbolentrychain_t));

    result->head = symbol->head;
    result->pos = symbol->head;

    DBUG_RETURN (result);
}

symbolentrychain_t *
SymbolTableEntryChainGet (const char *symbolname, symboltable_t *table)
{
    symbolentrychain_t *result;
    symboltablesymbol_t *symbol;

    DBUG_ENTER ("SymbolTableEntryChainGet");

    symbol = SymbolTableSymbolLookup (symbolname, table);

    result = SymbolTableEntryChainInit (symbol);

    DBUG_RETURN (result);
}

symbolentrychain_t *
SymbolTableEntryChainRelease (symbolentrychain_t *chain)
{
    DBUG_ENTER ("SymbolTableEntryChainRelease");

    chain = Free (chain);

    DBUG_RETURN (chain);
}

symbolentry_t *
SymbolTableEntryChainNext (symbolentrychain_t *chain)
{
    symbolentry_t *result;

    DBUG_ENTER ("SymbolTableEntryChainNext");

    result = chain->pos;

    if (chain->pos != NULL)
        chain->pos = chain->pos->next;

    DBUG_RETURN (result);
}

void
SymbolTableEntryChainReset (symbolentrychain_t *chain)
{
    DBUG_ENTER ("SymbolTableEntryChainReset");

    chain->pos = chain->head;

    DBUG_VOID_RETURN;
}

int
SymbolTableEntryChainHasMore (symbolentrychain_t *chain)
{
    DBUG_ENTER ("SymbolTableSymbolChainHasMore");

    DBUG_RETURN (chain->pos != NULL);
}

const char *
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

void
SymbolTableEntryPrint (symbolentry_t *entry)
{
    DBUG_ENTER ("SymbolTableEntryPrint");

    printf ("    %s\n", entry->name);

    DBUG_VOID_RETURN;
}

void
SymbolTableSymbolPrint (symboltablesymbol_t *symbol)
{
    symbolentry_t *entry;

    DBUG_ENTER ("SymbolTableSymbolPrint");

    printf ("Symbol: %s\n", symbol->name);

    entry = symbol->head;

    while (entry != NULL) {
        SymbolTableEntryPrint (entry);
        entry = entry->next;
    }

    printf ("\n");

    DBUG_VOID_RETURN;
}

void
SymbolTablePrint (symboltable_t *table)
{
    symboltablesymbol_t *symbol;

    DBUG_ENTER ("SymbolTablePrint");

    symbol = table->head;

    while (symbol != NULL) {
        SymbolTableSymbolPrint (symbol);
        symbol = symbol->next;
    }

    DBUG_VOID_RETURN;
}
