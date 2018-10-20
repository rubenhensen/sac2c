#ifndef _SAC_MODULEMANAGER_H_
#define _SAC_MODULEMANAGER_H_

#include "types.h"

/******************************************************************************
 *
 * Modulemanager
 *
 * Prefix: MODM
 *
 *****************************************************************************/
extern module_t *MODMloadModule (const char *name);
extern module_t *MODMunLoadModule (module_t *module);

extern const char *MODMgetModuleName (module_t *module);

extern const sttable_t *MODMgetSymbolTable (module_t *module);
extern stringset_t *MODMgetDependencyTable (module_t *module);

extern serfun_p MODMgetDeSerializeFunction (const char *symbol, module_t *module);
bool MODMmoduleExists (const char *name);


#endif /* _SAC_MODULEMANAGER_H_ */
