/*
 * $Id$
 */
#ifndef _SAC_COMPARISON_TO_ZERO_H_
#define _SAC_COMPARISON_TO_ZERO_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Introduce a comparison to zero in compare statements ( ctz_tab)
 *
 * Prefix: CTZ
 *
 *****************************************************************************/
extern node *CTZdoConditionalZeroComparison (node *arg_node);
extern node *CTZdoComparisonToZero (node *argnode);
extern node *CTZblock (node *arg_node, info *arg_info);
extern node *CTZassign (node *arg_node, info *arg_info);
extern node *CTZlet (node *arg_node, info *arg_info);
extern node *CTZprf (node *arg_node, info *arg_info);
extern node *CTZfundef (node *arg_node, info *arg_info);
extern node *CTZmodule (node *arg_node, info *arg_info);

#endif /* _SAC_COMPARISON_TO_ZERO_H_ */
