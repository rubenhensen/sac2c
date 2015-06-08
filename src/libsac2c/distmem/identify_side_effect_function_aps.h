#ifndef _SAC_IDENTIFY_SIDE_EFFECT_FUNCTION_APS_H_
#define _SAC_IDENTIFY_SIDE_EFFECT_FUNCTION_APS_H_

#include "types.h"

extern node *DMISEFAfundef (node *arg_node, info *arg_info);
extern node *DMISEFAap (node *arg_node, info *arg_info);

extern node *DMISEFAdoIdentifySideEffectFunctionApplications (node *syntax_tree);

#endif /* _SAC_IDENTIFY_SIDE_EFFECT_FUNCTION_APS_H_ */
