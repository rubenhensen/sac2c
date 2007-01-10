/*
 * $Id$
 */

#ifndef _SAC_WLTRANSFORM_H_
#define _SAC_WLTRANSFORM_H_

#include "types.h"

extern node *WLTRAdoWlTransform (node *syntax_tree);

extern node *WLTRAassign (node *arg_node, info *arg_info);
extern node *WLTRAcode (node *arg_node, info *arg_info);
extern node *WLTRAlet (node *arg_node, info *arg_info);
extern node *WLTRAwith (node *arg_node, info *arg_info);

extern bool WLTRAallStridesAreConstant (node *wlnode, bool trav_cont, bool trav_nextdim);

extern node *WLTRAinsertWlNodes (node *nodes, node *insert_nodes);
extern int WLTRAgridOffset (int new_bound1, int bound1, int step, int grid_b2);

#endif /* _SAC_WLTRANSFORM_H_ */
