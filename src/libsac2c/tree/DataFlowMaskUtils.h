#ifndef _SAC_DATAFLOWMASKUTILS_H_
#define _SAC_DATAFLOWMASKUTILS_H_

#include "types.h"

extern node *DFMUdfm2Rets (dfmask_t *mask);
extern node *DFMUdfm2Vardecs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2Args (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2ReturnExprs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2ApArgs (dfmask_t *mask, lut_t *lut);
extern node *DFMUdfm2LetIds (dfmask_t *mask, lut_t *lut);

#endif /* _SAC_DATAFLOWMASKUTILS_H_ */
