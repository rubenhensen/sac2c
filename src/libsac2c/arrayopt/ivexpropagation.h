/*
 * $Id: ivexpropagation.h 15815 2008-10-24 18:04:47Z rbe $
 */
#ifndef _SAC_INDEX_VECTOR_EXTREMA_PROPAGATION_H_
#define _SAC_INDEX_VECTOR_EXTREMA_PROPAGATION_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Template traversal ( ivexp_tab)
 *
 * Prefix: IVEXP
 *
 *****************************************************************************/
extern node *IVEXPdoIndexVectorExtremaProp (node *arg_node);
void IVEXPsetExtremumIfNotNull (node **snk, node *src);
extern node *IVEXPadjustExtremaBound (node *arg_node, info *arg_info, int k,
                                      node **vardecs, node **preassigns, char *tagit);
extern bool IVEXPisCheckWithids (node *exprs, node *curwith);
void IVEXPsetMinvalIfNotNull (node *snk, node *src, bool dup, node *arg1);
void IVEXPsetMaxvalIfNotNull (node *snk, node *src, bool dup, node *arg1);

extern node *IVEXPmodule (node *arg_node, info *arg_info);
extern node *IVEXPfundef (node *arg_node, info *arg_info);
extern node *IVEXPassign (node *arg_node, info *arg_info);
extern node *IVEXPlet (node *arg_node, info *arg_info);
extern node *IVEXPwith (node *arg_node, info *arg_info);
extern node *IVEXPcond (node *arg_node, info *arg_info);
extern node *IVEXPfuncond (node *arg_node, info *arg_info);
extern node *IVEXPpart (node *arg_node, info *arg_info);
extern node *IVEXPwhile (node *arg_node, info *arg_info);

#endif // _SAC_INDEX_VECTOR_EXTREMA_PROPAGATION_H_
