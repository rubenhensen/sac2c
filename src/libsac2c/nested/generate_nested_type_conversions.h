#ifndef _SAC_GENERATE_NESTED_TYPE_CONVERSIONS_H_
#define _SAC_GENERATE_NESTED_TYPE_CONVERSIONS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( gntc_tab)
 *
 * Prefix: GNTC
 *
 *****************************************************************************/
extern node *GNTCdoGenerateNestedTypeConversions (node *syntax_tree);

extern node *GNTCmodule (node *arg_node, info *arg_info);
extern node *GNTCtypedef (node *arg_node, info *arg_info);

#endif /* _SAC_GENERATE_NESTED_TYPE_CONVERSIONS_H_ */
