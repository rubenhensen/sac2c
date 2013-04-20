#ifndef _SAC_REUSEWITHARRAYS_H_
#define _SAC_REUSEWITHARRAYS_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse with-arrays traversal ( reuse_tab)
 *
 * Prefix: REUSE
 *
 *****************************************************************************/
extern node *REUSEdoGetReuseArrays (node *syntax_tree, node *fundef);

extern node *REUSEwith (node *arg_node, info *arg_info);
extern node *REUSEpart (node *arg_node, info *arg_info);
extern node *REUSEgenarray (node *arg_node, info *arg_info);
extern node *REUSEmodarray (node *arg_node, info *arg_info);
extern node *REUSEfold (node *arg_node, info *arg_info);
extern node *REUSEid (node *arg_node, info *arg_info);
extern node *REUSEids (node *arg_node, info *arg_info);
extern node *REUSElet (node *arg_node, info *arg_info);
extern node *REUSEprf (node *arg_node, info *arg_info);

#endif /* _SAC_REUSEWITHARRAYS_H_ */
