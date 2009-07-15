/*
 *
 * $Id: undossaivtransform.h 15657 2009-09-24 15:10:10Z rbe $
 *
 */

#ifndef _SAC_UNDOSSAIVTRANSFORM_H_
#define _SAC_UNDOSSAIVTRANSFORM_H_

#include "types.h"

/******************************************************************************
 *
 * Undo SSAIV traversal
 *
 * Prefix: USSAI
 *
 *****************************************************************************/

extern node *USSAIdoUnflattenWLGenerators (node *arg_node);

extern node *USSAIfundef (node *arg_node, info *arg_info);
extern node *USSAIblock (node *arg_node, info *arg_info);
extern node *USSAIvardec (node *arg_node, info *arg_info);
extern node *USSAIwith (node *arg_node, info *arg_info);
extern node *USSAIpart (node *arg_node, info *arg_info);
extern node *USSAIwithid (node *arg_node, info *arg_info);
extern node *USSAIid (node *arg_node, info *arg_info);
extern node *USSAIgenerator (node *arg_node, info *arg_info);
#endif /*  _SAC_UNDOSSAIVTRANSFORM_H_ */
