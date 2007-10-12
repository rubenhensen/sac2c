/*
 * $Id$
 */
#ifndef _SAC_INTRODUCE_USER_TRACE_CALLS_H_
#define _SAC_INTRODUCE_USER_TRACE_CALLS_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Introduce User Tracing Calls Traversal ( iutc_tab)
 *
 * Prefix: IUTC
 *
 *****************************************************************************/
extern node *IUTdoIntroduceUserTracingCalls (node *syntax_tree);

extern node *IUTCfundef (node *arg_node, info *arg_info);
extern node *IUTCreturn (node *arg_node, info *arg_info);
extern node *IUTCarg (node *arg_node, info *arg_info);
extern node *IUTCspids (node *arg_node, info *arg_info);
extern node *IUTCassign (node *arg_node, info *arg_info);
extern node *IUTCblock (node *arg_node, info *arg_info);

#endif /* _SAC_INTRODUCE_USER_TRACE_CALLS_H_ */
