#ifndef _SAC_IDENTIFY_SUBALLOCATED_ARRAYS_H_
#define _SAC_IDENTIFY_SUBALLOCATED_ARRAYS_H_

#include "types.h"

extern node *DMISAAlet (node *arg_node, info *arg_info);
extern node *DMISAAids (node *arg_node, info *arg_info);
extern node *DMISAAprf (node *arg_node, info *arg_info);

extern node *DMISAAdoIdentifySubAllocatedArrays (node *syntax_tree);

#endif /* _SAC_IDENTIFY_SUBALLOCATED_ARRAYS_H_ */
