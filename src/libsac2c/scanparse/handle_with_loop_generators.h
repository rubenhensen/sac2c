#ifndef _SAC_HANDLE_HANDLE_WITH_LOOP_GENERATORS_H_
#define _SAC_HANDLE_HANDLE_WITH_LOOP_GENERATORS_H_

#include "types.h"

/******************************************************************************
 *
 * Handle with loops traversal ( hwlg_tab)
 *
 * Prefix: HWLG
 *
 *****************************************************************************/
extern node *HWLGdoHandleWithLoops (node *arg_node);

extern node *HWLGlet (node *arg_node, info *arg_info);
extern node *HWLGassign (node *arg_node, info *arg_info);
extern node *HWLGwith (node *arg_node, info *arg_info);
extern node *HWLGcode (node *arg_node, info *arg_info);
extern node *HWLGgenarray (node *arg_node, info *arg_info);
extern node *HWLGmodarray (node *arg_node, info *arg_info);
extern node *HWLGspfold (node *arg_node, info *arg_info);
extern node *HWLGpropagate (node *arg_node, info *arg_info);

#endif /* _SAC_HANDLE_HANDLE_WITH_LOOP_GENERATORS_H_ */
