#ifndef _SAC_LIBBUILDER_H_
#define _SAC_LIBBUILDER_H_

#include "types.h"

/******************************************************************************
 *
 * Libbuilder
 *
 * Prefix: LIBB
 *
 *****************************************************************************/

extern void *BuildLibSearchStringCyg (const char *lib, strstype_t kind, void *rest);
extern node *LIBBcreateLibrary (node *syntax_tree);
extern node *LIBBcreateWrapperLibrary (node *syntax_tree);

#endif /* _SAC_LIBBUILDER_H_ */
