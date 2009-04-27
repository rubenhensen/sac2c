/*
 *
 * $Id: flattengenerators.h 15657 2007-11-13 13:57:30Z cg $
 *
 */

#ifndef _SAC_FLATTENGENERATORS_H_
#define _SAC_FLATTENGENERATORS_H_

#include "types.h"

/******************************************************************************
 *
 * Flattengenerator traversal ( flat_tab)
 *
 * Prefix: FLATG
 *
 *****************************************************************************/

extern node *FLATGdoFlatten (node *syntax_tree);

extern node *FLATGmodule (node *arg_node, info *arg_info);
extern node *FLATGfundef (node *arg_node, info *arg_info);
extern node *FLATGwith (node *arg_node, info *arg_info);
extern node *FLATGpart (node *arg_node, info *arg_info);
extern node *FLATGgenerator (node *arg_node, info *arg_info);
extern node *FLATGassign (node *arg_node, info *arg_info);

#endif /* _SAC_FLATTENGENERATORS_H_ */