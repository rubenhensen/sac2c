/*
 *
 * $Log$
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

extern void ICMCompileWL_NONFOLD_BEGIN (char *array, char *idx_vec, int dims,
                                        char **args);

extern void ICMCompileWL_FOLD_BEGIN (char *array, char *idx_vec, int dims, char **args);

extern void ICMCompileWL_FOLDVAR_BEGIN (char *array, char *idx_vec, int dims,
                                        char **args);

extern void ICMCompileWL_END (char *array, char *idx_vec, int dims, char **args);

extern void ICMCompileWL_ASSIGN (char *array, char *idx_vec, int dims, char **idx_scalars,
                                 int dim_expr, char *expr);

extern void ICMCompileWL_ASSIGN_INIT (char *array, char *idx_vec, int dims,
                                      char **idx_scalars, int dim_templ, char *templ);

extern void ICMCompileWL_ASSIGN_COPY (char *source, char *array, char *idx_vec, int dims,
                                      char **idx_scalars, int dim_templ, char *templ);

extern void ICMCompileWL_FOLD (char *array, char *idx_vec, int dims, char **idx_scalars,
                               int dim_expr, char *expr);

#endif /* _icm2c_wl_h */
