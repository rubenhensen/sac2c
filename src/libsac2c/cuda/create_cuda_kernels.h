/*****************************************************************************
 *
 *
 * Creating CUDA kernel functions
 *
 * prefix: CUKNL
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_CUDA_KERNELS_H_
#define _SAC_CREATE_CUDA_KERNELS_H_

#include "types.h"

extern node *CUKNLdoCreateCudaKernels (node *syntax_tree);
extern node *CUKNLfundef (node *arg_node, info *arg_info);
extern node *CUKNLassign (node *arg_node, info *arg_info);
extern node *CUKNLdo (node *arg_node, info *arg_info);
extern node *CUKNLwith (node *arg_node, info *arg_info);
extern node *CUKNLwith2 (node *arg_node, info *arg_info);
extern node *CUKNLlet (node *arg_node, info *arg_info);
extern node *CUKNLid (node *arg_node, info *arg_info);
extern node *CUKNLids (node *arg_node, info *arg_info);
extern node *CUKNLwithid (node *arg_node, info *arg_info);
extern node *CUKNLgenerator (node *arg_node, info *arg_info);
extern node *CUKNLpart (node *arg_node, info *arg_info);
extern node *CUKNLcode (node *arg_node, info *arg_info);
extern node *CUKNLgenarray (node *arg_node, info *arg_info);
extern node *CUKNLmodarray (node *arg_node, info *arg_info);

extern node *CUKNLprf (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_SPMD_FUNS_H_ */
