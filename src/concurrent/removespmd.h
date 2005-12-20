#ifndef _SAC_REMOVESPMD_H_
#define _SAC_REMVOESPMD_H_

#include "types.h"

/*****************************************************************************
 *
 * $Id: schedule.h 14489 2005-12-15 18:27:20Z ktr $
 *
 * Remove Spmd blocks travesal ( rmspmd_tab)
 *
 * prefix: RMSPMD
 *
 *****************************************************************************/
extern node *RMSPMDdoRemoveSpmdBlocks (node *syntax_tree);

extern node *RMSPMDfundef (node *arg_node, info *arg_info);
extern node *RMSPMDassign (node *arg_node, info *arg_info);
extern node *RMSPMDspmd (node *arg_node, info *arg_info);

#endif /* _SAC_REMOVESPMD_H_ */
