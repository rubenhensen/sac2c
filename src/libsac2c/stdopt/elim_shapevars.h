#ifndef _SAC_ELIM_SHAPEVARS_H_
#define _SAC_ELIM_SHAPEVARS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Eliminate shape variables traversal ( esv_tab)
 *
 * Prefix: ESV
 *
 *****************************************************************************/
extern node *ESVdoInsertShapeVariables (node *syntax_tree);

extern node *ESVfundef (node *arg_node, info *arg_info);
extern node *ESVavis (node *arg_node, info *arg_info);
extern node *ESVassign (node *arg_node, info *arg_info);
extern node *ESVlet (node *arg_node, info *arg_info);
extern node *ESVid (node *arg_node, info *arg_info);
extern node *ESVprf (node *arg_node, info *arg_info);

/*
 * Do not traverse
 *
 * N_withid
 */

#endif /* _SAC_INSERT_SHAPEVARS_H_ */
