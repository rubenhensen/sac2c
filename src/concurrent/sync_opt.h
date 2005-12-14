#ifndef _SAC_SYNC_OPT_H_
#define _SAC_SYNC_OPT_H_

#include "types.h"

/******************************************************************************
 *
 * $Id$
 *
 * SYNC optimization traversal ( synco_tab)
 *
 * Prefix: SYNCO
 *
 *****************************************************************************/
extern node *SYNCOassign (node *arg_node, info *arg_info);
extern node *SYNCOsync (node *arg_node, info *arg_info);

#endif /* _SAC_SYNC_OPT_H_ */
