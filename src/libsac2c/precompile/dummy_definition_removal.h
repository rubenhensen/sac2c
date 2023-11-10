#ifndef _SAC_DUMMY_DEFINITION_REMOVAL_H_
#define _SAC_DUMMY_DEFINITION_REMOVAL_H_

#include "types.h"

/******************************************************************************
 *
 * Dummy Definition Removal
 *
 * Prefix: DDR
 *
 * Part of the unused argument removal (UAR) pipeline.
 *   - stdopt/unused_argument_annotate
 *   - stdopt/unused_application_argument_removal
 *   - precompile/unused_function_argument_removal
 *   - precompile/dummy_definition_removal
 *
 ******************************************************************************/

extern node *DDRdoDummyDefinitionRemoval (node *arg_node);

extern node *DDRfundef (node *arg_node, info *arg_info);
extern node *DDRvardec (node *arg_node, info *arg_info);
extern node *DDRassign (node *arg_node, info *arg_info);
extern node *DDRap (node *arg_node, info *arg_info);
extern node *DDRexprs (node *arg_node, info *arg_info);
extern node *DDRids (node *arg_node, info *arg_info);

#endif /* _SAC_DUMMY_DEFINITION_REMOVAL_H_ */
