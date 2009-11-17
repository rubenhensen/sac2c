
#ifndef _SAC_SPLIT_PARTITIONS_H_
#define _SAC_SPLIT_PARTITIONS_H_

#include "types.h"

/******************************************************************************
 *
 *
 * Prefix: SPTN
 *
 *****************************************************************************/
extern node *SPTNdoSplitPartitions (node *arg_node);
extern node *SPTNwith (node *arg_node, info *arg_info);
extern node *SPTNpart (node *arg_node, info *arg_info);
extern node *SPTNgenerator (node *arg_node, info *arg_info);

#endif /* _SAC_SPLIT_PARTITIONS_H_ */
