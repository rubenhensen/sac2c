/* $Id$ */

#ifndef _SAC_WITHLOOP_OBJECTS_H_
#define _SAC_WITHLOOP_OBJECTS_H_

#include "types.h"

extern node *WOAid (node *arg_node, info *arg_info);
extern node *WOAlet (node *arg_node, info *arg_info);
extern node *WOAwith (node *arg_node, info *arg_info);

extern node *WOAdoWithloopObjectAnalysis (node *syntax_tree);

#endif /* _SAC_WITHLOOP_OBJECTS_H_ */
