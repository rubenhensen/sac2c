#ifndef _SAC_TRAVERSE_INJECT_H_
#define _SAC_TRAVERSE_INJECT_H_

#include "types.h"

extern travfunlist_t *TRAVmakeTravFunList (travfun_p fun);
extern travfunlist_t *TRAVappendTravFunList (travfunlist_t *funlist,
                                             travfunlist_t *newfunlist);
extern travfunlist_t *TRAVremoveTravFunListFun (travfunlist_t *funlist, travfun_p fun);
extern travfunlist_t *TRAVfreeTravFunList (travfunlist_t *funlist);
extern travfunlist_t *TRAVfreeTravFunListNode (travfunlist_t *funlist);

extern void TRAVaddPreFun (trav_t traversal, travfun_p prefun);
extern void TRAVaddPostFun (trav_t traversal, travfun_p postfun);
extern void TRAVremovePreFun (trav_t traversal, travfun_p prefun);
extern void TRAVremovePostFun (trav_t traversal, travfun_p postfun);

#endif /* _SAC_TRAVERSE_INJECT_H_ */
