/*****************************************************************************
 *
 * file:   tag_executionmode.h
 *
 * description:
 *   header file for tag_executionmode.c
 *
 *****************************************************************************/

#ifndef _CUDA_TAG_EXECUTIONMODE_H_
#define _CUDA_TAG_EXECUTIONMODE_H_

#include "types.h"

extern node *CUTEMdoTagExecutionmode (node *arg_node);

extern node *CUTEMassign (node *arg_node, info *arg_info);
extern node *CUTEMwith (node *arg_node, info *arg_info);
extern node *CUTEMap (node *arg_node, info *arg_info);
extern node *CUTEMid (node *arg_node, info *arg_info);
extern node *CUTEMlet (node *arg_node, info *arg_info);
extern node *CUTEMfundef (node *arg_node, info *arg_info);
extern node *CUTEMexprs (node *arg_node, info *arg_info);

#endif /* _CUDA_TAG_EXECUTIONMODE_H_ */
