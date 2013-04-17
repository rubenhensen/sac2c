#ifndef _SAC_REMOVE_VARDECS_H_
#define _SAC_REMOVE_VARDECS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Remove vardecs traversal ( temp_tab)
 *
 * Prefix: RMV
 *
 *****************************************************************************/
extern node *RMVdoRemoveVardecsOneFundef (node *fundef);

extern node *RMVfundef (node *arg_node, info *arg_info);
extern node *RMVblock (node *arg_node, info *arg_info);
extern node *RMVvardec (node *arg_node, info *arg_info);
extern node *RMVids (node *arg_node, info *arg_info);

#endif /* _SAC_REMOVE_VARDECS_H_ */
