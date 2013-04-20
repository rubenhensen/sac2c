#ifndef _SAC_MAKE_SLOW_CLONES_H_
#define _SAC_MAKE_SLOW_CLONES_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Clone functions that contain spawn to create slow clones for FP ( syn_tab)
 *
 * Prefix: MSC
 *
 *****************************************************************************/
extern node *MSCdoMakeSlowClones (node *arg_node);
extern node *MSCfundef (node *arg_node, info *arg_info);

#endif /* _SAC_MAKE_SLOW_CLONES_H_*/
