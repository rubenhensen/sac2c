/*
 *
 * $Log$
 * Revision 3.27  2005/09/27 16:20:18  sbs
 * implementation of SIMD_BEGIN and SIMD_END added.
 *
 * Revision 3.26  2005/08/24 10:21:26  ktr
 * added support for explicit with-loop offsets
 *
 * Revision 3.25  2005/06/21 23:39:39  sah
 * added setting of modarray wls subvar descriptors
 * using new C-icm WL_MODARRAY_SUBSHAPE
 *
 * Revision 3.24  2004/11/21 22:04:36  ktr
 * Ismop SacDevCamp 04
 *
 * Revision 3.23  2004/10/28 17:50:22  khf
 * splitted WL_OFFSET into WL_OFFSET and WL_OFFSET_SHAPE_FACR
 * to avoid mixed declarations and code
 *
 * Revision 3.22  2004/08/13 16:38:37  khf
 * splitted WL_BEGIN_OFFSET into WL_SCHEDULE__BEGIN and
 * WL_OFFSET, added WL_SCHEDULE__END, removed WL_BEGIN_OFFSET,
 * WL_END_OFFSET, WL_BEGIN, and WL_END
 *
 * Revision 3.21  2004/08/05 16:12:09  ktr
 * added WL_INC_OFFSET and modified WL_EMM_ASSIGN which now resembles
 * WL_ASSIGN without incrementing the WL_OFFSET.
 *
 * Revision 3.20  2004/08/02 16:17:49  ktr
 * renamed ND_WL_GENARRAY__SHAPE_id into ND_WL_GENARRAY__SHAPE_id_id
 * renamed ND_WL_GENARRAY__SHAPE_arr into ND_WL_GENARRAY__SHAPE_arr_id
 * added ND_WL_GENARRAY__SHAPE_id_arr
 * added ND_WL_SUBALLOC
 * added ND_WL_EMM_ASSIGN
 *
 * Revision 3.19  2004/03/10 00:10:17  dkrHH
 * old backend removed
 *
 * Revision 3.18  2003/09/25 13:43:40  dkr
 * new argument 'copyfun' added to some ICMs
 *
 * Revision 3.17  2003/09/17 14:17:14  dkr
 * some function parameters renamed
 *
 * Revision 3.16  2003/09/17 12:57:29  dkr
 * postfixes _nt, _any renamed into _NT, _ANY
 *
 * Revision 3.15  2003/03/14 13:22:46  dkr
 * all arguments of WL-ICMs are tagged now
 *
 * Revision 3.14  2002/10/29 19:10:34  dkr
 * signature of some ICMs modified
 *
 * Revision 3.13  2002/10/24 20:49:09  dkr
 * signature of ND_WL_GENARRAY__SHAPE_... modified
 *
 * Revision 3.12  2002/10/24 20:37:35  dkr
 * WL icms redesigned for dynamic shapes
 *
 * Revision 3.11  2002/08/05 20:42:19  dkr
 * ND_WL_GENARRAY__SHAPE... added
 *
 * Revision 3.10  2002/08/05 18:46:02  dkr
 * ND_WL_GENARRAY__SHAPE_... added
 *
 * Revision 3.9  2002/07/15 14:43:55  dkr
 * bug in WL_ASSIGN__COPY fixed
 *
 * Revision 3.8  2002/07/12 18:53:39  dkr
 * some modifications done
 *
 * Revision 3.6  2001/02/06 01:44:13  dkr
 * WL_NOOP_... replaced by WL_ADJUST_OFFSET
 *
 * Revision 3.5  2001/01/30 12:22:40  dkr
 * signature of ICMs WL_NOOP, WL_NOOP__OFFSET modified
 *
 * Revision 3.4  2001/01/19 11:54:01  dkr
 * some with-loop ICMs renamed
 *
 * Revision 3.2  2001/01/10 18:33:20  dkr
 * icm WL_ADJUST_OFFSET renamed into WL_SET_OFFSET
 *
 * Revision 3.1  2000/11/20 18:01:21  sacbase
 * new release made
 *
 * [ eliminated ]
 *
 * Revision 1.1  1998/05/03 14:06:11  dkr
 * Initial revision
 *
 */

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

extern void ICMCompileWL_DECLARE_SHAPE_FACTOR (char *to_NT, int to_sdim, char *idx_vec_NT,
                                               int dims);

extern void ICMCompileWL_DEFINE_SHAPE_FACTOR (char *to_NT, int to_sdim, char *idx_vec_NT,
                                              int dims);

extern void ICMCompileWL_SCHEDULE__END (int dims);

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

extern void ICMCompileWL_SIMD_BEGIN ();

extern void ICMCompileWL_SIMD_END ();

#endif /* _SAC_ICM2C_WL_H_ */
