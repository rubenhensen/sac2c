#ifndef _SAC_CHMR_H_
#define _SAC_CHMR_H_

#include "types.h"

/**
 * CHMR - CUDA Host Memory Reuse
 */

extern node *CHMRdoMemoryReuse (node *syntax_tree);

extern node *CHMRfundef (node *arg_node, info *arg_info);
extern node *CHMRprf (node *arg_node, info *arg_info);

#endif /* _SAC_CHMR_H_ */
