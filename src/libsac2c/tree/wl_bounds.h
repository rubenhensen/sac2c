#ifndef _SAC_WL_BOUNDS_H_
#define _SAC_WL_BOUNDS_H_

#include "types.h"

/******************************************************************************
 *
 *  Modul: wl_bounds
 *
 *  Prefix: WLB
 *
 *  Description:
 *
 *    The index bounds for a with-loop in internal representation (N_Nwith2)
 *    are stored in the AST as values of kind 'N_num' or 'N_id'.
 *
 *    This module provides functions for handling all the different
 *    representations of index bounds uniformly. Furthermore, it introduces
 *    a special value for the maximum of the iteration space IDX_SHAPE.
 *
 ******************************************************************************/
/*
 * symbolic bounds for strides/grids and IDX_MIN, IDX_MAX
 */
#define IDX_SHAPE (-1) /* equals the shape */
#define IDX_OTHER (-2) /* other */

extern bool WLBidOrNumEq (node *arg1, node *arg2);
extern bool WLBidOrNumLe (node *arg1, node *arg2, int shape);
extern node *WLBidOrNumMakeIndex (node *bound, int dim, node *wl_ids);

#endif /* _SAC_WL_BOUNDS_H_ */
