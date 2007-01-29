/*
 *
 * $Id:$
 *
 */

#ifndef _SAC_HANDLE_ZERO_GENERATOR_WITH_LOOPS__H_
#define _SAC_HANDLE_ZERO_GENERATOR_WITH_LOOPS__H_

#include "types.h"

/******************************************************************************
 *
 * Handle with loops traversal ( hzgwl_tab)
 *
 * Prefix: HZGWL
 *
 *****************************************************************************/
extern node *HWLGdoHandleZeroGeneratorWithLoops (node *arg_node);

extern node *HZGWLwith (node *arg_node, info *arg_info);
extern node *HZGWLmodarray (node *arg_node, info *arg_info);
extern node *HZGWLgenarray (node *arg_node, info *arg_info);
extern node *HZGWLspfold (node *arg_node, info *arg_info);
extern node *HZGWLpropagate (node *arg_node, info *arg_info);
extern node *HZGWLlet (node *arg_node, info *arg_info);
extern node *HZGWLassign (node *arg_node, info *arg_info);
extern node *HZGWLreturn (node *arg_node, info *arg_info);

#endif /* _SAC_HANDLE_ZERO_GENERATOR_WITH_LOOPS__H_ */
