/*
 *
 * $Log$
 * Revision 1.12  2005/09/29 14:03:32  sah
 * fixed an ugly bug...
 * STcopy now really copies a symbol table...
 *
 * Revision 1.11  2005/06/16 10:00:44  sah
 * fixed continue on error problem
 *
 * Revision 1.10  2005/05/18 13:56:51  sah
 * enabled caching of symboltables which
 * leads to a huge speedup when analysing use and import
 * from big modules
 *
 * Revision 1.9  2004/11/27 01:06:38  cg
 * Functions renamed according to new naming conventions.
 *
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
#include "str.h"
#include "memory.h"
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
STentryInit (const char *name, stentrytype_t type)
{
    stentry_t *result;

    DBUG_ENTER ("STentryInit");

    result = (stentry_t *)MEMmalloc (sizeof (stentry_t));

    result->name = STRcpy (name);
    result->type = type;
    result->next = NULL;

    DBUG_RETURN (result);
}

static stentry_t *
STentryDestroy (stentry_t *entry)
{
    stentry_t *result;

    DBUG_ENTER ("STentryDestroy");

    entry->name = MEMfree (entry->name);

    result = entry->next;

    entry = MEMfree (entry);

    DBUG_RETURN (result);
}

static stentry_t *
STentryCopy (const stentry_t *entry)
{
    stentry_t *result = NULL;

    DBUG_ENTER ("STentryCopy");

    if (entry != NULL) {
        result = (stentry_t *)MEMmalloc (sizeof (stentry_t));

        result->name = STRcpy (entry->name);
        result->type = entry->type;
        result->next = STentryCopy (entry->next);
    }

    DBUG_RETURN (result);
}

static bool
STentryEqual (stentry_t *one, stentry_t *two)
{
    bool result = TRUE;

    DBUG_ENTER ("STentryEqual");

    result = result && (!strcmp (one->name, two->name));
    result = result && (one->type == two->type);

    DBUG_RETURN (result);
}

/*
 * Functions for handling stsymbol_t
 */
static stsymbol_t *
STsymbolInit (const char *symbol, stvisibility_t vis)
{
    stsymbol_t *result;

    DBUG_ENTER ("STsymbolInit");

    result = (stsymbol_t *)MEMmalloc (sizeof (stsymbol_t));

    result->name = STRcpy (symbol);
    result->vis = vis;
    result->head = NULL;
    result->next = NULL;

    DBUG_RETURN (result);
}

static stsymbol_t *
STsymbolDestroy (stsymbol_t *symbol)
{
    stsymbol_t *result;

    DBUG_ENTER ("STsymbolDestroy");

    while (symbol->head != NULL)
        symbol->head = STentryDestroy (symbol->head);

    symbol->name = MEMfree (symbol->name);

    result = symbol->next;

    symbol = MEMfree (symbol);

    DBUG_RETURN (result);
}

static stsymbol_t *
STsymbolCopy (const stsymbol_t *symbol)
{
    stsymbol_t *result = NULL;

    DBUG_ENTER ("STsymbolCopy");

    if (symbol != NULL) {
        result = (stsymbol_t *)MEMmalloc (sizeof (stsymbol_t));

        result->name = STRcpy (symbol->name);
        result->vis = symbol->vis;
        result->head = STentryCopy (symbol->head);
        result->next = STsymbolCopy (symbol->next);
    }

    DBUG_RETURN (result);
}

static void
STentryAdd (stentry_t *entry, stsymbol_t *symbol)
{
    stentry_t *pos;
    bool found = FALSE;

    DBUG_ENTER ("STsymbolAdd");

    /* check whether entry already exists */
    pos = symbol->head;

    while ((pos != NULL) && (!found)) {
        found = STentryEqual (pos, entry);
        pos = pos->next;
    }

    if (found) {
        entry = STentryDestroy (entry);
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
STsymbolAdd (stsymbol_t *symbol, sttable_t *table)
{
    DBUG_ENTER ("STsymbolAdd");

    symbol->next = table->head;
    table->head = symbol;

    DBUG_VOID_RETURN;
}

static stsymbol_t *
STlookupSymbol (const char *symbol, const sttable_t *table)
{
    stsymbol_t *result;

    DBUG_ENTER ("STlookupSymbol");

    result = table->head;

    while ((result != NULL) && (strcmp (result->name, symbol))) {
        result = result->next;
    }

    DBUG_RETURN (result);
}

sttable_t *
STinit ()
{
    sttable_t *result;

    DBUG_ENTER ("STinit");

    result = (sttable_t *)MEMmalloc (sizeof (sttable_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

sttable_t *
STdestroy (sttable_t *table)
{
    DBUG_ENTER ("STdestroy");

    while (table->head != NULL) {
        table->head = STsymbolDestroy (table->head);
    }

    table = MEMfree (table);

    DBUG_RETURN (table);
}

sttable_t *
STcopy (const sttable_t *table)
{
    sttable_t *result = NULL;

    DBUG_ENTER ("STcopy");

    if (table != NULL) {
        result = (sttable_t *)MEMmalloc (sizeof (sttable_t));

        result->head = STsymbolCopy (table->head);
    }

    DBUG_RETURN (result);
}

static void
STentryInsert (const char *symbolname, stvisibility_t vis, stentry_t *entry,
               sttable_t *table)
{
    stsymbol_t *symbol;

    DBUG_ENTER ("STentryInsert");

    symbol = STlookupSymbol (symbolname, table);

    if (symbol == NULL) {
        symbol = STsymbolInit (symbolname, vis);
        STsymbolAdd (symbol, table);
    }

    DBUG_ASSERT ((vis == symbol->vis), "found symbol with mixed visibility!");

    STentryAdd (entry, symbol);

    DBUG_VOID_RETURN;
}

void
STadd (const char *symbol, stvisibility_t vis, const char *name, stentrytype_t type,
       sttable_t *table)
{
    stentry_t *entry;

    DBUG_ENTER ("STadd");

    entry = STentryInit (name, type);
    STentryInsert (symbol, vis, entry, table);

    DBUG_VOID_RETURN;
}

void
STremove (const char *symbol, sttable_t *table)
{
    stsymbol_t *symp;

    DBUG_ENTER ("STremove");

    symp = STlookupSymbol (symbol, table);

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

        symp = STsymbolDestroy (symp);
    }

    DBUG_VOID_RETURN;
}

bool
STcontains (const char *symbol, const sttable_t *table)
{
    bool result;

    DBUG_ENTER ("STcontains");

    result = (STlookupSymbol (symbol, table) != NULL);

    DBUG_RETURN (result);
}

bool
STcontainsEntry (const char *name, const sttable_t *table)
{
    stsymbol_t *symbol;
    stentry_t *entry;
    bool result = FALSE;

    DBUG_ENTER ("STcontainsEntry");

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
STget (const char *symbol, const sttable_t *table)
{
    DBUG_ENTER ("STget");

    DBUG_RETURN (STlookupSymbol (symbol, table));
}

stentry_t *
STgetFirstEntry (const char *symbol, const sttable_t *table)
{
    stentry_t *result;
    stsymbol_t *symbolp;

    DBUG_ENTER ("STgetFirstEntry");

    symbolp = STlookupSymbol (symbol, table);
    result = symbolp->head;

    DBUG_RETURN (result);
}

/*
 * Functions for stsymboliterator_t
 */
stsymboliterator_t *
STsymbolIteratorGet (const sttable_t *table)
{
    stsymboliterator_t *result;

    DBUG_ENTER ("STsymbolIteratorGet");

    result = (stsymboliterator_t *)MEMmalloc (sizeof (stsymboliterator_t));

    result->head = table->head;
    result->pos = table->head;

    DBUG_RETURN (result);
}

stsymboliterator_t *
STsymbolIteratorRelease (stsymboliterator_t *iterator)
{
    DBUG_ENTER ("STsymbolIteratorRelease");

    iterator = MEMfree (iterator);

    DBUG_RETURN (iterator);
}

stsymbol_t *
STsymbolIteratorNext (stsymboliterator_t *iterator)
{
    stsymbol_t *result;

    DBUG_ENTER ("STsymbolIteratorNext");

    if (iterator->pos == NULL) {
        result = NULL;
    } else {
        result = iterator->pos;
        iterator->pos = iterator->pos->next;
    }

    DBUG_RETURN (result);
}

void
STsymbolIteratorReset (stsymboliterator_t *iterator)
{
    DBUG_ENTER ("STsymbolIteratorReset");

    iterator->head = iterator->pos;

    DBUG_VOID_RETURN;
}

int
STsymbolIteratorHasMore (stsymboliterator_t *iterator)
{
    DBUG_ENTER ("STsymbolIteratorHasMore");

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions for stentryiterator_t
 */
static stentryiterator_t *
STentryIteratorInit (stsymbol_t *symbol)
{
    stentryiterator_t *result;

    DBUG_ENTER ("STentryIteratorInit");

    result = (stentryiterator_t *)MEMmalloc (sizeof (stentryiterator_t));

    if (symbol != NULL) {
        result->head = symbol->head;
        result->pos = symbol->head;
    } else {
        /*
         * create an empty iterator for unknown symbols
         */
        result->head = NULL;
        result->pos = NULL;
    }

    DBUG_RETURN (result);
}

stentryiterator_t *
STentryIteratorGet (const char *symbolname, const sttable_t *table)
{
    stentryiterator_t *result;
    stsymbol_t *symbol;

    DBUG_ENTER ("STentryIteratorGet");

    symbol = STlookupSymbol (symbolname, table);

    result = STentryIteratorInit (symbol);

    DBUG_RETURN (result);
}

stentryiterator_t *
STentryIteratorRelease (stentryiterator_t *iterator)
{
    DBUG_ENTER ("STentryIteratorRelease");

    iterator = MEMfree (iterator);

    DBUG_RETURN (iterator);
}

stentry_t *
STentryIteratorNext (stentryiterator_t *iterator)
{
    stentry_t *result;

    DBUG_ENTER ("STentryIteratorNext");

    result = iterator->pos;

    if (iterator->pos != NULL)
        iterator->pos = iterator->pos->next;

    DBUG_RETURN (result);
}

void
STentryIteratorReset (stentryiterator_t *iterator)
{
    DBUG_ENTER ("STentryIteratorReset");

    iterator->pos = iterator->head;

    DBUG_VOID_RETURN;
}

int
STentryIteratorHasMore (stentryiterator_t *iterator)
{
    DBUG_ENTER ("STentryIteratorHasMore");

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions to access stsymbol_t
 */
const char *
STsymbolName (stsymbol_t *symbol)
{
    DBUG_ENTER ("STsymbolName");

    DBUG_RETURN (symbol->name);
}

stvisibility_t
STsymbolVisibility (stsymbol_t *symbol)
{
    DBUG_ENTER ("STsymbolVisibility");

    DBUG_RETURN (symbol->vis);
}

/*
 * Functions to access stentry_t
 */

const char *
STentryName (stentry_t *entry)
{
    DBUG_ENTER ("STentryName");

    DBUG_ASSERT ((entry != NULL), "STentryName called with NULL argument");

    DBUG_RETURN (entry->name);
}

stentrytype_t
STentryType (stentry_t *entry)
{
    DBUG_ENTER ("STentryType");

    DBUG_ASSERT ((entry != NULL), "STentryType called with NULL argument");

    DBUG_RETURN (entry->type);
}

/*
 * functions for printing
 */
static void
STentryPrint (stentry_t *entry)
{
    DBUG_ENTER ("STentryPrint");

    printf ("    %s\n", entry->name);

    DBUG_VOID_RETURN;
}

static void
STsymbolPrint (stsymbol_t *symbol)
{
    stentry_t *entry;
    char *visname;

    DBUG_ENTER ("STsymbolPrint");

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
        STentryPrint (entry);
        entry = entry->next;
    }

    printf ("\n");

    DBUG_VOID_RETURN;
}

void
STprint (const sttable_t *table)
{
    stsymbol_t *symbol;

    DBUG_ENTER ("STprint");

    symbol = table->head;

    while (symbol != NULL) {
        STsymbolPrint (symbol);
        symbol = symbol->next;
    }

    DBUG_VOID_RETURN;
}
