/*
 * $Id$
 *
 */

#ifndef _SAC_LOOP_AND_COND_SCALARIZATION_OUT_H_
#define _SAC_LOOP_AND_COND_SCALARIZATION_OUT_H_

#include "types.h"

extern node *LACSOdoLoopScalarization (node *arg_node);

extern node *LACSOmodule (node *arg_node, info *arg_info);
extern node *LACSOfundef (node *arg_node, info *arg_info);
extern node *LACSOlet (node *arg_node, info *arg_info);
extern node *LACSOassign (node *arg_node, info *arg_info);
extern node *LACSOap (node *arg_node, info *arg_info);
extern node *LACSOprf (node *arg_node, info *arg_info);
extern node *LACSOid (node *arg_node, info *arg_info);
extern node *LACSOreturn (node *arg_node, info *arg_info);
extern node *LACSOcond (node *arg_node, info *arg_info);
extern node *LACSOfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_LOOP_AND_COND_SCALARIZATION_OUT_H_ */
