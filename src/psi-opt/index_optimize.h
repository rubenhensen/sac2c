/*
 * $Log$
 * Revision 1.1  2005/09/14 21:26:38  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_INDEX_OPTIMIZE_H_
#define _SAC_INDEX_OPTIMIZE_H_

#include "types.h"

extern node *IVEOprf (node *arg_node, info *arg_info);

extern node *IVEOdoIndexVectorEliminationOptimisation (node *syntax_tree);
#endif /* _SAC_INDEX_OPTIMIZE_H_ */
