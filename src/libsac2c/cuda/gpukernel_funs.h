#ifndef _SAC_GPUKERNEL_FUNS_H_
#define _SAC_GPUKERNEL_FUNS_H_

#include "types.h"

extern void GKFcheckGpuKernelPragma (node *spap, struct location loc);
extern node *GKFcheckGridBlock (node *args, struct location loc);


extern gpukernelres_t * GKFcompGpuKernelPragma (node *spap,
                                          unsigned int bnum, char **bounds);
extern gpukernelres_t * GKFcompGridBlock (node *num, gpukernelres_t *inner);
extern gpukernelres_t * GKFcompGen ( unsigned bnum, char **bounds);


#define ARGS( nargs) ARG##nargs
#define ARG0 
#define ARG1 node *arg,

#define WLP(checkfun, compfun, str, nargs)                                \
    extern node *checkfun (node *args);                                  \
    extern gpukernelres_t *compfun ( ARGS( nargs) gpukernelres_t *inner);
#include "gpukernel_funs.mac"
#undef WLP
#undef ARGS
#undef ARG0
#undef ARG1

#endif /* _SAC_GPUKERNEL_FUNS_H_ */
