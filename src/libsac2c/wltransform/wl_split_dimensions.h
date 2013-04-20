#ifndef _SAC_WL_SPLIT_DIMENSIONS_H_
#define _SAC_WL_SPLIT_DIMENSIONS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * With-Loop Split Dimensions ( temp_tab)
 *
 * Prefix: WLSD
 *
 *****************************************************************************/
extern node *WLSDdoWithLoopSplitDimensions (node *syntax_tree);

extern node *WLSDfundef (node *arg_node, info *arg_info);
extern node *WLSDassign (node *arg_node, info *arg_info);
extern node *WLSDlet (node *arg_node, info *arg_info);
extern node *WLSDblock (node *arg_node, info *arg_info);
extern node *WLSDwith (node *arg_node, info *arg_info);
extern node *WLSDwith2 (node *arg_node, info *arg_info);
extern node *WLSDwithid (node *arg_node, info *arg_info);
extern node *WLSDwlseg (node *arg_node, info *arg_info);
extern node *WLSDwlstride (node *arg_node, info *arg_info);
extern node *WLSDwlgrid (node *arg_node, info *arg_info);
extern node *WLSDwlblock (node *arg_node, info *arg_info);

#endif /* _SAC_WL_SPLIT_DIMENSIONS_H_ */
