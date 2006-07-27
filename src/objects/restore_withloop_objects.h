/* $Id$ */

#ifndef _SAC_RESTORE_WITHLOOP_OBJECTS_H_
#define _SAC_RESTORE_WITHLOOP_OBJECTS_H_

#include "types.h"

extern node *RWOAassign (node *arg_node, info *arg_info);
extern node *RWOAblock (node *arg_node, info *arg_info);
extern node *RWOAfundef (node *arg_node, info *arg_info);
extern node *RWOAid (node *arg_node, info *arg_info);
extern node *RWOAlet (node *arg_node, info *arg_info);
extern node *RWOAprf (node *arg_node, info *arg_info);
extern node *RWOAvardec (node *arg_node, info *arg_info);
extern node *RWOAwith2 (node *arg_node, info *arg_info);

extern node *RWOAdoRestoreWithloopObjectAnalysis (node *syntax_tree);

#endif /* _SAC_RESTORE_WITHLOOP_OBJECTS_H_ */
