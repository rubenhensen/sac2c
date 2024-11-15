#ifndef _SAC_CWLE_TRAV_H_
#define _SAC_CWLE_TRAV_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( temp_tab)
 *
 * Prefix: CWLE
 *
 *****************************************************************************/
extern node *CWLEdoCopyWithLoopElimination (node *arg_node);

extern node *CWLEfundef (node *arg_node, info *arg_info);
extern node *CWLEwith (node *arg_node, info *arg_info);
extern node *CWLElet (node *arg_node, info *arg_info);
extern node *CWLEcode (node *arg_node, info *arg_info);
extern node *CWLEassign (node *arg_node, info *arg_info);
extern node *CWLEids (node *arg_node, info *arg_info);
extern node *CWLEarg (node *arg_node, info *arg_info);

extern node *CWLEfindCopyPartitionSrcWl (node *withid, node *cexpr);

#endif /* _SAC_CWLE_TARV_H_ */
