/*
 *
 * $Log$
 * Revision 1.1  2004/11/07 18:04:39  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _RESOLVE_PRAGMAS_H
#define _RESOLVE_PRAGMAS_H

#include "types.h"

extern node *RSPFundef (node *arg_node, info *info);
extern node *RSPModul (node *arg_node, info *info);

extern void DoResolvePragmas (node *syntax_tree);

#endif /* _RESOLVE_PRAGMAS_H */
