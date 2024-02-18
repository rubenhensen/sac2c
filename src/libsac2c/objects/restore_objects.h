#ifndef _SAC_RESTORE_OBJECTS_H_
#define _SAC_RESTORE_OBJECTS_H_

#include "types.h"

extern node *RESOid (node *arg_node, info *arg_info);
extern node *RESOap (node *arg_node, info *arg_info);
extern node *RESOlet (node *arg_node, info *arg_info);
extern node *RESOassign (node *arg_node, info *arg_info);
extern node *RESOblock (node *arg_node, info *arg_info);
extern node *RESOfundef (node *arg_node, info *arg_info);
extern node *RESOmodule (node *arg_node, info *arg_info);
extern node *RESOpropagate (node *arg_node, info *arg_info);

extern node *RESOdoRestoreObjects (node *syntax_tree);

#endif /* _SAC_RESTORE_OBJECTS_H_ */
