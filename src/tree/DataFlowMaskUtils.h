/*
 *
 * $Log$
 * Revision 3.6  2004/11/30 21:57:31  ktr
 * exxxtreme codebrushing.
 *
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

extern types *DFMUdfm2ReturnTypes (dfmask_t *mask);
extern node *DFMUdfm2Rets (dfmask_t *mask);
extern node *DFMUdfm2Vardecs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2Args (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2ReturnExprs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2ApArgs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2LetIds (dfmask_t *mask, lut_t *lut);

#endif /* _SAC_DATAFLOWMASKUTILS_H_ */
