/*****************************************************************************
 *
 * file:   tag_executionmode.h
 *
 * description:
 *   header file for tag_executionmode.c
 *
 *****************************************************************************/

#ifndef _TAG_CUDA_LAC_H_
#define _TAG_CUDA_LAC_H_

#include "types.h"

extern node *TCULACdoTagCudaLac (node *ap, node *ids, node *fundef_args);

extern node *TCULACfundef (node *arg_node, info *arg_info);
extern node *TCULACwith (node *arg_node, info *arg_info);
extern node *TCULACap (node *arg_node, info *arg_info);

#endif /* _CUDA_TAG_EXECUTIONMODE_H_ */
