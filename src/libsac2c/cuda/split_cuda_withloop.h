
#ifndef _SAC_SPLIT_CUDA_WITHLOOP_H_
#define _SAC_SPLIT_CUDA_WITHLOOP_H_

#include "types.h"

/******************************************************************************
 *
 * Handle with loops traversal ( hwlg_tab)
 *
 * Prefix: SCUWL
 *
 *****************************************************************************/
extern node *SCUWLdoSplitCudaWithloops (node *arg_node);
extern node *SCUWLfundef (node *arg_node, info *arg_info);
extern node *SCUWLassign (node *arg_node, info *arg_info);
extern node *SCUWLlet (node *arg_node, info *arg_info);
extern node *SCUWLwith (node *arg_node, info *arg_info);
extern node *SCUWLgenarray (node *arg_node, info *arg_info);
extern node *SCUWLmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_SPLIT_CUDA_WITHLOOP_H_ */
