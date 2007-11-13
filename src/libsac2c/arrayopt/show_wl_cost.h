/*
 * $Id$
 */

#ifndef _SAC_SHOW_WL_COST_H_
#define _SAC_SHOW_WL_COST_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Show With Loop Cost information
 *
 * prefix: SHWLC
 *
 *****************************************************************************/

extern node *SHWLCprintPreFun (node *arg_node, info *arg_info);
extern node *SHWLCactivate (node *syntax_tree);
extern node *SHWLCdeactivate (node *syntax_tree);

#endif /* _SAC_SHOW_WL_COST_H_ */
