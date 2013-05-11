#ifndef _SAC_CHECK_MEM_H_
#define _SAC_CHECK_MEM_H_

#include "types.h"

#ifndef DBUG_OFF
extern void CHKMdeinitialize (void);

extern node *CHKMdoMemCheck (node *syntax_tree);
extern void CHKMtouch (void *shifted_ptr, info *arg_info);

/*
 * CHKMisNode is called for every attribute in the tree
 */
extern void CHKMisNode (void *, nodetype);

extern void CHKMdoNotReport (void *shifted_ptr);

extern void CHKMcheckLeakedMemory (void);

#else
#define CHKMdeinitialize(x)
#define CHKMdoMemCheck(x) x
#define CHKMtouch(x, y)
#define CHKMisNode(x)
#define CHKMdoNotReport(x)
#define CHKMcheckLeakedMemory(x)
#endif

#endif /* _SAC_CHECK_MEM_H_ */
