
#ifndef _SAC_ICM2C_CUDA_H_
#define _SAC_ICM2C_CUDA_H_

#include "types.h"

extern void ICMCompileCUDA_FUN_AP (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileCUDA_FUN_DEC (char *funname, int vararg_cnt, char **vararg);
extern void ICMCompileCUDA_WLIDS (char *wlids_NT, int wlids_NT_dim, int array_dim,
                                  int wlids_dim, char *hasstepwith);
extern void ICMCompileCUDA_WLIDXS (char *wlidxs_NT, char *array_NT, int array_dim,
                                   char **var_ANY);
extern void ICMCompileCUDA_WL_ASSIGN (char *val_NT, int val_sdim, char *to_NT,
                                      int to_sdim, char *off_NT);
extern void ICMCompileCUDA_MEM_TRANSFER (char *to_NT, char *from_NT, char *basetype,
                                         char *direction);
extern void ICMCompileCUDA_GRID_BLOCK (int bounds_count, char **var_ANY);
extern void ICMCompileCUDA_WL_SUBALLOC (char *sub_NT, int sub_dim, char *to_NT,
                                        int to_dim, char *off_NT);
extern void ICMCompileCUDA_PRF_IDX_SEL__DATA (char *to_NT, int to_sdim, char *from_NT,
                                              int from_sdim, char *idx_ANY,
                                              char *basetype);
extern void ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxS__DATA (char *to_NT, int to_sdim,
                                                         char *from_NT, int from_sdim,
                                                         char *idx_ANY, char *val_scalar,
                                                         char *basetype);
extern void ICMCompileCUDA_PRF_IDX_MODARRAY_AxSxA__DATA (char *to_NT, int to_sdim,
                                                         char *from_NT, int from_sdim,
                                                         char *idx_ANY, char *val_array,
                                                         char *basetype);

/* extern void ICMCompileCUDA_FUN_RET(); */

#endif /* _SAC_ICM2C_CUDA_H_ */