#ifndef _SAC_LIFT_WITH3_BODIES_H_
#define _SAC_LIFT_WITH3_BODIES_H_

#include "types.h"

/******************************************************************************
 *
 * lw3 traversal ( lw3)
 *
 * Prefix: LW3
 *
 *****************************************************************************/
extern node *LW3doLiftWith3 (node *syntaxtree);

extern node *LW3module (node *arg_node, info *arg_info);
extern node *LW3range (node *arg_node, info *arg_info);
extern node *LW3fundef (node *arg_node, info *arg_info);
extern node *LW3with3 (node *arg_node, info *arg_info);

#endif /* _SAC_LIFT_WITH3_BODIES_H_ */
