/*
 *
 * $Log$
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
 * some modifications for TAGGED_ARRAYS done
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

#ifndef _icm2c_wl_h
#define _icm2c_wl_h

extern void ICMCompileND_WL_GENARRAY__SHAPE_id (char *to_nt, int to_sdim, char *shp_nt,
                                                char *val_nt, int val_sdim);
extern void ICMCompileND_WL_GENARRAY__SHAPE_arr (char *to_nt, int to_sdim, int shp_size,
                                                 char **shpa_any, char *val_nt,
                                                 int val_sdim);

extern void ICMCompileWL_BEGIN__OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt,
                                        int dims);
extern void ICMCompileWL_BEGIN (char *to_nt, int to_sdim, char *idx_vec_nt, int dims);
extern void ICMCompileWL_END__OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt,
                                      int dims);
extern void ICMCompileWL_END (char *to_nt, int to_sdim, char *idx_vec_nt, int dims);

extern void ICMCompileWL_ASSIGN (char *val_nt, int val_sdim, char *to_nt, int to_dim,
                                 char *idx_vec_nt, int dims, char **idxa_scl);

extern void ICMCompileWL_ASSIGN__INIT (char *to_nt, int to_sdim, char *idx_vec_nt,
                                       int dims, char **idxa_scl);

extern void ICMCompileWL_ASSIGN__COPY (char *from_nt, char *to_nt, int to_sdim,
                                       char *idx_vec_nt, int dims, char **idxa_scl);

extern void ICMCompileWL_FOLD__OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt,
                                       int dims, char **idxa_scl);

extern void ICMCompileWL_FOLD (char *to_nt, int to_sdim, char *idx_vec_nt, int dims,
                               char **idxa_scl);

extern void ICMCompileWL_INIT_OFFSET (char *to_nt, int to_sdim, char *idx_vec_nt,
                                      int dims);

extern void ICMCompileWL_ADJUST_OFFSET (int dim, char *to_nt, int to_sdim,
                                        char *idx_vec_nt, int dims, char **idxa_scl);

extern void ICMCompileWL_SET_OFFSET (int dim, int first_block_dim, char *to_nt,
                                     int to_sdim, char *idx_vec_nt, int dims,
                                     char **idxa_scl);

#endif /* _icm2c_wl_h */
