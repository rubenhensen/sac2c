
#ifndef _SAC_CUDA_UTILS_H_
#define _SAC_CUDA_UTILS_H_

#include "types.h"

#define ISHOST2DEVICE(assign)                                                            \
    (assign == NULL                                                                      \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                     \
            ? FALSE                                                                      \
            : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                  \
                 ? FALSE                                                                 \
                 : (PRF_PRF (ASSIGN_RHS (assign)) == F_host2device ? TRUE : FALSE))))

#define ISDEVICE2HOST(assign)                                                            \
    (assign == NULL                                                                      \
       ? FALSE                                                                           \
       : (NODE_TYPE (ASSIGN_INSTR (assign)) != N_let                                     \
            ? FALSE                                                                      \
            : (NODE_TYPE (ASSIGN_RHS (assign)) != N_prf                                  \
                 ? FALSE                                                                 \
                 : (PRF_PRF (ASSIGN_RHS (assign)) == F_device2host ? TRUE : FALSE))))

extern node *CUnthApArg (node *args, int n);
extern simpletype CUh2dSimpleTypeConversion (simpletype sty);
extern simpletype CUd2hSimpleTypeConversion (simpletype sty);
extern simpletype CUd2shSimpleTypeConversion (simpletype sty);
extern bool CUisDeviceTypeNew (ntype *ty);
extern bool CUisShmemTypeNew (ntype *ty);
extern bool CUisDeviceTypeOld (types *ty);
extern bool CUisShmemTypeOld (types *ty);
extern bool CUisDeviceArrayTypeNew (ntype *ty);

#endif
