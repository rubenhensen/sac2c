/*
 *
 * $Id$
 *
 */

#ifndef _SAC_HANDLE_HANDLE_WITH_LOOP_OPERATORS_H_
#define _SAC_HANDLE_HANDLE_WITH_LOOP_OPERATORS_H_

#include "types.h"

/******************************************************************************
 *
 * Handle with loops traversal ( hwl_tab)
 *
 * Prefix: HWLO
 *
 *****************************************************************************/
extern node *HWLOdoHandleWithLoops (node *arg_node);

extern node *HWLOassign (node *arg_node, info *arg_info);
extern node *HWLOlet (node *arg_node, info *arg_info);
extern node *HWLOwith (node *arg_node, info *arg_info);
extern node *HWLOgenarray (node *arg_node, info *arg_info);
extern node *HWLOmodarray (node *arg_node, info *arg_info);
extern node *HWLOspfold (node *arg_node, info *arg_info);
extern node *HWLOpropagate (node *arg_node, info *arg_info);

#endif /* _SAC_HANDLE_HANDLE_WITH_LOOP_OPERATORS_H_ */
