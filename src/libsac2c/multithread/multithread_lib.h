#ifndef _SAC_MULTITHREAD_LIB_H_
#define _SAC_MULTITHREAD_LIB_H_

#include "types.h"

#define MUTH_SPLITPHASE_ENABLED TRUE

extern node *MUTHLIBexpandFundefName (node *fundef, char *prefix);
extern void MUTHLIBtagAllocs (node *withloop, mtexecmode_t executionmode);
extern char *MUTHLIBdecodeExecmode (mtexecmode_t execmode);

#endif /* _SAC_MULTITHREAD_LIB_H_ */
