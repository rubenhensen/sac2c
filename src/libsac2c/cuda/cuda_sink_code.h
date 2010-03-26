/*****************************************************************************
 *
 * file:   cuda_sink_code.h
 *
 * description:
 *   header file for cuda_sink_code.c
 *
 *****************************************************************************/

#ifndef _CUDA_SINK_CODE_H_
#define _CUDA_SINK_CODE_H_

#include "types.h"

extern node *CUSKCdoSinkCode (node *arg_node);

extern node *CUSKCassign (node *arg_node, info *arg_info);
extern node *CUSKCids (node *arg_node, info *arg_info);
extern node *CUSKCid (node *arg_node, info *arg_info);
extern node *CUSKCwith (node *arg_node, info *arg_info);
extern node *CUSKClet (node *arg_node, info *arg_info);
extern node *CUSKCfundef (node *arg_node, info *arg_info);
extern node *CUSKCblock (node *arg_node, info *arg_info);

#endif /* _CUDA_TAG_EXECUTIONMODE_H_ */
