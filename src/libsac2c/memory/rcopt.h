#ifndef _SAC_RCOPT_H_
#define _SAC_RCOPT_H_

#include "types.h"

/******************************************************************************
 *
 * Reference counting optimizations traversal (emrco_tab)
 *
 * prefix: EMRCO
 *
 *****************************************************************************/
extern node *EMRCOdoRefCountOpt (node *syntax_tree);

extern node *EMRCOassign (node *arg_node, info *arg_info);
extern node *EMRCOblock (node *arg_node, info *arg_info);
extern node *EMRCOfold (node *arg_node, info *arg_info);
extern node *EMRCOfundef (node *arg_node, info *arg_info);
extern node *EMRCOgenarray (node *arg_node, info *arg_info);
extern node *EMRCOlet (node *arg_node, info *arg_info);
extern node *EMRCOmodarray (node *arg_node, info *arg_info);
extern node *EMRCOprf (node *arg_node, info *arg_info);

#endif /* _SAC_RCOPT_H_ */
