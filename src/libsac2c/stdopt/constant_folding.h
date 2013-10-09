#ifndef _SAC_constantfolding_h_
#define _SAC_constantfolding_h_

#include "types.h"

/** <!--********************************************************************-->
 *
 * file:   constantfolding.h
 *
 * prefix: CF
 *
 *****************************************************************************/
extern node *CFdoConstantFolding (node *fundef);
extern bool CFisFullyConstantNode (node *arg_node);
extern node *CFunflattenSimpleScalars (node *arg_node);
extern node *CFcreateConstExprsFromType (ntype *type);

extern node *CFfundef (node *arg_node, info *arg_info);
extern node *CFblock (node *arg_node, info *arg_info);
extern node *CFassign (node *arg_node, info *arg_info);
extern node *CFcond (node *arg_node, info *arg_info);
extern node *CFlet (node *arg_node, info *arg_info);
extern node *CFids (node *arg_node, info *arg_info);
extern node *CFarray (node *arg_node, info *arg_info);
extern node *CFprf (node *arg_node, info *arg_info);
extern node *CFwith (node *arg_node, info *arg_info);
extern node *CFpart (node *arg_node, info *arg_info);
extern node *CFcode (node *arg_node, info *arg_info);
extern node *CFfuncond (node *arg_node, info *arg_info);
extern node *CFprf_shape (node *arg_node, info *arg_info);
extern node *CFprf_reshape (node *arg_node, info *arg_info);

#endif /* _SAC_constantfolding_h_ */
