

#ifndef _DATA_ACCESS_ANALYSIS_H_
#define _DATA_ACCESS_ANALYSIS_H_

#include "types.h"

extern node *DAAdoDataAccessAnalysis (node *syntax_tree);
extern node *DAAfundef (node *arg_node, info *arg_info);
extern node *DAAap (node *arg_node, info *arg_info);
extern node *DAAwith (node *arg_node, info *arg_info);
extern node *DAAassign (node *arg_node, info *arg_info);
extern node *DAApart (node *arg_node, info *arg_info);
extern node *DAAcode (node *arg_node, info *arg_info);
extern node *DAAprf (node *arg_node, info *arg_info);

#endif
