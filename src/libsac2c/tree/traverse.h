#ifndef _SAC_TRAVERSE_H_
#define _SAC_TRAVERSE_H_

#include "types.h"

#include "traverse_helper.h"

extern node *TRAVdo (node *arg_node, info *arg_info);
extern node *TRAVopt (node *arg_node, info *arg_info);
extern node *TRAVcont (node *arg_node, info *arg_info);

extern lac_info_t *TRAVlacNewInfo (bool);
extern lac_info_t *TRAVlacFreeInfo (lac_info_t *lac_info);
extern bool TRAVlacIsSuccOf (node *succ, node *parent, lac_info_t *lac_info);
extern node *TRAVlacDo (node *arg_node, info *arg_info, lac_info_t *lac_info);
extern node *TRAVlacOpt (node *arg_node, info *arg_info, lac_info_t *lac_info);

extern void TRAVpush (trav_t traversal);
extern void TRAVpushAnonymous (anontrav_t *anontraversal, travfun_p deffun);
extern trav_t TRAVpop (void);
extern const char *TRAVgetName (void);

#ifndef DBUG_OFF
extern void TRAVprintStack (void);
#endif

extern char *TRAVtmpVar (void);
extern char *TRAVtmpVarName (const char *postfix);

#endif /* _SAC_TRAVERSE_H_ */
