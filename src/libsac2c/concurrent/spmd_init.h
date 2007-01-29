/*****************************************************************************
 *
 * $Id$
 *
 * SPMD Init traversal (spmdinit_tab)
 *
 * prefix: SPMDI
 *
 *****************************************************************************/

#ifndef _SAC_SPMD_INIT_H_
#define _SAC_SPMD_INIT_H_

#include "types.h"

extern node *SPMDIdoSpmdInit (node *syntax_tree);

extern node *SPMDImodule (node *arg_node, info *arg_info);
extern node *SPMDIfundef (node *arg_node, info *arg_info);
extern node *SPMDIassign (node *arg_node, info *arg_info);
extern node *SPMDIlet (node *arg_node, info *arg_info);
extern node *SPMDIids (node *arg_node, info *arg_info);
extern node *SPMDIid (node *arg_node, info *arg_info);
extern node *SPMDIwith2 (node *arg_node, info *arg_info);
extern node *SPMDIfold (node *arg_node, info *arg_info);
extern node *SPMDIgenarray (node *arg_node, info *arg_info);
extern node *SPMDImodarray (node *arg_node, info *arg_info);

#endif /* _SAC_SPMD_INIT_H_ */
