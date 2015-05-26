#ifndef _SAC_IDENTIFY_SIDE_EFFECT_FUNCTIONS_H_
#define _SAC_IDENTIFY_SIDE_EFFECT_FUNCTIONS_H_

#include "types.h"

extern node *DMISEFfundef (node *arg_node, info *arg_info);
extern node *DMISEFarg (node *arg_node, info *arg_info);
extern node *DMISEFap (node *arg_node, info *arg_info);
extern node *DMISEFexprs (node *arg_node, info *arg_info);

extern node *DMISEFdoIdentifySideEffectFunctions (node *syntax_tree);

#endif /* _SAC_IDENTIFY_SIDE_EFFECT_FUNCTIONS_H_ */
