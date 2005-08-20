/*
 *
 * $Log$
 * Revision 1.2  2005/08/20 19:08:02  ktr
 * starting brushing
 *
 * Revision 1.1  2005/07/16 19:00:20  sbs
 * Initial revision
 *
 *
 */

#ifndef _SAC_INDEX_INFER_H_
#define _SAC_INDEX_INFER_H_

#include "types.h"

extern node *IVEdoIndexVectorEliminationInference (node *syntax_tree);

extern node *IVEIprf (node *arg_node, info *arg_info);
extern node *IVEIassign (node *arg_node, info *arg_info);

#endif /* _SAC_INDEX_INFER_H_ */
