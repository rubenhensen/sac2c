#ifndef _SAC_COMPARISON_TO_ZERO_GUARDS_H_
#define _SAC_COMPARISON_TO_ZERO_GUARDS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Introduce a comparison to zero in certain
 * guards, for the same reasons it is done in relationals in CTZ.
 *
 * Prefix: CTZ
 *
 *****************************************************************************/
extern node *CTZGdoConditionalZeroComparisonGuards (node *arg_node);
extern node *CTZGdoComparisonToZeroGuards (node *arg_node);
extern node *CTZGblock (node *arg_node, info *arg_info);
extern node *CTZGassign (node *arg_node, info *arg_info);
extern node *CTZGlet (node *arg_node, info *arg_info);
extern node *CTZGprf (node *arg_node, info *arg_info);
extern node *CTZGfundef (node *arg_node, info *arg_info);

#endif /* _SAC_COMPARISON_TO_ZERO_GUARDS_H_ */
