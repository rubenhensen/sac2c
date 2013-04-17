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
