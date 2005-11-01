/*
 *
 * $Id$
 *
 *
 */

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
extern node *WLPGdoWlPartitionGenerationOpt (node *arg_node);

extern node *WLPGmodule (node *arg_node, info *arg_info);
extern node *WLPGfundef (node *arg_node, info *arg_info);
extern node *WLPGassign (node *arg_node, info *arg_info);
extern node *WLPGlet (node *arg_node, info *arg_info);
extern node *WLPGap (node *arg_node, info *arg_info);

extern node *WLPGwith (node *arg_node, info *arg_info);

/* functions which are also used by wlanalysis.c */
extern int WLPGnormalizeStepWidth (node **step, node **width);

#endif /* _SAC_WLPARTIONGENERATION_H_ */
