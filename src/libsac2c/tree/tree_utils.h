#ifndef _SAC_TREE_UTILS_H_
#define _SAC_TREE_UTILS_H_

/*
 * $Id$
 */

#include "types.h"

extern bool TULSisZeroTripGenerator (node *lb, node *ub, node *width);
extern bool TULSisFullGenerator (node *generator, node *operator);
extern node *TUremoveUnusedCodes (node *arg_node);
extern node *TUremoveUnusedCodeBlock (node *arg_node);
extern node *TUmakeIntVec (int i, node **preassign, node **vardec);
extern node *TUscalarizeVector (node *arg_node, node **preassigns, node **vardecs);
extern node *TUmoveAssign (node *avismax, node *preassigns);

#endif /* _SAC_TREE_UTILS_H_ */
