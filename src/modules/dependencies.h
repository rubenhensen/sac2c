/* $Id$ */

#ifndef _SAC_DEPENDENCIES_H_
#define _SAC_DEPENDENCIES_H_

#include "types.h"

extern void DEPgenerateDependencyTable (stringset_t *deps);
extern stringset_t *DEPbuildDependencyClosure (stringset_t *deps);
extern node *DEPdoPrintDependencies (node *syntax_tree);

#endif /* _DEPENDENCIES_H */
