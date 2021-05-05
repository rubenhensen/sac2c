#ifndef _SAC_GPUKERNEL_COMP_FUNS_H_
#define _SAC_GPUKERNEL_COMP_FUNS_H_

#include "types.h"

extern void GKCOcompHostKernelPragma (node *spap, unsigned int bnum, char **bounds);
extern void GKCOcompGPUDkernelPragma (node *spap, char* iv_var, unsigned int bnum, char **bounds);
gpukernelres_t* GKCOcompGen(unsigned int bnum, char** bounds, gpukernelres_t* inner);
gpukernelres_t * GKCOcompInvGen ( char* iv_var, char** bounds, gpukernelres_t* res);

char* GKCOvarCreate(gpukernelres_t*gkr, char* postfix);

void GKCOcompCheckStart(gpukernelres_t* res);
void GKCOcompCheckKernel(gpukernelres_t* res);
extern void GKCOcompCheckEnd(void);
void GKCOcompCheckGPUkernelRes(unsigned int bnum, char** bounds, gpukernelres_t* res);

#define ARGS( nargs) ARG##nargs
#define ARG0
#define ARG1 node *arg,

#define WLP(fun, nargs, checkfun)                                                                           \
    gpukernelres_t *GKCOcomp ## fun ( ARGS( nargs) gpukernelres_t *inner);                           \
    gpukernelres_t *GKCOcompInv ## fun ( ARGS( nargs) gpukernelres_t *inner);
#include "gpukernel_funs.mac"
#undef WLP
#undef ARGS
#undef ARG0
#undef ARG1

#endif /* _SAC_GPUKERNEL_COMP_FUNS_H_ */
