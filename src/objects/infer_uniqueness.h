/* $Id$ */

#ifndef _INFER_UNIQUENESS_H_
#define _INFER_UNIQUENESS_H_

#include "types.h"

extern node *IUap (node *arg_node, info *arg_info);
extern node *IUassign (node *arg_node, info *arg_info);
extern node *IUfundef (node *arg_node, info *arg_info);
extern node *IUlet (node *arg_node, info *arg_info);

extern node *IUdoInferUniqueness (node *syntax_tree);

#endif
