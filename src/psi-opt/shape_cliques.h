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
extern int SCIAppendAvisToShapeClique (node *avis1, node *avis2, info *arg_info);
extern dfmask_t *SCICreateShapeClique (node *avis1, info *arg_info);

extern void SCIDestroyShapeCliques (node *arg_node, info *arg_info);
extern void SCIInitializeShapeCliques (node *arg_node, info *arg_info);
extern dfmask_t *SCIShapeCliqueIDToCliquePtr (int ShapeCliqueID, info *arg_info);

#endif /* _SAC_SHAPE_CLIQUE_H */
