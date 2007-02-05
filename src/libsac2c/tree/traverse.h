/*
 *
 * $Id$
 *
 */

#ifndef _SAC_TRAVERSE_H_
#define _SAC_TRAVERSE_H_

#include "types.h"

extern node *TRAVdo (node *arg_node, info *arg_info);
extern node *TRAVcont (node *arg_node, info *arg_info);
extern void TRAVpush (trav_t traversal);
extern trav_t TRAVpop ();
extern const char *TRAVgetName ();
extern void TRAVsetPreFun (trav_t traversal, travfun_p prefun);
extern void TRAVsetPostFun (trav_t traversal, travfun_p postfun);

extern char *TRAVtmpVar (void);
extern char *TRAVtmpVarName (char *postfix);

#endif /* _SAC_TRAVERSE_H_ */
