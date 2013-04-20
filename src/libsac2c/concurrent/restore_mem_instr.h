#ifndef _SAC_RESTORE_MEM_INSTR_H_
#define _SAC_RESTORE_MEM_INSTR_H_

#include "types.h"

extern node *MTRMImodule (node *arg_node, info *arg_info);
extern node *MTRMIfundef (node *arg_node, info *arg_info);
extern node *MTRMIblock (node *arg_node, info *arg_info);
extern node *MTRMIassign (node *arg_node, info *arg_info);
extern node *MTRMIwiths (node *arg_node, info *arg_info);
extern node *MTRMIwith (node *arg_node, info *arg_info);
extern node *MTRMIwith2 (node *arg_node, info *arg_info);
extern node *MTRMIwithid (node *arg_node, info *arg_info);
extern node *MTRMIid (node *arg_node, info *arg_info);

extern node *MTRMIdoRestoreMemoryInstrs (node *syntax_tree);

#endif /* _SAC_RESTORE_MEM_INSTR_H_ */
