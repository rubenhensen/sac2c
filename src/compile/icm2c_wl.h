/*
 *
 * $Log$
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
                                        char **idx_scalar);

extern void ICMCompileWL_FOLD_BEGIN (char *array, char *idx_vec, int dims,
                                     char **idx_scalar);

extern void ICMCompileWL_END (char *array, char *idx_vec, int dims, char **idx_scalar);

extern void ICMCompileWL_ASSIGN (char *expr, char *array, char *idx_vec, int dims,
                                 char **idx_scalar);

extern void ICMCompileWL_FOLD (char *expr, char *array, char *idx_vec, int dims,
                               char **idx_scalar);

#endif /* _icm2c_wl_h */
