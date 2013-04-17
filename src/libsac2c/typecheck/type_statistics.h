#ifndef _SAC_TYPE_STATISTICS_H_
#define _SAC_TYPE_STATISTICS_H_

#include "types.h"

extern node *TSdoPrintTypeStatistics (node *arg_node);
extern node *TSfundef (node *arg_node, info *arg_info);
extern node *TSarg (node *arg_node, info *arg_info);
extern node *TSvardec (node *arg_node, info *arg_info);

#endif /* _SAC_TYPE_STATISTICS_H_ */
