/*
 *
 * $Log$
 * Revision 1.1  2003/09/20 14:20:20  dkr
 * Initial revision
 *
 */

#ifndef _icm2c_prf_h_
#define _icm2c_prf_h_

#include "types.h"

#ifdef TAGGED_ARRAYS

extern void ICMCompileND_PRF_SHAPE__DATA (char *to_NT, int to_sdim, char *from_NT,
                                          int from_sdim);

extern void ICMCompileND_PRF_RESHAPE__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT);

extern void ICMCompileND_PRF_RESHAPE__SHAPE_arr (char *to_NT, int to_sdim, int shp_size,
                                                 char **shpa_ANY);

extern void ICMCompileND_PRF_SEL__SHAPE_id (char *to_NT, int to_sdim, char *from_NT,
                                            int from_sdim, char *idx_NT);

extern void ICMCompileND_PRF_SEL__SHAPE_arr (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, int idx_size,
                                             char **idxs_ANY);

extern void ICMCompileND_PRF_SEL__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                           int from_sdim, char *idx_NT, int idx_size,
                                           char *copyfun);

extern void ICMCompileND_PRF_SEL__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                            int from_sdim, int idx_size, char **idxs_ANY,
                                            char *copyfun);

extern void ICMCompileND_PRF_MODARRAY__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                                int from_sdim, char *idx_NT, int idx_size,
                                                char *val_ANY, char *copyfun);

extern void ICMCompileND_PRF_MODARRAY__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                                 int from_sdim, int idx_size,
                                                 char **idxs_ANY, char *val_ANY,
                                                 char *copyfun);

extern void ICMCompileND_PRF_IDX_SEL__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, char *idx_ANY);

extern void ICMCompileND_PRF_IDX_SEL__DATA (char *to_NT, int to_sdim, char *from_NT,
                                            int from_sdim, char *idx_ANY, char *copyfun);

extern void ICMCompileND_PRF_IDX_MODARRAY__DATA (char *to_NT, int to_sdim, char *from_NT,
                                                 int from_sdim, char *idx_ANY,
                                                 char *val_ANY, char *copyfun);

extern void ICMCompileND_PRF_TAKE__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                          int from_sdim, char *cnt_ANY);

extern void ICMCompileND_PRF_TAKE__DATA (char *to_NT, int to_sdim, char *from_NT,
                                         int from_sdim, char *cnt_ANY);

extern void ICMCompileND_PRF_DROP__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                          int from_sdim, char *cnt_ANY);

extern void ICMCompileND_PRF_DROP__DATA (char *to_NT, int to_sdim, char *from_NT,
                                         int from_sdim, char *cnt_ANY);

extern void ICMCompileND_PRF_CAT__SHAPE (char *to_NT, int to_sdim, char *from1_NT,
                                         int from1_sdim, char *from2_NT, int from2_sdim);

#else /* TAGGED_ARRAYS */

extern void ICMCompileND_KD_SEL_CxA_S (char *a, char *res, int dim, char **vi);

extern void ICMCompileND_KD_SEL_CxA_A (int dima, char *a, char *res, int dimv, char **vi);

extern void ICMCompileND_KD_SEL_VxA_S (char *a, char *res, int dim, char *v);

extern void ICMCompileND_KD_SEL_VxA_A (int dima, char *a, char *res, int dimv, char *v);

extern void ICMCompileND_KD_TAKE_CxA_A (int dima, char *a, char *res, int dimv,
                                        char **vi);

extern void ICMCompileND_KD_DROP_CxA_A (int dima, char *a, char *res, int dimv,
                                        char **vi);

extern void ICMCompileND_KD_CAT_SxAxA_A (int dima, char **ar, char *res, int catdim);

extern void ICMCompileND_KD_ROT_CxSxA_A (int rotdim, char **numstr, int dima, char *a,
                                         char *res);

extern void ICMCompileND_PRF_MODARRAY_AxCxS (char *res_btype, int dimres, char *res,
                                             char *old, char **value, int dimv,
                                             char **vi);

extern void ICMCompileND_PRF_MODARRAY_AxVxS (char *res_btype, int dimres, char *res,
                                             char *old, char **value, int dim, char *v);

extern void ICMCompileND_PRF_MODARRAY_AxCxA (char *res_btype, int dimres, char *res,
                                             char *old, char *val, int dimv, char **vi);

extern void ICMCompileND_PRF_MODARRAY_AxVxA (char *res_btype, int dimres, char *res,
                                             char *old, char *val, int dim, char *v);

#endif /* TAGGED_ARRAYS */

#endif /* _icm2c_prf_h_ */
