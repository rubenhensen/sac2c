/*
 *
 * $Id$
 */

#ifndef _SAC_PATTERN_MATCH_H_
#define _SAC_PATTERN_MATCH_H_

#include "types.h"
#include "pattern_match_attribs.h"

/** needs to be moved into types.h: */
typedef struct PAT pattern;

/**
 * Design of the shiny new pattern matcher:
 */
extern bool PMmatchExact (pattern *pat, node *expr);
extern bool PMmatchFlat (pattern *pat, node *expr);
extern bool PMmatchFlatSkipExtrema (pattern *pat, node *expr);

/**
 * Pattern-DSL:
 */
extern pattern *PMvar (int num_attribs, ...);
extern pattern *PMconst (int num_attribs, ...);
extern pattern *PMint (int num_attribs, ...);
extern pattern *PMarray (int num_attribs, ...);
extern pattern *PMprf (int num_attribs, ...);
/* extern pattern *PMretryAny( int *i, int *l, int num_pats, ...); */
/* extern pattern *PMretryAll( int *i, int *l, int num_pats, ...); */
extern pattern *PMskip (int num_attribs, ...);
/* extern pattern *PMpair( pattern *p1, pattern *p2); */

/**
 * utils:
 */
extern pattern *PMfree (pattern *p);

/*******************************************************************************
 *******************************************************************************
 **  ATTENTION: all PMO operations are considered obsolete!!!
 **             Please do use the PM versions instead!!!
 **/
extern bool PMO (node *res);
extern node *PMOvar (node **var, node *arg_node);
extern node *PMOlastVar (node **var, node *arg_node);
extern node *PMOlastVarGuards (node **var, node *arg_node);

extern node *PMObool (node *stack);
extern node *PMOchar (node *stack);
extern node *PMOnum (node *stack);
extern node *PMOfloat (node *stack);
extern node *PMOdouble (node *stack);

extern node *PMOboolVal (bool val, node *stack);
extern node *PMOcharVal (char val, node *stack);
extern node *PMOnumVal (int val, node *stack);
extern node *PMOfloatVal (float val, node *stack);
extern node *PMOdoubleVal (double val, node *stack);

extern node *PMOprf (prf fun, node *arg_node);
extern node *PMOarray (constant **frameshape, node **array, node *arg_node);
extern node *PMOarrayConstructor (constant **frameshape, node **array, node *arg_node);
extern node *PMOarrayConstructorGuards (constant **frameshape, node **array,
                                        node *arg_node);
extern node *PMOshapePrimogenitor (node *stack);
extern node *PMOsaashape (node **shp, node **array, node *stack);

extern node *PMOconst (constant **co, node **conode, node *arg_node);
extern node *PMOintConst (constant **co, node **conode, node *arg_node);

extern node *PMOforEachI (node *(*pattern) (int, node *stack), node *stack);

extern node *PMOany (node **expr, node *stack);
extern node *PMOexprs (node **exprs, node *stack);
extern node *PMOpartExprs (node *exprs, node *stack);

/*******************************************************************************
 ******************************************************************************/

#endif /* _SAC_PATTERN_MATCH_H_ */
