/*
 * $Id$
 */
#ifndef _SAC_COUNT_SPAWN_SYNC_H_
#define _SAC_COUNT_SPAWN_SYNC_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Count the number of spawns and syncs in a function ( syn_tab)
 *
 * Prefix: CSS
 *
 *****************************************************************************/
extern node *CSSdoCountSpawnSync (node *arg_node);
extern node *CSSfundef (node *arg_node, info *arg_info);
extern node *CSSap (node *arg_node, info *arg_info);
extern node *CSSlet (node *arg_node, info *arg_info);
extern node *CSSprf (node *arg_node, info *arg_info);

#endif /* _SAC_COUNT_SPAWN_SYNC_H_ */
