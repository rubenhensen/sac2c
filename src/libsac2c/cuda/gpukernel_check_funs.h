#ifndef _SAC_GPUKERNEL_CHECK_FUNS_H_
#define _SAC_GPUKERNEL_CHECK_FUNS_H_

#include "types.h"

extern void GKCHcheckGpuKernelPragma (node *spap, struct location loc);
extern node *GKCHcheckGridBlock (node *args, struct location loc);
extern void checkArgsLength(node* args, size_t length, const char* name) ;
extern void checkNumLesseqDim(node* args, size_t length, const char* name);
extern void checkLbZero(char* lb_str, node* loc, char* name, size_t dim);

#define WLP(fun, nargs, checkfun)                     \
extern node *GKCHcheck ## fun (node *args);
#include "gpukernel_funs.mac"
#undef WLP

#endif /* _SAC_GPUKERNEL_CHECK_FUNS_H_ */
