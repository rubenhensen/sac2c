/*
 *
 * $Log$
 * Revision 1.2  2004/11/21 20:18:08  khf
 * the big 2004 codebrushing event
 *
 * Revision 1.1  2004/10/20 08:22:54  khf
 * Initial revision
 *
 *
 *
 */

#include "types.h"

#ifndef _SAC_RESOLVEDEPENDENCIES_H_
#define _SAC_RESOLVEDEPENDENCIES_H_

extern node *RDEPENDdoResolveDependencies (node *assigns, node *cexprs, node *withid,
                                           node *fusionable_wl);

extern node *RDEPENDassign (node *arg_node, info *arg_info);
extern node *RDEPENDprf (node *arg_node, info *arg_info);

#endif /* _SAC_RESOLVEDEPENDENCIES_H_ */
