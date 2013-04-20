#ifndef _SAC_SSAWLUNROLL_H_
#define _SAC_SSAWLUNROLL_H_

#include "types.h"

/*****************************************************************************
 *
 * SSAWLUnroll
 *
 * prefix: WLU
 *
 *****************************************************************************/

extern node *WLURap (node *arg_node, info *arg_info);
extern node *WLURassign (node *arg_node, info *arg_info);
extern node *WLURfundef (node *arg_node, info *arg_info);
extern node *WLURwith (node *arg_node, info *arg_info);

extern node *WLURdoWithloopUnroll (node *syntax_tree);

#endif /* _SAC_SSAWLUNROLL_H_ */
