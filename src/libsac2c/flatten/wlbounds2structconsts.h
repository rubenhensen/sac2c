#ifndef _SAC_BOUNDS2STRUCTCONSTS_H_
#define _SAC_BOUNDS2STRUCTCONSTS_H_

#include "types.h"

/******************************************************************************
 *
 * WLbounds2structConsts traversal ( wlbsc_tab)
 *
 * Prefix: WLBSC
 *
 *****************************************************************************/
extern node *WLBSCdoWlbounds2structConsts (node *arg_node);
extern node *WLBSCdoWlbounds2nonFlatStructConsts (node *arg_node);

extern node *WLBSCmodule (node *arg_node, info *arg_info);
extern node *WLBSCfundef (node *arg_node, info *arg_info);
extern node *WLBSCassign (node *arg_node, info *arg_info);
extern node *WLBSCwith (node *arg_node, info *arg_info);
extern node *WLBSCpart (node *arg_node, info *arg_info);
extern node *WLBSCgenerator (node *arg_node, info *arg_info);
extern node *WLBSCgenarray (node *arg_node, info *arg_info);

#endif /* _SAC_BOUNDS2STRUCTCONSTS_H_ */
