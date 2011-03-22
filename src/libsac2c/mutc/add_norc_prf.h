/*
 * $Id$
 */
#ifndef _SAC_ADD_NORC_PRF_H_
#define _SAC_ADD_NORC_PRF_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Add NoRC PRF ( anrp_tab)
 *
 * Prefix: ANRP
 *
 *****************************************************************************/
extern node *ANRPdoAddNorcPrf (node *syntax_tree);
extern node *ANRPwith3 (node *arg_node, info *arg_info);
extern node *ANRPassign (node *arg_node, info *arg_info);
extern node *ANRPap (node *arg_node, info *arg_info);
extern node *ANRPid (node *arg_node, info *arg_info);
extern node *ANRPfundef (node *arg_node, info *arg_info);
#endif /* _SAC_ADD_NORC_PRF_H_ */
