#ifndef _SAC_GPUKERNEL_FUNS_H_
#define _SAC_GPUKERNEL_FUNS_H_

#include "types.h"

extern void GKFcheckGpuKernelPragma (node *spap, struct location loc);

extern node *GKFcheckGridBlock (node *args, struct location loc);

#define WLP(checkfun, str)                                                              \
    extern node *checkfun (node *args);
#include "gpukernel_funs.mac"
#undef WLP

#endif /* _SAC_GPUKERNEL_FUNS_H_ */
