/*
 *
 * $Log$
 * Revision 1.8  2004/11/25 21:59:21  sah
 * COMPILES:
 *
 *
 * Revision 1.1  2004/09/22 11:37:27  sah
 * Initial revision
 */

#include "symboltable.h"
#include "dbug.h"
#include "internal_lib.h"
#include "types.h"
#include <string.h>

struct ST_SYMBOLITERATOR_T {
    stsymbol_t *head;
    stsymbol_t *pos;
};

struct ST_ENTRY_T {
    char *name;
    stentrytype_t type;
    stentry_t *next;
};

struct ST_ENTRYITERATOR_T {
    stentry_t *head;
    stentry_t *pos;
};

struct ST_SYMBOL_T {
    char *name;
    stentry_t *head;
    stvisibility_t vis;
    stsymbol_t *next;
};

struct ST_SYMBOLTABLE_T {
    stsymbol_t *head;
};

/*
 * Functions for handling stentry_t types
 */

static stentry_t *
STEntryInit (const char *name, stentrytype_t type)
{
    stentry_t *result;

    DBUG_ENTER ("STEntryInit");

    result = (stentry_t *)ILIBmalloc (sizeof (stentry_t));

    result->name = ILIBstringCopy (name);
    result->type = type;
    result->next = NULL;

    DBUG_RETURN (result);
}

static stentry_t *
STEntryDestroy (stentry_t *entry)
{
    stentry_t *result;

    DBUG_ENTER ("STEntryDestroy");

    entry->name = ILIBfree (entry->name);

    result = entry->next;

    entry = ILIBfree (entry);

    DBUG_RETURN (result);
}

static bool
STEntryEqual (stentry_t *one, stentry_t *two)
{
    bool result = TRUE;

    DBUG_ENTER ("STEntryEqual");

    result = result && (!strcmp (one->name, two->name));
    result = result && (one->type == two->type);

    DBUG_RETURN (result);
}

/*
 * Functions for handling stsymbol_t
 */
static stsymbol_t *
STSymbolInit (const char *symbol, stvisibility_t vis)
{
    stsymbol_t *result;

    DBUG_ENTER ("STSymbolInit");

    result = (stsymbol_t *)ILIBmalloc (sizeof (stsymbol_t));

    result->name = ILIBstringCopy (symbol);
    result->vis = vis;
    result->head = NULL;
    result->next = NULL;

    DBUG_RETURN (result);
}

static stsymbol_t *
STSymbolDestroy (stsymbol_t *symbol)
{
    stsymbol_t *result;

    DBUG_ENTER ("STSymbolDestroy");

    while (symbol->head != NULL)
        symbol->head = STEntryDestroy (symbol->head);

    symbol->name = ILIBfree (symbol->name);

    result = symbol->next;

    symbol = ILIBfree (symbol);

    DBUG_RETURN (result);
}

static void
STEntryAdd (stentry_t *entry, stsymbol_t *symbol)
{
    stentry_t *pos;
    bool found = FALSE;

    DBUG_ENTER ("STSymbolAdd");

    /* check whether entry already exists */
    pos = symbol->head;

    while ((pos != NULL) && (!found)) {
        found = STEntryEqual (pos, entry);
        pos = pos->next;
    }

    if (found) {
        entry = STEntryDestroy (entry);
    } else {
        entry->next = symbol->head;
        symbol->head = entry;
    }

    DBUG_VOID_RETURN;
}

/*
 * Functions for handling sttable_t
 */
static void
STSymbolAdd (stsymbol_t *symbol, sttable_t *table)
{
    DBUG_ENTER ("STSymbolAdd");

    symbol->next = table->head;
    table->head = symbol;

    DBUG_VOID_RETURN;
}

static stsymbol_t *
STLookupSymbol (const char *symbol, sttable_t *table)
{
    stsymbol_t *result;

    DBUG_ENTER ("STLookupSymbol");

    result = table->head;

    while ((result != NULL) && (strcmp (result->name, symbol))) {
        result = result->next;
    }

    DBUG_RETURN (result);
}

sttable_t *
STInit ()
{
    sttable_t *result;

    DBUG_ENTER ("STInit");

    result = (sttable_t *)ILIBmalloc (sizeof (sttable_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

sttable_t *
STDestroy (sttable_t *table)
{
    DBUG_ENTER ("STDestroy");

    while (table->head != NULL) {
        table->head = STSymbolDestroy (table->head);
    }

    table = ILIBfree (table);

    DBUG_RETURN (table);
}

static void
STEntryInsert (const char *symbolname, stvisibility_t vis, stentry_t *entry,
               sttable_t *table)
{
    stsymbol_t *symbol;

    DBUG_ENTER ("STEntryInsert");

    symbol = STLookupSymbol (symbolname, table);

    if (symbol == NULL) {
        symbol = STSymbolInit (symbolname, vis);
        STSymbolAdd (symbol, table);
    }

    DBUG_ASSERT ((vis == symbol->vis), "found symbol with mixed visibility!");

    STEntryAdd (entry, symbol);

    DBUG_VOID_RETURN;
}

void
STAdd (const char *symbol, stvisibility_t vis, const char *name, stentrytype_t type,
       sttable_t *table)
{
    stentry_t *entry;

    DBUG_ENTER ("STAdd");

    entry = STEntryInit (name, type);
    STEntryInsert (symbol, vis, entry, table);

    DBUG_VOID_RETURN;
}

void
STRemove (const char *symbol, sttable_t *table)
{
    stsymbol_t *symp;

    DBUG_ENTER ("STRemove");

    symp = STLookupSymbol (symbol, table);

    if (symp != NULL) {
        if (table->head == symp) {
            table->head = symp->next;
        } else {
            stsymbol_t *pos = table->head;

            while (pos->next != symp) {
                pos = pos->next;
            }

            pos->next = symp->next;
        }

        symp = STSymbolDestroy (symp);
    }

    DBUG_VOID_RETURN;
}

bool
STContains (const char *symbol, sttable_t *table)
{
    bool result;

    DBUG_ENTER ("STContains");

    result = (STLookupSymbol (symbol, table) != NULL);

    DBUG_RETURN (result);
}

bool
STContainsEntry (const char *name, sttable_t *table)
{
    stsymbol_t *symbol;
    stentry_t *entry;
    bool result = FALSE;

    DBUG_ENTER ("STContainsEntry");

    symbol = table->head;

    while ((symbol != NULL) && (!result)) {
        entry = symbol->head;

        while ((entry != NULL) && (!result)) {
            result = !strcmp (entry->name, name);

            entry = entry->next;
        }
        symbol = symbol->next;
    }

    DBUG_RETURN (result);
}

stsymbol_t *
STGet (const char *symbol, sttable_t *table)
{
    DBUG_ENTER ("STGet");

    DBUG_RETURN (STLookupSymbol (symbol, table));
}

stentry_t *
STGetFirstEntry (const char *symbol, sttable_t *table)
{
    stentry_t *result;
    stsymbol_t *symbolp;

    DBUG_ENTER ("STGetFirstEntry");

    symbolp = STLookupSymbol (symbol, table);
    result = symbolp->head;

    DBUG_RETURN (result);
}

/*
 * Functions for stsymboliterator_t
 */
stsymboliterator_t *
STSymbolIteratorGet (sttable_t *table)
{
    stsymboliterator_t *result;

    DBUG_ENTER ("STSymbolIteratorGet");

    result = (stsymboliterator_t *)ILIBmalloc (sizeof (stsymboliterator_t));

    result->head = table->head;
    result->pos = table->head;

    DBUG_RETURN (result);
}

stsymboliterator_t *
STSymbolIteratorRelease (stsymboliterator_t *iterator)
{
    DBUG_ENTER ("STSymbolIteratorRelease");

    iterator = ILIBfree (iterator);

    DBUG_RETURN (iterator);
}

stsymbol_t *
STSymbolIteratorNext (stsymboliterator_t *iterator)
{
    stsymbol_t *result;

    DBUG_ENTER ("STSymbolIteratorNext");

    if (iterator->pos == NULL) {
        result = NULL;
    } else {
        result = iterator->pos;
        iterator->pos = iterator->pos->next;
    }

    DBUG_RETURN (result);
}

void
STSymbolIteratorReset (stsymboliterator_t *iterator)
{
    DBUG_ENTER ("STSymbolIteratorReset");

    iterator->head = iterator->pos;

    DBUG_VOID_RETURN;
}

int
STSymbolIteratorHasMore (stsymboliterator_t *iterator)
{
    DBUG_ENTER ("STSymbolIteratorHasMore");

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions for stentryiterator_t
 */
static stentryiterator_t *
STEntryIteratorInit (stsymbol_t *symbol)
{
    stentryiterator_t *result;

    DBUG_ENTER ("STEntryIteratorInit");

    result = (stentryiterator_t *)ILIBmalloc (sizeof (stentryiterator_t));

    result->head = symbol->head;
    result->pos = symbol->head;

    DBUG_RETURN (result);
}

stentryiterator_t *
STEntryIteratorGet (const char *symbolname, sttable_t *table)
{
    stentryiterator_t *result;
    stsymbol_t *symbol;

    DBUG_ENTER ("STEntryIteratorGet");

    symbol = STLookupSymbol (symbolname, table);

    result = STEntryIteratorInit (symbol);

    DBUG_RETURN (result);
}

stentryiterator_t *
STEntryIteratorRelease (stentryiterator_t *iterator)
{
    DBUG_ENTER ("STEntryIteratorRelease");

    iterator = ILIBfree (iterator);

    DBUG_RETURN (iterator);
}

stentry_t *
STEntryIteratorNext (stentryiterator_t *iterator)
{
    stentry_t *result;

    DBUG_ENTER ("STEntryIteratorNext");

    result = iterator->pos;

    if (iterator->pos != NULL)
        iterator->pos = iterator->pos->next;

    DBUG_RETURN (result);
}

void
STEntryIteratorReset (stentryiterator_t *iterator)
{
    DBUG_ENTER ("STEntryIteratorReset");

    iterator->pos = iterator->head;

    DBUG_VOID_RETURN;
}

int
STEntryIteratorHasMore (stentryiterator_t *iterator)
{
    DBUG_ENTER ("STEntryIteratorHasMore");

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions to access stsymbol_t
 */
const char *
STSymbolName (stsymbol_t *symbol)
{
    DBUG_ENTER ("STSymbolName");

    DBUG_RETURN (symbol->name);
}

stvisibility_t
STSymbolVisibility (stsymbol_t *symbol)
{
    DBUG_ENTER ("STSymbolVisibility");

    DBUG_RETURN (symbol->vis);
}

/*
 * Functions to access stentry_t
 */

const char *
STEntryName (stentry_t *entry)
{
    DBUG_ENTER ("STEntryName");

    DBUG_ASSERT ((entry != NULL), "STEntryName called with NULL argument");

    DBUG_RETURN (entry->name);
}

stentrytype_t
STEntryType (stentry_t *entry)
{
    DBUG_ENTER ("STEntryType");

    DBUG_ASSERT ((entry != NULL), "STEntryType called with NULL argument");

    DBUG_RETURN (entry->type);
}

/*
 * functions for printing
 */
static void
STEntryPrint (stentry_t *entry)
{
    DBUG_ENTER ("STEntryPrint");

    printf ("    %s\n", entry->name);

    DBUG_VOID_RETURN;
}

static void
STSymbolPrint (stsymbol_t *symbol)
{
    stentry_t *entry;
    char *visname;

    DBUG_ENTER ("STSymbolPrint");

    switch (symbol->vis) {
    case SVT_local:
        visname = "local";
        break;
    case SVT_provided:
        visname = "provided";
        break;
    case SVT_exported:
        visname = "exported";
        break;
    default:
        visname = "unkown";
        break;
    }

    printf ("Symbol: %s [%s]\n", symbol->name, visname);

    entry = symbol->head;

    while (entry != NULL) {
        STEntryPrint (entry);
        entry = entry->next;
    }

    printf ("\n");

    DBUG_VOID_RETURN;
}

void
STPrint (sttable_t *table)
{
    stsymbol_t *symbol;

    DBUG_ENTER ("STPrint");

    symbol = table->head;

    while (symbol != NULL) {
        STSymbolPrint (symbol);
        symbol = symbol->next;
    }

    DBUG_VOID_RETURN;
}
