/*
 *
 * $Log$
 * Revision 1.2  2004/11/21 22:45:20  sbs
 * SacDevCamp04
 *
 * Revision 1.1  2004/11/07 18:04:39  sah
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_RESOLVE_PRAGMAS_H
#define _SAC_RESOLVE_PRAGMAS_H

#include "types.h"

extern node *RSPfundef (node *arg_node, info *info);
extern node *RSPmodule (node *arg_node, info *info);

extern void RSPdoResolvePragmas (node *syntax_tree);

#endif /* _SAC_RESOLVE_PRAGMAS_H */
