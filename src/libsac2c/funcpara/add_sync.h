/*
 * $Id:
 */
#ifndef _SAC_ADD_SYNC_H_
#define _SAC_ADD_SYNC_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Add a sync statement to match spawned functions ( syn_tab)
 *
 * Prefix: SYN
 *
 *****************************************************************************/
extern node *SYNdoAddSync (node *arg_node);
extern node *SYNap (node *arg_node, info *arg_info);

#endif /* _SAC_ADD_SYNC_H_*/
