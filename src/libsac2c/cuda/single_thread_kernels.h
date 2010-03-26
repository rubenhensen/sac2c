/*****************************************************************************
 *
 * file:   single_thread_kernels.h
 *
 * description:
 *   header file for single_thread_kernels.c
 *
 *****************************************************************************/

#ifndef _SINGLE_THREAD_KERNELS_H_
#define _SINGLE_THREAD_KERNELS_H_

#include "types.h"

extern node *STKNLdoSingleThreadKernels (node *syntax_tree);

extern node *STKNLmodule (node *arg_node, info *arg_info);
extern node *STKNLfundef (node *arg_node, info *arg_info);
extern node *STKNLassign (node *arg_node, info *arg_info);
extern node *STKNLcudast (node *arg_node, info *arg_info);

#endif /* _CUDA_CREATE_CELLS_H_ */
