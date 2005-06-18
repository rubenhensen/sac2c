/*
 *
 * $Log$
 * Revision 1.4  2005/06/18 18:06:00  sah
 * moved entire dependency handling to dependencies.c
 * the dependency table is now created shortly prior
 * to c code generation
 *
 * Revision 1.3  2005/06/01 18:01:24  sah
 * finished printing of dependencies
 *
 * Revision 1.2  2004/11/22 16:57:41  ktr
 * SACDevCamp 04 Ismop
 *
 * Revision 1.1  2004/11/08 19:05:01  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_DEPENDENCIES_H_
#define _SAC_DEPENDENCIES_H_

#include "types.h"

extern void DEPgenerateDependencyTable (stringset_t *deps);
extern node *DEPdoResolveDependencies (node *syntax_tree);
extern node *DEPdoPrintDependencies (node *syntax_tree);

#endif /* _DEPENDENCIES_H */
