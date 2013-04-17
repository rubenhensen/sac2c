/*****************************************************************************
 *
 * file:   propagate_executionmode.h
 *
 * description:
 *   header file for propagate_executionmode.c
 *
 *****************************************************************************/

#ifndef _SAC_PROPAGATE_EXECUTIONMODE_H_
#define _SAC_PROPAGATE_EXECUTIONMODE_H_

#include "types.h"

extern node *PEMdoPropagateExecutionmode (node *arg_node);

extern node *PEMfundef (node *arg_node, info *arg_info);

extern node *PEMassign (node *arg_node, info *arg_info);

extern node *PEMap (node *arg_node, info *arg_info);

extern node *PEMcond (node *arg_node, info *arg_info);

extern node *PEMwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_PROPAGATE_EXECUTIONMODE_H_ */
