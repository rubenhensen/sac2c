#ifndef _SAC_CCTOOLS_H_
#define _SAC_CCTOOLS_H_

#include "types.h"

/******************************************************************************
 *
 * CCtools
 *
 * Prefix: CCT
 *
 *****************************************************************************/

extern char *CCTperformTask (ccm_task_t task);
extern node *CCTrunTools (node *syntax_tree);
extern char *CCTperformTaskCwrapper (ccm_task_t task);

#endif /* _SAC_CCTOOLS_H_ */
