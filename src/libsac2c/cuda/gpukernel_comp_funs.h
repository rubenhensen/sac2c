#ifndef _SAC_GPUKERNEL_COMP_FUNS_H_
#define _SAC_GPUKERNEL_COMP_FUNS_H_

#include "types.h"

extern void GKCOcompHostKernelPragma (node *spap, unsigned int bnum, char **bounds);
extern void GKCOcompGPUDkernelPragma (node *spap, char* iv_var, unsigned int bnum, char **bounds);
void GKCOcompCheckGPUkernelRes(unsigned int bnum, char** bounds, gpukernelres_t* res);
gpukernelres_t* GKCOcompGen(unsigned int bnum, char** bounds, gpukernelres_t* inner);
extern gpukernelres_t * GKCOcompInvGen ( char* iv_var, char** bounds, gpukernelres_t* res);

char* GKCOvarCreate(gpukernelres_t*gkr, char* postfix);

#define ARGS( nargs) ARG##nargs
#define ARG0
#define ARG1 node *arg,

#define WLP(fun, nargs, checkfun)                                                                           \
    extern gpukernelres_t *GKCOcomp ## fun ( ARGS( nargs) gpukernelres_t *inner);                           \
    extern gpukernelres_t *GKCOcompInv ## fun ( ARGS( nargs) gpukernelres_t *inner);
#include "gpukernel_funs.mac"
#undef WLP
#undef ARGS
#undef ARG0
#undef ARG1

#endif /* _SAC_GPUKERNEL_COMP_FUNS_H_ */
