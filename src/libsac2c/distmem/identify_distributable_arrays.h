#ifndef _SAC_IDENTIFY_DISTRIBUTABLE_ARRAYS_H_
#define _SAC_IDENTIFY_DISTRIBUTABLE_ARRAYS_H_

#include "types.h"

extern node *DMIDAlet (node *arg_node, info *arg_info);
extern node *DMIDAids (node *arg_node, info *arg_info);
extern node *DMIDAfundef (node *arg_node, info *arg_info);
extern node *DMIDAarg (node *arg_node, info *arg_info);

extern node *DMIDAdoIdentifyDistributableArrays (node *syntax_tree);

#endif /* _SAC_IDENTIFY_DISTRIBUTABLE_ARRAYS_H_ */
