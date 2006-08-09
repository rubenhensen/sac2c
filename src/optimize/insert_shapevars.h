/*
 * $Id$
 */
#ifndef _SAC_INSERT_SHAPEVARS_H_
#define _SAC_INSERT_SHAPEVARS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Insert shape variables traversal ( isv_tab)
 *
 * Prefix: ISV
 *
 *****************************************************************************/
extern node *ISVdoInsertShapeVariables (node *syntax_tree);

extern node *ISVfundef (node *arg_node, info *arg_info);
extern node *ISVavis (node *arg_node, info *arg_info);
extern node *ISVblock (node *arg_node, info *arg_info);
extern node *ISVassign (node *arg_node, info *arg_info);
extern node *ISVlet (node *arg_node, info *arg_info);
extern node *ISVids (node *arg_node, info *arg_info);
extern node *ISVid (node *arg_node, info *arg_info);
extern node *ISVprf (node *arg_node, info *arg_info);
extern node *ISVwith (node *arg_node, info *arg_info);
extern node *ISVpart (node *arg_node, info *arg_info);
extern node *ISVcode (node *arg_node, info *arg_info);
extern node *ISVcond (node *arg_node, info *arg_info);
extern node *ISVfuncond (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_SHAPEVARS_H_ */
