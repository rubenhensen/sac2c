/**
 * @file namespaces.h
 * @brief function declarations for handling namespace identifiers
 *        as uses throughout the compiler
 * @author Stephan Herhut
 * @date 2005-07-11
 */

#ifndef _SAC_NAMESPACES_H_
#define _SAC_NAMESPACES_H_

#include "types.h"

extern namespace_t *NSgetNamespace (const char *module);
extern namespace_t *NSgetRootNamespace ();

extern namespace_t *NSdupNamespace (const namespace_t *ns);
extern namespace_t *NSfreeNamespace (namespace_t *ns);

extern bool NSequals (const namespace_t *one, const namespace_t *two);

extern const char *NSgetName (const namespace_t *ns);
extern const char *NSgetModule (const namespace_t *ns);

extern namespace_t *NSbuildView (const namespace_t *orig);
extern bool NShasView (const namespace_t *ns);

extern void NSserializeNamespace (FILE *file, const namespace_t *ns);
extern namespace_t *NSdeserialzeNamespace (int id);
extern view_t *NSdeserializeView (const char *module, int id, view_t *next);

extern void NSgenerateNamespaceMap ();
#endif /* _SAC_NAMESPACES_H_ */
