/*****************************************************************************
 *
 * $Id$
 *
 *****************************************************************************/

#ifndef _SAC_DEAD_VARDEC_REMOVAL_H_
#define _SAC_DEAD_VARDEC_REMOVAL_H_

#include "types.h"

extern node *DVRmodule (node *arg_node, info *arg_info);
extern node *DVRfundef (node *arg_node, info *arg_info);
extern node *DVRblock (node *arg_node, info *arg_info);
extern node *DVRvardec (node *arg_node, info *arg_info);
extern node *DVRids (node *arg_node, info *arg_info);
extern node *DVRid (node *arg_node, info *arg_info);

extern node *DVRdoDeadVardecRemoval (node *syntax_tree);

#endif /* _SAC_DEAD_VARDEC_REMOVAL_H_ */
