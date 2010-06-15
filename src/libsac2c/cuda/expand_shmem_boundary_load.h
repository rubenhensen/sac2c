

#ifndef _EXPAND_SHMEM_BOUNDARY_LOAD_H_
#define _EXPAND_SHMEM_BOUNDARY_LOAD_H_

#include "types.h"

extern node *ESBLdoExpandShmemBoundaryLoad (node *syntax_tree);
extern node *ESBLfundef (node *arg_node, info *arg_info);
extern node *ESBLassign (node *arg_node, info *arg_info);
extern node *ESBLprf (node *arg_node, info *arg_info);

#endif
