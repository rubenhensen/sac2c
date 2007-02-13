/*
 * $Id: wl_cost_check.h dpa $
 */

#ifndef _SAC_WL_COST_CHECK_H_
#define _SAC_WL_COST_CHECK_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * With Loop Cost Check traversal ( wlcc_tab)
 *
 * Prefix: WLCC
 *
 *****************************************************************************/
extern node *WLCCdoWLCostCheck (node *with);

extern node *WLCCwith (node *arg_node, info *arg_info);
extern node *WLCCcode (node *arg_node, info *arg_info);
extern node *WLCCprf (node *arg_node, info *arg_info);
extern node *WLCCap (node *arg_node, info *arg_info);

#endif /* _SAC_WL_COST_CHECK_H_ */
