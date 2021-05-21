#ifndef _SAC_PATTERN_MATCH_H_
#define _SAC_PATTERN_MATCH_H_

#ifndef DBUG_OFF
#define PMSTART "> %2d: "
#define PMINDENT "      "
#define PMEND "< %2d "
#endif

#include "types.h"

#include "pattern_match_attribs.h"
#include "pattern_match_modes.h"

/**
 * Design of the shiny new pattern matcher:
 */
extern bool PMmatch (pattern *pat, pm_mode_t *pm_mode, node *expr);

extern bool PMmatchExact (pattern *pat, node *expr);
extern bool PMmatchFlat (pattern *pat, node *expr);
extern bool PMmatchFlatSkipExtrema (pattern *pat, node *expr);
extern bool PMmatchFlatSkipGuards (pattern *pat, node *expr);
extern bool PMmatchFlatSkipExtremaAndGuards (pattern *pat, node *expr);
extern bool PMmatchFlatWith (pattern *pat, node *expr);

extern node *PMmultiExprs (int num_nodes, ...);

/**
 * Pattern-DSL:
 */
extern pattern *PMvar (unsigned int num_attribs, ...);

extern pattern *PMparam (unsigned int num_attribs, ...);
extern pattern *PMany (unsigned int num_attribs, ...);
extern pattern *PMtrue (unsigned int num_attribs, ...);
extern pattern *PMfalse (unsigned int num_attribs, ...);
extern pattern *PMconst (unsigned int num_attribs, ...);
extern pattern *PMint (unsigned int num_attribs, ...);
extern pattern *PMarray (unsigned int num_attribs, ...);
extern pattern *PMprf (unsigned int num_attribs, ...);
extern pattern *PMretryAny (int *i, int *l, unsigned int num_pats, ...);
extern pattern *PMretryAll (int *i, int *l, unsigned int num_pats, ...);
extern pattern *PMskip (unsigned int num_attribs, ...);
extern pattern *PMskipN (int *n, unsigned int num_attribs, ...);
extern pattern *PMmulti (unsigned int num_pats, ...);
extern pattern *PMwith (unsigned int num_attribs, ...);
extern pattern *PMwith3 (unsigned int num_attribs, ...);
extern pattern *PMrange (unsigned int num_attribs, ...);
extern pattern *PMlink (unsigned int num_attribs, ...);
/**
 * selectors
 */
extern pattern *PMSrange (unsigned int num_attribs, ...);

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
extern node *PMOnumbyte (node *stack);
extern node *PMOnumubyte (node *stack);
extern node *PMOnumint (node *stack);
extern node *PMOnumuint (node *stack);
extern node *PMOnumshort (node *stack);
extern node *PMOnumushort (node *stack);
extern node *PMOnumlong (node *stack);
extern node *PMOnumulong (node *stack);
extern node *PMOnumlonglong (node *stack);
extern node *PMOnumulonglong (node *stack);

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
