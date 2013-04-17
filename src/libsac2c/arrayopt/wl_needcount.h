#ifndef _SAC_WL_NEEDCOUNT_H_
#define _SAC_WL_NEEDCOUNT_H_

#include "types.h"

/******************************************************************************
 *
 * Needcount of With Loop traversal ( wlnc_tab)
 *
 * prefix: WLNC
 *
 *****************************************************************************/
extern node *WLNCdoWLNeedCount (node *fundef);

extern node *WLNCfundef (node *arg_node, info *arg_info);
extern node *WLNCblock (node *arg_node, info *arg_info);
extern node *WLNCavis (node *arg_node, info *arg_info);
extern node *WLNCwith (node *arg_node, info *arg_info);
extern node *WLNCpart (node *arg_node, info *arg_info);
extern node *WLNCcode (node *arg_node, info *arg_info);
extern node *WLNCprf (node *arg_node, info *arg_info);
extern node *WLNCap (node *arg_node, info *arg_info);
extern node *WLNCid (node *arg_node, info *arg_info);

#endif /* _SAC_WL_NEEDCOUNT_H_ */
