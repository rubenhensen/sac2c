#ifndef _SAC_PLUR_H_
#define _SAC_PLUR_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PLUR
 *
 *****************************************************************************/
extern node *PLURfundef (node *arg_node, info *arg_info);
extern node *PLURpart (node *arg_node, info *arg_info);
extern node *PLURdoPolyhedralLoopUnrolling (node *arg_node);

#endif /* _SAC_PLUR_H_ */
