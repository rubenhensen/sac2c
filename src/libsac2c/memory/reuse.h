#ifndef _SAC_REUSE_H_
#define _SAC_REUSE_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse inference traversal ( emri_tab)
 *
 * prefix: EMRI
 *
 *****************************************************************************/
extern node *EMRIdoReuseInference (node *syntax_tree);

extern node *EMRIassign (node *arg_node, info *arg_info);
extern node *EMRIlet (node *arg_node, info *arg_info);
extern node *EMRIprf (node *arg_node, info *arg_info);
extern node *EMRIgenarray (node *arg_node, info *arg_info);
extern node *EMRImodarray (node *arg_node, info *arg_info);
extern node *EMRIap (node *arg_node, info *arg_info);
extern node *EMRIfundef (node *arg_node, info *arg_info);

#endif /* _SAC_REUSE_H_ */
