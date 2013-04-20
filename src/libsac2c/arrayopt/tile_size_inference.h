/*****************************************************************************
 *
 * file:   tile_size_inference.h
 *
 * prefix: TSI
 *
 * description:
 *
 *   This compiler module realizes an inference scheme for the selection
 *   of appropriate tile sizes. This is used by the code generation in
 *   order to create tiled target code.
 *
 *
 *****************************************************************************/

#ifndef _SAC_TILE_SIZE_INFERENCE_H_
#define _SAC_TILE_SIZE_INFERENCE_H_

#include "types.h"

#define NUM_OF_CACHEPARAM 3
#define CSIZE_INDEX 0
#define LSIZE_INDEX 1
#define DTYPE_INDEX 2

extern node *TSIdoTileSizeInference (node *arg_node);

#ifndef TSI_DEACTIVATED
extern node *TSIfundef (node *arg_node, info *arg_info);
extern node *TSIblock (node *arg_node, info *arg_info);
extern node *TSInwith (node *arg_node, info *arg_info);
extern node *TSIncode (node *arg_node, info *arg_info);
#endif /* TSI_DEACTIVATED */
#endif /* _SAC_TILE_SIZE_INFERENCE_H_  */
