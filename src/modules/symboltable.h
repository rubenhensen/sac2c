/*
 *
 * $Log$
 * Revision 1.1  2004/09/22 11:37:36  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SYMBOLTABLE_H
#define _SYMBOLTABLE_H

typedef enum { STE_funbody, STE_funhead, STE_typedef, STE_objdef } symbolentrytype_t;

typedef struct SYMBOLENTRY_T symbolentry_t;
typedef struct SYMBOLCHAIN_T symbolchain_t;
typedef struct SYMBOLTABLE_T symboltable_t;

extern symboltable_t *SymbolTableInit ();
extern symboltable_t *SymbolTableDestroy (symboltable_t *table);
extern void SymbolTableAdd (char *symbol, char *name, symbolentrytype_t type,
                            symboltable_t *table);
extern symbolchain_t *SymbolTableChainGet (char *symbol, symboltable_t *table);
extern symbolchain_t *SymbolTableChainRelease (symbolchain_t *chain);
extern symbolentry_t *SymbolTableChainNext (symbolchain_t *chain);
extern void SymbolTableChainReset (symbolchain_t *chain);

extern char *SymbolTableEntryName (symbolentry_t *entry);
extern symbolentrytype_t SymbolTableEntryType (symbolentry_t *entry);

#endif /* _SYMBOLTABLE_H */
