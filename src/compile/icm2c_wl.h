/*
 *
 * $Log$
 * Revision 3.7  2002/07/10 19:24:36  dkr
 * no changes done
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

extern void ICMCompileWL_BEGIN__OFFSET (char *target, char *idx_vec, int dims);
extern void ICMCompileWL_BEGIN (char *target, char *idx_vec, int dims);
extern void ICMCompileWL_END__OFFSET (char *target, char *idx_vec, int dims);
extern void ICMCompileWL_END (char *target, char *idx_vec, int dims);

extern void ICMCompileWL_ASSIGN (int dims_expr, char *expr, int dims_target, char *target,
                                 char *idx_vec, int dims, char **idxs_nt);

extern void ICMCompileWL_ASSIGN__INIT (int dims_target, char *target, char *idx_vec,
                                       int dims, char **idxs_nt);

extern void ICMCompileWL_ASSIGN__COPY (char *source, int dims_target, char *target,
                                       char *idx_vec, int dims, char **idxs_nt);

extern void ICMCompileWL_FOLD__OFFSET (int dims_target, char *target, char *idx_vec,
                                       int dims, char **idxs_nt);

extern void ICMCompileWL_FOLD (int dims_target, char *target, char *idx_vec, int dims,
                               char **idxs_nt);

extern void ICMCompileWL_INIT_OFFSET (int dims_target, char *target, char *idx_vec,
                                      int dims);

extern void ICMCompileWL_ADJUST_OFFSET (int dim, int dims_target, char *target,
                                        char *idx_vec, int dims, char **idxs_nt);

extern void ICMCompileWL_SET_OFFSET (int dim, int first_block_dim, int dims_target,
                                     char *target, char *idx_vec, int dims,
                                     char **idxs_nt);

#endif /* _icm2c_wl_h */
