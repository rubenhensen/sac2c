#ifndef _SAC_SYMBOLTABLE_H_
#define _SAC_SYMBOLTABLE_H_

#include "types.h"

/******************************************************************************
 *
 * Symboltable
 *
 * Prefix: ST
 *
 *****************************************************************************/

/*
 * Functions for handling symbol tables
 */

extern sttable_t *STinit (void);
extern sttable_t *STdestroy (sttable_t *table);
extern sttable_t *STcopy (const sttable_t *table);
extern void STadd (const char *symbol, int visbility, const char *name, int type,
                   void *table, unsigned int argc);

extern void STremove (const char *symbol, sttable_t *table);
extern bool STcontains (const char *symbol, const sttable_t *table);
extern bool STcontainsEntry (const char *name, const sttable_t *table);
extern stsymbol_t *STget (const char *symbol, const sttable_t *table);
extern stentry_t *STgetFirstEntry (const char *symbol, const sttable_t *table);

/*
 * Symbol iterator functions
 */
extern stsymboliterator_t *STsymbolIteratorGet (const sttable_t *table);
extern stsymboliterator_t *STsymbolIteratorRelease (stsymboliterator_t *iterator);
extern stsymbol_t *STsymbolIteratorNext (stsymboliterator_t *iterator);
extern void STsymbolIteratorReset (stsymboliterator_t *iterator);
extern int STsymbolIteratorHasMore (stsymboliterator_t *iterator);

/*
 * Entry iterator functions
 */
extern stentryiterator_t *STentryIteratorGet (const char *symbol, const sttable_t *table);
extern stentryiterator_t *STentryIteratorRelease (stentryiterator_t *iterator);
extern stentry_t *STentryIteratorNext (stentryiterator_t *iterator);
extern void STentryIteratorReset (stentryiterator_t *iterator);
extern int STentryIteratorHasMore (stentryiterator_t *iterator);
extern bool STsymbolIteratorSymbolArityIs (stsymboliterator_t *iterator, unsigned arity);

/*
 * Functions to access table symbols
 */
extern const char *STsymbolName (stsymbol_t *symbol);
extern stvisibility_t STsymbolVisibility (stsymbol_t *symbol);

/*
 * Functions to access table entries
 */
extern const char *STentryName (stentry_t *entry);
extern stentrytype_t STentryType (stentry_t *entry);
extern stentrytype_t STentryArgc (stentry_t *entry);

/*
 * Functions to print symbol tables
 */
extern void STprint (const sttable_t *table);
extern stentrytype_t STsymbolGetEntryType (stsymbol_t *);
#endif /* _SAC_SYMBOLTABLE_H_ */
