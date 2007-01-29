/*
 *
 * $Log$
 * Revision 1.1  2005/09/27 17:23:18  sbs
 * Initial revision
 *
 *
 *
 */

#ifndef _SAC_SIMD_INFER_H_
#define _SAC_SIMD_INFER_H_

#include "types.h"

extern node *SIMDdoInferSIMD (node *syntax_tree);

extern node *SIMDap (node *arg_node, info *arg_info);
extern node *SIMDprf (node *arg_node, info *arg_info);
extern node *SIMDarray (node *arg_node, info *arg_info);
extern node *SIMDwith2 (node *arg_node, info *arg_info);
extern node *SIMDcode (node *arg_node, info *arg_info);
extern node *SIMDwlstride (node *arg_node, info *arg_info);
extern node *SIMDwlgrid (node *arg_node, info *arg_info);

#endif /* _SAC_SIMD_INFER_H_ */
