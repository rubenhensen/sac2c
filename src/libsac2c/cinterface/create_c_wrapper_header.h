#ifndef _SAC_CREATE_C_WRAPPER_HEADER_H_
#define _SAC_CREATE_C_WRAPPER_HEADER_H_

#include "types.h"

extern node *CCWHdoCreateCWrapper (node *syntax_tree);

extern node *CCWHfunbundle (node *arg_node, info *arg_info);
extern node *CCWHfundef (node *arg_node, info *arg_info);
extern node *CCWHarg (node *arg_node, info *arg_info);
extern node *CCWHret (node *arg_node, info *arg_info);
extern node *CCWHtypedef (node *arg_node, info *arg_info);
extern node *CCWHmodule (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_C_WRAPPER_HEADER_H_ */
