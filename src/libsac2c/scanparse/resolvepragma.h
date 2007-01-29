/*
 *
 * $Log$
 * Revision 1.4  2004/11/26 19:18:30  skt
 * RSPmodule added
 *
 * Revision 1.3  2004/11/26 18:50:16  sbs
 * arg and ret added
 *
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

extern node *RSPmodule (node *arg_node, info *info);
extern node *RSPtypedef (node *arg_node, info *info);
extern node *RSPobjdef (node *arg_node, info *info);
extern node *RSPfundef (node *arg_node, info *info);
extern node *RSParg (node *arg_node, info *info);
extern node *RSPret (node *arg_node, info *info);

extern node *RSPdoResolvePragmas (node *syntax_tree);

#endif /* _SAC_RESOLVE_PRAGMAS_H */
