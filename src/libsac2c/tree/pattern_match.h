/*
 *
 * $Id$
 */

#ifndef _SAC_PATTERN_MATCH_H_
#define _SAC_TREE_BASIC_H_

extern bool PM (node *res);
extern node *PMvar (node **var, node *arg_node);
extern node *PMprf (prf fun, node *arg_node);

extern node *PMconst_new (constant **co, node **conode, node *arg_node);
extern node *PMintConst_new (constant **co, node **conode, node *arg_node);
extern node *PMarray_new (constant **frameshape, node **array, node *arg_node);

extern node *PMarray (node **var, node *arg_node);
extern node *PMconst (node **var, node *arg_node);
extern node *PMintConst (node **var, node *arg_node);
extern node *PMfollowId (node *arg_node);

#endif /* _SAC_TREE_BASIC_H_ */
