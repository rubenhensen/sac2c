/*
 *
 * $Log$
 * Revision 1.2  2004/09/23 21:14:23  sah
 * ongoing implementation
 *
 * Revision 1.1  2004/09/21 20:38:09  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _MODULEMANAGER_H
#define _MODULEMANAGER_H

#include "types.h"
#include "symboltable.h"

typedef struct MODULE_T module_t;
typedef node *(*serfun_p) ();

extern module_t *LoadModule (const char *name);
extern module_t *UnLoadModule (module_t *module);

extern const char *GetModuleName (module_t *module);

extern symboltable_t *GetSymbolTable (module_t *module);
extern serfun_p GetDeSerializeFunction (const char *symbol, module_t *module);

#endif /* _MODULEMANAGER_H */
