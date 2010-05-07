

#ifndef _SAC_PREPARE_KERNEL_GENERATION_H_
#define _SAC_PREPARE_KERNEL_GENERATION_H_

#include "types.h"

extern node *PKNLGdoPrepareKernelGeneration (node *arg_node);
extern node *PKNLGwith (node *arg_node, info *arg_info);
extern node *PKNLGassign (node *arg_node, info *arg_info);
extern node *PKNLGprf (node *arg_node, info *arg_info);

#endif
