/*****************************************************************************
 *
 * file:   cell_growth.h
 *
 * description:
 *   header file for cell_growth.c
 *
 *****************************************************************************/

#ifndef _SAC_CELL_GROWTH_H_
#define _SAC_CELL_GROWTH_H_

#include "types.h"

extern node *CEGROdoCellGrowth (node *arg_node);

extern node *CEGROblock (node *arg_node, info *arg_info);

extern node *CEGROassign (node *arg_node, info *arg_info);

#endif /* _SAC_CELL_GROWTH_H_ */
