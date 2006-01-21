/*
 * $Id$
 */

#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "types.h"

extern void *CHKMregisterMem (int size, void *aptr);
extern void *CHKMunregisterMem (void *bptr);

extern node *CHKMdoTreeWalk (node *syntax_tree, info *arg_info);
extern node *CHKMspaceLeaks (node *arg_node, info *arg_info);

extern void CHKMsetNodeType (node *bptr, nodetype newnodetype);
extern int CHKMgetSize (void *orig_address);

#endif /* _SAC_CHECK_MEM_H_ */
