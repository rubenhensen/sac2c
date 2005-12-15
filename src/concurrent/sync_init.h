#ifndef _SAC_SYNC_INIT_H_
#define _SAC_SYNC_INIT_H_

#include "types.h"

/******************************************************************************
 *
 * $Id$
 *
 * SYNC-Block init traversal ( syncinit_tab)
 *
 * Prefix: SYNCI
 *
 *****************************************************************************/
extern node *SYNCIdoSyncInit (node *arg_node);

extern node *SYNCIfundef (node *arg_node, info *arg_info);
extern node *SYNCIassign (node *arg_node, info *arg_info);

#endif /* _SAC_SYNC_INIT_H_ */
