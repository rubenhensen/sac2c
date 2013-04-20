/*****************************************************************************
 *
 * file:   create_cells.h
 *
 * description:
 *   header file for create_cells.c
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_CELLS_H_
#define _SAC_CREATE_CELLS_H_

#include "types.h"

extern node *CRECEdoCreateCells (node *arg_node);

extern node *CRECEblock (node *arg_node, info *arg_info);

extern node *CRECEassign (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_CELLS_H_ */
