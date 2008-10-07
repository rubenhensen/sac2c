/*
 *
 * $Id: unflatten.h 15657 2009-09-24 15:10:10Z rbe $
 *
 */

#ifndef _SAC_UNFLATTEN_H_
#define _SAC_UNFLATTEN_H_

#include "types.h"

/******************************************************************************
 *
 * Unflatten traversal
 *
 * Prefix: UFL
 *
 *****************************************************************************/

extern node *UFLdoUnflattenWLGenerators (node *arg_node);

extern node *UFLfundef (node *arg_node, info *arg_info);
extern node *UFLblock (node *arg_node, info *arg_info);
extern node *UFLvardec (node *arg_node, info *arg_info);
extern node *UFLwith (node *arg_node, info *arg_info);
extern node *UFLpart (node *arg_node, info *arg_info);
extern node *UFLwithid (node *arg_node, info *arg_info);
extern node *UFLid (node *arg_node, info *arg_info);
#endif /* _SAC_UNFLATTEN_H_ */
