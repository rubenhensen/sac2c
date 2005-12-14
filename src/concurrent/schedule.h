#ifndef _SAC_SCHEDULE_H_
#define _SAC_SCHEDULE_H_

#include "types.h"

/*****************************************************************************
 *
 * $Id$
 *
 * Schedule travesal (sched_tab)
 *
 * prefix: SCHED
 *
 *****************************************************************************/
extern node *SCHEDwlseg (node *arg_node, info *arg_info);
extern node *SCHEDwlsegvar (node *arg_node, info *arg_info);
extern node *SCHEDsync (node *arg_node, info *arg_info);
extern node *SCHEDwith2 (node *arg_node, info *arg_info);

#endif /* _SAC_SCHEDULE_H_ */
