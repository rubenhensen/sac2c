/*
 * $Id$
 */

#ifndef _SAC__FREE_DISPATCH_INFORMATION_H_
#define _SAC__FREE_DISPATCH_INFORMATION_H_

#include "types.h"

extern node *FDIfundef (node *arg_node, info *arg_info);
extern node *FDIap (node *arg_node, info *arg_info);
extern node *FDImodule (node *arg_node, info *arg_info);

extern node *FDIdoFreeDispatchInformation (node *module);

#endif /* _SAC__FREE_DISPATCH_INFORMATION_H_ */
