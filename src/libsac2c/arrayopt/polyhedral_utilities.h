#ifndef _SAC_PHUT_H_
#define _SAC_PHUT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PHUT
 *
 *****************************************************************************/

// POLYLIB input matrix definitions, as found in Chapter 9 (Example)
// of the Polyulib User's Manual, page 35.

#define PLEQUALITY 0
#define PLINEQUALITY 1

extern bool *PHUTcreateMatrix (unsigned rows, unsigned cols, bool vals);
// above bool are a lie

extern void PHUTclearColumnIndices (node *arg_node, node *fundef);
extern node *PHUTcollectAffineNids (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprs (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprsForGuard (node *arg_node, node *fundef, int *numvars,
                                              prf relfn);
extern node *PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info);
extern int PHUTcheckIntersection (node *exprs1, node *exprs2, node *exprs3, node *exprs4,
                                  node *idlist, int numvars);
extern node *PHUTgenerateIdentityExprs (int numvars);
extern node *PHUTgenerateAffineExprsForPwl (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprsForCwl (node *arg_node, node *fundef, int *numvars);
extern node *PHUTsetClearAvisPart (node *arg_node, node *val);

#endif /* _SAC_PHUT_H_ */
