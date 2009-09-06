/*****************************************************************************
 *
 *
 * Flatten elements of lower and upper bound of generators.
 *
 * prefix: CNSTASS
 *
 *****************************************************************************/

#ifndef _SAC_CREATE_CONSTANT_ASSIGNMENTS_H_
#define _SAC_CREATE_CONSTANT_ASSIGNMENTS_H_

#include "types.h"

extern node *CNSTASSdoCUDAconstantAssignment (node *syntax_tree);
extern node *CNSTASSfundef (node *arg_node, info *arg_info);
extern node *CNSTASSassign (node *arg_node, info *arg_info);
extern node *CNSTASSwith (node *arg_node, info *arg_info);
extern node *CNSTASSgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_CREATE_SPMD_FUNS_H_ */
