/*
 * $Id$
 */
#ifndef _SAC_LIVE_VARIABLE_ANALYSIS_H_
#define _SAC_LIVE_VARIABLE_ANALYSIS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Determine live variables for spawn and sync statements ( syn_tab)
 *
 * Prefix: LVA
 *
 *****************************************************************************/
extern node *LVAdoLiveVariableAnalysis (node *arg_node);
extern node *LVAfundef (node *arg_node, info *arg_info);
extern node *LVAassign (node *arg_node, info *arg_info);
extern node *LVAids (node *arg_node, info *arg_info);
extern node *LVAid (node *arg_node, info *arg_info);

#endif /* _SAC_LIVE_VARIABLE_ANALYSIS_H_ */
