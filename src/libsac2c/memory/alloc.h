#ifndef _SAC_ALLOC_H_
#define _SAC_ALLOC_H_

#include "types.h"

/******************************************************************************
 *
 * Explicit allocation traversal ( emal_tab)
 *
 * prefix: EMAL
 *
 *****************************************************************************/
extern node *EMALdoAlloc (node *syntax_tree);

extern node *EMALbool (node *arg_node, info *arg_info);
extern node *EMALchar (node *arg_node, info *arg_info);
extern node *EMALdouble (node *arg_node, info *arg_info);
extern node *EMALfloat (node *arg_node, info *arg_info);
extern node *EMALnum (node *arg_node, info *arg_info);
extern node *EMALnumbyte (node *arg_node, info *arg_info);

extern node *EMALap (node *arg_node, info *arg_info);
extern node *EMALbreak (node *arg_node, info *arg_info);
extern node *EMALarray (node *arg_node, info *arg_info);
extern node *EMALassign (node *arg_node, info *arg_info);
extern node *EMALcode (node *arg_node, info *arg_info);
extern node *EMALfold (node *arg_node, info *arg_info);
extern node *EMALfuncond (node *arg_node, info *arg_info);
extern node *EMALfundef (node *arg_node, info *arg_info);
extern node *EMALgenarray (node *arg_node, info *arg_info);
extern node *EMALid (node *arg_node, info *arg_info);
extern node *EMALlet (node *arg_node, info *arg_info);
extern node *EMALmodarray (node *arg_node, info *arg_info);
extern node *EMALprf (node *arg_node, info *arg_info);
extern node *EMALpropagate (node *arg_node, info *arg_info);
extern node *EMALwith (node *arg_node, info *arg_info);
extern node *EMALwith2 (node *arg_node, info *arg_info);
extern node *EMALwithid (node *arg_node, info *arg_info);
extern node *EMALwith3 (node *arg_node, info *arg_info);
extern node *EMALrange (node *arg_node, info *arg_info);

#endif /* _SAC_ALLOC_H_ */
