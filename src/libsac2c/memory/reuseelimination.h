#ifndef _SAC_REUSEELIMINATION_H_
#define _SAC_REUSEELIMINATION_H_

#include "types.h"

/******************************************************************************
 *
 * Reuse elimination traversal ( emre_tab)
 *
 * Prefix: EMRE
 *
 *****************************************************************************/
extern node *EMREdoReuseElimination (node *syntax_tree);

extern node *EMREassign (node *arg_node, info *arg_info);
extern node *EMREblock (node *arg_node, info *arg_info);
extern node *EMREcond (node *arg_node, info *arg_info);
extern node *EMREfundef (node *arg_node, info *arg_info);
extern node *EMREgenarray (node *arg_node, info *arg_info);
extern node *EMRElet (node *arg_node, info *arg_info);
extern node *EMREid (node *arg_node, info *arg_info);
extern node *EMREmodarray (node *arg_node, info *arg_info);
extern node *EMREprf (node *arg_node, info *arg_info);
extern node *EMREvardec (node *arg_node, info *arg_info);

#endif /* _SAC_REUSEELIMINATION_H_ */
