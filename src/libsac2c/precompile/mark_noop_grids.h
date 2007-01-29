/**
 * $Id$
 */

#ifndef _SAC_MARK_NOOP_GRIDS_H_
#define _SAC_MARK_NOOP_GRIDS_H_

#include "types.h"

extern node *MNGwlblock (node *arg_node, info *arg_info);
extern node *MNGwlublock (node *arg_node, info *arg_info);
extern node *MNGwlstride (node *arg_node, info *arg_info);
extern node *MNGwlstridevar (node *arg_node, info *arg_info);
extern node *MNGwlgrid (node *arg_node, info *arg_info);
extern node *MNGwlgridvar (node *arg_node, info *arg_info);
extern node *MNGcode (node *arg_node, info *arg_info);
extern node *MNGlet (node *arg_node, info *arg_info);

extern node *MNGdoMarkNoopGrids (node *syntax_tree);

#endif /* _SAC_MARK_NOOP_GRIDS_H_ */
