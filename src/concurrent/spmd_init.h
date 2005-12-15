#ifndef _SAC_SPMD_INIT_H_
#define _SAC_SPMD_INIT_H_

#include "types.h"

/*****************************************************************************
 *
 * $Id$
 *
 * SPMD Init traversal (spmdinit_tab)
 *
 * prefix: SPMDI
 *
 *****************************************************************************/
extern node *SPMDIdoSpmdInit (node *syntax_tree);

extern node *SPMDIfundef (node *arg_node, info *arg_info);
extern node *SPMDIassign (node *arg_node, info *arg_info);

#endif /* _SAC_SPMD_INIT_H_ */
