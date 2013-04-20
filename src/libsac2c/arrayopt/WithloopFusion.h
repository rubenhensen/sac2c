#ifndef _SAC_WITHLOOPFUSION_H_
#define _SAC_WITHLOOPFUSION_H_

#include "types.h"

/******************************************************************************
 *
 * With-Loop-Fusion traversal ( wlfs_tab)
 *
 * Prefix: WLFS
 *
 *****************************************************************************/
extern node *WLFSdoWithloopFusion (node *arg_node);

extern node *WLFSfundef (node *arg_node, info *arg_info);
extern node *WLFSblock (node *arg_node, info *arg_info);
extern node *WLFSassign (node *arg_node, info *arg_info);
extern node *WLFSid (node *arg_node, info *arg_info);

extern node *WLFSwith (node *arg_node, info *arg_info);
extern node *WLFSgenarray (node *arg_node, info *arg_info);
extern node *WLFSmodarray (node *arg_node, info *arg_info);
extern node *WLFSfold (node *arg_node, info *arg_info);
extern node *WLFSpart (node *arg_node, info *arg_info);
extern node *WLFSgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_WITHLOOPFUSION_H_ */
