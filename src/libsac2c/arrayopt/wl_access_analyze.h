/*****************************************************************************
 *
 * file:   wl_access_analyze.h
 *
 * prefix: WLAA
 *
 * description:
 *
 *   This compiler module analyzes array accesses within withloops.
 *   It is used by the tilesize inference.
 *
 *
 *****************************************************************************/

#ifndef _SAC_WL_ACCESS_ANALYZE_H_
#define _SAC_WL_ACCESS_ANALYZE_H_

#include "types.h"

extern node *WLAAdoAccessAnalyze (node *arg_node);

#ifndef WLAA_DEACTIVATED

extern node *WLAAfundef (node *arg_node, info *arg_info);
extern node *WLAAblock (node *arg_node, info *arg_info);
extern node *WLAAwith (node *arg_node, info *arg_info);
extern node *WLAAcode (node *arg_node, info *arg_info);
extern node *WLAAassign (node *arg_node, info *arg_info);
extern node *WLAAlet (node *arg_node, info *arg_info);
extern node *WLAAap (node *arg_node, info *arg_info);
extern node *WLAAid (node *arg_node, info *arg_info);
extern node *WLAAdo (node *arg_node, info *arg_info);
extern node *WLAAcond (node *arg_node, info *arg_info);
extern node *WLAAprf (node *arg_node, info *arg_info);
#endif /* WLAA_DEACTIVATED */
#endif /* _SAC_WL_ACCESS_ANALYZE_H_  */
