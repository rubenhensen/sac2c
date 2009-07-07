#ifndef _SAC_DESTRUCT_H_
#define _SAC_DESTRUCT_H_

#include "types.h"

extern node *DESdoDeStruct (node *syntax_tree);

extern node *DESmodule (node *arg_node, info *arg_info);
extern node *DESfundef (node *arg_node, info *arg_info);
extern node *DESret (node *arg_node, info *arg_info);
extern node *DESarg (node *arg_node, info *arg_info);
extern node *DESexprs (node *arg_node, info *arg_info);

#endif /* _SAC_DESTRUCT_H_ */
