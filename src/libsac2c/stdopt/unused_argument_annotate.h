#ifndef _SAC_UNUSED_ARGUMENT_ANNOTATE_H_
#define _SAC_UNUSED_ARGUMENT_ANNOTATE_H_

#include "types.h"

/******************************************************************************
 *
 * Unused Argument Annotate
 *
 * Prefix: UAA
 *
 * Part of the unused argument removal (UAR) pipeline.
 *   - stdopt/unused_argument_annotate
 *   - stdopt/unused_application_argument_removal
 *   - precompile/unused_function_argument_removal
 *   - precompile/dummy_definition_removal
 *
 ******************************************************************************/

extern node *UAAdoUnusedArgumentAnnotate (node *arg_node);

extern node *UAAfundef (node *arg_node, info *arg_info);
extern node *UAAarg (node *arg_node, info *arg_info);
extern node *UAAid (node *arg_node, info *arg_info);

#endif /* _SAC_UNUSED_ARGUMENT_ANNOTATE_H_ */
