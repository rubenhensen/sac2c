/*
 * $Id$
 */
#ifndef _SAC_MOVE_CONST_H_
#define _SAC_MOVE_CONST_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Move Const traversal
 *
 * Prefix: MC
 *
 *****************************************************************************/
extern node *MCdoMoveConst (node *syntax_tree);

extern node *MCfundef (node *arg_node, info *arg_info);
extern node *MClet (node *arg_node, info *arg_info);
extern node *MCassign (node *arg_node, info *arg_info);

#endif /* _SAC_MOVE_CONST_H_ */
