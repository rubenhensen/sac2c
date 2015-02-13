/** <!--********************************************************************-->
 *
 * Polyhedral With-Loop-Folding-Inference traversal
 *
 * Prefix: PWLFI
 *
 *****************************************************************************/

#ifndef _SAC_POLYHEDRAL_WLFI_H_
#define _SAC_POLYHEDRAL_WLFI_H_

#include "types.h"

extern node *PWLFdoPolyhedralWithLoopFolding (node *arg_node);
extern node *PWLFIfundef (node *arg_node, info *arg_info);
extern node *PWLFIblock (node *arg_node, info *arg_info);
extern node *PWLFIassign (node *arg_node, info *arg_info);
extern node *PWLFIlet (node *arg_node, info *arg_info);
extern node *PWLFIcond (node *arg_node, info *arg_info);
extern node *PWLFIfuncond (node *arg_node, info *arg_info);
extern node *PWLFIwhile (node *arg_node, info *arg_info);
extern node *PWLFIid (node *arg_node, info *arg_info);
extern node *PWLFIwith (node *arg_node, info *arg_info);
extern node *PWLFIpart (node *arg_node, info *arg_info);
extern node *PWLFIcode (node *arg_node, info *arg_info);
extern node *PWLFImodarray (node *arg_node, info *arg_info);
extern node *PWLFIgenerator (node *arg_node, info *arg_info);
extern node *PWLFIprf (node *arg_node, info *arg_info);

#endif /* _SAC_POLYHEDRAL_WLFI_H_ */
