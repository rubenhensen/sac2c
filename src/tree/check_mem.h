/*
 * $Id$
 */

/*
 * $Id$ check_mem.h
 */

#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "internal_lib.h"

extern void *CHKMregisterMem (int size, void *aptr);
extern void *CHKMunregisterMem (void *bptr);

extern node *CHKMdoTreeWalk (node *syntax_tree, info *arg_info);
extern void CHKMspaceLeaks ();

extern void CHKMsetNodyType (node *bptr, nodetype newnodetype);
extern int CHKMgetSize (void *orig_address);

#endif /*  _SAC_CHECK_MEM_H_ */
