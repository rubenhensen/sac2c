#ifndef _SAC_GPUKERNEL_CHECK_FUNS_H_
#define _SAC_GPUKERNEL_CHECK_FUNS_H_

#include "types.h"

extern void GKCHcheckGpuKernelPragma (node *spap, struct location loc);
extern node *GKCHcheckGridBlock (node *args, struct location loc);

#define WLP(fun, nargs)                     \
extern node *GKCHcheck ## fun (node *args);
#include "gpukernel_funs.mac"
#undef WLP

#endif /* _SAC_GPUKERNEL_CHECK_FUNS_H_ */
