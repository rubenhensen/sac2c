/*
 *
 * $Log$
 * Revision 1.1  2004/09/21 20:38:09  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _MODULEMANAGER_H
#define _MODULEMANAGER_H

#include "types.h"

typedef struct MODULE_T module_t;
typedef node *(*serfun_t) ();

extern module_t *LoadModule (char *name);
extern module_t *UnLoadModule (module_t *module);

extern serfun_t GetDeSerializeFunction (char *symbol, module_t *module);

#endif /* _MODULEMANAGER_H */
