/*
 *
 * $Log$
 * Revision 1.5  2005/09/27 02:52:11  sah
 * hopefully, IVE is now back to its own power. Due to
 * lots of other compiler bugs, this code is not fully
 * tested, yet.
 *
 * Revision 1.4  2005/08/24 10:19:41  ktr
 * added WLIDXlet
 *
 * Revision 1.3  2005/08/20 23:59:54  ktr
 * Should work...
 *
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

extern node *IVEIdoIndexVectorEliminationInference (node *syntax_tree);
extern node *IVEIprintPreFun (node *arg_node, info *arg_info);

extern node *IVEIprf (node *arg_node, info *arg_info);
extern node *IVEIassign (node *arg_node, info *arg_info);
extern node *IVEIfundef (node *arg_node, info *arg_info);
extern node *IVEIblock (node *arg_node, info *arg_info);
extern node *IVEIavis (node *arg_node, info *arg_info);
extern node *IVEIid (node *arg_node, info *arg_info);
extern node *IVEIap (node *arg_node, info *arg_info);
extern node *IVEIlet (node *arg_node, info *arg_info);
extern node *FindMatchingVarShape (node *avis, node *ivavis);

#endif /* _SAC_INDEX_INFER_H_ */
