/*
 * $Id$
 */
#ifndef _SAC_MOVE_ASSIGNS_H_
#define _SAC_MOVE_ASSIGNS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Move Syncs traversal ( ma_tab)
 *
 * Prefix: MA
 *
 *****************************************************************************/
extern node *MAdoMoveAssigns (node *syntax_tree, pattern *pat, bool block);

extern node *MAlet (node *arg_node, info *arg_info);
extern node *MAassign (node *arg_node, info *arg_info);

#endif /* _SAC_MOVE_ASSIGNS_H_ */
