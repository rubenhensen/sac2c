/*
 * $Id$
 *
 */

#ifndef _SAC_LOOP_SCALARIZATION_H_
#define _SAC_LOOP_SCALARIZATION_H_

#include "types.h"

extern node *LSdoLoopScalarization (node *arg_node);
extern node *LSdoLoopScalarizationOneFundef (node *arg_node);

extern node *LSfundef (node *arg_node, info *arg_info);
extern node *LSarg (node *arg_node, info *arg_info);
extern node *LSassign (node *arg_node, info *arg_info);
extern node *LSap (node *arg_node, info *arg_info);
extern node *LSprf (node *arg_node, info *arg_info);
extern node *LSid (node *arg_node, info *arg_info);

#endif /* _SAC_LOOP_SCALARIZATION_H_ */
