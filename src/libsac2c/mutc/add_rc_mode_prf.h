/*
 * $Id$
 */
#ifndef _SAC_ADD_NORC_PRF_H_
#define _SAC_ADD_NORC_PRF_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Add RC Mode PRF ( armp_tab)
 *
 * Prefix: ARMP
 *
 *****************************************************************************/
extern node *ARMPdoAddNorcPrf (node *syntax_tree);
extern node *ARMPwith3 (node *arg_node, info *arg_info);
extern node *ARMPassign (node *arg_node, info *arg_info);
extern node *ARMPap (node *arg_node, info *arg_info);
extern node *ARMPid (node *arg_node, info *arg_info);
extern node *ARMPfundef (node *arg_node, info *arg_info);
#endif /* _SAC_ADD_NORC_PRF_H_ */
