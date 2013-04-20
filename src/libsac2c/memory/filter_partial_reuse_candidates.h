/** <!--********************************************************************-->
 *
 * Filter Partial Reuse Candidates traversal ( fprc_tab)
 *
 * Prefix: FPRC
 *
 *****************************************************************************/

#ifndef _SAC_FILTER_PARTIAL_REUSE_CANDIDATES_H_
#define _SAC_FILTER_PARTIAL_REUSE_CANDIDATES_H_

#include "types.h"

extern node *FPRCdoFilterPartialReuseCandidates (node *syntax_tree);

extern node *FPRCblock (node *arg_node, info *arg_info);
extern node *FPRCavis (node *arg_node, info *arg_info);
extern node *FPRCassign (node *arg_node, info *arg_info);
extern node *FPRCwith (node *arg_node, info *arg_info);
extern node *FPRCgenarray (node *arg_node, info *arg_info);
extern node *FPRCid (node *arg_node, info *arg_info);
extern node *FPRCprf (node *arg_node, info *arg_info);

#endif /* _SAC_FILTER_PARTIAL_REUSE_CANDIDATES_H_ */
