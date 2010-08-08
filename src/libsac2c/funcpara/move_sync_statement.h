/*
 * $Id$
 */
#ifndef _SAC_MOVE_SYNC_STATEMENT_H_
#define _SAC_MOVE_SYNC_STATEMENT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Move sync statements to increase distance from spawn ( syn_tab)
 *
 * Prefix: MSS
 *
 *****************************************************************************/
extern node *MSSdoMoveSyncStatement (node *arg_node);
extern node *MSSfundef (node *arg_node, info *arg_info);
extern node *MSSassign (node *arg_node, info *arg_info);
extern node *MSSid (node *arg_node, info *arg_info);

#endif /* _SAC_MOVE_SYNC_STATEMENT_H_ */
