/**
 * @file namespaces.h
 * @brief function declarations for handling namespace identifiers
 *        as used throughout the compiler
 */

#ifndef _SAC_NAMESPACES_H_
#define _SAC_NAMESPACES_H_

#include "types.h"

extern int NSaddMapping (const char *module, void *_view);

extern namespace_t *NSgetNamespace (const char *module);
extern namespace_t *NSgetRootNamespace (void);
extern namespace_t *NSgetInitNamespace (void);
extern namespace_t *NSgetCWrapperNamespace (void);
extern namespace_t *NSgetMTNamespace (const namespace_t *orig);
extern namespace_t *NSgetSTNamespace (const namespace_t *orig);
extern namespace_t *NSgetXTNamespace (const namespace_t *orig);

extern namespace_t *NSdupNamespace (namespace_t *ns);
extern namespace_t *NSfreeNamespace (namespace_t *ns);
extern void NStouchNamespace (namespace_t *ns, info *arg_info);

extern bool NSequals (const namespace_t *one, const namespace_t *two);

extern const char *NSgetName (const namespace_t *ns);
extern const char *NSgetModule (const namespace_t *ns);

extern namespace_t *NSbuildView (const namespace_t *orig);

extern void NSserializeNamespace (FILE *file, const namespace_t *ns);
extern namespace_t *NSdeserializeNamespace (int id);
extern void *NSdeserializeView (const char *module, int id, void *_next);

extern void NSgenerateNamespaceMap (void);

extern void xfree_namespace_pool (void);

#endif /* _SAC_NAMESPACES_H_ */
