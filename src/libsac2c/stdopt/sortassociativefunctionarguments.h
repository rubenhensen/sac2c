
#ifndef _SAC_SORTASSOCIATIVEFUNCTIONARGUMENTS_H_
#define _SAC_SORTASSOCIATIVEFUNCTIONARGUMENTS_H_

#include "types.h"

/******************************************************************************
 *
 * Sort Associative Function Arguments
 *
 * Prefix: SAFA
 *
 *****************************************************************************/
extern node *SAFAdoSortAssociativeFunctionArguments (node *arg_node);

extern node *SAFAblock (node *arg_node, info *arg_info);
extern node *SAFAassign (node *arg_node, info *arg_info);
extern node *SAFAlet (node *arg_node, info *arg_info);
extern node *SAFAprf (node *arg_node, info *arg_info);
extern node *SAFAfundef (node *arg_node, info *arg_info);
extern node *SAFAmodule (node *arg_node, info *arg_info);

#endif /* _SAC_SORTASSOCIATIVEFUNCTIONARGUMENTS_H_ */
