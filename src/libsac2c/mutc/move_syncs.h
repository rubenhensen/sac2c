/*
 * $Id$
 */
#ifndef _SAC_MOVE_SYNCS_H_
#define _SAC_MOVE_SYNCS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Move Syncs traversal ( ms_tab)
 *
 * Prefix: MS
 *
 *****************************************************************************/
extern node *MSdoMoveSyncs (node *syntax_tree);

extern node *MSlet (node *arg_node, info *arg_info);
extern node *MSprf (node *arg_node, info *arg_info);
extern node *MSassign (node *arg_node, info *arg_info);

#endif /* _SAC_MOVE_SYNCS_H_ */
