/*****************************************************************************
 *
 * $Id$
 *
 * Removing propagates
 *
 * prefix: RMPR
 *
 *****************************************************************************/

#ifndef _SAC_REMOVE_PROPAGATES_H_
#define _SAC_REMOVE_PROPAGATES_H_

#include "types.h"

extern node *RMPRdoRemovePropagates (node *syntax_tree);

extern node *RMPRmodule (node *arg_node, info *arg_info);
extern node *RMPRfundef (node *arg_node, info *arg_info);
extern node *RMPRassign (node *arg_node, info *arg_info);
extern node *RMPRwith2 (node *arg_node, info *arg_info);
extern node *RMPRprf (node *arg_node, info *arg_info);

#endif /* _SAC_REMOVE_PROPAGATES_H_ */
