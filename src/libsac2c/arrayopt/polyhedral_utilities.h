#ifndef _SAC_PHUT_H_
#define _SAC_PHUT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PHUT
 *
 *****************************************************************************/
extern node *PHUTgenerateAffineExprs (node *arg_node, node *fundef, lut_t *varlut,
                                      int islclass, int loopcount);
extern node *PHUTgenerateAffineExprsForGuard (prf fn, node *arg1, node *arg2,
                                              node *fundef, prf relfn, lut_t *varlut,
                                              int stridesignum);
extern node *PHUTcollectAffineExprsLocal (node *arg_node, node *fundef, lut_t *varlut,
                                          node *res, int islclass, int loopcount);
extern int PHUTcheckIntersection (node *exprspwl, node *exprscwl, node *exprsintr,
                                  node *exprs4, lut_t *varlut, char opcode, char *lhsname,
                                  node *setvaravis);
extern node *PHUTgenerateAffineExprsForPwl (node *arg_node, node *fundef, lut_t *varlut);
extern node *PHUTgenerateAffineExprsForCwl (node *arg_node, node *fundef, lut_t *varlut);
extern node *PHUTgenerateAffineExprsForPwlfIntersect (node *cwliv, node *pwliv,
                                                      lut_t *varlut, node *fundef);
extern node *PHUTcollectWlGenerator (node *arg_node, node *fundef, lut_t *varlut,
                                     node *res, int loopcount);
extern bool PHUTisCompatibleAffineTypes (node *arg_node);
extern bool PHUTisCompatibleAffinePrf (prf nprf);
extern node *PHUTskipChainedAssigns (node *arg_node);
extern int PHUTgetLoopCount (node *exprs, lut_t *varlut);
extern void PHUTwriteUnionSet (FILE *handle, node *exprs, lut_t *varlut, char *tag,
                               bool isrelation, char *lhsname);
extern node *PHUTanalyzeLoopDependentVariable (node *nid, node *rcv, node *fundef,
                                               lut_t *varlut, int loopcount, node *aft);
extern bool PHUTinsertVarIntoLut (node *arg_node, lut_t *varlut, node *fundef,
                                  int islclass);
extern void PHUTsetIslClass (node *arg_node, int islclass);
extern void PHUTsetIslTree (node *avis, node *aft);
extern void PHUTpolyEpilogOne (lut_t *varlut);
extern bool PHUTisPositive (node *arg_node, node *aft, node *fundef, lut_t *varlut);
extern bool PHUTisNegative (node *arg_node, node *aft, node *fundef, lut_t *varlut);
extern bool PHUTisNonPositive (node *arg_node, node *aft, node *fundef, lut_t *varlut);
extern bool PHUTisNonNegative (node *arg_node, node *aft, node *fundef, lut_t *varlut);
extern int PHUTsignum (node *arg, node *aft, node *fundef, lut_t *varlut, node *ids);
extern node *PHUTcollectCondprf (node *fundef, lut_t *varlut, int loopcount);
extern void PHUTprintIslAffineFunctionTree (node *arg_node);
extern node *PHUThandleRelational (int stridesign, node *arg1, node *arg2, prf relprf);

#endif /* _SAC_PHUT_H_ */
