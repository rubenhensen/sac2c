/*
 *
 * $Id$
 */

#ifndef _SAC_PATTERN_MATCH_H_
#define _SAC_TREE_BASIC_H_

extern bool PM (node *res);
extern node *PMvar (node **var, node *arg_node);
extern node *PMprf (prf fun, node *arg_node);
extern node *PMconst (node **var, node *arg_node);
extern node *PMarray (node **var, node *arg_node);
extern node *FindConstArray (node *expr);

#endif /* _SAC_TREE_BASIC_H_ */
