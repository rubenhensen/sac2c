#ifndef _SAC_ICM2C_PRF_H_
#define _SAC_ICM2C_PRF_H_

extern void ICMCompileND_PRF_SHAPE_A__DATA (char *to_NT, int to_sdim, char *from_NT,
                                            int from_sdim);

extern void ICMCompileND_PRF_RESHAPE_VxA__SHAPE_id (char *to_NT, int to_sdim,
                                                    char *shp_NT);

extern void ICMCompileND_PRF_RESHAPE_VxA__SHAPE_arr (char *to_NT, int to_sdim,
                                                     int shp_size, char **shpa_ANY);
extern void ICMCompileND_PRF_SEL_VxA__SHAPE_id (char *to_NT, int to_sdim, char *from_NT,
                                                int from_sdim, char *idx_NT);

extern void ICMCompileND_PRF_SEL_VxA__SHAPE_arr (char *to_NT, int to_sdim, char *from_NT,
                                                 int from_sdim, int idx_size,
                                                 char **idxs_ANY);

extern void ICMCompileND_PRF_SEL_VxA__DATA_id_Local (char *to_NT, int to_sdim,
                                                     char *from_NT, int from_sdim,
                                                     char *idx_NT, int idx_size,
                                                     char *copyfun);

extern void ICMCompileND_PRF_SEL_VxA__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                               int from_sdim, char *idx_NT, int idx_size,
                                               char *copyfun);

extern void ICMCompileND_PRF_SEL_VxA__DATA_arr_Local (char *to_NT, int to_sdim,
                                                      char *from_NT, int from_sdim,
                                                      int idx_size, char **idxs_ANY,
                                                      char *copyfun);

extern void ICMCompileND_PRF_SEL_VxA__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                                int from_sdim, int idx_size,
                                                char **idxs_ANY, char *copyfun);
extern void ICMCompileND_PRF_SIMD_SEL_VxA__DATA_id (char *to_NT, int to_sdim,
                                                    char *from_NT, int from_sdim,
                                                    char *idx_NT, int idx_size,
                                                    char *copyfun, int simd_length,
                                                    char *base_type);

extern void ICMCompileND_PRF_SIMD_SEL_VxA__DATA_arr (char *to_NT, int to_sdim,
                                                     char *from_NT, int from_sdim,
                                                     int idx_size, char **idxs_ANY,
                                                     char *copyfun, int simd_length,
                                                     char *base_type);

extern void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_id (char *to_NT, int to_sdim,
                                                      char *from_NT, int from_sdim,
                                                      char *idx_NT, int idx_size,
                                                      char *val_scalar, char *copyfun);

extern void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_arr (char *to_NT, int to_sdim,
                                                       char *from_NT, int from_sdim,
                                                       int idx_size, char **idxs_ANY,
                                                       char *val_scalar, char *copyfun);

extern void ICMCompileND_PRF_MODARRAY_AxVxA__DATA_id (char *to_NT, int to_sdim,
                                                      char *from_NT, int from_sdim,
                                                      char *idx_NT, int idx_size,
                                                      char *val_array, char *copyfun);

extern void ICMCompileND_PRF_MODARRAY_AxVxA__DATA_arr (char *to_NT, int to_sdim,
                                                       char *from_NT, int from_sdim,
                                                       int idx_size, char **idxs_ANY,
                                                       char *val_array, char *copyfun);

extern void ICMCompileND_PRF_IDX_SEL__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, char *idx_ANY);

extern void ICMCompileND_PRF_IDX_SEL__DATA_Local (char *to_NT, int to_sdim, char *from_NT,
                                                  int from_sdim, char *idx_ANY,
                                                  char *copyfun);

extern void ICMCompileND_PRF_IDX_SEL__DATA (char *to_NT, int to_sdim, char *from_NT,
                                            int from_sdim, char *idx_ANY, char *copyfun);

extern void ICMCompileND_PRF_IDX_MODARRAY_AxSxS__DATA (char *to_NT, int to_sdim,
                                                       char *from_NT, int from_sdim,
                                                       char *idx_ANY, char *val_scalar,
                                                       char *copyfun);

extern void ICMCompileND_PRF_IDX_MODARRAY_AxSxA__DATA (char *to_NT, int to_sdim,
                                                       char *from_NT, int from_sdim,
                                                       char *idx_ANY, char *val_array,
                                                       char *copyfun);

extern void ICMCompileND_PRF_TAKE_SxV__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                              int from_sdim, char *cnt_ANY);

extern void ICMCompileND_PRF_TAKE_SxV__DATA (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, char *cnt_ANY, char *copyfun);

extern void ICMCompileND_PRF_DROP_SxV__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                              int from_sdim, char *cnt_ANY);

extern void ICMCompileND_PRF_DROP_SxV__DATA (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, char *cnt_ANY, char *copyfun);

extern void ICMCompileND_PRF_CAT_VxV__SHAPE (char *to_NT, int to_sdim, char *from1_NT,
                                             int from1_sdim, char *from2_NT,
                                             int from2_sdim);

extern void ICMCompileND_PRF_SHAPE_SEL__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                                 int from_sdim, char *idx_NT);

extern void ICMCompileND_PRF_IDX_SHAPE_SEL__DATA (char *to_NT, int to_sdim, char *from_NT,
                                                  int from_sdim, char *idx_ANY);

extern void ICMCompileND_PRF_PROP_OBJ_IN (unsigned int vararg_cnt, char **vararg);

extern void ICMCompileND_PRF_PROP_OBJ_OUT (unsigned int vararg_cnt, char **vararg);

extern void ICMCompileND_PRF_TYPE_CONSTRAINT_AKS (char *to_NT, char *from_NT, int dim,
                                                  int *shp);

extern void ICMCompileND_PRF_SAME_SHAPE (char *to_NT, char *from_NT, int from_sdim,
                                         char *from2_NT, int from2_sdim);

extern void ICMCompileND_PRF_VAL_LT_SHAPE_VxA (char *to_NT, char *from_NT, char *from2_NT,
                                               int from2_sdim);

extern void ICMCompileND_PRF_VAL_LT_VAL_SxS (char *to_NT, char *from_NT, char *from2_NT,
                                             int from2_sdim);

extern void ICMCompileND_PRF_VAL_LE_VAL_SxS (char *to_NT, char *from_NT, char *from2_NT,
                                             int from2_sdim);

extern void ICMCompileND_PRF_VAL_LT_VAL_VxV (char *to_NT, char *from_NT, char *from2_NT,
                                             int from2_sdim);

extern void ICMCompileND_PRF_PROD_MATCHES_PROD_SHAPE (char *to_NT, char *from_NT,
                                                      char *from2_NT, int from2_sdim);

extern void ICMCompileND_PRF_COND (char *to_NT, char *cond_NT, char *then_NT,
                                   char *else_NT);

extern void ICMCompileND_PRF_VAL_LT_VAL_VxV (char *to_NT, char *from_NT, char *from2_NT,
                                             int from2_sdim);

#endif /* _SAC_ICM2C_PRF_H_ */
