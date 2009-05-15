

#ifndef _SAC_CUDA_TYPE_CONVERSION_H_
#define _SAC_CUDA_TYPE_CONVERSION_H_

#include "types.h"

extern node *CUTYCVdoCUDAtypeConversion (node *arg_node);
extern node *CUTYCVwith2 (node *arg_node, info *arg_info);
extern node *CUTYCVfundef (node *arg_node, info *arg_info);
extern node *CUTYCVid (node *arg_node, info *arg_info);
extern node *CUTYCVlet (node *arg_node, info *arg_info);
extern node *CUTYCVassign (node *arg_node, info *arg_info);
extern node *CUTYCVwith (node *arg_node, info *arg_info);

#endif
