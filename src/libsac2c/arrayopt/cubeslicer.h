/*
 * $Id$
 */
#ifndef _SAC_CUBESLICER_H_
#define _SAC_CUBESLICER_H_

#include "types.h"

typedef enum {
    INTERSECT_unknown,
    INTERSECT_null,
    INTERSECT_notnull,
    INTERSECT_sliceneeded,
    INTERSECT_exact,
} intersect_type_t;

/** <!--********************************************************************-->
 *
 * Algebraic With-Loop-Folding Cube Slicer traversal
 *
 * Prefix: CUBSL
 *
 *****************************************************************************/
extern node *CUBSLdoAlgebraicWithLoopFoldingCubeSlicing (node *arg_node);
extern intersect_type_t CUBSLfindMatchingPart (node *arg_node, node *consumerpart,
                                               node *producerWL, info *arg_info,
                                               node **producerpart);
extern bool matchGeneratorField (node *fa, node *fb);

extern node *CUBSLfundef (node *arg_node, info *arg_info);
extern node *CUBSLassign (node *arg_node, info *arg_info);
extern node *CUBSLlet (node *arg_node, info *arg_info);
extern node *CUBSLwith (node *arg_node, info *arg_info);
extern node *CUBSLpart (node *arg_node, info *arg_info);
extern node *CUBSLprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUBESLICER_H_ */
