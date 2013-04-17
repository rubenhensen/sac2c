#ifndef _SAC_WITHLOOP_FLATTENING_H_
#define _SAC_WITHLOOP_FLATTENING_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( wlflt_tab)
 *
 * Prefix: WLFLT
 *
 *****************************************************************************/
extern node *WLFLTdoWithloopFlattening (node *syntax_tree);

extern node *WLFLTid (node *arg_node, info *arg_info);
extern node *WLFLTavis (node *arg_node, info *arg_info);
extern node *WLFLTwith (node *arg_node, info *arg_info);
extern node *WLFLTgenarray (node *arg_node, info *arg_info);
extern node *WLFLTassign (node *arg_node, info *arg_info);
extern node *WLFLTlet (node *arg_node, info *arg_info);
extern node *WLFLTblock (node *arg_node, info *arg_info);
extern node *WLFLTfundef (node *arg_node, info *arg_info);
extern node *WLFLTwithid (node *arg_node, info *arg_info);
extern node *WLFLTgenerator (node *arg_node, info *arg_info);
extern node *WLFLTids (node *arg_node, info *arg_info);

#endif /* _SAC_WITHLOOP_FLATTENING_H_ */
