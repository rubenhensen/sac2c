/*
 * $Id$
 */
#ifndef _SAC_INSERT_SYMB_ARRAYATTR_H_
#define _SAC_INSERT_SYMB_ARRAYATTR_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Insert symbolic array attributes traversal ( isaa_tab)
 *
 * Prefix: ISAA
 *
 *****************************************************************************/
extern node *ISAAdoInsertShapeVariables (node *syntax_tree);

extern node *ISAAfundef (node *arg_node, info *arg_info);
extern node *ISAAavis (node *arg_node, info *arg_info);
extern node *ISAAblock (node *arg_node, info *arg_info);
extern node *ISAAassign (node *arg_node, info *arg_info);
extern node *ISAAlet (node *arg_node, info *arg_info);
extern node *ISAAids (node *arg_node, info *arg_info);
extern node *ISAAid (node *arg_node, info *arg_info);
extern node *ISAAwith (node *arg_node, info *arg_info);
extern node *ISAApart (node *arg_node, info *arg_info);
extern node *ISAAcode (node *arg_node, info *arg_info);
extern node *ISAAcond (node *arg_node, info *arg_info);
extern node *ISAAfuncond (node *arg_node, info *arg_info);
extern node *ISAAap (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_SHAPEVARS_H_ */
