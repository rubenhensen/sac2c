#ifndef _SAC_ICM2C_WL_H_
#define _SAC_ICM2C_WL_H_

extern void ICMCompileND_WL_GENARRAY__SHAPE_id_arr (char *to_NT, int to_sdim,
                                                    char *shp_NT, int val_size,
                                                    char **vals_ANY);
extern void ICMCompileND_WL_GENARRAY__SHAPE_id_id (char *to_NT, int to_sdim, char *shp_NT,
                                                   char *val_NT, int val_sdim);
extern void ICMCompileND_WL_GENARRAY__SHAPE_arr_id (char *to_NT, int to_sdim,
                                                    int shp_size, char **shp_ANY,
                                                    char *val_NT, int val_sdim);

extern void ICMCompileWL_SCHEDULE__BEGIN (int dims);
extern void ICMCompileWL_DIST_SCHEDULE__BEGIN (int dims, bool is_distributable,
                                               char *to_NT, char *to_basetype);
void ICMCompileWL3_SCHEDULE__BEGIN (int lb, char *idx_nt, int ub, int chunksz,
                                    bool need_unroll);

extern void ICMCompileWL_DECLARE_SHAPE_FACTOR (char *to_NT, int to_sdim, char *idx_vec_NT,
                                               int dims);

extern void ICMCompileWL_DEFINE_SHAPE_FACTOR (char *to_NT, int to_sdim, char *idx_vec_NT,
                                              int dims);

extern void ICMCompileWL_SCHEDULE__END (int dims);
extern void ICMCompileWL3_SCHEDULE__END (char *idx_nt);

extern void ICMCompileWL_SUBALLOC (char *sub_NT, char *to_NT, char *off_NT);

extern void ICMCompileWL_ASSIGN (char *val_NT, int val_sdim, char *to_NT, int to_sdim,
                                 char *idx_vec_NT, int dims, char *off_NT, char *copyfun);

extern void ICMCompileWL_MODARRAY_SUBSHAPE (char *sub_NT, char *idx_NT, int dims,
                                            char *to_NT);

extern void ICMCompileWL_FOLD (char *to_NT, int to_sdim, char *idx_vec_NT, int dims,
                               char **idxa_scl_NT);

extern void ICMCompileWL_INIT_OFFSET (char *off_NT, char *to_NT, int to_sdim,
                                      char *idx_vec_NT, int dims);

extern void ICMCompileWL_ADJUST_OFFSET (char *off_NT, int dim, char *to_NT, int to_sdim,
                                        char *idx_vec_NT, int dims, char **idxa_scl_NT);

extern void ICMCompileWL_SET_OFFSET (char *off_NT, int dim, int first_block_dim,
                                     char *to_NT, int to_sdim, char *idx_vec_NT, int dims,
                                     char **idxa_scl_NT);

#endif /* _SAC_ICM2C_WL_H_ */
