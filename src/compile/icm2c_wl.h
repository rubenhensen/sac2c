/*
 *
 * $Log$
 * Revision 2.1  1999/02/23 12:42:45  sacbase
 * new release made
 *
 * Revision 1.17  1998/10/29 20:37:59  dkr
 * signature of ICM WL_FOLD_NOOP changed
 *
 * Revision 1.16  1998/08/07 16:05:48  dkr
 * some ICM changed
 *
 * Revision 1.15  1998/08/07 12:37:37  dkr
 * signature of WL_ADJUST_OFFSET changed
 *
 * Revision 1.14  1998/06/29 08:55:51  cg
 * added new multi-threaded versions of new with-loop begin/end ICMs
 * added compilation of ICM WL_MT_SCHEDULER_SET_OFFSET
 *
 * Revision 1.13  1998/06/24 10:37:11  dkr
 * WL_(NON)FOLD_BEGIN/END are now h-icms
 *
 * Revision 1.12  1998/06/19 18:29:51  dkr
 * added WL_NONFOLD_END, WL_FOLD_END
 *
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

extern void ICMCompileWL_NONFOLD_BEGIN (char *target, char *idx_vec, int dims);
extern void ICMCompileWL_FOLD_BEGIN (char *target, char *idx_vec, int dims);
extern void ICMCompileWL_NONFOLD_END (char *target, char *idx_vec, int dims);
extern void ICMCompileWL_FOLD_END (char *target, char *idx_vec, int dims);

extern void ICMCompileWL_ASSIGN (int dims_expr, char *expr, int dims_target, char *target,
                                 char *idx_vec, int dims, char **idx_scalars);

extern void ICMCompileWL_ASSIGN_INIT (int dims_target, char *target, char *idx_vec,
                                      int dims, char **idx_scalars);

extern void ICMCompileWL_ASSIGN_COPY (char *source, int dims_target, char *target,
                                      char *idx_vec, int dims, char **idx_scalars);

extern void ICMCompileWL_FOLD_NOOP (int dims_target, char *target, char *idx_vec,
                                    int dims, char **idx_scalars);

extern void ICMCompileWL_INIT_OFFSET (int dims_target, char *target, char *idx_vec,
                                      int dims_wl);

extern void ICMCompileWL_ADJUST_OFFSET (int dim, int first_block_dim, int dims_target,
                                        char *target, char *idx_vec, int dims,
                                        char **idx_scalars);

#endif /* _icm2c_wl_h */
