#ifndef _SAC_PHUT_H_
#define _SAC_PHUT_H_

#include "types.h"

/** <!--********************************************************************-->
 *
 * Prefix: PHUT
 *
 *****************************************************************************/

// PolyLib input matrix format is found in Chapter 9 (Example)
// of the Polyulib User's Manual, page 35.

// PolyLib matrix column definitions
#define PLFUN 0
#define PLFUNEQUALITY 0
#define PLFUNINEQUALITY 1
// PLWID is a fake variable that we use to support GENERATOR_WIDTH as an inner loop on W
// E.g., 0 <= W < WIDTH
#define PLWID 1
// PLAPV is a fake variable that we use to represent STEP*N, as part of
// an Arithmetic Progession Vector
#define PLAPV 2
// PLFAKEN is a fake variable used to represent the unknown N in PLAPV.
#define PLFAKEN 3
// PLVARS are the affine variables used in the expression.
// They start at column PLVARS and go upwards, except the last column, which
// holds constant values.
#define PLVARS 4

// PolyLib interface function return codes
// These are ORed together, so more than one result
// can appear at one time.

#define POLY_INVALID 2
#define POLY_UNKNOWN 4
#define POLY_EMPTYSET_ABC 8
#define POLY_EMPTYSET_ABD 16
#define POLY_MATCH_AB 32
#define POLY_EMPTYSET_AB 64

extern bool *PHUTcreateMatrix (unsigned rows, unsigned cols, bool vals);
// above bool are a lie

extern void PHUTclearColumnIndices (node *arg_node, node *fundef);
extern node *PHUTcollectAffineNids (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprs (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprsForGuard (node *arg_node, node *fundef, int *numvars,
                                              prf relfn);
extern node *PHUTcollectAffineExprsLocal (node *arg_node, info *arg_info, node *res);
extern int PHUTcheckIntersection (node *exprs1, node *exprs2, node *exprs3, node *exprs4,
                                  node *idlist, int numvars);
extern node *PHUTgenerateIdentityExprs (int numvars);
extern node *PHUTgenerateAffineExprsForPwl (node *arg_node, node *fundef, int *numvars);
extern node *PHUTgenerateAffineExprsForCwl (node *arg_node, node *fundef, int *numvars);
extern node *PHUTsetClearAvisPart (node *arg_node, node *val);

#endif /* _SAC_PHUT_H_ */
