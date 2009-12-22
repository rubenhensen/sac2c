/*
 * $Id$
 */
#ifndef _SAC_CUBESLICER_H_
#define _SAC_CUBESLICER_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Algebraic With-Loop-Folding Cube Slicer traversal
 *
 * Prefix: CUBSL
 *
 *****************************************************************************/
extern node *CUBSLdoAlgebraicWithLoopFoldingCubeSlicing (node *arg_node);
extern node *ExtractNthItem (int itemno, node *idx);
extern node *FindMatchingPart (node *arg_node, info *arg_info, node *consumerpart,
                               node *producerWL);
extern bool matchGeneratorField (node *fa, node *fb);

extern node *CUBSLfundef (node *arg_node, info *arg_info);
extern node *CUBSLwith (node *arg_node, info *arg_info);
extern node *CUBSLpart (node *arg_node, info *arg_info);
extern node *CUBSLcode (node *arg_node, info *arg_info);
extern node *CUBSLlet (node *arg_node, info *arg_info);
extern node *CUBSLprf (node *arg_node, info *arg_info);

#endif /* _SAC_CUBESLICER_H_ */
