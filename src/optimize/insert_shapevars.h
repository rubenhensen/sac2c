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
extern node *ISVvardec (node *arg_node, info *arg_info);
extern node *ISVavis (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_SHAPEVARS_H_ */
