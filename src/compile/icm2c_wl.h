/*
 *
 * $Log$
 * Revision 1.11  1998/06/09 16:46:42  dkr
 * changed signature of WL_NONFOLD_BEGIN, WL_FOLD_BEGIN
 *
 * Revision 1.10  1998/05/24 00:44:30  dkr
 * changed signature of some WL_ICMs
 *
 * Revision 1.9  1998/05/19 15:42:31  dkr
 * ICMs for fold changed
 *
 * Revision 1.8  1998/05/16 16:38:44  dkr
 * WL_END is now a h-icm
 *
 * Revision 1.7  1998/05/15 23:56:03  dkr
 * changed signature and name of some macros
 *
 * Revision 1.6  1998/05/14 21:37:45  dkr
 * changed some ICMs
 *
 * Revision 1.5  1998/05/12 22:46:04  dkr
 * added some macros for fold
 *
 * Revision 1.4  1998/05/12 18:50:11  dkr
 * changed some ICMs
 *
 * Revision 1.3  1998/05/07 16:20:11  dkr
 * changed signature of ICMs
 *
 * Revision 1.2  1998/05/04 15:35:46  dkr
 * added WL_ASSIGN
 *
 * Revision 1.1  1998/05/03 14:06:11  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _icm2c_wl_h
#define _icm2c_wl_h

extern void ICMCompileWL_NONFOLD_BEGIN (char *array, char *idx_vec);

extern void ICMCompileWL_FOLD_BEGIN (char *target, char *idx_vec);

extern void ICMCompileWL_ASSIGN (int dims_expr, char *expr, int dims_target, char *target,
                                 char *idx_vec, int dims, char **idx_scalars);

extern void ICMCompileWL_ASSIGN_INIT (int dims_target, char *target, char *idx_vec,
                                      int dims, char **idx_scalars);

extern void ICMCompileWL_ASSIGN_COPY (char *source, int dims_target, char *target,
                                      char *idx_vec, int dims, char **idx_scalars);

extern void ICMCompileWL_FOLD_NOOP (int dim, int dims_target, char *target, char *idx_vec,
                                    int dims, char **idx_scalars, int cnt_bounds,
                                    char **bounds);

extern void ICMCompileWL_ADJUST_OFFSET (int dim, int dims_target, char *target,
                                        char *idx_vec, int dims, char **idx_scalars);

#endif /* _icm2c_wl_h */
