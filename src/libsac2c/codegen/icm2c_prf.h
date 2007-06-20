/*
 *
 * $Log$
 * Revision 1.5  2005/08/09 18:55:15  ktr
 * F_shape_A_sel and F_idx_shape_sel are implemented by means of C-ICMs now
 *
 * Revision 1.4  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 1.3  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 1.2  2003/09/25 13:44:05  dkr
 * new argument 'copyfun' added to some ICMs
 *
 * Revision 1.1  2003/09/20 14:20:20  dkr
 * Initial revision
 *
 */

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

extern void ICMCompileND_PRF_SEL_VxA__DATA_id (char *to_NT, int to_sdim, char *from_NT,
                                               int from_sdim, char *idx_NT, int idx_size,
                                               char *copyfun);

extern void ICMCompileND_PRF_SEL_VxA__DATA_arr (char *to_NT, int to_sdim, char *from_NT,
                                                int from_sdim, int idx_size,
                                                char **idxs_ANY, char *copyfun);

extern void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_id (char *to_NT, int to_sdim,
                                                      char *from_NT, int from_sdim,
                                                      char *idx_NT, int idx_size,
                                                      char *val_ANY, char *copyfun);

extern void ICMCompileND_PRF_MODARRAY_AxVxS__DATA_arr (char *to_NT, int to_sdim,
                                                       char *from_NT, int from_sdim,
                                                       int idx_size, char **idxs_ANY,
                                                       char *val_ANY, char *copyfun);

extern void ICMCompileND_PRF_IDX_SEL__SHAPE (char *to_NT, int to_sdim, char *from_NT,
                                             int from_sdim, char *idx_ANY);

extern void ICMCompileND_PRF_IDX_SEL__DATA (char *to_NT, int to_sdim, char *from_NT,
                                            int from_sdim, char *idx_ANY, char *copyfun);

extern void ICMCompileND_PRF_IDX_MODARRAY__DATA (char *to_NT, int to_sdim, char *from_NT,
                                                 int from_sdim, char *idx_ANY,
                                                 char *val_ANY, char *copyfun);

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

extern void ICMCompileND_PRF_PROP_OBJ_IN (int vararg_cnt, char **vararg);

extern void ICMCompileND_PRF_PROP_OBJ_OUT (int vararg_cnt, char **vararg);

#endif /* _SAC_ICM2C_PRF_H_ */
