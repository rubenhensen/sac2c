#ifndef _SAC_ALIASANALYSIS_H_
#define _SAC_ALIASANALYSIS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Alias analysis traversal (emaa_tab)
 *
 * prefix: EMAA
 *
 * Nodes which MUST NOT be traversed
 *
 * - N_return
 * - N_array
 * - N_objdef
 *
 *****************************************************************************/
extern node *EMAAdoAliasAnalysis (node *syntax_tree);

extern node *EMAAap (node *arg_node, info *arg_info);
extern node *EMAAarg (node *arg_node, info *arg_info);
extern node *EMAAassign (node *arg_node, info *arg_info);
extern node *EMAAcode (node *arg_node, info *arg_info);
extern node *EMAAcond (node *arg_node, info *arg_info);
extern node *EMAAfold (node *arg_node, info *arg_info);
extern node *EMAAfuncond (node *arg_node, info *arg_info);
extern node *EMAAfundef (node *arg_node, info *arg_info);
extern node *EMAAgenarray (node *arg_node, info *arg_info);
extern node *EMAAid (node *arg_node, info *arg_info);
extern node *EMAAlet (node *arg_node, info *arg_info);
extern node *EMAAmodarray (node *arg_node, info *arg_info);
extern node *EMAAprf (node *arg_node, info *arg_info);
extern node *EMAAwith (node *arg_node, info *arg_info);
extern node *EMAAwith2 (node *arg_node, info *arg_info);
extern node *EMAAwith3 (node *arg_node, info *arg_info);
extern node *EMAAvardec (node *arg_node, info *arg_info);

#endif /* _SAC_ALIASANALYSIS_H_ */
