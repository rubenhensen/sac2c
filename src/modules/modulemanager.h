/*
 *
 * $Log$
 * Revision 1.6  2004/11/14 15:22:18  sah
 * changed signature of serfun_p
 *
 * Revision 1.5  2004/10/28 17:18:07  sah
 * added support for dependency tables
 *
 * Revision 1.4  2004/10/26 09:32:36  sah
 * changed functiontype for serialize functions
 *
 * Revision 1.3  2004/10/25 11:58:47  sah
 * major code cleanup
 *
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
#include "stringset.h"

typedef struct MODULE_T module_t;
typedef node *(*serfun_p) ();

extern module_t *LoadModule (const char *name);
extern module_t *UnLoadModule (module_t *module);

extern const char *GetModuleName (module_t *module);

extern STtable_t *GetSymbolTable (module_t *module);
extern stringset_t *GetDependencyTable (module_t *module);

extern serfun_p GetDeSerializeFunction (const char *symbol, module_t *module);

#endif /* _MODULEMANAGER_H */
