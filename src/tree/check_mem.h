/*
 * $Id$
 */

#ifdef SHOW_MALLOC

#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "types.h"

extern node *CHKMdoCheckMemory (node *syntax_tree);
extern node *CHKMassignSpaceLeaks (node *arg_node, info *arg_info);

extern void *CHKMregisterMem (int size, void *orig_ptr);
extern void *CHKMunregisterMem (void *shifted_ptr);

extern void CHKMsetNodeType (node *shifted_ptr, nodetype newnodetype);
extern void CHKMsetLocation (node *shifted_ptr, char *file, int line);
extern void CHKMsetSubphase (node *shifted_ptr, char *subphase);
extern void CHKMsetTraversal (node *shifted_ptr, trav_t *traversal);

extern int CHKMgetSize (node *shifted_ptr);

#endif /* _SAC_CHECK_MEM_H_ */

#endif /* SHOW_MALLOC */
