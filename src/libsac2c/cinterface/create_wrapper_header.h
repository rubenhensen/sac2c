/* $Id$ */

#ifndef _SAC_CREATE_WRAPPER_HEADER_H_
#define _SAC_CREATE_WRAPPER_HEADER_H_

#include "types.h"

extern node *CWHdoCreateWrapperHeader (node *syntax_tree);

extern node *CWHfunbundle (node *arg_node, info *arg_info);
extern node *CWHfundef (node *arg_node, info *arg_info);
extern node *CWHarg (node *arg_node, info *arg_info);
extern node *CWHret (node *arg_node, info *arg_info);
extern node *CWHmodule (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_WRAPPER_HEADER_H_ */
