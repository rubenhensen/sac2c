#ifndef _SAC_GENERATE_COPY_AND_FREE_H_
#define _SAC_GENERATE_COPY_AND_FREE_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Generate Copy and Free Traversal (gcf_tab)
 *
 * Prefix: GCF
 *
 *****************************************************************************/
extern node *GCFdoGenerateCopyAndFree (node *syntax_tree);

extern node *GCFtypedef (node *arg_node, info *arg_info);
extern node *GCFmodule (node *arg_node, info *arg_info);

#endif /* _SAC_GENERATE_COPY_AND_FREE_H_ */
