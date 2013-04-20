#ifndef _SAC_EXTEND_WRAPPER_TYPES_H_
#define _SAC_EXTEND_WRAPPER_TYPES_H_

#include "types.h"

extern node *EWTfundef (node *arg_node, info *arg_info);

extern node *EWTdoExtendWrapperTypes (node *syntax_tree);
extern node *EWTdoExtendWrapperTypesAfterTC (node *syntax_tree);

#endif /* _SAC_EXTEND_WRAPPER_TYPES_H_ */
