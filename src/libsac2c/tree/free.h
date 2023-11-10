#ifndef _SAC_FREE_H_
#define _SAC_FREE_H_

#include "types.h"

/*
 * Top-level free functions.
 *
 * These are the only functions which should be called from outside
 * the free module for freeing syntax (sub) trees.
 */

extern node *FREEdoFreeNode (node *arg_node);
extern node *FREEoptFreeNode (node *arg_node);
extern node *FREEdoFreeTree (node *arg_node);
extern node *FREEoptFreeTree (node *arg_node);

extern node *FREEfreeZombie (node *fundef);
extern node *FREEremoveAllZombies (node *arg_node);

/*
 * user functions for non-node data
 */

extern index_info *FREEfreeIndexInfo (index_info *fr);
extern nodelist *FREEfreeNodelist (nodelist *fr);
extern nodelist *FREEfreeNodelistNode (nodelist *nl);
extern access_t *FREEfreeOneAccess (access_t *fr);
extern access_t *FREEfreeAllAccess (access_t *fr);
extern argtab_t *FREEfreeArgtab (argtab_t *argtab);

/*
 * special purpose functions
 */

extern node *FREEzombify (node *arg_node);

/*
 * traversal functions
 */

#include "free_node.h"

#endif /* _SAC_FREE_H_ */
