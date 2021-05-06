
#ifndef _SAC_ICM2C_CUDA_H_
#define _SAC_ICM2C_CUDA_H_

#include "types.h"

extern void ICMCompileCUDA_GLOBALFUN_DECL (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileCUDA_GLOBALFUN_DEF_BEGIN (char *funname, unsigned int vararg_cnt,
                                                char **vararg);
extern void ICMCompileCUDA_GLOBALFUN_DEF_END (char *funname, unsigned int vararg_cnt,
                                              char **vararg);
extern void ICMCompileCUDA_GLOBALFUN_AP (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileCUDA_ST_GLOBALFUN_AP (char *funname, unsigned int vararg_cnt, char **vararg);
extern void ICMCompileCUDA_GLOBALFUN_RET (char *funname, unsigned int vararg_cnt, char **vararg);

extern void ICMCompileCUDA_WLIDS (char *wlids_NT, int wlids_NT_dim, int array_dim,
                                  int wlids_dim, char *iv_NT, char *hasstepwith);
extern void ICMCompileCUDA_WLIDXS (char *wlidxs_NT, int wlidxs_NT_dim, char *array_NT,
                                   int array_dim, char **var_ANY);
extern void ICMCompileCUDA_THREADIDX (char *to_NT, int dim, int dim_pos);
extern void ICMCompileCUDA_BLOCKDIM (char *to_NT, int dim, int dim_pos);
extern void ICMCompileCUDA_WL_ASSIGN (char *val_NT, int val_sdim, char *to_NT,
                                      int to_sdim, char *off_NT);
extern void ICMCompileCUDA_MEM_TRANSFER (char *to_NT, char *from_NT, char *basetype,
                                         char *direction);
extern void ICMCompileCUDA_MEM_TRANSFER_START (char *to_NT, char *from_NT, char *basetype,
                                               char *direction);
extern void ICMCompileCUDA_MEM_TRANSFER_END (char *var_NT);
extern void ICMCompileCUDA_MEM_PREFETCH (char *var_NT, char *basetype, int device);
extern void ICMCompileCUDA_THREAD_SPACE (node *spap, unsigned int bnum, char **bounds);
extern void ICMCompileCUDA_INDEX_SPACE (node *spap, unsigned int bnum, char **bounds);
extern void ICMCompileCUDA_WL_SUBALLOC (char *sub_NT, int sub_dim, char *to_NT,
                                        int to_dim, char *off_NT);
extern void ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxA__DATA (char *to_NT, int to_sdim,
                                                         char *from_NT, int from_sdim,
                                                         char *idx_ANY, char *val_array,
                                                         char *basetype);

extern void ICMCompileCUDA_DECL_KERNEL_ARRAY (char *var_NT, char *basetype, int sdim,
                                              int *shp);
extern void ICMCompileCUDA_DECL_SHMEM_ARRAY (char *var_NT, char *basetype, int sdim,
                                             int *shp);
extern void ICMCompileCUDA_SHMEM_BOUNDARY_CHECK (char *to_NT, int dim_pos, char *idx_NT,
                                                 int offset);
extern void ICMCompileCUDA_ASSIGN (char *to_NT, int to_sdim, char *from_NT, int from_sdim,
                                   char *copyfun);

extern void ICMCompileCUDA_COND_WL_ASSIGN (char *cond_NT, char *shmemidx_NT,
                                           char *shmem_NT, char *devidx_NT,
                                           char *devmem_NT);

#endif /* _SAC_ICM2C_CUDA_H_ */
