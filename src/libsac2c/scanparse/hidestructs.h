#ifndef _SAC_HIDESTRUCTS_H_
#define _SAC_HIDESTRUCTS_H_

#include "types.h"

#define STRUCT_TYPE "_struct_"
#define STRUCT_GET (STRUCT_TYPE "get_")
#define STRUCT_SET (STRUCT_TYPE "set_")

extern node *HSdoDestruct (node *syntax_tree);

extern node *HSmodule (node *arg_node, info *arg_info);
extern node *HSstructelem (node *arg_node, info *arg_info);
extern node *HSstructdef (node *arg_node, info *arg_info);

#endif /* _SAC_HIDESTRUCTS_H_ */
