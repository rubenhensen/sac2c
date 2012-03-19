/*
 *
 * $Log$
 * Revision 1.11  2005/05/18 13:56:51  sah
 * enabled caching of symboltables which
 * leads to a huge speedup when analysing use and import
 * from big modules
 *
 * Revision 1.10  2004/11/23 21:18:06  ktr
 * fixed some type names.
 *
 * Revision 1.9  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.8  2004/11/21 11:22:03  sah
 * removed some old ast infos
 *
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

extern sttable_t *STinit ();
extern sttable_t *STdestroy (sttable_t *table);
extern sttable_t *STcopy (const sttable_t *table);
extern void STadd (const char *symbol, stvisibility_t visbility, const char *name,
                   stentrytype_t type, sttable_t *table, unsigned);
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

/*
 * Functions to print symbol tables
 */
extern void STprint (const sttable_t *table);
extern stentrytype_t STsymbolGetEntryType (stsymbol_t *);
#endif /* _SAC_SYMBOLTABLE_H_ */
