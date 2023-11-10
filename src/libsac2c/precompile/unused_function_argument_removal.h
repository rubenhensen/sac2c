#ifndef _SAC_UNUSED_FUNCTION_ARGUMENT_REMOVAL_H_
#define _SAC_UNUSED_FUNCTION_ARGUMENT_REMOVAL_H_

#include "types.h"

/******************************************************************************
 *
 * Unused Function Arguments Removal
 *
 * Prefix: UFAR
 *
 * Part of the unused argument removal (UAR) pipeline.
 *   - stdopt/unused_argument_annotate
 *   - stdopt/unused_application_argument_removal
 *   - precompile/unused_function_argument_removal
 *   - precompile/dummy_definition_removal
 *
 ******************************************************************************/

extern node *UFARdoUnusedFunctionArgumentRemoval (node *arg_node);

extern node *UFARfundef (node *arg_node, info *arg_info);
extern node *UFARarg (node *arg_node, info *arg_info);

#endif /* _SAC_UNUSED_FUNCTION_ARGUMENT_REMOVAL_H_ */
