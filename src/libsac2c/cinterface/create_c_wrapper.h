/* $Id$ */

#ifndef _SAC_CREATE_C_WRAPPER_H_
#define _SAC_CREATE_C_WRAPPER_H_

#include "types.h"

extern node *CCWdoCreateCWrapper (node *syntax_tree);

extern node *CCWfunbundle (node *arg_node, info *arg_info);
extern node *CCWfundef (node *arg_node, info *arg_info);
extern node *CCWarg (node *arg_node, info *arg_info);
extern node *CCWret (node *arg_node, info *arg_info);
extern node *CCWmodule (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_C_WRAPPER_H_ */
