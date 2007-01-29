/*
 *
 * $Log$
 * Revision 3.27  2005/03/10 09:41:09  cg
 * Added FREEzombify()
 *
 * Revision 3.26  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 3.25  2004/09/30 19:50:48  sah
 * renamed FreeCSEInfo to FreeCSEinfo
 *
 * Revision 3.24  2004/08/10 13:42:56  sah
 * renamed FreeNWithId to FreeNWithid and
 * added switch to new xml generated free
 *
 */

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
extern node *FREEdoFreeTree (node *arg_node);

extern node *FREEfreeZombie (node *fundef);
extern node *FREEremoveAllZombies (node *arg_node);

/*
 * user functions for non-node data
 */

extern index_info *FREEfreeIndexInfo (index_info *fr);
extern shpseg *FREEfreeShpseg (shpseg *fr);
extern types *FREEfreeOneTypes (types *fr);
extern types *FREEfreeAllTypes (types *fr);
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
