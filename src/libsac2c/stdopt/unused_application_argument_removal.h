#ifndef _SAC_UNUSED_APPLICATION_ARGUMENT_REMOVAL_H_
#define _SAC_UNUSED_APPLICATION_ARGUMENT_REMOVAL_H_

#include "types.h"

/******************************************************************************
 *
 * Unused Application Argument Removal
 *
 * Prefix: UAAR
 *
 * Part of the unused argument removal (UAR) pipeline.
 *   - stdopt/unused_argument_annotate
 *   - stdopt/unused_application_argument_removal
 *   - precompile/unused_function_argument_removal
 *   - precompile/dummy_definition_removal
 *
 ******************************************************************************/

extern node *UAARdoUnusedApplicationArgumentRemoval (node *arg_node);

extern node *UAARfundef (node *arg_node, info *arg_info);
extern node *UAARlet (node *arg_node, info *arg_info);
extern node *UAARap (node *arg_node, info *arg_info);
extern node *UAARid (node *arg_node, info *arg_info);

#endif /* _SAC_UNUSED_APPLICATION_ARGUMENT_REMOVAL_H_ */
