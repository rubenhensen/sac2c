/*
 *
 * $Log$
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

#ifndef _icm2c_wl_h_
#define _icm2c_wl_h_

extern void ICMCompileND_WL_GENARRAY__SHAPE_id (char *to_NT, int to_sdim, char *shp_NT,
                                                char *val_NT, int val_sdim);
extern void ICMCompileND_WL_GENARRAY__SHAPE_arr (char *to_NT, int to_sdim, int shp_size,
                                                 char **shpa_ANY, char *val_NT,
                                                 int val_sdim);

extern void ICMCompileWL_BEGIN__OFFSET (char *to_NT, int to_sdim, char *idx_vec_NT,
                                        int dims);
extern void ICMCompileWL_BEGIN (char *to_NT, int to_sdim, char *idx_vec_NT, int dims);
extern void ICMCompileWL_END__OFFSET (char *to_NT, int to_sdim, char *idx_vec_NT,
                                      int dims);
extern void ICMCompileWL_END (char *to_NT, int to_sdim, char *idx_vec_NT, int dims);

extern void ICMCompileWL_ASSIGN (char *val_NT, int val_sdim, char *to_NT, int to_dim,
                                 char *idx_vec_NT, int dims, char **idxa_scl_NT);

extern void ICMCompileWL_ASSIGN__INIT (char *to_NT, int to_sdim, char *idx_vec_NT,
                                       int dims, char **idxa_scl_NT);

extern void ICMCompileWL_ASSIGN__COPY (char *from_NT, char *to_NT, int to_sdim,
                                       char *idx_vec_NT, int dims, char **idxa_scl_NT);

extern void ICMCompileWL_FOLD__OFFSET (char *to_NT, int to_sdim, char *idx_vec_NT,
                                       int dims, char **idxa_scl_NT);

extern void ICMCompileWL_FOLD (char *to_NT, int to_sdim, char *idx_vec_NT, int dims,
                               char **idxa_scl_NT);

extern void ICMCompileWL_INIT_OFFSET (char *to_NT, int to_sdim, char *idx_vec_NT,
                                      int dims);

extern void ICMCompileWL_ADJUST_OFFSET (int dim, char *to_NT, int to_sdim,
                                        char *idx_vec_NT, int dims, char **idxa_scl_NT);

extern void ICMCompileWL_SET_OFFSET (int dim, int first_block_dim, char *to_NT,
                                     int to_sdim, char *idx_vec_NT, int dims,
                                     char **idxa_scl_NT);

#endif /* _icm2c_wl_h_ */
