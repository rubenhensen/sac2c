#include "types.h"

/******************************************************************************
 *
 * While2Do traversal ( w2d_tab)
 *
 * Prefix: W2D
 *
 *****************************************************************************/

#ifndef SAC_WHILE2DO_H_
#define SAC_WHILE2DO_H_

extern node *W2DdoTransformWhile2Do (node *ast);
extern node *W2Dwhile (node *arg_node, info *arg_info);

#endif /* SAC_WHILE2DO_H_ */
