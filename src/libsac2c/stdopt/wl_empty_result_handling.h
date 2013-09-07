#ifndef _SAC_WL_EMPTY_RESULT_HANDLING_H_
#define _SAC_WL_EMPTY_RESULT_HANDLING_H_
/*
 *
 * $Id$
 *
 */

#include "types.h"

/******************************************************************************
 *
 * With-loop Empty Result Handling traversal ( wlerh_tab)
 *
 * Prefix: WLERH
 *
 *****************************************************************************/
extern node *WLERHdoWithLoopEmptyResultHandling (node *syntax_tree);

extern node *WLERHfundef (node *arg_node, info *arg_info);
extern node *WLERHassign (node *arg_node, info *arg_info);
extern node *WLERHlet (node *arg_node, info *arg_info);
extern node *WLERHwith (node *arg_node, info *arg_info);
extern node *WLERHgenarray (node *arg_node, info *arg_info);
extern node *WLERHmodarray (node *arg_node, info *arg_info);
extern node *WLERHfold (node *arg_node, info *arg_info);
extern node *WLERHbreak (node *arg_node, info *arg_info);
extern node *WLERHpropagate (node *arg_node, info *arg_info);
extern node *WLERHcode (node *arg_node, info *arg_info);
extern node *WLERHpart (node *arg_node, info *arg_info);
extern node *WLERHgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_WL_EMPTY_RESULT_HANDLING_H_ */
