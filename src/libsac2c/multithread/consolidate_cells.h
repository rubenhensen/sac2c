/*****************************************************************************
 *
 * file:   consolidate_cells.h
 *
 * description:
 *   header file for consolidate_cells.c
 *
 *****************************************************************************/

#ifndef _SAC_CONSOLIDATE_CELLS_H_
#define _SAC_CONSOLIDATE_CELLS_H_

#include "types.h"

extern node *CONCELdoConsolidateCells (node *arg_node);

extern node *CONCELfundef (node *arg_node, info *arg_info);

extern node *CONCELex (node *arg_node, info *arg_info);

extern node *CONCELst (node *arg_node, info *arg_info);

extern node *CONCELmt (node *arg_node, info *arg_info);

#endif /* _SAC_CONSOLIDATE_CELLS_H_ */
