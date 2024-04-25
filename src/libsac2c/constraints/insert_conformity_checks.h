#ifndef _SAC_INSERT_CONFORMITY_CHECKS_H_
#define _SAC_INSERT_CONFORMITY_CHECKS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( temp_tab)
 *
 * Prefix: TEMP
 *
 *****************************************************************************/
extern node *ICCdoInsertConformityChecks (node *syntax_tree);

extern node *ICCfundef (node *arg_node, info *arg_info);
extern node *ICCblock (node *arg_node, info *arg_info);
extern node *ICCassign (node *arg_node, info *arg_info);
extern node *ICClet (node *arg_node, info *arg_info);
extern node *ICCprf (node *arg_node, info *arg_info);
extern node *ICCwith (node *arg_node, info *arg_info);
extern node *ICCgenerator (node *arg_node, info *arg_info);
extern node *ICCcode (node *arg_node, info *arg_info);
extern node *ICCgenarray (node *arg_node, info *arg_info);
extern node *ICCmodarray (node *arg_node, info *arg_info);
extern node *ICCfold (node *arg_node, info *arg_info);
extern node *ICCbreak (node *arg_node, info *arg_info);
extern node *ICCpropagate (node *arg_node, info *arg_info);

#endif /* _SAC_INSERT_CONFORMITY_CHECKS_H_ */
