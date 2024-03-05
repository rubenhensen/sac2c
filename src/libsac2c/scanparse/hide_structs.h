#ifndef _SAC_HIDE_STRUCTS_H_
#define _SAC_HIDE_STRUCTS_H_

#include "types.h"

#define STRUCT_TYPE_PREFIX "_struct_"
#define STRUCT_CON_PREFIX "_struct_con_"
#define STRUCT_GETTER_PREFIX "_struct_get_"
#define STRUCT_SETTER_PREFIX "_struct_set_"

extern node *HSdoHideStructs (node *syntax_tree);

extern node *HSmodule (node *arg_node, info *arg_info);
extern node *HSstructdef (node *arg_node, info *arg_info);
extern node *HSstructelem (node *arg_node, info *arg_info);

#endif /* _SAC_HIDE_STRUCTS_H_ */
