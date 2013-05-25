/******************************************************************************
 *
 * Flattengenerator traversal ( flat_tab)
 *
 * Prefix: FLATG
 *
 *****************************************************************************/

#ifndef _SAC_FLATTENGENERATORS_H_
#define _SAC_FLATTENGENERATORS_H_

#include "types.h"

extern node *FLATGdoFlatten (node *syntax_tree);
extern node *FLATGexpression2Avis (node *arg_node, node **vardecs, node **preassigns,
                                   ntype *restype);
extern node *FLATGflattenExprsChain (node *arg_node, node **vardecs, node **preassigns,
                                     ntype *restype);

extern node *FLATGmodule (node *arg_node, info *arg_info);
extern node *FLATGfundef (node *arg_node, info *arg_info);
extern node *FLATGwith (node *arg_node, info *arg_info);
extern node *FLATGpart (node *arg_node, info *arg_info);
extern node *FLATGgenerator (node *arg_node, info *arg_info);
extern node *FLATGassign (node *arg_node, info *arg_info);
extern node *FLATGcond (node *arg_node, info *arg_info);
extern node *FLATGdo (node *arg_node, info *arg_info);
extern node *FLATGwhile (node *arg_node, info *arg_info);
extern node *FLATGfuncond (node *arg_node, info *arg_info);
extern node *FLATGexprs (node *arg_node, info *arg_info);
extern node *FLATGlet (node *arg_node, info *arg_info);
extern node *FLATGprf (node *arg_node, info *arg_info);

#endif /* _SAC_FLATTENGENERATORS_H_ */
