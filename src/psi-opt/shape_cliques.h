/*
 *
 * $Id$
 *
 */

#ifndef _SAC_SHAPE_CLIQUE_H
#define _SAC_SHAPE_CLIQUE_H

#include "types.h"

extern node *SCIdoShapeCliqueInference (node *syntax_tree);

extern node *SCIgenarray (node *arg_node, info *arg_info);
extern node *SCIlet (node *arg_node, info *arg_info);
extern node *SCImodarray (node *arg_node, info *arg_info);
extern node *SCIfundef (node *arg_node, info *arg_info);
extern node *SCIprf (node *arg_node, info *arg_info);
extern bool SAAAvisesAreSameShape (node *avis1, node *avis2);

/* utility functions potentially used by other phases */

extern bool SCIAvisesAreInSameShapeClique (node *avis1, node *avis2);
extern node *SCIFindMarkedAvisInSameShapeClique (node *avis1);
extern int SCIShapeCliqueNumber (node *avis, node *arg_node);
extern node *SCIfindShapeCliqueForShape (shape *shp, node *arg_node);

#endif /* _SAC_SHAPE_CLIQUE_H */
