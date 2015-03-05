#ifndef _SAC_PHUT_H_
#define _SAC_PHUT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PHUT
 *
 *****************************************************************************/

extern bool *PHUTcreateMatrix (unsigned rows, unsigned cols, bool vals);
// above bool are a lie

extern void PHUTclearColumnIndices (node *arg_node, node *fundef);
extern node *PHUTcollectAffineNids (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprs (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprsForGuard (node *arg_node, node *fundef, int *numvars);
extern node *PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info);
extern bool PHUTcheckIntersection (node *exprs1, node *exprs2, node *exprs3,
                                   node *idlist);
extern node *PHUTgenerateIdentityExprs (int numvars);

// POLYLIB input matrix definitions, as found in Chapter 9 (Example)
// of the Polyulib User's Manual, page 35.

#define PLEQUALITY 0
#define PLINEQUALITY 1

#endif /* _SAC_PHUT_H_ */
