#ifndef _SAC_TREE_UTILS_H_
#define _SAC_TREE_UTILS_H_

/*
 * $Id$
 */

#include "types.h"

extern bool TULSisZeroTripGenerator (node *lb, node *ub, node *width);
extern bool TULSisFullGenerator (node *generator, node *operator);
extern node *TUremoveUnusedCodes (node *arg_node);

#endif /* _SAC_TREE_UTILS_H_ */
