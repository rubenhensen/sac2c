/* $Id$ */

#ifndef _SAC_RESOLVE_OBJECTS_H_
#define _SAC_RESOLVE_OBJECTS_H_

#include "types.h"

extern node *RSOmodule (node *arg_node, info *arg_info);
extern node *RSOfundef (node *arg_node, info *arg_info);
extern node *RSOglobobj (node *arg_node, info *arg_info);
extern node *RSOap (node *arg_node, info *arg_info);

extern node *RSOdoResolveObjects (node *syntax_tree);

#endif /* _SAC_RESOLVE_OBJECTS_H_ */
