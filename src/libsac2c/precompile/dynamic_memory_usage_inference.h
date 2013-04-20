#ifndef _SAC_DYNAMIC_MEMORY_USAGE_INFERENCE_H_
#define _SAC_DYNAMIC_MEMORY_USAGE_INFERENCE_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Dynamic Memory Usage Inference traversal ( temp_tab)
 *
 * Prefix: DMUI
 *
 *****************************************************************************/
extern node *DMUIdoDynamicMemoryUsageInference (node *syntax_tree);

extern node *DMUIfundef (node *arg_node, info *arg_info);
extern node *DMUIprf (node *arg_node, info *arg_info);

#endif /* _SAC_DYNAMIC_MEMORY_USAGE_INFERENCE_H_ */
