/*
 *
 * $Log$
 * Revision 3.10  2005/08/20 20:08:32  sah
 * IVE-rewrite: skeleton implementation
 *
 */

#ifndef _SAC_INDEX_H_
#define _SAC_INDEX_H_

#include "types.h"

extern node *IVEdoIndexVectorElimination (node *syntax_tree);

extern char *IVEchangeId (char *varname, shape *shp);

extern node *IVEfundef (node *arg_node, info *arg_info);
extern node *IVEassign (node *arg_node, info *arg_info);
extern node *IVEprf (node *arg_node, info *arg_info);
extern node *IVEids (node *arg_node, info *arg_info);
extern node *IVEap (node *arg_node, info *arg_info);

#endif /* _SAC_INDEX_H_ */
