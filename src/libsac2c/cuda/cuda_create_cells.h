/*****************************************************************************
 *
 * file:   cuda_create_cells.h
 *
 * description:
 *   header file for cuda_create_cells.c
 *
 *****************************************************************************/

#ifndef _CUDA_CREATE_CELLS_H_
#define _CUDA_CREATE_CELLS_H_

#include "types.h"

extern node *CUCCdoCreateCells (node *arg_node);
extern node *CUCCassign (node *arg_node, info *arg_info);
extern node *CUCCfundef (node *arg_node, info *arg_info);

#endif /* _CUDA_CREATE_CELLS_H_ */
