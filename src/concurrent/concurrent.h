#ifndef _SAC_CONCURRENT_H_
#define _SAC_CONCURRENT_H_

#include "types.h"

/*****************************************************************************
 *
 * $Id$
 *
 * Concurrent traversal (conc_tab)
 *
 * prefix: CONC
 *
 *****************************************************************************/
extern node *CONCdoConcurrent (node *syntax_tree);

extern node *CONCmodule (node *arg_node, info *arg_info);
extern node *CONCfundef (node *arg_node, info *arg_info);

#endif /* _SAC_CONCURRENT_H_ */
