/*
 * $Log$
 * Revision 1.1  2005/07/15 15:53:39  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_DISPATCHFUNCALLS_H_
#define _SAC_DISPATCHFUNCALLS_H_

#include "types.h"

node *DFCdoDispatchFunCalls (node *ast);

node *DFCmodule (node *arg_node, info *arg_info);
node *DFCfundef (node *arg_node, info *arg_info);
node *DFCap (node *arg_node, info *arg_info);
node *DFCwith (node *arg_node, info *arg_info);
node *DFCgenarray (node *arg_node, info *arg_info);
node *DFCfold (node *arg_node, info *arg_info);

#endif /* _SAC_DISPATCHFUNCALLS_H_ */
