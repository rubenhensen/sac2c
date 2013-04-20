/******************************************************************************
 *
 * Data reuse traversal
 *
 * Prefix: EMDR
 *
 *****************************************************************************/

#ifndef _SAC_DATAREUSE_H_
#define _SAC_DATAREUSE_H_

#include "types.h"

extern node *EMDRdoDataReuse (node *syntax_tree);

extern node *EMDRap (node *arg_node, info *arg_info);
extern node *EMDRassign (node *arg_node, info *arg_info);
extern node *EMDRwithid (node *arg_node, info *arg_info);
extern node *EMDRwith (node *arg_node, info *arg_info);
extern node *EMDRwith2 (node *arg_node, info *arg_info);
extern node *EMDRwith3 (node *arg_node, info *arg_info);
extern node *EMDRrange (node *arg_node, info *arg_info);
extern node *EMDRcode (node *arg_node, info *arg_info);
extern node *EMDRcond (node *arg_node, info *arg_info);
extern node *EMDRfundef (node *arg_node, info *arg_info);
extern node *EMDRlet (node *arg_node, info *arg_info);
extern node *EMDRprf (node *arg_node, info *arg_info);
extern node *EMDRgenarray (node *arg_node, info *arg_info);
extern node *EMDRmodarray (node *arg_node, info *arg_info);

#endif /* _SAC_DATAREUSE_H_ */
