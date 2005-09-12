/*
 *
 * $Log$
 * Revision 1.1  2005/09/12 16:19:20  sah
 * Initial revision
 *
 *
 */

#ifndef _SAC_INDEX_ELIMINATE_H_
#define _SAC_INDEX_ELIMINATE_H_
#include "types.h"

extern node *IVEdoIndexVectorElimination (node *syntax_tree);

extern char *IVEchangeId (char *varname, shape *shp);

extern node *IVEfundef (node *arg_node, info *arg_info);
extern node *IVEassign (node *arg_node, info *arg_info);
extern node *IVElet (node *arg_node, info *arg_info);
extern node *IVEprf (node *arg_node, info *arg_info);
extern node *IVEids (node *arg_node, info *arg_info);
extern node *IVEblock (node *arg_node, info *arg_info);
extern node *IVEarg (node *arg_node, info *arg_info);
extern node *IVEwith (node *arg_node, info *arg_info);
extern node *IVEcode (node *arg_node, info *arg_info);

#endif /* _SAC_INDEX_ELIMINATE_H_ */
