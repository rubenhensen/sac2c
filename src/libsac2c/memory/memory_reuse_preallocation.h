#ifndef _SAC_MEMREUSEPRE_H_
#define _SAC_MEMREUSEPRE_H_

#include "types.h"

/******************************************************************************
 *
 * memory reuse preallocation for EMR candidates
 *
 *****************************************************************************/
extern node *MRPdoPreallocation (node *syntax_tree);

extern node *MRPfundef (node *syntax_tree, info *arg_info);
extern node *MRPap (node *arg_node, info *arg_info);
extern node *MRPwith (node *arg_node, info *arg_info);
extern node *MRPgenarray (node *arg_node, info *arg_info);
extern node *MRPmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_MEMREUSEPRE_H_ */
