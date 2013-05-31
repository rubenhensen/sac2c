#ifndef _SAC_TREE_UTILS_H_
#define _SAC_TREE_UTILS_H_

#include "types.h"

extern bool TULSisZeroTripGenerator (node *lb, node *ub, node *width);
extern bool TULSisFullGenerator (node *generator, node *op);
extern node *TUremoveUnusedCodes (node *arg_node);
extern node *TUremoveUnusedCodeBlock (node *arg_node);
extern node *TUmakeIntVec (int i, node **preassign, node **vardec);
extern node *TUscalarizeVector (node *arg_node, node **preassigns, node **vardecs);
extern node *TUmoveAssign (node *avismax, node *preassigns);
extern bool TULSisValuesMatch (node *arg1, node *arg2);
extern int TULSsearchAssignChainForAssign (node *chn, node *assgn);

#endif /* _SAC_TREE_UTILS_H_ */
