
#ifndef _SAC_WL_DESCALARIZATION_H_
#define _SAC_WL_DESCALARIZATION_H_

#include "types.h"

/******************************************************************************
 *
 * WLUS traversal ( wlus_tab)
 *
 * prefix: WLUS
 *
 *****************************************************************************/
extern node *WLDSdoWithloopDescalarization (node *fundef);
extern node *WLDSfundef (node *arg_node, info *arg_info);
extern node *WLDSwith (node *arg_node, info *arg_info);
extern node *WLDSlet (node *arg_node, info *arg_info);
extern node *WLDSpart (node *arg_node, info *arg_info);
extern node *WLDSwithid (node *arg_node, info *arg_info);
extern node *WLDSgenerator (node *arg_node, info *arg_info);
// extern node *WLUScode ( node *arg_node, info *arg_info);

#endif /* _SAC_WLUS_H_ */
