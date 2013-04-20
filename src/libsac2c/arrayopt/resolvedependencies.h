#ifndef _SAC_RESOLVEDEPENDENCIES_H_
#define _SAC_RESOLVEDEPENDENCIES_H_

#include "types.h"

/******************************************************************************
 *
 * Resolve dependencies traversal ( rdepend_tab)
 *
 * Prefix: RDEPEND
 *
 *****************************************************************************/
extern node *RDEPENDdoResolveDependencies (node *assigns, node *cexprs, node *withid,
                                           node *fusionable_wl);

extern node *RDEPENDassign (node *arg_node, info *arg_info);
extern node *RDEPENDprf (node *arg_node, info *arg_info);

#endif /* _SAC_RESOLVEDEPENDENCIES_H_ */
