#ifndef _SAC_DEADLOCALFUNCTIONREMOVAL_H_
#define _SAC_DEADLOCALFUNCTIONREMOVAL_H_

#include "types.h"

extern node *DLFRdoDeadLocalFunctionRemoval (node *arg_node);

extern node *DLFRfundef (node *arg_node, info *arg_info);
extern node *DLFRap (node *arg_node, info *arg_info);

#endif /* _DEADLOCALFUNCTIONREMOVAL_H_ */
