/*
 *
 * $Log$
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

/******************************************************************************
 *
 * Dependencie Resolution traversal
 *
 * Prefix: DEP
 *
 *****************************************************************************/
extern node *DEPdoResolveDependencies (node *syntax_tree);

#endif /* _DEPENDENCIES_H */
