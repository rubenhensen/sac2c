#ifndef _SAC_MANAGE_OBJECT_INITIALISERS_H_
#define _SAC_MANAGE_OBJECT_INITIALISERS_H_

#include "types.h"

extern node *MOIfundef (node *arg_node, info *arg_info);
extern node *MOIassign (node *arg_node, info *arg_info);
extern node *MOIid (node *arg_node, info *arg_info);
extern node *MOIprf (node *arg_node, info *arg_info);

extern node *MOIdoManageObjectInitialisers (node *syntax_tree);

#endif /* _SAC_MANAGE_OBJECT_INITIALISERS_H_ */
