/*
 *
 * $Log$
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

extern STtable_t *STinit ();
extern STtable_t *STdestroy (STtable_t *table);
extern void STadd (const char *symbol, STvisibility_t visbility, const char *name,
                   STentrytype_t type, STtable_t *table);
extern void STremove (const char *symbol, STtable_t *table);
extern bool STcontains (const char *symbol, STtable_t *table);
extern bool STcontainsEntry (const char *name, STtable_t *table);
extern STsymbol_t *STget (const char *symbol, STtable_t *table);
extern STentry_t *STgetFirstEntry (const char *symbol, STtable_t *table);

/*
 * Symbol iterator functions
 */
extern STsymboliterator_t *STsymbolIteratorGet (STtable_t *table);
extern STsymboliterator_t *STsymbolIteratorRelease (STsymboliterator_t *iterator);
extern STsymbol_t *STsymbolIteratorNext (STsymboliterator_t *iterator);
extern void STsymbolIteratorReset (STsymboliterator_t *iterator);
extern int STsymbolIteratorHasMore (STsymboliterator_t *iterator);

/*
 * Entry iterator functions
 */
extern STentryiterator_t *STentryIteratorGet (const char *symbol, STtable_t *table);
extern STentryiterator_t *STentryIteratorRelease (STentryiterator_t *iterator);
extern STentry_t *STentryIteratorNext (STentryiterator_t *iterator);
extern void STentryIteratorReset (STentryiterator_t *iterator);
extern int STentryIteratorHasMore (STentryiterator_t *iterator);

/*
 * Functions to access table symbols
 */
extern const char *STsymbolName (STsymbol_t *symbol);
extern STvisibility_t STsymbolVisibility (STsymbol_t *symbol);

/*
 * Functions to access table entries
 */
extern const char *STentryName (STentry_t *entry);
extern STentrytype_t STentryType (STentry_t *entry);

/*
 * Functions to print symbol tables
 */
extern void STprint (STtable_t *table);

#endif /* _SAC_SYMBOLTABLE_H_ */
