

#ifndef _INFER_REUSABLE_ARRAYS_H_
#define _INFER_REUSABLE_ARRAYS_H_

#include "types.h"

extern node *IRAdoInterReusableArrays (node *syntax_tree);
extern node *IRAfundef (node *arg_node, info *arg_info);
extern node *IRAwith (node *arg_node, info *arg_info);
extern node *IRAassign (node *arg_node, info *arg_info);
extern node *IRApart (node *arg_node, info *arg_info);
extern node *IRAcode (node *arg_node, info *arg_info);
extern node *IRAprf (node *arg_node, info *arg_info);

#endif
