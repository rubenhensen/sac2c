/*****************************************************************************
 *
 * $Id$
 *
 * Remove Spmd blocks traversal ( rmspmd_tab)
 *
 * prefix: RMSPMD
 *
 *****************************************************************************/

#ifndef _SAC_REMOVESPMD_H_
#define _SAC_REMVOESPMD_H_

#include "types.h"

extern node *RMSPMDdoRemoveSpmdBlocks (node *syntax_tree);

extern node *RMSPMDmodule (node *arg_node, info *arg_info);
extern node *RMSPMDfundef (node *arg_node, info *arg_info);
extern node *RMSPMDassign (node *arg_node, info *arg_info);
extern node *RMSPMDspmd (node *arg_node, info *arg_info);

#endif /* _SAC_REMOVESPMD_H_ */
