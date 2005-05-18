/*
 *
 * $Log$
 * Revision 1.9  2005/05/18 13:56:51  sah
 * enabled caching of symboltables which
 * leads to a huge speedup when analysing use and import
 * from big modules
 *
 * Revision 1.8  2004/11/23 21:18:06  ktr
 * fixed some type names.
 *
 * Revision 1.7  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
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

#endif /* _SAC_MODULEMANAGER_H_ */
