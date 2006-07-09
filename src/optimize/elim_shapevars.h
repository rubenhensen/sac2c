/*
 * $Id$
 */
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

extern node *ESVavis (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_SHAPEVARS_H_ */
