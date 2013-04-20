/******************************************************************************
 *
 * MarkMemVals traversal ( mmv_tab)
 *
 * Prefix: MMV
 *
 *****************************************************************************/

#ifndef _SAC_MARKMEMVALS_H_
#define _SAC_MARKMEMVALS_H_

#include "types.h"

extern node *MMVdoMarkMemVals (node *syntax_tree);

extern node *MMVblock (node *arg_node, info *arg_info);
extern node *MMVbreak (node *arg_node, info *arg_info);
extern node *MMVcode (node *arg_node, info *arg_info);
extern node *MMVdo (node *arg_node, info *arg_info);
extern node *MMVfold (node *arg_node, info *arg_info);
extern node *MMVfundef (node *arg_node, info *arg_info);
extern node *MMVmodule (node *arg_node, info *arg_info);
extern node *MMVgenarray (node *arg_node, info *arg_info);
extern node *MMVpropagate (node *arg_node, info *arg_info);
extern node *MMVid (node *arg_node, info *arg_info);
extern node *MMVids (node *arg_node, info *arg_info);
extern node *MMVap (node *arg_node, info *arg_info);
extern node *MMVlet (node *arg_node, info *arg_info);
extern node *MMVmodarray (node *arg_node, info *arg_info);
extern node *MMVprf (node *arg_node, info *arg_info);
extern node *MMVwith (node *arg_node, info *arg_info);
extern node *MMVwith2 (node *arg_node, info *arg_info);
extern node *MMVwith3 (node *arg_node, info *arg_info);
extern node *MMVwlseg (node *arg_node, info *arg_info);
extern node *MMVreturn (node *arg_node, info *arg_info);
extern node *MMVret (node *arg_node, info *arg_info);

#endif /* _SAC_MARKMEMVALS_H_ */
