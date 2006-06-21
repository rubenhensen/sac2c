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
extern node *SCIAppendAvisToShapeClique (node *avis1, node *avis2);
extern void SCIBuildAKSShapeCliques (node *arg_node);
extern void SCIBuildAKSShapeCliqueOne (node *arg_node, node *avis);
extern bool SCIeqShapes (ntype *a, ntype *b);

/* utility functions potentially used by other phases */

extern void SCIShapeCliquePrintIDs (node *arg_node);
extern void SCIPrintShapeCliqueNames (node *avis, bool printall);
extern bool SCIAvisesAreInSameShapeClique (node *avis1, node *avis2);
extern void SCIResetAllShapeCliques (node *arg_node, info *arg_info);
extern void SCIResetIsSCIPrinted (node *arg_node);

#endif /* _SAC_SHAPE_CLIQUE_H */
