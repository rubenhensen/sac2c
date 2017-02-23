
#ifndef _SAC_WITH_LOOP_UTILITIES_H_
#define _SAC_WITH_LOOP_UTILITIES_H_

#include "types.h"

extern bool WLUTisEmptyPartitionCodeBlock (node *partn);
extern bool WLUTisIdsMemberPartition (node *arg_node, node *partn);
extern node *WLUTfindArrayForBound (node *bnd);
extern bool WLUTisCopyPartition (node *partn);
extern node *WLUTfindCopyPartition (node *partn);
extern node *WLUTfindCopyPartitionFromCexpr (node *cexpr, node *withidvec);
extern bool WLUTisEmptyGenerator (node *partn);
extern node *WLUTremoveUnusedCodes (node *codes);

#endif /* _SAC_WITH_LOOP_UTILITIES_H_ */
