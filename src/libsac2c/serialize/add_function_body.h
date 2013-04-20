#ifndef _SAC_ADD_FUNCTION_BODY_H_
#define _SAC_ADD_FUNCTION_BODY_H_

#include "types.h"

extern node *AFBdoAddFunctionBody (node *fundef);

extern node *AFBfundef (node *arg_node, info *arg_info);
extern node *AFBreturn (node *arg_node, info *arg_info);
extern node *AFBap (node *arg_node, info *arg_info);
extern node *AFBblock (node *arg_node, info *arg_info);
extern node *AFBarg (node *arg_node, info *arg_info);

#endif
