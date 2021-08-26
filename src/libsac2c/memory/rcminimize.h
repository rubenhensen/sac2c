#ifndef _SAC_RCMINIMIZE_H_
#define _SAC_RCMINIMIZE_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Refcount minimization traversal ( rcm_tab)
 *
 * prefix: RCM
 *
 * nodes which must NOT be traversed:
 *   - N_objdef
 *   - N_withid
 *
 *****************************************************************************/
extern node *RCMdoRefcountMinimization (node *syntax_tree);

extern node *RCMap (node *arg_node, info *arg_info);
extern node *RCMarg (node *arg_node, info *arg_info);
extern node *RCMassign (node *arg_node, info *arg_info);
extern node *RCMcode (node *arg_node, info *arg_info);
extern node *RCMfold (node *arg_node, info *arg_info);
extern node *RCMrange (node *arg_node, info *arg_info);
extern node *RCMcond (node *arg_node, info *arg_info);
extern node *RCMfuncond (node *arg_node, info *arg_info);
extern node *RCMfundef (node *arg_node, info *arg_info);
extern node *RCMid (node *arg_node, info *arg_info);
extern node *RCMids (node *arg_node, info *arg_info);
extern node *RCMlet (node *arg_node, info *arg_info);
extern node *RCMprf (node *arg_node, info *arg_info);
extern node *RCMreturn (node *arg_node, info *arg_info);

#endif /* _SAC_RCMINIMIZE_H_ */
