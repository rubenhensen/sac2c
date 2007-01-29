/*
 * $Log$
 * Revision 1.7  2005/04/19 17:36:18  ktr
 * Complete rewrite
 *
 */

#ifndef _SAC_UNDOSSATRANSFORM_H_
#define _SAC_UNDOSSATRANSFORM_H_

#include "types.h"

/*****************************************************************************
 *
 * Undo SSA-Transform traversal ( ussa_tab)
 *
 * prefix: USSAT
 *
 *
 *****************************************************************************/
extern node *USSATdoUndoSsaTransform (node *syntax_tree);

extern node *USSATvardec (node *arg_node, info *arg_info);
extern node *USSATid (node *arg_node, info *arg_info);
extern node *USSATids (node *arg_node, info *arg_info);
extern node *USSATcond (node *arg_node, info *arg_info);
extern node *USSATfuncond (node *arg_node, info *arg_info);
extern node *USSATlet (node *arg_node, info *arg_info);
extern node *USSATassign (node *arg_node, info *arg_info);
extern node *USSATfundef (node *arg_node, info *arg_info);
extern node *USSATblock (node *arg_node, info *arg_info);

#endif /* _SAC_UNDOSSATRANSFORM_H_ */
