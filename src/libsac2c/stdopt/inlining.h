#ifndef _SAC_INLINING_H_
#define _SAC_INLINING_H_

#include "types.h"

extern node *INLdoInlining (node *arg_node);

extern node *INLmodule (node *arg_node, info *arg_info);
extern node *INLfundef (node *arg_node, info *arg_info);
extern node *INLassign (node *arg_node, info *arg_info);
extern node *INLlet (node *arg_node, info *arg_info);
extern node *INLap (node *arg_node, info *arg_info);
extern node *INLcode (node *arg_node, info *arg_info);

#endif /* _SAC_INLINING_H_ */
