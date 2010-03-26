

#ifndef _SAC_ANNOTATE_CUDA_LACFUN_H_
#define _SAC_ANNOTATE_CUDA_LACFUN_H_

#include "types.h"

extern node *ACULACdoAnnotateCUDALacfun (node *arg_node);
extern node *ACULACfundef (node *arg_node, info *arg_info);
extern node *ACULACassign (node *arg_node, info *arg_info);
extern node *ACULACap (node *arg_node, info *arg_info);
extern node *ACULACwith (node *arg_node, info *arg_info);
extern node *ACULACwith2 (node *arg_node, info *arg_info);

#endif
