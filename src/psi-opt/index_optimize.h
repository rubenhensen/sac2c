/*
 * $Log$
 * Revision 1.4  2005/09/27 20:22:11  sah
 * wlidx offsets are used now whenever possible.
 *
 * Revision 1.3  2005/09/27 02:52:11  sah
 * hopefully, IVE is now back to its own power. Due to
 * lots of other compiler bugs, this code is not fully
 * tested, yet.
 *
 * Revision 1.2  2005/09/15 11:08:52  sah
 * now, wloffsets are used whenever possible
 *
 * Revision 1.1  2005/09/14 21:26:38  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_INDEX_OPTIMIZE_H_
#define _SAC_INDEX_OPTIMIZE_H_

#include "types.h"

extern node *IVEOlet (node *arg_node, info *arg_info);
extern node *IVEOwith (node *arg_node, info *arg_info);
extern node *IVEOcode (node *arg_node, info *arg_info);
extern node *IVEOprf (node *arg_node, info *arg_info);
extern node *IVEOfundef (node *arg_node, info *arg_info);
extern node *IVEOassign (node *arg_node, info *arg_info);

extern node *IVEOdoIndexVectorEliminationOptimisation (node *syntax_tree);
#endif /* _SAC_INDEX_OPTIMIZE_H_ */
