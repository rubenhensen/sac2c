#ifndef _SAC_MTDCR_H_
#define _SAC_MTDCR_H_

#include "types.h"

extern node *MTDCRmodule (node *arg_node, info *arg_info);
extern node *MTDCRfundef (node *arg_node, info *arg_info);
extern node *MTDCRblock (node *arg_node, info *arg_info);
extern node *MTDCRassign (node *arg_node, info *arg_info);
extern node *MTDCRlet (node *arg_node, info *arg_info);
extern node *MTDCRprf (node *arg_node, info *arg_info);
extern node *MTDCRids (node *arg_node, info *arg_info);
extern node *MTDCRid (node *arg_node, info *arg_info);

extern node *MTDCRdoMtDeadCodeRemoval (node *syntax_tree);

#endif /* _SAC_MTDCR_H_ */
