/*
 *
 * $Log$
 * Revision 1.1  1998/05/03 14:06:11  dkr
 * Initial revision
 *
 *
 *
 */

#ifndef _icm2c_wl_h
#define _icm2c_wl_h

extern void ICMCompileWL_BEGIN (char *array, char *offset, int dims, char **idx_scalar);

extern void ICMCompileWL_END (char *array, char *offset, int dims, char **idx_scalar);

#endif /* _icm2c_wl_h */
