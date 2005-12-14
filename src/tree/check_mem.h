/*
 * $Id$
 */

/*
 * $Id$ check_mem.h
 */

#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "internal_lib.h"

extern void *CMregisterMem (int size, void *aptr);
extern void *CMunregisterMem (void *bptr);

extern int CMgetSize (void *orig_address);
extern void CMsetNodyType (node *bptr, nodetype newnodetype);

#endif /*  _SAC_CHECK_MEM_H_ */
