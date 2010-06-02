

#ifndef _REMOVE_UNUSED_LAC_H_
#define _REMOVE_UNUSED_LAC_H_

#include "types.h"

extern node *RLACdoRemoveUnusedLac (node *syntax_tree);
extern node *RLACmodule (node *arg_node, info *arg_info);
extern node *RLACfundef (node *arg_node, info *arg_info);
extern node *RLACap (node *arg_node, info *arg_info);

#endif
