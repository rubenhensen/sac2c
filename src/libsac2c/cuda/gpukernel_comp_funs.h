#ifndef _SAC_GPUKERNEL_COMP_FUNS_H_
#define _SAC_GPUKERNEL_COMP_FUNS_H_

#include "types.h"

extern void GKCOcompHostKernelPragma (node *spap,
                                                  unsigned int bnum, char **bounds);
extern gpukernelres_t * GKCOcompGridBlock (node *num, gpukernelres_t *inner);
extern gpukernelres_t * GKCOcompGen ( unsigned bnum, char **bounds, pass_t pass);


#define ARGS( nargs) ARG##nargs
#define ARG0 
#define ARG1 node *arg,

#define WLP(fun, nargs, checkfun)                                                  \
    extern gpukernelres_t *GKCOcomp ## fun ( ARGS( nargs) gpukernelres_t *inner);
#include "gpukernel_funs.mac"
#undef WLP
#undef ARGS
#undef ARG0
#undef ARG1

#endif /* _SAC_GPUKERNEL_COMP_FUNS_H_ */
