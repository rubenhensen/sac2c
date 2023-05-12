
#ifndef _SAC_WITH_LOOP_UTILITIES_H_
#define _SAC_WITH_LOOP_UTILITIES_H_

#include "types.h"

extern bool WLUTisEmptyPartitionCodeBlock (node *partn);
extern bool WLUTisIdsMemberPartition (node *arg_node, node *partn);
extern node *WLUTfindArrayForBound (node *bound);
extern bool WLUTisCopyPartition (node *partn);
extern node *WLUTfindCopyPartition (node *partn);
extern node *WLUTfindCopyPartitionFromCexpr (node *cexpr, node *withidvec);
extern bool WLUTisEmptyGenerator (node *partn);
extern node *WLUTremoveUnusedCodes (node *codes);
extern bool WLUTisGenarrayScalar (node *arg_node, bool nowithid);
extern node *WLUTgetGenarrayScalar (node *arg_node, bool nowithid);
extern bool WLUTisSingleOpWl (node *arg_node);
extern node *WLUTid2With (node *arg_node);

#endif /* _SAC_WITH_LOOP_UTILITIES_H_ */
