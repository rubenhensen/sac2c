/*
 *
 * $Log$
 * Revision 1.7  2004/11/17 19:47:04  sah
 * added visibility support
 *
 * Revision 1.6  2004/10/25 11:58:47  sah
 * major code cleanup
 *
 * Revision 1.5  2004/10/22 15:17:20  sah
 * added STE_wrapperbody, STE_wrapperhead
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
 * Revision 1.1  2004/09/22 11:37:36  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

#include "types.h"

typedef enum {
    SET_funbody,
    SET_funhead,
    SET_typedef,
    SET_objdef,
    SET_wrapperbody,
    SET_wrapperhead,
    SET_namespace
} STentrytype_t;

typedef enum { SVT_local, SVT_provided, SVT_exported } STvisibility_t;

typedef struct ST_ENTRY_T STentry_t;
typedef struct ST_SYMBOLITERATOR_T STsymboliterator_t;
typedef struct ST_ENTRYITERATOR_T STentryiterator_t;
typedef struct ST_SYMBOLTABLE_T STtable_t;
typedef struct ST_SYMBOL_T STsymbol_t;

/*
 * Functions for handling symbol tables
 */
extern STtable_t *STInit ();
extern STtable_t *STDestroy (STtable_t *table);
extern void STAdd (const char *symbol, STvisibility_t visbility, const char *name,
                   STentrytype_t type, STtable_t *table);
extern void STRemove (const char *symbol, STtable_t *table);
extern bool STContains (const char *symbol, STtable_t *table);
extern STsymbol_t *STGet (const char *symbol, STtable_t *table);
extern STentry_t *STGetFirstEntry (const char *symbol, STtable_t *table);

/*
 * Symbol iterator functions
 */
extern STsymboliterator_t *STSymbolIteratorGet (STtable_t *table);
extern STsymboliterator_t *STSymbolIteratorRelease (STsymboliterator_t *iterator);
extern STsymbol_t *STSymbolIteratorNext (STsymboliterator_t *iterator);
extern void STSymbolIteratorReset (STsymboliterator_t *iterator);
extern int STSymbolIteratorHasMore (STsymboliterator_t *iterator);

/*
 * Entry iterator functions
 */
extern STentryiterator_t *STEntryIteratorGet (const char *symbol, STtable_t *table);
extern STentryiterator_t *STEntryIteratorRelease (STentryiterator_t *iterator);
extern STentry_t *STEntryIteratorNext (STentryiterator_t *iterator);
extern void STEntryIteratorReset (STentryiterator_t *iterator);
extern int STEntryIteratorHasMore (STentryiterator_t *iterator);

/*
 * Functions to access table symbols
 */
extern const char *STSymbolName (STsymbol_t *symbol);
extern STvisibility_t STSymbolVisibility (STsymbol_t *symbol);

/*
 * Functions to access table entries
 */
extern const char *STEntryName (STentry_t *entry);
extern STentrytype_t STEntryType (STentry_t *entry);

/*
 * Functions to print symbol tables
 */
extern void STPrint (STtable_t *table);

#endif /* _SYMBOLTABLE_H */
