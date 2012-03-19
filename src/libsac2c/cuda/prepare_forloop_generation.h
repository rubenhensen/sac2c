

#ifndef _SAC_PREPARE_FORLOOP_GENERATION_H_
#define _SAC_PREPARE_FORLOOP_GENERATION_H_

#include "types.h"

extern node *PFGdoPrepareForloopGeneration (node *arg_node);
extern node *PFGfundef (node *arg_node, info *arg_info);
extern node *PFGassign (node *arg_node, info *arg_info);
extern node *PFGlet (node *arg_node, info *arg_info);
extern node *PFGdo (node *arg_node, info *arg_info);
extern node *PFGid (node *arg_node, info *arg_info);
extern node *PFGprf (node *arg_node, info *arg_info);

#endif
