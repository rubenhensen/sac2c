/*
 *
 * $Id$
 *
 */

#ifndef _SAC_TRAVERSE_H_
#define _SAC_TRAVERSE_H_

#include "types.h"

#include "traverse_helper.h"

extern node *TRAVdo (node *arg_node, info *arg_info);
extern node *TRAVopt (node *arg_node, info *arg_info);
extern node *TRAVcont (node *arg_node, info *arg_info);

extern lac_info_t *TRAVlacNewInfo ();
extern lac_info_t *TRAVlacFreeInfo (lac_info_t *lac_info);
extern node *TRAVlacDoFun (node *fundef, info *arg_info, lac_info_t *lac_info);
extern node *TRAVlacContBody (node *block, info *arg_info, lac_info_t *lac_info);
extern node *TRAVlacOptNext (node *fundef, info *arg_info, lac_info_t *lac_info);

extern void TRAVpush (trav_t traversal);
extern void TRAVpushAnonymous (anontrav_t *anontraversal, travfun_p deffun);
extern trav_t TRAVpop ();
extern const char *TRAVgetName ();
extern void TRAVsetPreFun (trav_t traversal, travfun_p prefun);
extern void TRAVsetPostFun (trav_t traversal, travfun_p postfun);

#ifndef DBUG_OFF
extern void TRAVprintStack ();
#endif

extern char *TRAVtmpVar (void);
extern char *TRAVtmpVarName (char *postfix);

#endif /* _SAC_TRAVERSE_H_ */
