/*
 *
 * $Log$
 * Revision 3.5  2004/11/23 11:10:38  khf
 * codebrushing DevCamp
 *
 * Revision 3.4  2004/11/23 10:05:24  sah
 * SaC DevCamp 04
 *
 * Revision 3.3  2004/09/27 10:40:26  sah
 * Added DFM2ProductType and DFM2FunctionType
 * both are needed by lac2fun to generate the
 * ntype function signature for fresh generated
 * LaC-funs
 *
 * Revision 1.1  2000/01/21 16:52:09  dkr
 * Initial revision
 *
 */

#ifndef _SAC_DATAFLOWMASKUTILS_H_
#define _SAC_DATAFLOWMASKUTILS_H_

#include "types.h"

typedef int (*fun_t) (dfmask_t *mask, char *id, node *avis);

extern dfmstack_t *DFMUgenerateDfmStack ();
extern int DFMUisEmptyDfmStack (dfmstack_t *stack);
extern void DFMUremoveDfmStack (dfmstack_t **stack);

extern void DFMUpushDfmStack (dfmstack_t **stack, dfmask_t *mask);
extern dfmask_t *DFMUpopDfmStack (dfmstack_t **stack);

extern void DFMUforeachDfmStack (dfmstack_t *stack, fun_t fun, char *id, node *decl);
extern void DFMUwhileDfmStack (dfmstack_t *stack, fun_t fun, char *id, node *decl);

extern types *DFMUdfm2ReturnTypes (dfmask_t *mask);
extern ntype *DFMUdfm2ProductType (dfmask_t *mask);
extern ntype *DFMUdfm2FunctionType (dfmask_t *in, dfmask_t *out, node *fundef);
extern node *DFMUdfm2Vardecs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2Args (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2ReturnExprs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2ApArgs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2LetIds (dfmask_t *mask, lut_t *lut);

extern dfmask_t *DFMUduplicateMask (dfmask_t *mask, dfmask_base_t *base);

#endif /* _SAC_DATAFLOWMASKUTILS_H_ */
