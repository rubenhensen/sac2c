#ifndef _SAC_WLPARTIONGENERATION_H_
#define _SAC_WLPARTIONGENERATION_H_

#include "types.h"

/******************************************************************************
 *
 * WLPartition Generation traversal ( wlpg_tab)
 *
 * Prefix: WLPG
 *
 *****************************************************************************/
extern node *WLPGdoWlPartitionGeneration (node *arg_node);

extern node *WLPGmodule (node *arg_node, info *arg_info);
extern node *WLPGfundef (node *arg_node, info *arg_info);
extern node *WLPGassign (node *arg_node, info *arg_info);
extern node *WLPGwith (node *arg_node, info *arg_info);

#endif /* _SAC_WLPARTIONGENERATION_H_ */
