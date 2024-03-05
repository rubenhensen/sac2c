#ifndef _SAC_DISSOLVE_STRUCTS_H_
#define _SAC_DISSOLVE_STRUCTS_H_

#include "types.h"

extern node *DSSdoDeStruct (node *syntax_tree);

extern node *DSSmodule (node *arg_node, info *arg_info);
extern node *DSSfundef (node *arg_node, info *arg_info);
extern node *DSSblock (node *arg_node, info *arg_info);
extern node *DSSvardec (node *arg_node, info *arg_info);
extern node *DSSassign (node *arg_node, info *arg_info);
extern node *DSSlet (node *arg_node, info *arg_info);
extern node *DSSreturn (node *arg_node, info *arg_info);
extern node *DSSexprs (node *arg_node, info *arg_info);
extern node *DSSwith (node *arg_node, info *arg_info);
extern node *DSScode (node *arg_node, info *arg_info);
extern node *DSSmodarray (node *arg_node, info *arg_info);
extern node *DSSgenarray (node *arg_node, info *arg_info);
extern node *DSSfold (node *arg_node, info *arg_info);
extern node *DSSarray (node *arg_node, info *arg_info);
extern node *DSSap (node *arg_node, info *arg_info);
extern node *DSSprf (node *arg_node, info *arg_info);
extern node *DSSarg (node *arg_node, info *arg_info);
extern node *DSSret (node *arg_node, info *arg_info);
extern node *DSStype (node *arg_node, info *arg_info);
extern node *DSSids (node *arg_node, info *arg_info);
extern node *DSSid (node *arg_node, info *arg_info);
extern node *DSSavis (node *arg_node, info *arg_info);

#endif /* _SAC_DISSOLVE_STRUCTS_H_ */
