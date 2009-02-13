/*
 * $Id$
 */
#ifndef _SAC_DISTRIBUTE_THREADS_H_
#define _SAC_DISTRIBUTE_THREADS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Distribute threads traversal ( dst_tab)
 *
 * Prefix: DST
 *
 *****************************************************************************/
extern node *DSTdoDistributeThreads (node *syntax_tree);

extern node *DSTmodule (node *arg_node, info *arg_info);
extern node *DSTfundef (node *arg_node, info *arg_info);
extern node *DSTap (node *arg_node, info *arg_info);
extern node *DSTwith3 (node *arg_node, info *arg_info);
extern node *DSTrange (node *arg_node, info *arg_info);

#endif /* _SAC_DISTRIBUTE_THREADS_H_ */
