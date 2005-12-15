#ifndef _SAC_SPMD_LIFT_H_
#define _SAC_SPMD_LIFT_H_

#include "types.h"

/*****************************************************************************
 *
 * $Id$
 *
 * SPMD lift ( spmdlift_tab)
 *
 * prefix: SPMDL
 *
 *****************************************************************************/
extern node *SPMDLdoSpmdLift (node *syntax_tree);

extern node *SPMDLfundef (node *arg_node, info *arg_info);
extern node *SPMDLspmd (node *arg_node, info *arg_info);
extern node *SPMDLwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_SPMD_LIFT_H_ */
