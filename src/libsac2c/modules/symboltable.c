#include "symboltable.h"

#define DBUG_PREFIX "UNDEFINED"
#include "debug.h"

#include "str.h"
#include "memory.h"
#include "types.h"

struct ST_SYMBOLITERATOR_T {
    stsymbol_t *head;
    stsymbol_t *pos;
};

struct ST_ENTRY_T {
    char *name;

    stentrytype_t type;

    /* In case it is a function we keep argument count.  */
    unsigned argc;

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
STentryInit (const char *name, stentrytype_t type, unsigned argc)
{
    stentry_t *result;

    DBUG_ENTER ();

    result = (stentry_t *)MEMmalloc (sizeof (stentry_t));

    result->name = STRcpy (name);
    result->type = type;
    result->argc = argc;
    result->next = NULL;

    DBUG_RETURN (result);
}

static stentry_t *
STentryDestroy (stentry_t *entry)
{
    stentry_t *result;

    DBUG_ENTER ();

    entry->name = MEMfree (entry->name);

    result = entry->next;

    entry = MEMfree (entry);

    DBUG_RETURN (result);
}

static stentry_t *
STentryCopy (const stentry_t *entry)
{
    stentry_t *result = NULL;

    DBUG_ENTER ();

    if (entry != NULL) {
        result = (stentry_t *)MEMmalloc (sizeof (stentry_t));

        result->name = STRcpy (entry->name);
        result->type = entry->type;
        result->argc = entry->argc;
        result->next = STentryCopy (entry->next);
    }

    DBUG_RETURN (result);
}

static bool
STentryEqual (stentry_t *one, stentry_t *two)
{
    bool result = TRUE;

    DBUG_ENTER ();

    result = result && STReq (one->name, two->name);
    result = result && (one->type == two->type);
    result = result && one->argc == two->argc;

    DBUG_RETURN (result);
}

/*
 * Functions for handling stsymbol_t
 */
static stsymbol_t *
STsymbolInit (const char *symbol, stvisibility_t vis)
{
    stsymbol_t *result;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

/*
 * Functions for handling sttable_t
 */
static void
STsymbolAdd (stsymbol_t *symbol, sttable_t *table)
{
    DBUG_ENTER ();

    symbol->next = table->head;
    table->head = symbol;

    DBUG_RETURN ();
}

static stsymbol_t *
STlookupSymbol (const char *symbol, const sttable_t *table)
{
    stsymbol_t *result;

    DBUG_ENTER ();

    result = table->head;

    while ((result != NULL) && !STReq (result->name, symbol)) {
        result = result->next;
    }

    DBUG_RETURN (result);
}

sttable_t *
STinit ()
{
    sttable_t *result;

    DBUG_ENTER ();

    result = (sttable_t *)MEMmalloc (sizeof (sttable_t));

    result->head = NULL;

    DBUG_RETURN (result);
}

sttable_t *
STdestroy (sttable_t *table)
{
    DBUG_ENTER ();

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

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    symbol = STlookupSymbol (symbolname, table);

    if (symbol == NULL) {
        symbol = STsymbolInit (symbolname, vis);
        STsymbolAdd (symbol, table);
    }

    DBUG_ASSERT (vis == symbol->vis, "found symbol with mixed visibility!");

    STentryAdd (entry, symbol);

    DBUG_RETURN ();
}

void
STadd (const char *symbol, int _vis, const char *name, int _type, void *_table,
       unsigned int argc)
{
    stentry_t *entry;
    stvisibility_t vis = (stvisibility_t)_vis;
    stentrytype_t type = (stentrytype_t)_type;
    sttable_t *table = (sttable_t *)_table;

    DBUG_ENTER ();

    entry = STentryInit (name, type, argc);
    STentryInsert (symbol, vis, entry, table);

    DBUG_RETURN ();
}

void
STremove (const char *symbol, sttable_t *table)
{
    stsymbol_t *symp;

    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

bool
STcontains (const char *symbol, const sttable_t *table)
{
    bool result;

    DBUG_ENTER ();

    result = (STlookupSymbol (symbol, table) != NULL);

    DBUG_RETURN (result);
}

bool
STcontainsEntry (const char *name, const sttable_t *table)
{
    stsymbol_t *symbol;
    stentry_t *entry;
    bool result = FALSE;

    DBUG_ENTER ();

    symbol = table->head;

    while ((symbol != NULL) && (!result)) {
        entry = symbol->head;

        while ((entry != NULL) && (!result)) {
            result = STReq (entry->name, name);

            entry = entry->next;
        }
        symbol = symbol->next;
    }

    DBUG_RETURN (result);
}

stsymbol_t *
STget (const char *symbol, const sttable_t *table)
{
    DBUG_ENTER ();

    DBUG_RETURN (STlookupSymbol (symbol, table));
}

stentry_t *
STgetFirstEntry (const char *symbol, const sttable_t *table)
{
    stentry_t *result;
    stsymbol_t *symbolp;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    result = (stsymboliterator_t *)MEMmalloc (sizeof (stsymboliterator_t));

    result->head = table->head;
    result->pos = table->head;

    DBUG_RETURN (result);
}

stsymboliterator_t *
STsymbolIteratorRelease (stsymboliterator_t *iterator)
{
    DBUG_ENTER ();

    iterator = MEMfree (iterator);

    DBUG_RETURN (iterator);
}

stsymbol_t *
STsymbolIteratorNext (stsymboliterator_t *iterator)
{
    stsymbol_t *result;

    DBUG_ENTER ();

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
    DBUG_ENTER ();

    iterator->head = iterator->pos;

    DBUG_RETURN ();
}

int
STsymbolIteratorHasMore (stsymboliterator_t *iterator)
{
    DBUG_ENTER ();

    DBUG_RETURN (iterator->pos != NULL);
}

bool
STsymbolIteratorSymbolArityIs (stsymboliterator_t *iterator, unsigned arity)
{
    DBUG_ENTER ();

    stentry_t *entry = iterator->pos->head;
    while (entry) {
        if (entry->argc == arity)
            DBUG_RETURN (TRUE);

        entry = entry->next;
    }

    DBUG_RETURN (FALSE);
}

/*
 * Functions for stentryiterator_t
 */
static stentryiterator_t *
STentryIteratorInit (stsymbol_t *symbol)
{
    stentryiterator_t *result;

    DBUG_ENTER ();

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

    DBUG_ENTER ();

    symbol = STlookupSymbol (symbolname, table);

    result = STentryIteratorInit (symbol);

    DBUG_RETURN (result);
}

stentryiterator_t *
STentryIteratorRelease (stentryiterator_t *iterator)
{
    DBUG_ENTER ();

    iterator = MEMfree (iterator);

    DBUG_RETURN (iterator);
}

stentry_t *
STentryIteratorNext (stentryiterator_t *iterator)
{
    stentry_t *result;

    DBUG_ENTER ();

    result = iterator->pos;

    if (iterator->pos != NULL)
        iterator->pos = iterator->pos->next;

    DBUG_RETURN (result);
}

void
STentryIteratorReset (stentryiterator_t *iterator)
{
    DBUG_ENTER ();

    iterator->pos = iterator->head;

    DBUG_RETURN ();
}

int
STentryIteratorHasMore (stentryiterator_t *iterator)
{
    DBUG_ENTER ();

    DBUG_RETURN (iterator->pos != NULL);
}

/*
 * Functions to access stsymbol_t
 */
const char *
STsymbolName (stsymbol_t *symbol)
{
    DBUG_ENTER ();

    DBUG_RETURN (symbol->name);
}

stvisibility_t
STsymbolVisibility (stsymbol_t *symbol)
{
    DBUG_ENTER ();

    DBUG_RETURN (symbol->vis);
}

/*
 * Functions to access stentry_t
 */

const char *
STentryName (stentry_t *entry)
{
    DBUG_ENTER ();

    DBUG_ASSERT (entry != NULL, "STentryName called with NULL argument");

    DBUG_RETURN (entry->name);
}

stentrytype_t
STentryType (stentry_t *entry)
{
    DBUG_ENTER ();

    DBUG_ASSERT (entry != NULL, "STentryType called with NULL argument");

    DBUG_RETURN (entry->type);
}

stentrytype_t
STentryArgc (stentry_t *entry)
{
    DBUG_ENTER ();

    DBUG_ASSERT (entry != NULL, "STentryType called with NULL argument");

    DBUG_RETURN ((stentrytype_t)entry->argc);
}

/*
 * functions for printing
 */
static void
STentryPrint (stentry_t *entry)
{
    DBUG_ENTER ();

    printf ("    %s\n", entry->name);

    DBUG_RETURN ();
}

static void
STsymbolPrint (stsymbol_t *symbol)
{
    stentry_t *entry;
    char *visname;

    DBUG_ENTER ();

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

    DBUG_RETURN ();
}

void
STprint (const sttable_t *table)
{
    stsymbol_t *symbol;

    DBUG_ENTER ();

    symbol = table->head;

    while (symbol != NULL) {
        STsymbolPrint (symbol);
        symbol = symbol->next;
    }

    DBUG_RETURN ();
}

stentrytype_t
STsymbolGetEntryType (stsymbol_t *symbol)
{
    DBUG_ENTER ();

    DBUG_RETURN (symbol->head->type);
}
#undef DBUG_PREFIX
