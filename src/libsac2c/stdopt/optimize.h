/*
 * $Id$
 */

#ifndef _SAC_OPTIMIZE_H_
#define _SAC_OPTIMIZE_H_

#include "types.h"

extern node *OPTdoPrintStatistics (node *syntax_tree);

extern node *OPTrunStabilizationCycle (node *arg_node);
extern node *OPTrunOptimizationCycle (node *arg_node);

#endif /* _SAC_OPTIMIZE_H_ */
