/*****************************************************************************
 *
 * file:   UndoSSATransform.h
 *
 * prefix: USSAT
 *
 * description: Undo SSA-Transform traversal ( ussa_tab)
 *
 *****************************************************************************/

#ifndef _SAC_UNDOSSATRANSFORM_H_
#define _SAC_UNDOSSATRANSFORM_H_

#include "types.h"

extern node *USSATdoUndoSsaTransform (node *syntax_tree);

extern node *USSATvardec (node *arg_node, info *arg_info);
extern node *USSATavis (node *arg_node, info *arg_info);
extern node *USSATid (node *arg_node, info *arg_info);
extern node *USSATids (node *arg_node, info *arg_info);
extern node *USSATcond (node *arg_node, info *arg_info);
extern node *USSATfuncond (node *arg_node, info *arg_info);
extern node *USSATlet (node *arg_node, info *arg_info);
extern node *USSATassign (node *arg_node, info *arg_info);
extern node *USSATfundef (node *arg_node, info *arg_info);
extern node *USSATblock (node *arg_node, info *arg_info);

#endif /* _SAC_UNDOSSATRANSFORM_H_ */
