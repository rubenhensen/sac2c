#ifndef _SAC_CONSTRUCT_BUNDLES_H_
#define _SAC_CONSTRUCT_BUNDLES_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Construct Bundles traversal ( cbl_tab)
 *
 * Prefix: CBL
 *
 *****************************************************************************/
extern node *CBLdoConstructBundles (node *syntax_tree);

extern node *CBLfundef (node *arg_node, info *arg_info);
extern node *CBLmodule (node *arg_node, info *arg_info);

#endif /* _SAC_CONSTRUCT_BUNDLES_H_ */
