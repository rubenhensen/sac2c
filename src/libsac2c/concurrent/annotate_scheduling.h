/*****************************************************************************
 *
 * $Id$
 *
 * Schedule travesal (sched_tab)
 *
 * prefix: SCHED
 *
 *****************************************************************************/

#ifndef _SAC_ANNOTATE_SCHEDULING_H_
#define _SAC_ANNOTATE_SCHEDULING_H_

#include "types.h"

extern node *MTASdoAnnotateScheduling (node *syntax_tree);

extern node *MTASmodule (node *arg_node, info *arg_info);
extern node *MTASfundef (node *arg_node, info *arg_info);
extern node *MTASwith2 (node *arg_node, info *arg_info);
extern node *MTASwlseg (node *arg_node, info *arg_info);
extern node *MTASwlsegvar (node *arg_node, info *arg_info);

#endif /* _SAC_ANNOTATE_SCHEDULING_H_ */
