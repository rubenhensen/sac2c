#ifndef _SAC_REUSEBRANCHING_H_
#define _SAC_REUSEBRANCHING_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse branching traversal ( emrb_tab)
 *
 * Prefix: EMRB
 *
 *****************************************************************************/
extern node *EMRBdoReuseBranching (node *syntax_tree);

extern node *EMRBassign (node *arg_node, info *arg_info);
extern node *EMRBcode (node *arg_node, info *arg_info);
extern node *EMRBfundef (node *arg_node, info *arg_info);
extern node *EMRBgenarray (node *arg_node, info *arg_info);
extern node *EMRBids (node *arg_node, info *arg_info);
extern node *EMRBmodarray (node *arg_node, info *arg_info);
extern node *EMRBprf (node *arg_node, info *arg_info);
extern node *EMRBwith (node *arg_node, info *arg_info);
extern node *EMRBwith2 (node *arg_node, info *arg_info);
extern node *EMRBwithid (node *arg_node, info *arg_info);

#endif /* _SAC_REUSEBRANCHING_H_ */
