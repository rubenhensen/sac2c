/*
 *
 * $Log$
 * Revision 1.7  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
 * Revision 1.6  2004/11/17 19:47:04  sah
 * added visibility support
 *
 * Revision 1.5  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.4  2004/10/22 13:23:14  sah
 * added some functions
 * this entire things needs a
 * rewrite (memo to myself ;)
 *
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
#include "types.h"
#include <string.h>

struct ST_SYMBOLITERATOR_T {
    STsymbol_t *head;
    STsymbol_t *pos;
};

struct ST_ENTRY_T {
    char *name;
    STentrytype_t type;
    STentry_t *next;
};

struct ST_ENTRYITERATOR_T {
    STentry_t *head;
    STentry_t *pos;
};

struct ST_SYMBOL_T {
    char *name;
    STentry_t *head;
    STvisibility_t vis;
    STsymbol_t *next;
};

struct ST_SYMBOLTABLE_T {
    STsymbol_t *head;
};

/*
 * Functions for handling STentry_t types
 */

static STentry_t *
STEntryInit (const char *name, STentrytype_t type)
{
    STentry_t *result;

    DBUG_ENTER ("STEntryInit");

    result = (STentry_t *)Malloc (sizeof (STentry_t));

    result->name = StringCopy (name);
    result->type = type;
    result->next = NULL;

    DBUG_RETURN (result);
}

static STentry_t *
STEntryDestroy (STentry_t *entry)
{
    STentry_t *result;

    DBUG_ENTER ("STEntryDestroy");

    entry->name = Free (entry->name);

    result = entry->next;

    entry = Free (entry);

    DBUG_RETURN (result);
}

static bool
STEntryEqual (STentry_t *one, STentry_t *two)
{
    bool result = TRUE;

    DBUG_ENTER ("STEntryEqual");

    result = result && (!strcmp (one->name, two->name));
    result = result && (one->type == two->type);

    DBUG_RETURN (result);
}

/*
 * Functions for handling STsymbol_t
 */
static STsymbol_t *
STSymbolInit (const char *symbol, STvisibility_t vis)
{
    STsymbol_t *result;

    DBUG_ENTER ("STSymbolInit");

    result = (STsymbol_t *)Malloc (sizeof (STsymbol_t));

    result->name = StringCopy (symbol);
    result->vis = vis;
    result->head = NULL;
    result->next = NULL;

    DBUG_RETURN (result);
}

static STsymbol_t *
STSymbolDestroy (STsymbol_t *symbol)
{
    STsymbol_t *result;

    DBUG_ENTER ("STSymbolDestroy");

    while (symbol->head != NULL)
        symbol->head = STEntryDestroy (symbol->head);

    symbol->name = Free (symbol->name);

    result = symbol->next;

    symbol = Free (symbol);

    DBUG_RETURN (result);
}

static void
STEntryAdd (STentry_t *entry, STsymbol_t *symbol)
{
    STentry_t *pos;
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
 * Functions for handling STtable_t
 */
static void
STSymbolAdd (STsymbol_t *symbol, STtable_t *table)
{
    DBUG_ENTER ("STSymbolAdd");

    symbol->next = table->head;
    table->head = symbol;

    DBUG_VOID_RETURN;
}

static STsymbol_t *
STLookupSymbol (const char *symbol, STtable_t *table)
{
    STsymbol_t *result;

    DBUG_ENTER ("STLookupSymbol");

    result = table->head;

    while ((result != NULL) && (strcmp (result->name, symbol))) {
        result = result->next;
    }

    DBUG_RETURN (result);
}

STtable_t *
STInit ()
{
    STtable_t *result;

    DBUG_ENTER ("STInit");

    result = (STtable_t *)Malloc (sizeof (STtable_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

STtable_t *
STDestroy (STtable_t *table)
{
    DBUG_ENTER ("STDestroy");

    while (table->head != NULL) {
        table->head = STSymbolDestroy (table->head);
    }

    table = Free (table);

    DBUG_RETURN (table);
}

static void
STEntryInsert (const char *symbolname, STvisibility_t vis, STentry_t *entry,
               STtable_t *table)
{
    STsymbol_t *symbol;

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
STAdd (const char *symbol, STvisibility_t vis, const char *name, STentrytype_t type,
       STtable_t *table)
{
    STentry_t *entry;

    DBUG_ENTER ("STAdd");

    entry = STEntryInit (name, type);
    STEntryInsert (symbol, vis, entry, table);

    DBUG_VOID_RETURN;
}

void
STRemove (const char *symbol, STtable_t *table)
{
    STsymbol_t *symp;

    DBUG_ENTER ("STRemove");

    symp = STLookupSymbol (symbol, table);

    if (symp != NULL) {
        if (table->head == symp) {
            table->head = symp->next;
        } else {
            STsymbol_t *pos = table->head;

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
STContains (const char *symbol, STtable_t *table)
{
    bool result;

    DBUG_ENTER ("STContains");

    result = (STLookupSymbol (symbol, table) != NULL);

    DBUG_RETURN (result);
}

bool
STContainsEntry (const char *name, STtable_t *table)
{
    STsymbol_t *symbol;
    STentry_t *entry;
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

STsymbol_t *
STGet (const char *symbol, STtable_t *table)
{
    DBUG_ENTER ("STGet");

    DBUG_RETURN (STLookupSymbol (symbol, table));
}

STentry_t *
STGetFirstEntry (const char *symbol, STtable_t *table)
{
    STentry_t *result;
    STsymbol_t *symbolp;

    DBUG_ENTER ("STGetFirstEntry");

    symbolp = STLookupSymbol (symbol, table);
    result = symbolp->head;

    DBUG_RETURN (result);
}

/*
 * Functions for STsymboliterator_t
 */
STsymboliterator_t *
STSymbolIteratorGet (STtable_t *table)
{
    STsymboliterator_t *result;

    DBUG_ENTER ("STSymbolIteratorGet");

    result = (STsymboliterator_t *)Malloc (sizeof (STsymboliterator_t));

    result->head = table->head;
    result->pos = table->head;

    DBUG_RETURN (result);
}

STsymboliterator_t *
STSymbolIteratorRelease (STsymboliterator_t *iterator)
{
    DBUG_ENTER ("STSymbolIteratorRelease");

    iterator = Free (iterator);

    DBUG_RETURN (iterator);
}

STsymbol_t *
STSymbolIteratorNext (STsymboliterator_t *iterator)
{
    STsymbol_t *result;

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
STSymbolIteratorReset (STsymboliterator_t *iterator)
{
    DBUG_ENTER ("STSymbolIteratorReset");

    iterator->head = iterator->pos;

    DBUG_VOID_RETURN;
}

int
STSymbolIteratorHasMore (STsymboliterator_t *iterator)
{
    DBUG_ENTER ("STSymbolIteratorHasMore");

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions for STentryiterator_t
 */
static STentryiterator_t *
STEntryIteratorInit (STsymbol_t *symbol)
{
    STentryiterator_t *result;

    DBUG_ENTER ("STEntryIteratorInit");

    result = (STentryiterator_t *)Malloc (sizeof (STentryiterator_t));

    result->head = symbol->head;
    result->pos = symbol->head;

    DBUG_RETURN (result);
}

STentryiterator_t *
STEntryIteratorGet (const char *symbolname, STtable_t *table)
{
    STentryiterator_t *result;
    STsymbol_t *symbol;

    DBUG_ENTER ("STEntryIteratorGet");

    symbol = STLookupSymbol (symbolname, table);

    result = STEntryIteratorInit (symbol);

    DBUG_RETURN (result);
}

STentryiterator_t *
STEntryIteratorRelease (STentryiterator_t *iterator)
{
    DBUG_ENTER ("STEntryIteratorRelease");

    iterator = Free (iterator);

    DBUG_RETURN (iterator);
}

STentry_t *
STEntryIteratorNext (STentryiterator_t *iterator)
{
    STentry_t *result;

    DBUG_ENTER ("STEntryIteratorNext");

    result = iterator->pos;

    if (iterator->pos != NULL)
        iterator->pos = iterator->pos->next;

    DBUG_RETURN (result);
}

void
STEntryIteratorReset (STentryiterator_t *iterator)
{
    DBUG_ENTER ("STEntryIteratorReset");

    iterator->pos = iterator->head;

    DBUG_VOID_RETURN;
}

int
STEntryIteratorHasMore (STentryiterator_t *iterator)
{
    DBUG_ENTER ("STEntryIteratorHasMore");

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions to access STsymbol_t
 */
const char *
STSymbolName (STsymbol_t *symbol)
{
    DBUG_ENTER ("STSymbolName");

    DBUG_RETURN (symbol->name);
}

STvisibility_t
STSymbolVisibility (STsymbol_t *symbol)
{
    DBUG_ENTER ("STSymbolVisibility");

    DBUG_RETURN (symbol->vis);
}

/*
 * Functions to access STentry_t
 */

const char *
STEntryName (STentry_t *entry)
{
    DBUG_ENTER ("STEntryName");

    DBUG_ASSERT ((entry != NULL), "STEntryName called with NULL argument");

    DBUG_RETURN (entry->name);
}

STentrytype_t
STEntryType (STentry_t *entry)
{
    DBUG_ENTER ("STEntryType");

    DBUG_ASSERT ((entry != NULL), "STEntryType called with NULL argument");

    DBUG_RETURN (entry->type);
}

/*
 * functions for printing
 */
static void
STEntryPrint (STentry_t *entry)
{
    DBUG_ENTER ("STEntryPrint");

    printf ("    %s\n", entry->name);

    DBUG_VOID_RETURN;
}

static void
STSymbolPrint (STsymbol_t *symbol)
{
    STentry_t *entry;
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
STPrint (STtable_t *table)
{
    STsymbol_t *symbol;

    DBUG_ENTER ("STPrint");

    symbol = table->head;

    while (symbol != NULL) {
        STSymbolPrint (symbol);
        symbol = symbol->next;
    }

    DBUG_VOID_RETURN;
}
