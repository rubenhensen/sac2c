/*
 *
 * $Log$
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
    STE_funbody,
    STE_funhead,
    STE_typedef,
    STE_objdef,
    STE_namespace
} symbolentrytype_t;

typedef struct SYMBOLENTRY_T symbolentry_t;
typedef struct SYMBOLCHAIN_T symbolchain_t;
typedef struct SYMBOLENTRYCHAIN_T symbolentrychain_t;
typedef struct SYMBOLTABLE_T symboltable_t;

extern symboltable_t *SymbolTableInit ();
extern symboltable_t *SymbolTableDestroy (symboltable_t *table);
extern void SymbolTableAdd (const char *symbol, const char *name, symbolentrytype_t type,
                            symboltable_t *table);
extern void SymbolTableRemove (const char *symbol, symboltable_t *table);
extern bool SymbolTableContains (const char *symbol, symboltable_t *table);

extern symbolchain_t *SymbolTableSymbolChainGet (symboltable_t *table);
extern symbolchain_t *SymbolTableSymbolChainRelease (symbolchain_t *chain);
extern const char *SymbolTableSymbolChainNext (symbolchain_t *chain);
extern void SymbolTableSymbolChainReset (symbolchain_t *chain);
extern int SymbolTableSymbolChainHasMore (symbolchain_t *chain);

extern symbolentrychain_t *SymbolTableEntryChainGet (const char *symbol,
                                                     symboltable_t *table);
extern symbolentrychain_t *SymbolTableEntryChainRelease (symbolentrychain_t *chain);
extern symbolentry_t *SymbolTableEntryChainNext (symbolentrychain_t *chain);
extern void SymbolTableEntryChainReset (symbolentrychain_t *chain);
extern int SymbolTableEntryChainHasMore (symbolentrychain_t *chain);

extern const char *SymbolTableEntryName (symbolentry_t *entry);
extern symbolentrytype_t SymbolTableEntryType (symbolentry_t *entry);

extern void SymbolTablePrint (symboltable_t *table);

#endif /* _SYMBOLTABLE_H */
