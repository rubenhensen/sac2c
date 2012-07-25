/*****************************************************************************
 *
 * $Id$
 *
 * Multithreading cost model traversal
 *
 * prefix: MTCM
 *
 *****************************************************************************/

#ifndef _SAC_COST_MODEL_H_
#define _SAC_COST_MODEL_H_

#include "types.h"

extern node *MTCMdoRunCostModel (node *syntax_tree);

extern node *MTCMmodule (node *arg_node, info *arg_info);
extern node *MTCMfundef (node *arg_node, info *arg_info);
extern node *MTCMblock (node *arg_node, info *arg_info);
extern node *MTCMassign (node *arg_node, info *arg_info);
extern node *MTCMlet (node *arg_node, info *arg_info);
extern node *MTCMwith2 (node *arg_node, info *arg_info);
extern node *MTCMwith (node *arg_node, info *arg_info);
extern node *MTCMwiths (node *arg_node, info *arg_info);
extern node *MTCMfold (node *arg_node, info *arg_info);
extern node *MTCMgenarray (node *arg_node, info *arg_info);
extern node *MTCMmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_COST_MODEL_H_ */
