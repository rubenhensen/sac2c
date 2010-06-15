

#ifndef _SHARED_MEMORY_REUSE_H_
#define _SHARED_MEMORY_REUSE_H_

#include "types.h"

extern node *SHMEMdoSharedMemoryReuse (node *syntax_tree);
extern node *SHMEMfundef (node *arg_node, info *arg_info);
extern node *SHMEMassign (node *arg_node, info *arg_info);
extern node *SHMEMwith (node *arg_node, info *arg_info);
extern node *SHMEMpart (node *arg_node, info *arg_info);
extern node *SHMEMcode (node *arg_node, info *arg_info);
extern node *SHMEMprf (node *arg_node, info *arg_info);

#endif
