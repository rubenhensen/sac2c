#ifndef _SAC_STRIP_CONFORMITY_CHECKS_H_
#define _SAC_STRIP_CONFORMITY_CHECKS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( temp_tab)
 *
 * Prefix: TEMP
 *
 *****************************************************************************/
extern node *SCCdoStripConformityChecks (node *syntax_tree);

extern node *SCCblock (node *arg_node, info *arg_info);
extern node *SCCassign (node *arg_node, info *arg_info);
extern node *SCClet (node *arg_node, info *arg_info);
extern node *SCCprf (node *arg_node, info *arg_info);
extern node *SCCid (node *arg_node, info *arg_info);
extern node *SCCvardec (node *arg_node, info *arg_info);

#endif /* _SAC_STRIP_CONFORMITY_CHECKS_H_ */
