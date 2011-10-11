

#ifndef _SAC_PREPARE_KERNEL_GENERATION_H_
#define _SAC_PREPARE_KERNEL_GENERATION_H_

#include "types.h"

extern node *PKNLGdoPrepareKernelGeneration (node *arg_node);
extern node *PKNLGfundef (node *arg_node, info *arg_info);
extern node *PKNLGwith (node *arg_node, info *arg_info);
extern node *PKNLGwith2 (node *arg_node, info *arg_info);
extern node *PKNLGgenarray (node *arg_node, info *arg_info);
extern node *PKNLGassign (node *arg_node, info *arg_info);
extern node *PKNLGlet (node *arg_node, info *arg_info);
extern node *PKNLGprf (node *arg_node, info *arg_info);
extern node *PKNLGcond (node *arg_node, info *arg_info);

#endif
