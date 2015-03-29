/** <!--********************************************************************-->
 *
 * Polyhedral With-Loop-Folding-traversal
 *
 * Prefix: PWLF
 *
 *****************************************************************************/

#ifndef _SAC_POLYHEDRAL_WLF_H_
#define _SAC_POLYHEDRAL_WLF_H_

#include "types.h"

extern node *PWLFdoPolyhedralWithLoopFolding (node *arg_node);
extern node *PWLFfundef (node *arg_node, info *arg_info);
extern node *PWLFblock (node *arg_node, info *arg_info);
extern node *PWLFassign (node *arg_node, info *arg_info);
extern node *PWLFlet (node *arg_node, info *arg_info);
extern node *PWLFcond (node *arg_node, info *arg_info);
extern node *PWLFfuncond (node *arg_node, info *arg_info);
extern node *PWLFwhile (node *arg_node, info *arg_info);
extern node *PWLFid (node *arg_node, info *arg_info);
extern node *PWLFwith (node *arg_node, info *arg_info);
extern node *PWLFpart (node *arg_node, info *arg_info);
extern node *PWLFcode (node *arg_node, info *arg_info);
extern node *PWLFmodarray (node *arg_node, info *arg_info);
extern node *PWLFgenerator (node *arg_node, info *arg_info);
extern node *PWLFprf (node *arg_node, info *arg_info);

#endif /* _SAC_POLYHEDRAL_WLF_H_ */
