/*
 *
 * $Id$
 */

#ifndef _SAC_PATTERN_MATCH_H_
#define _SAC_TREE_BASIC_H_

extern bool PM (node *res);
extern node *PMvar (node **var, node *arg_node);
extern node *PMprf (prf fun, node *arg_node);

extern node *PMconst (constant **co, node **conode, node *arg_node);
extern node *PMintConst (constant **co, node **conode, node *arg_node);
extern node *PMarray (constant **frameshape, node **array, node *arg_node);
extern node *PMarrayConstructor (constant **frameshape, node **array, node *arg_node);

#endif /* _SAC_TREE_BASIC_H_ */
