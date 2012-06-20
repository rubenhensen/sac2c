/*
 * $Id$
 *
 */

#ifndef _SAC_LOOP_AND_COND_SCALARIZATION_H_
#define _SAC_LOOP_AND_COND_SCALARIZATION_H_

#include "types.h"

extern node *LACSdoLoopScalarization (node *arg_node);

extern node *LACSmodule (node *arg_node, info *arg_info);
extern node *LACSfundef (node *arg_node, info *arg_info);
extern node *LACSarg (node *arg_node, info *arg_info);
extern node *LACSassign (node *arg_node, info *arg_info);
extern node *LACSap (node *arg_node, info *arg_info);
extern node *LACSprf (node *arg_node, info *arg_info);
extern node *LACSid (node *arg_node, info *arg_info);

#endif /* _SAC_LOOP_AND_COND_SCALARIZATION_H_ */
