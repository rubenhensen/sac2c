/*
 * $Id$
 */

#ifndef _SAC_DEADFUNCTIONREMOVAL_H_
#define _SAC_DEADFUNCTIONREMOVAL_H_

#include "types.h"

extern node *DFRdoDeadFunctionRemoval (node *arg_node);

extern node *DFRmodule (node *arg_node, info *arg_info);
extern node *DFRobjdef (node *arg_node, info *arg_info);
extern node *DFRfundef (node *arg_node, info *arg_info);
extern node *DFRarg (node *arg_node, info *arg_info);
extern node *DFRap (node *arg_node, info *arg_info);
extern node *DFRfold (node *arg_node, info *arg_info);

#endif /* _DEADFUNCTIONREMOVAL_H_ */
