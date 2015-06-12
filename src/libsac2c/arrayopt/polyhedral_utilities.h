#ifndef _SAC_PHUT_H_
#define _SAC_PHUT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PHUT
 *
 *****************************************************************************/
extern node *PHUTgenerateAffineExprs (node *arg_node, node *fundef, lut_t *varlut);
extern node *PHUTgenerateAffineExprsForGuard (prf fn, node *arg1, node *arg2,
                                              node *fundef, prf relfn, lut_t *varlut);
extern node *PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info, node *res);
extern int PHUTcheckIntersection (node *exprs1, node *exprs2, node *exprs3, node *exprs4,
                                  node *exprsuf, node *exprsuc, lut_t *varlut,
                                  char opcode);
extern node *PHUTgenerateAffineExprsForPwl (node *arg_node, node *fundef, lut_t *varlut);
extern node *PHUTgenerateAffineExprsForCwl (node *arg_node, node *fundef, lut_t *varlut);
extern node *PHUTgenerateAffineExprsForPwlfIntersect (node *cwliv, node *pwliv,
                                                      lut_t *varlut);
extern node *PHUTcollectWlGenerator (node *arg_node, info *arg_info, node *res);
extern bool PHUTisCompatibleAffineTypes (node *arg_node);
extern bool PHUTisCompatibleAffinePrf (prf nprf);
extern node *PHUTskipChainedAssigns (node *arg_node);

#endif /* _SAC_PHUT_H_ */
