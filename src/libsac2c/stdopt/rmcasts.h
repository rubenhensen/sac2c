/*****************************************************************************
 *
 * file:   rmcasts.h
 *
 * prefix: RC
 *
 * description:
 *   this module removes all casts from AST
 *
 *****************************************************************************/

#ifndef SAC_RMCASTS_H_
#define SAC_RMCASTS_H_

#include "types.h"

extern node *RCdoRemoveCasts (node *ast);
extern node *RClet (node *arg_node, info *arg_info);
extern node *RCcast (node *arg_node, info *arg_info);
extern node *RCavis (node *arg_node, info *arg_info);
extern node *RCarray (node *arg_node, info *arg_info);
extern node *RCret (node *arg_node, info *arg_info);
extern node *RCobjdef (node *arg_node, info *arg_info);
extern node *RCstructelem (node *arg_node, info *arg_info);
extern node *RCtype (node *arg_node, info *arg_info);

#endif /* SAC_RMCASTS_H_ */
