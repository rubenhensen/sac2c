/*
 * $Log$
 * Revision 1.3  2005/09/12 13:57:49  ktr
 * added ...OneFundef variant
 *
 * Revision 1.2  2005/07/26 14:32:08  sah
 * moved creation of special fold funs to
 * dispatchfuncall as new2old is running
 * prior to the module system which again relies
 * on the fact that no foldfuns have been
 * created, yet.
 *
 * Revision 1.1  2005/07/15 15:53:39  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_DISPATCHFUNCALLS_H_
#define _SAC_DISPATCHFUNCALLS_H_

#include "types.h"

node *DFCdoDispatchFunCalls (node *ast);
node *DFCdoDispatchFunCallsOneFundef (node *ast);
node *DFCdoDispatchFunCallsOneFundefAnon (node *arg_node, info *arg_info);

node *DFCmodule (node *arg_node, info *arg_info);
node *DFCfundef (node *arg_node, info *arg_info);
node *DFCap (node *arg_node, info *arg_info);
node *DFCwith (node *arg_node, info *arg_info);
node *DFCgenarray (node *arg_node, info *arg_info);
node *DFCmodarray (node *arg_node, info *arg_info);
node *DFCfold (node *arg_node, info *arg_info);
node *DFCpropagate (node *arg_node, info *arg_info);
node *DFClet (node *arg_node, info *arg_info);

#endif /* _SAC_DISPATCHFUNCALLS_H_ */
