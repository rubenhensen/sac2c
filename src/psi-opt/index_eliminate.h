/*
 *
 * $Log$
 * Revision 1.3  2005/09/23 14:03:22  sah
 * extended index_eliminate to drag index offsets across
 * lacfun boundaries.
 *
 * Revision 1.2  2005/09/15 17:13:56  ktr
 * removed IVE renaming function which was obsolete due to explicit
 * offset variables
 *
 * Revision 1.1  2005/09/12 16:19:20  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_INDEX_ELIMINATE_H_
#define _SAC_INDEX_ELIMINATE_H_
#include "types.h"

extern node *IVEdoIndexVectorElimination (node *syntax_tree);

extern node *IVEfundef (node *arg_node, info *arg_info);
extern node *IVEassign (node *arg_node, info *arg_info);
extern node *IVElet (node *arg_node, info *arg_info);
extern node *IVEprf (node *arg_node, info *arg_info);
extern node *IVEids (node *arg_node, info *arg_info);
extern node *IVEblock (node *arg_node, info *arg_info);
extern node *IVEarg (node *arg_node, info *arg_info);
extern node *IVEwith (node *arg_node, info *arg_info);
extern node *IVEcode (node *arg_node, info *arg_info);
extern node *IVEap (node *arg_node, info *arg_info);

#endif /* _SAC_INDEX_ELIMINATE_H_ */
