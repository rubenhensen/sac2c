#ifndef _SAC_EXPLICITCOPY_H_
#define _SAC_EXPLICITCOPY_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Explicit Copy traversal ( emec_tab)
 *
 * Prefix: EMEC
 *
 *****************************************************************************/
extern node *EMECdoExplicitCopy (node *syntax_tree);

extern node *EMECassign (node *arg_node, info *arg_info);
extern node *EMECfundef (node *arg_node, info *arg_info);
extern node *EMECap (node *arg_node, info *arg_info);
extern node *EMECprf (node *arg_node, info *arg_info);

#endif /* _SAC_EXPLICITCOPY_H_ */
