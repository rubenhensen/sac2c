/*
 *
 * $Id$
 */

#ifndef _SAC_PATTERN_MATCH_H_
#define _SAC_TREE_BASIC_H_

extern bool PM (node *res);
extern node *PMvar (node **var, node *arg_node);

extern node *PMbool (node *stack);
extern node *PMchar (node *stack);
extern node *PMnum (node *stack);
extern node *PMfloat (node *stack);
extern node *PMdouble (node *stack);

extern node *PMprf (prf fun, node *arg_node);
extern node *PMarray (constant **frameshape, node **array, node *arg_node);
extern node *PMarrayConstructor (constant **frameshape, node **array, node *arg_node);
extern node *PMsaashape (node **shp, node **array, node *stack);

extern node *PMconst (constant **co, node **conode, node *arg_node);
extern node *PMintConst (constant **co, node **conode, node *arg_node);

extern node *PMforEachI (node *(*pattern) (int, node *stack), node *stack);
extern node *PMexprsEqual (node *exprs, node *stack);

extern node *PManyExpr (node *stack);
extern node *PManyExprs (node *stack);

#endif /* _SAC_TREE_BASIC_H_ */
