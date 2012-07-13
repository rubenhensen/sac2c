/*
 * $Id$
 */

#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "types.h"

#ifndef DBUG_OFF
extern void CHKMdeinitialize (void);

extern node *CHKMdoMemCheck (node *syntax_tree);
extern void CHKMtouch (void *shifted_ptr, info *arg_info);
extern node *CHKMappendErrorNodes (node *arg_node, info *arg_info);

extern void *CHKMregisterMem (int size, void *orig_ptr);
extern void *CHKMunregisterMem (void *shifted_ptr);

extern void CHKMsetNodeType (node *shifted_ptr, nodetype newnodetype);
extern void CHKMsetLocation (void *shifted_ptr, char *file, int line);
extern void CHKMsetSubphase (node *shifted_ptr, char *subphase);
extern void CHKMsetTraversal (node *shifted_ptr, trav_t *traversal);

extern void CHKMdoNotReport (void *shifted_ptr);

extern int CHKMgetSize (void *shifted_ptr);

#else
#define CHKMdeinitialize()
#define CHKMregisterMem(size, orig_ptr) orig_ptr
#define CHKMsetLocation(shifted_ptr, file, line)
#define CHKMsetNodeType(shifted_ptr, newnodetype) ;
#define CHKMgetSize(shifted_ptr) 0
#define CHKMunregisterMem(shifted_ptr) shifted_ptr
#define CHKMappendErrorNodes(arg_node, arg_info)
#define CHKMtouch(shifted_ptr, arg_info)
#define CHKMdoNotReport(shifter_ptr)
#endif /* DBUG_OFF */

#endif /* _SAC_CHECK_MEM_H_ */
