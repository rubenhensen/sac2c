/*
 * $Log$
 * Revision 1.2  2005/06/28 20:55:43  cg
 * Separate traversal for transforming while-loops into do-loops reactivated.
 *
 * Revision 1.1  2001/04/18 15:38:34  nmw
 * Initial revision
 *
 *
 */

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
