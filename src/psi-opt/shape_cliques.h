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
extern node *SCIAppendAvisToShapeClique (node *avis1, node *avis2, info *arg_info);

/* utility functions potentially used by other phases */

extern void SCIShapeCliquePrintIDs (node *arg_node, info *arg_info);
extern bool SCIAvisesAreInSameShapeClique (node *avis1, node *avis2);
extern void SCIResetAllShapeCliques (node *arg_node, info *arg_info);
extern node *SCIShapeCliqueCanonicalID (node *avis1);

#endif /* _SAC_SHAPE_CLIQUE_H */
