/*****************************************************************************
 *
 * $Id: create_spmd_funs.h 15844 2008-11-04 00:11:37Z cg $
 *
 * Creating SPMD functions
 *
 * prefix: MTSPMDF
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_CUDA_KERNELS_H_
#define _SAC_CREATE_CUDA_KERNELS_H_

#include "types.h"

extern node *CUKNLdoCreateCudaKernels (node *syntax_tree);

extern node *CUKNLmodule (node *arg_node, info *arg_info);
extern node *CUKNLfundef (node *arg_node, info *arg_info);
// extern node *CUKNLblock( node *arg_node, info *arg_info);
// extern node *CUKNLvardec( node *arg_node, info *arg_info);
extern node *CUKNLwith (node *arg_node, info *arg_info);
extern node *CUKNLlet (node *arg_node, info *arg_info);
extern node *CUKNLid (node *arg_node, info *arg_info);
extern node *CUKNLids (node *arg_node, info *arg_info);
extern node *CUKNLwithid (node *arg_node, info *arg_info);
extern node *CUKNLgenerator (node *arg_node, info *arg_info);
extern node *CUKNLpart (node *arg_node, info *arg_info);
extern node *CUKNLcode (node *arg_node, info *arg_info);
extern node *CUKNLgenarray (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_SPMD_FUNS_H_ */
