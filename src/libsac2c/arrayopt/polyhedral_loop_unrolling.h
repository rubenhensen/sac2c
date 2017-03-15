#ifndef _SAC_PLUR_H_
#define _SAC_PLUR_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PLUR
 *
 *****************************************************************************/
extern node *PLURdoPolyhedralLoopUnrolling (node *arg_node);
extern node *PLURfundef (node *arg_node, info *arg_info);
extern node *PLURpart (node *arg_node, info *arg_info);
extern node *PLURassign (node *arg_node, info *arg_info);
extern node *PLURlet (node *arg_node, info *arg_info);
extern node *PLURap (node *arg_node, info *arg_info);

#endif /* _SAC_PLUR_H_ */
